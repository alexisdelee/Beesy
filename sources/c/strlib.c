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

#include "../h/strlib.h"

#define NEW(type, size) malloc(sizeof(type) * (size))


void str(char **source, const char *string)
{
    if(*source != NULL) free(*source);

    // *source = malloc(sizeof(char) * (strlen(string) + 1));
    *source = NEW(char, strlen(string) + 1);
    strcpy(*source, string);
}

void strconcat(char **dest, int limiter, ...)
{
    char *value = NULL, *copy = NULL, *string = NULL;
    va_list args;
    int counter = 0;

    va_start(args, limiter);

    str(&string, "");
    for( ; counter < limiter; counter++){
        value = va_arg(args, char *);
        str(&copy, string);

        if(string != NULL) free(string);
        string = NEW(char, strlen(copy) + strlen(value) + 1);
        if(string == NULL) return;
        strcpy(string, copy);
        strcat(string, value);
    }

    va_end(args);

    if(*dest != NULL) free(*dest);
    *dest = NEW(char, strlen(string) + 1);
    if(*dest == NULL) return;
    strcpy(*dest, string);

    if(string != NULL) free(string);
    if(copy != NULL) free(copy);
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
