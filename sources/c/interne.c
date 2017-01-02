/*
** Filename: interne.c
**
** Made by: Alexis Delee and Laureen Martina Cahill
**
** Description: set of functions used in internal application
*/

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <limits.h>
#include <errno.h>

#include "../../ext/SHA1/include/sha1.h"
#include "../h/strlib.h"
#include "../h/interne.h"

#define NEW(type, size) malloc(sizeof(type) * (size))
#define ERR_DEBUG(err) __FILE__, __LINE__ + err

int hostLocation(const char *localisation, char *hostDir)
{
    FILE *HOSTfile = NULL;
    char content[257] = "";

    if(strlen(hostDir) == 0){
        HOSTfile = fopen(localisation, "r");
        if(HOSTfile != NULL){
            if(fgets(content, sizeof content / sizeof *content, HOSTfile) != NULL){
                if(content[strlen(content) - 1] == '\n'){
                    content[strlen(content) - 1] = '\0';
                }

                strcpy(hostDir, content);
                if(content[strlen(content) - 1] != '/') strcat(hostDir, "/");

                fclose(HOSTfile);
                return 1;
            }

            fclose(HOSTfile);
        }
    } else {
        return 1;
    }

    printf("[TypeError] problem with the host file\n%s:%d\n\n", ERR_DEBUG(1));
    return -1;
}

int fsize(FILE *file)
{
    int size = 0;

    if(file != NULL){
      fseek(file, 0, SEEK_END);
      size = ftell(file);
      fseek(file, 0, SEEK_SET);
    }

    return size;
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

int fsha1(char *hash, const char *path)
{
    FILE *file = NULL;
    char *content = NULL, *salt = NULL;
    long size;

    file = fopen(path, "r");
    if(file != NULL){
        size = fsize(file);

        content = NEW(char, ++size);
        if(content == NULL) return -1;
        salt = NEW(char, size + strlen(path));
        if(salt == NULL) return -1;

        fgets(content, size, file);
        strcpy(salt, path); // using a salt to keep the uniqueness of the file
        strcat(salt, content);
        sha1(hash, salt);

        fclose(file);
    } else {
        return -1;
    }

    return strfree(1, 2, &content, &salt);
}

int XOR(const char *password, const char *raw, const char *encrypted, int decrypt)
{
    FILE *source = NULL;
    FILE *target = NULL;
    int pos = 0, c;

    source = fopen(raw, "rb");
    if(source != NULL){
        target = fopen(encrypted, "wb");
        if(target != NULL){
            while((c = fgetc(source)) != EOF){
                if(decrypt == 0){
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
            return -1;
        }
    } else {
        return -1;
    }

    return 1;
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

int jsoneq(const char *json, jsmntok_t *tok, const char *s)
{
    if(tok->type == JSMN_STRING && (int)strlen(s) == tok->end - tok->start && strncmp(json + tok->start, s, tok->end - tok->start) == 0){
        return 0;
    } else {
        return -1;
    }
}

int readJson(FILE *fcollection, const char *search, int mode, Result *result)
{
    int r, i, j, cursor = 0, currentMaxSize, currentMaxSizeMatches, length;
    char *json_string = NULL, *endptr = NULL, *temporaire = NULL;
    jsmn_parser p;
    jsmntok_t t[MAXSIZE];

    json_string = NEW(char *, MAXSIZE);
    if(json_string == NULL) return -1;
    while(fgets(json_string, MAXSIZE, fcollection) != NULL){
        if(strlen(json_string) == 1) continue;

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
                    result->_matches[cursor] = NEW(char, length);
                    if(result->_matches[cursor] == NULL) return strfree(-1, 1, &json_string);
                    strcpy(result->_matches[cursor], temporaire);

                    currentMaxSizeMatches = length;
                } else {
                    result->_matches = realloc(result->_matches, sizeof(char *) * (cursor + 1));
                    if(result->_matches == NULL) return strfree(-1, 1, &json_string);

                    if(length > currentMaxSizeMatches){
                        currentMaxSizeMatches = length;
                        for(j = 0; j < cursor - 1; j++){
                            result->_matches[j] = realloc(result->_matches[j], sizeof(char) * (currentMaxSizeMatches + 1));
                        }
                    }
                    result->_matches[cursor] = NEW(char, currentMaxSizeMatches + 1);
                    if(result->_matches[cursor] == NULL) return strfree(-1, 1, &json_string);
                    strcpy(result->_matches[cursor], temporaire);
                }

                if(result->_match != NULL) free(result->_match);
                result->_match = NEW(char *, t[i + 1].end - t[i + 1].start);
                if(result->_match == NULL) return strfree(-1, 1, &json_string);
                sprintf(result->_match, "%.*s", t[i + 1].end - t[i + 1].start, json_string + t[i + 1].start);

                if(mode & 0x001){
                    if(!cursor){
                        result->_integer = NEW(long int, 1);
                        if(result->_integer == NULL) return strfree(-1, 1, &json_string);
                        result->_integer[cursor] = strtol(result->_match, &endptr, 10);
                    } else {
                        result->_integer = realloc(result->_integer, sizeof(long int) * (cursor + 1));
                        if(result->_integer == NULL) return strfree(-1, 1, &json_string);
                        result->_integer[cursor] = strtol(result->_match, &endptr, 10);
                    }

                    if((errno == ERANGE && (result->_integer[cursor] == LONG_MAX || result->_integer[cursor] == LONG_MIN))
                       || (errno != 0 && result->_integer[cursor] == 0)
                       || (endptr == result->_match)){
                        return strfree(-1, 1, &json_string);
                    }
                } else if(mode & 0x002){
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
                } else if(mode & 0x004){
                    if(!cursor){
                        result->_string = NEW(char *, 1);
                        if(result->_string == NULL) return strfree(-1, 1, &json_string);
                        result->_string[cursor] = NEW(char, strlen(result->_match));
                        if(result->_string[cursor] == NULL) return strfree(-1, 1, &json_string);
                        strcpy(result->_string[cursor], result->_match);

                        currentMaxSize = strlen(result->_match);
                    } else {
                        result->_string = realloc(result->_string, sizeof(char *) * (cursor + 1));
                        if(result->_string == NULL) return strfree(-1, 1, &json_string);

                        if(strlen(result->_match) > currentMaxSize){
                            currentMaxSize = strlen(result->_match);
                            for(j = 0; j < cursor - 1; j++){
                                result->_string[j] = realloc(result->_string[j], sizeof(char) * (currentMaxSize + 1));
                            }
                        }
                        result->_string[cursor] = NEW(char, currentMaxSize + 1);
                        if(result->_string[cursor] == NULL) return strfree(-1, 1, &json_string);
                        strcpy(result->_string[cursor], result->_match);
                    }
                }

                cursor++;
                break;
            }
        }

        if(temporaire != NULL) free(temporaire);
        if(json_string != NULL) free(json_string);
        json_string = NEW(char *, MAXSIZE);
        if(json_string == NULL) return -1;
    }

    result->_size = cursor;

    return strfree(1, 1, &json_string);
}

void quickSort(void *_array, int first, int last, int mode, char **matches)
{
    int left = first - 1, right = last + 1;
    long int pivotInteger = ((long int *)_array)[first], tmpInteger;
    double pivotDouble = ((double *)_array)[first], tmpDouble;
    char pivotString, *tmpString = NULL;
    char *copy = NULL;

    if(first >= last) return;

    while(1){
        if(mode & 0x001){
            do right--; while(((long int *)_array)[right] > pivotInteger);
            do left++; while(((long int *)_array)[left] < pivotInteger);
        } else if(mode & 0x002){
            do right--; while(((double *)_array)[right] > pivotDouble);
            do left++; while(((double *)_array)[left] < pivotDouble);
        } else if(mode & 0x004){
            pivotString = ((char **)_array)[first][0];

            do right--; while(((char **)_array)[right][0] > pivotString);
            do left++; while(((char **)_array)[left][0] < pivotString);
        }

        if(left < right){
            if(mode & 0x001){
                tmpInteger = ((long int *)_array)[left];
                ((long int *)_array)[left] = ((long int *)_array)[right];
                ((long int *)_array)[right] = tmpInteger;
            } else if(mode & 0x002){
                tmpDouble = ((double *)_array)[left];
                ((double *)_array)[left] = ((double *)_array)[right];
                ((double *)_array)[right] = tmpDouble;
            } else if(mode & 0x004){
                tmpString = realloc(((char **)_array)[left], strlen(((char **)_array)[left]) + 1);
                if(tmpString == NULL) return;
                ((char **)_array)[left] = realloc(((char **)_array)[right], strlen(((char **)_array)[right]) + 1);
                if(((char **)_array)[left] == NULL) return;
                ((char **)_array)[right] = realloc(tmpString, strlen(tmpString) + 1);
                if(((char **)_array)[right] == NULL) return;
            }

            copy = realloc(matches[left], strlen(matches[left]) + 1);
            if(copy == NULL) return;
            matches[left] = realloc(matches[right], strlen(matches[right]) + 1);
            if(matches[left] == NULL) return;
            matches[right] = realloc(copy, strlen(copy) + 1);
            if(matches[right] == NULL) return;
        } else {
            break;
        }
    }

    quickSort(_array, first, right, mode, matches);
    quickSort(_array, right + 1, last, mode, matches);
}
