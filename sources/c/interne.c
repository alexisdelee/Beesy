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

#include "../../ext/SHA1/include/sha1.h"
#include "../h/strlib.h"
#include "../h/interne.h"

#define NEW(type, size) malloc(sizeof(type) * (size))
#define ERR_DEBUG(err) __FILE__, __LINE__ + err

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
    if(strchr(string, '_') != NULL) return -1;
    if(strstr(string, "~$") != NULL) return -1;

    return 1;
}

int check(int limiter, ...)
{
    char **path;
    va_list args;
    int counter = 0;
    int error = 0;

    va_start(args, limiter);
    for( ; counter < limiter; counter++){
        path = va_arg(args, char **);

        if(strlen(*path) > 64
           || prohibitedCharacters(*path) == -1){
            error = 1;
            break;
        }
    }

    va_end(args);

    if(error) return EINVAL;
    else return 0;
}

int sha1(char *hash, const char *password)
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
    SHA1Reset(0);

    return 1;
}

int _xor(const char *password, const char *raw, const char *encrypted, short mode)
{
    FILE *source = NULL;
    FILE *target = NULL;
    int pos = 0, c;

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

                if(pos != strlen(password)) pos++;
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
        if(!mode){
            if(strstr(".", readFile->d_name) == NULL
               && strstr("..", readFile->d_name) == NULL
               && strpbrk("~$", readFile->d_name) != NULL){
                status = strconcat(&tmpPath, 3, directory, "/", readFile->d_name);
                if(status)
                    return status;

                /* newPath = NEW(char *, strlen(tmpPath) - 2);
                if(newPath == NULL) return ENOMEM;
                strcpy(newPath, readFile->d_name + 2); */

                newPath = extend(strlen(tmpPath) - 3, readFile->d_name + 2);
                if(newPath == NULL) return ENOMEM;

                status = strconcat(&newPath, 3, directory, "/", newPath);
                if(status)
                    return strfree(status, 1, &tmpPath);
            } else {
                continue;
            }
        } else {
            if(strstr(".", readFile->d_name) == NULL
               && strstr("..", readFile->d_name) == NULL
               && strpbrk("~$", readFile->d_name) == NULL){
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

    if(mode & 0x01){
        copyInteger = *(long *)first;
        *(long *)first = *(long *)second;
        *(long *)second = copyInteger;
    } else if(mode & 0x02){
        copyDouble = *(double *)first;
        *(double *)first = *(double *)second;
        *(double *)second = copyDouble;
    } else if(mode & 0x04){
        copyString = extend(strlen(*(char **)first), *(char **)first);
        if(copyString == NULL) return -1;

        if(*(char **)first != NULL) free(*(char **)first);
        *(char **)first = extend(strlen(*(char **)second), *(char **)second);
        if(*(char **)first == NULL) return -1;

        if(*(char **)second != NULL) free((char **)second);
        *(char **)second = extend(strlen(copyString), copyString);
        if(*(char **)second == NULL) return -1;

        if(copyString != NULL) free(copyString);
    }

    return 1;
}

int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
    if(tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start && strncmp(json + tok->start, s, tok->end - tok->start) == 0){
        return 0;
    } else {
        return -1;
    }
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
    char *endptr = NULL;
    char *temporaire = NULL;
    jsmn_parser p;
    jsmntok_t t[sizeLine];

    json_string = NEW(char *, sizeLine);
    if(json_string == NULL) return -1;
    while(fgets(json_string, sizeLine, fcollection) != NULL){
        jsmn_init(&p);
        r = jsmn_parse(&p, json_string, strlen(json_string), t, sizeof(t) / sizeof(t[0]));
        if(r < 0){
            printf("Failed to parse JSON: %d\n", r);
            return strfree(-1, 1, &json_string);
        }

        if(r < 1 || t[0].type != JSMN_OBJECT){
            printf("Object expected\n");
            return strfree(-1, 1, &json_string);
        }

        for(i = 0; i < r; i++){
            if(jsoneq(json_string, &t[i], search) == 0){
                if(temporaire != NULL) free(temporaire);
                temporaire = extend(strlen(json_string) - 1, json_string);
                if(temporaire == NULL) return strfree(-1, 1, &json_string);

                length = strlen(temporaire);
                if(!cursor){
                    result->_matches = NEW(char *, 1);
                    if(result->_matches == NULL) return strfree(-1, 1, &json_string);

                    result->_matches[cursor] = extend(length, temporaire);
                    if(result->_matches[cursor] == NULL) return strfree(-1, 1, &json_string);

                    currentMaxSizeMatches = length;
                } else {
                    result->_matches = realloc(result->_matches, sizeof(char *) * (cursor + 1));
                    if(result->_matches == NULL) return strfree(-1, 1, &json_string);

                    if(length > currentMaxSizeMatches){
                        currentMaxSizeMatches = length;
                        for(j = 0; j < cursor - 1; j++){
                            result->_matches[j] = extend(currentMaxSizeMatches, result->_matches[j]);
                            if(result->_matches[j] == NULL) return strfree(-1, 1, &json_string);
                        }
                    }

                    result->_matches[cursor] = extend(currentMaxSizeMatches, temporaire);
                    if(result->_matches[cursor]);
                }

                if(result->_match != NULL) free(result->_match);
                result->_match = NEW(char *, t[i + 1].end - t[i + 1].start);
                if(result->_match == NULL) return strfree(-1, 1, &json_string);
                sprintf(result->_match, "%.*s", t[i + 1].end - t[i + 1].start, json_string + t[i + 1].start);

                if(mode & 0x01){
                    if(!cursor){
                        result->_integer = NEW(long, 1);
                        if(result->_integer == NULL) return strfree(-1, 1, &json_string);
                        result->_integer[cursor] = strtol(result->_match, &endptr, 10);
                    } else {
                        result->_integer = realloc(result->_integer, sizeof(long) * (cursor + 1));
                        if(result->_integer == NULL) return strfree(-1, 1, &json_string);
                        result->_integer[cursor] = strtol(result->_match, &endptr, 10);
                    }

                    if((errno == ERANGE && (result->_integer[cursor] == LONG_MAX || result->_integer[cursor] == LONG_MIN))
                       || (errno != 0 && result->_integer[cursor] == 0)
                       || (endptr == result->_match)){
                        return strfree(-1, 1, &json_string);
                    }
                } else if(mode & 0x02){
                    if(!cursor){
                        result->_real = NEW(double, 1);
                        if(result->_real == NULL) return strfree(-1, 1, &json_string);
                        result->_real[cursor] = strtol(result->_match, &endptr, 10);
                    } else {
                        result->_real = realloc(result->_real, sizeof(double) * (cursor + 1));
                        if(result->_real == NULL) return strfree(-1, 1, &json_string);
                        result->_real[cursor] = strtol(result->_match, &endptr, 10);
                    }

                    if((errno == ERANGE && (result->_real[cursor] == LONG_MAX || result->_integer[cursor] == LONG_MIN))
                       || (errno != 0 && result->_real[cursor] == 0)
                       || (endptr == result->_match)){
                        return strfree(-1, 1, &json_string);
                    }
                } else if(mode & 0x04){
                    if(!cursor){
                        result->_string = NEW(char *, 1);
                        if(result->_string == NULL) return strfree(-1, 1, &json_string);

                        result->_string[cursor] = extend(strlen(result->_match) - 1, result->_match);
                        if(result->_string[cursor] == NULL) return strfree(-1, 1, &json_string);

                        currentMaxSize = strlen(result->_match);
                    } else {
                        result->_string = realloc(result->_string, sizeof(char *) * (cursor + 1));
                        if(result->_string == NULL) return strfree(-1, 1, &json_string);

                        if(strlen(result->_match) > currentMaxSize){
                            currentMaxSize = strlen(result->_match);
                            for(j = 0; j < cursor - 1; j++){
                                result->_string[j] = extend(currentMaxSize + 1, result->_string[j]);
                                if(result->_string[j] == NULL) return strfree(-1, 1, &json_string);
                            }
                        }

                        result->_string[cursor] = extend(currentMaxSize, result->_match);
                        if(result->_string[cursor] == NULL) return strfree(-1, 1, &json_string);
                    }
                }

                cursor++;
                break;
            }
        }

        if(temporaire != NULL) free(temporaire);
        if(json_string != NULL) free(json_string);

        json_string = NEW(char *, sizeLine);
        if(json_string == NULL) return -1;
    }

    result->_size = cursor;

    return strfree(1, 1, &json_string);
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
        if(mode & 0x01){
            do right--; while(((long *)_array)[right] > pivotInteger);
            do left++; while(((long *)_array)[left] < pivotInteger);
        } else if(mode & 0x02){
            do right--; while(((double *)_array)[right] > pivotDouble);
            do left++; while(((double *)_array)[left] < pivotDouble);
        } else if(mode & 0x04){
            pivotString = ((char **)_array)[first][0];

            do right--; while(((char **)_array)[right][0] > pivotString);
            do left++; while(((char **)_array)[left][0] < pivotString);
        }

        if(left < right){
            if(mode & 0x01){
                if(_swap(mode, &((long *)_array)[left], &((long *)_array)[right]) == -1) return;
            } else if(mode & 0x02){
                if(_swap(mode, &((double *)_array)[left], &((double *)_array)[right]) == -1) return;
            } else if(mode & 0x04){
                if(_swap(mode, &((char **)_array)[left], &((char **)_array)[right]) == -1) return;
            }

            if(_swap(0x04, &matches[left], &matches[right]) == -1) return;
        } else {
            break;
        }
    }

    quickSort(_array, first, right, mode, matches);
    quickSort(_array, right + 1, last, mode, matches);
}
