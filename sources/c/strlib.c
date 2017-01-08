/*
** Filename: strlib.c
**
** Made by: Alexis Delée and Laureen Martina Cahill
**
** Description: set of functions that simplifies the handling of strings
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include "../h/strlib.h"
#include "../h/interne.h"

#define NEW(type, size) malloc(sizeof(type) * (size))

int str(char **source, const char *string)
{
    if(*source != NULL) free(*source);

    *source = NEW(char, strlen(string) + 1);
    if(*source == NULL) return ENOMEM;
    strcpy(*source, string);

    return 0;
}

int strconcat(char **dest, int limiter, ...)
{
    char *value = NULL;
    char *copy = NULL;
    char *string = NULL;
    va_list args;
    int counter = 0;
    int status;

    va_start(args, limiter);

    status = str(&string, "");
    if(status)
        return status;
    for( ; counter < limiter; counter++){
        value = va_arg(args, char *);
        status = str(&copy, string);
        if(status)
            return status;

        if(string != NULL) free(string);
        string = NEW(char, strlen(copy) + strlen(value) + 1);
        if(string == NULL) return ENOMEM;
        strcpy(string, copy);
        strcat(string, value);
    }

    va_end(args);

    if(*dest != NULL) free(*dest);
    *dest = NEW(char, strlen(string) + 1);
    if(*dest == NULL) return ENOMEM;
    strcpy(*dest, string);

    if(string != NULL) free(string);
    if(copy != NULL) free(copy);

    return 0;
}

int strfree(int r_value, int limiter, ...)
{
    char **value;
    va_list args;
    int counter = 0;

    va_start(args, limiter);

    for( ; counter < limiter; counter++){
        value = va_arg(args, char **);
        if(value != NULL) free(*value);
    }

    va_end(args);

    return r_value;
}

char *strtrim(char *string)
{
    char *strim;

    while(isspace((unsigned char)*string)) string++;

    if(*string == 0)
    return string;

    strim = string + strlen(string) - 1;
    while(strim > string && isspace((unsigned char)*strim)) strim--;

    *(strim + 1) = 0;

    return string;
}

int strsplit(char *string, const char *delimiter, int *length, char ***array)
{
    char *pstring;
    int index = 0;

    for(pstring = strtok(string, delimiter); pstring; pstring = strtok(NULL, delimiter)){
        *array = realloc(*array, sizeof(char **) * ++index);
        if(*array == NULL) return ENOMEM;

        *(*array + index - 1) = pstring;
    }

    *length = index;

    return 0;
}
