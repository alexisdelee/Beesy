/*
** Filename: interne.c
**
** Made by: Alexis Delée and Laureen Martina Cahill
**
** Description: set of functions used in internal application
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <direct.h>
#include <dirent.h>
#include <math.h>

#include "../../ext/SHA1/include/sha1.h"
#include "../h/strlib.h"
#include "../h/interne.h"

#define NEW(type, size) malloc(sizeof(type) * (size))

long fsize(FILE *file)
{
    long size = 0;

    if(file != NULL){
      fseek(file, 0, SEEK_END);
      size = ftell(file);
      fseek(file, 0, SEEK_SET);
    }

    return size;
}

int prohibitedCharacters(const char *string)
{
    // if(strchr(string, '_') != NULL) return EINVAL;
    // if(strchr(string, '.') != NULL) return EINVAL;
    if(strstr(string, "~$") != NULL) return EINVAL;

    return 0;
}

int check(int size, int limiter, ...)
{
    char **path;
    va_list args;
    int counter = 0;
    int error = 0;

    va_start(args, limiter);

    for( ; counter < limiter; counter++){
        path = va_arg(args, char **);

        if(strlen(*path) > size
           || prohibitedCharacters(*path)){
            error = 1;
            break;
        }
    }

    va_end(args);

    if(error) return EINVAL;
    else return 0;
}

int parseString(Settings *settings, char *string)
{
    char *strim = NULL;
    char *options = NULL;
    int status;
    int index;

    strim = strtrim(string);

    status = check(72, 1, &strim);
    if(status)
        return status;

    if((index = strcspn(strim, "=")) == strlen(strim))
        return EINTR;

    options = strim + index + 1;

    if(strstr(strim, "basedir") != NULL){
        if(strlen(options) > 64) return EINTR;
        strncpy(settings->baseDir, options, 64);
        printf("[OK] %s\n", strim);
    } else if (strstr(strim, "security") != NULL) {
        if(!strcmp("true", options)){
            settings->permission |= SECURITY;
            printf("[OK] %s\n", strim);
        } else if(!strcmp("false", options)){
            printf("[OK] %s\n", strim);
        }
    }

    return 0;
}

void sha1(char *hash, const char *password)
{
    SHA1Context sha;
    uint8_t uint8_digest[20];
    char string_digest[41] = "";
    char string_digest_tmp[41] = "";
    char _string[2];
    int iteration = 0;

    SHA1Reset(&sha);
    SHA1Input(&sha, (const unsigned char *)password, strlen(password));
    SHA1Result(&sha, uint8_digest);

    for( ; iteration < 20; iteration++){
        sprintf(_string, "%02X", uint8_digest[iteration]);
        strcpy(string_digest_tmp, _string);
        strcat(string_digest, string_digest_tmp);
    }

    strcpy(hash, string_digest);
    hash[40] = '\0';
    SHA1Reset(0);
}

int _xor(const char *password, const char *raw, const char *encrypted, short mode)
{
    FILE *source = NULL;
    FILE *target = NULL;
    int pos = 0;
    int c;
    int size = strlen(password);

    source = fopen(raw, "rb");
    if(source != NULL){
        target = fopen(encrypted, "wb");
        if(target != NULL){
            while((c = fgetc(source)) != EOF){
                if(mode == 0){
                    c ^= password[pos];
                    c = ~c;
                } else {
                    c = ~c;
                    c ^= password[pos];
                }

                fprintf(target, "%c", c);

                if(pos != size) pos++;
                else pos = 0;
            }

            fclose(source);
            fclose(target);
        } else {
            fclose(source);
            return EINTR;
        }
    } else {
        return EINTR;
    }

    return 0;
}

int confidential(const char *directory, const char *secret, short mode)
{
    DIR *folder = NULL;
    struct dirent *readFile = NULL;
    char *tmpPath = NULL;
    char *newPath = NULL;
    int status;

    folder = opendir(directory);
    if(folder == NULL) return ENOENT;

    while((readFile = readdir(folder)) != NULL){
        if(strstr(".", readFile->d_name) != NULL
           || strstr("..", readFile->d_name) != NULL
           || !strcmp("passwd", readFile->d_name)){
            continue;
        }

        if(!mode){
            if(strpbrk("~$", readFile->d_name) != NULL){
                status = strconcat(&tmpPath, 3, directory, "/", readFile->d_name);
                if(status)
                    return status;

                newPath = extend(strlen(tmpPath) - 3, readFile->d_name + 2);
                if(newPath == NULL) return ENOMEM;

                status = strconcat(&newPath, 3, directory, "/", newPath);
                if(status)
                    return strfree(status, 1, &tmpPath);
            } else {
                continue;
            }
        } else {
            if(strpbrk("~$", readFile->d_name) == NULL){
                status = strconcat(&tmpPath, 3, directory, "/", readFile->d_name);
                if(status)
                    return status;

                status = strconcat(&newPath, 3, directory, "/~$", readFile->d_name);
                if(status)
                    return strfree(status, 1, &tmpPath);
            } else {
                continue;
            }
        }

        status = _xor(secret, tmpPath, newPath, mode);
        if(status)
            return strfree(status, 2, &tmpPath, &newPath);

        if(!mode) remove(tmpPath);
    }

    if(closedir(folder) == -1) return strfree(ENOENT, 2, &tmpPath, &newPath);
    else return strfree(0, 2, &tmpPath, &newPath);
}

char *extend(int size, const char *content)
{
    char *temporaire = NULL;

    temporaire = NEW(char, size + 1);
    if(temporaire == NULL) return NULL;

    temporaire[size] = '\0';
    strncpy(temporaire, content, size);

    return temporaire;
}

int _swap(int mode, void *first, void *second)
{
    char *copyString = NULL;
    double copyDouble;
    long copyInteger;

    if(mode & INTEGER){
        copyInteger = *(long *)first;
        *(long *)first = *(long *)second;
        *(long *)second = copyInteger;
    } else if(mode & REAL){
        copyDouble = *(double *)first;
        *(double *)first = *(double *)second;
        *(double *)second = copyDouble;
    } else if(mode & STRING){
        copyString = extend(strlen(*(char **)first), *(char **)first);
        if(copyString == NULL) return EINTR;

        if(*(char **)first != NULL) free(*(char **)first);
        *(char **)first = extend(strlen(*(char **)second), *(char **)second);
        if(*(char **)first == NULL) return EINTR;

        if(*(char **)second != NULL) free((char **)second);
        *(char **)second = extend(strlen(copyString), copyString);
        if(*(char **)second == NULL) return EINTR;

        if(copyString != NULL) free(copyString);
    }

    return 0;
}

int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
    if(tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start && strncmp(json + tok->start, s, tok->end - tok->start) == 0){
        return 0;
    } else {
        return -1;
    }
}

int jsonType(Result *result, int mode, int cursor, int *maxSize)
{
    int i;
    char *endptr = NULL;

    if(mode & INTEGER){
        if(!cursor){
            result->_integer = NEW(long, 1);
            if(result->_integer == NULL) return ENOMEM;
            result->_integer[cursor] = strtol(result->_match, &endptr, 10);
        } else {
            result->_integer = realloc(result->_integer, sizeof(long) * (cursor + 1));
            if(result->_integer == NULL) return ENOMEM;
            result->_integer[cursor] = strtol(result->_match, &endptr, 10);
        }

        if((errno == ERANGE && (result->_integer[cursor] == LONG_MAX || result->_integer[cursor] == LONG_MIN))
           || (errno != 0 && result->_integer[cursor] == 0)
           || (endptr == result->_match)){
            return ENOMEM;
        }
    } else if(mode & REAL){
        if(!cursor){
            result->_real = NEW(double, 1);
            if(result->_real == NULL) return ENOMEM;
            result->_real[cursor] = strtod(result->_match, &endptr);
        } else {
            result->_real = realloc(result->_real, sizeof(double) * (cursor + 1));
            if(result->_real == NULL) ENOMEM;
            result->_real[cursor] = strtod(result->_match, &endptr);
        }

        if((errno == ERANGE && (result->_real[cursor] == HUGE_VAL || result->_real[cursor] == -HUGE_VAL))
           || (errno != 0 && result->_real[cursor] == 0)
           || (endptr == result->_match)){
            return ENOMEM;
        }
    } else if(mode & STRING){
        if(!cursor){
            result->_string = NEW(char *, 1);
            if(result->_string == NULL) return ENOMEM;

            result->_string[cursor] = extend(strlen(result->_match), result->_match);
            if(result->_string[cursor] == NULL) return ENOMEM;

            *maxSize = strlen(result->_match);
        } else {
            result->_string = realloc(result->_string, sizeof(char *) * (cursor + 1));
            if(result->_string == NULL) return ENOMEM;

            if(strlen(result->_match) > *maxSize){
                *maxSize = strlen(result->_match);
                for(i = 0; i < cursor - 1; i++){
                    result->_string[i] = extend(*maxSize + 1, result->_string[i]);
                    if(result->_string[i] == NULL) return ENOMEM;
                }
            }

            result->_string[cursor] = extend(*maxSize, result->_match);
            if(result->_string[cursor] == NULL) return ENOMEM;
        }
    }

    return 0;
}

int readJson(long sizeLine, FILE *fcollection, const char *search, int mode, Result *result)
{
    int r;
    int i;
    int j;
    int cursor = 0;
    int currentMaxSize;
    int currentMaxSizeMatches;
    int length;
    char *json_string = NULL;
    char *temporaire = NULL;
    jsmn_parser parser;
    jsmntok_t tokens[100]; // by default, 100 JSON tokens

    json_string = NEW(char *, sizeLine);
    if(json_string == NULL) return -1;
    while(fgets(json_string, sizeLine, fcollection) != NULL){
        jsmn_init(&parser);
        r = jsmn_parse(&parser, json_string, strlen(json_string), tokens, sizeof(tokens) / sizeof(tokens[0]));
        if(r < 0){
            printf("Failed to parse JSON: %d\n", r);
            return strfree(ENOMEM, 1, &json_string);
        }

        if(r < 1 || tokens[0].type != JSMN_OBJECT){
            printf("Object expected\n");
            return strfree(ENOMEM, 1, &json_string);
        }

        for(i = 0; i < r; i++){
            if(!jsoneq(json_string, &tokens[i], search)){
                if(temporaire != NULL) free(temporaire);
                temporaire = extend(strlen(json_string) - 1, json_string);
                if(temporaire == NULL) return strfree(ENOMEM, 1, &json_string);

                length = strlen(temporaire);
                if(!cursor){
                    result->_matches = NEW(char *, 1);
                    if(result->_matches == NULL) return strfree(ENOMEM, 2, &json_string, &temporaire);

                    result->_matches[cursor] = extend(length, temporaire);
                    if(result->_matches[cursor] == NULL) return strfree(ENOMEM, 2, &json_string, &temporaire);

                    currentMaxSizeMatches = length;
                } else {
                    result->_matches = realloc(result->_matches, sizeof(char *) * (cursor + 1));
                    if(result->_matches == NULL) return strfree(ENOMEM, 2, &json_string, &temporaire);

                    if(length > currentMaxSizeMatches){
                        currentMaxSizeMatches = length;
                        for(j = 0; j < cursor - 1; j++){
                            result->_matches[j] = extend(currentMaxSizeMatches, result->_matches[j]);
                            if(result->_matches[j] == NULL) return strfree(ENOMEM, 2, &json_string, &temporaire);
                        }
                    }

                    result->_matches[cursor] = extend(currentMaxSizeMatches, temporaire);
                    if(result->_matches[cursor] == NULL) return strfree(ENOMEM, 2, &json_string, &temporaire);
                }

                if(result->_match != NULL) free(result->_match);
                result->_match = NEW(char *, tokens[i + 1].end - tokens[i + 1].start);
                if(result->_match == NULL) return strfree(ENOMEM, 2, &json_string, &temporaire);
                sprintf(result->_match, "%.*s", tokens[i + 1].end - tokens[i + 1].start, json_string + tokens[i + 1].start);

                jsonType(result, mode, cursor, &currentMaxSize);

                cursor++;
                break;
            }
        }

        strfree(0, 2, &json_string, &temporaire);

        json_string = NEW(char *, sizeLine);
        if(json_string == NULL) return EINTR;
    }

    result->_size = cursor;
    return strfree(0, 1, &json_string);
}

int writeJson(char **json_string, const char *criteria, const char *value, int type, int cursor, int size)
{
    int isString = (type & STRING ? 2 : 0); // equal to 2 for quotation marks

    if(cursor == 0){
        *json_string = NEW(char, 2);
        if(*json_string == NULL) return ENOMEM;
        sprintf(*json_string, "{");
    }

    if(cursor != 0){
        *json_string = realloc(*json_string, sizeof(char) * (strlen(*json_string) + 1 + 1 + strlen(criteria) + 1 + 1 + strlen(value) + isString + 1));
        if(*json_string == NULL) return ENOMEM;
        sprintf(*json_string, "%s,", *json_string);
    } else {
        *json_string = realloc(*json_string, sizeof(char) * (strlen(*json_string) + 1 + strlen(criteria) + 1 + 1 + strlen(value) + isString + 1));
        if(*json_string == NULL) return ENOMEM;
    }

    if(type & STRING){
        sprintf(*json_string, "%s\"%s\":\"%s\"", *json_string, criteria, value);
    } else {
        sprintf(*json_string, "%s\"%s\":%s", *json_string, criteria, value);
    }

    if(cursor == size - 1){
        *json_string = realloc(*json_string, sizeof(char) * (strlen(*json_string) + 1 + 1));
        if(*json_string == NULL) return ENOMEM;
        sprintf(*json_string, "%s}", *json_string);
    }

    return 0;
}

void quickSort(void *_array, int first, int last, int mode, char **matches)
{
    int left = first - 1;
    int right = last + 1;
    long pivotInteger = ((long *)_array)[first];
    double pivotDouble = ((double *)_array)[first];
    char pivotString;

    if(first >= last) return;

    while(1){
        if(mode & INTEGER){
            do right--; while(((long *)_array)[right] > pivotInteger);
            do left++; while(((long *)_array)[left] < pivotInteger);
        } else if(mode & REAL){
            do right--; while(((double *)_array)[right] > pivotDouble);
            do left++; while(((double *)_array)[left] < pivotDouble);
        } else if(mode & STRING){
            pivotString = ((char **)_array)[first][0];

            do right--; while(((char **)_array)[right][0] > pivotString);
            do left++; while(((char **)_array)[left][0] < pivotString);
        }

        if(left < right){
            if(mode & INTEGER){
                if(_swap(mode, &((long *)_array)[left], &((long *)_array)[right])) return;
            } else if(mode & REAL){
                if(_swap(mode, &((double *)_array)[left], &((double *)_array)[right])) return;
            } else if(mode & STRING){
                if(_swap(mode, &((char **)_array)[left], &((char **)_array)[right])) return;
            }

            if(_swap(STRING, &matches[left], &matches[right])) return;
        } else {
            break;
        }
    }

    quickSort(_array, first, right, mode, matches);
    quickSort(_array, right + 1, last, mode, matches);
}

void replaceCopy(const char *oldPath, const char *newPath)
{
    remove(oldPath);
    rename(newPath, oldPath);
}

int createCopy(Settings *settings, const char *originalPath, const char *copyPath, Request request)
{
    FILE *fcollection = NULL;
    FILE *fcopy = NULL;
    char *temporaire = NULL;
    char *content = NULL;
    int index;
    int limit = 0;

    fcollection = fopen(originalPath, "r");
    if(fcollection != NULL){
        fcopy = fopen(copyPath, "w");
        if(fcopy != NULL){
            content = NEW(char *, 6632);
            if(content == NULL) return strfree(ENOMEM, 2, &originalPath, &copyPath);
            while(fgets(content, 6632, fcollection) != NULL){
                temporaire = extend(strlen(content) - 1, content);
                if(temporaire == NULL) return strfree(ENOMEM, 2, &originalPath, &copyPath);

                for(index = 0; index < request.length; index++){
                    if(strstr(temporaire, request.document[index]) != NULL){
                        limit = 1;
                        break;
                    }
                }

                if(limit){
                    limit = 0;
                    if(temporaire != NULL) free(temporaire);
                    continue;
                }

                printf("%s\n", temporaire);
                fprintf(fcopy, "%s\n", temporaire);

                strfree(0, 2, &content, &temporaire);
                content = NEW(char *, 6632);
                if(content == NULL) return strfree(ENOMEM, 2, &originalPath, &copyPath);
            }

            fclose(fcopy);
        } else {
            fclose(fcollection);
            return strfree(EINTR, 2, &originalPath, &copyPath);
        }

        fclose(fcollection);
        replaceCopy(originalPath, copyPath);
    } else {
        return strfree(EINTR, 3, &originalPath, &copyPath, &content);
    }

    return 0;
}

int bsd(const char *seed, int size)
{
    int checksum = 0;
    int index;

    for(index = 0; index < size; index++){
        checksum = (checksum >> 1) + ((checksum & 1) << 15);
        checksum += seed[index];
        checksum &= 0xffff;
    }

    return checksum;
}
