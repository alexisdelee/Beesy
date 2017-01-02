/*
** Filename: strlib.c
**
** Made by: Alexis Delee and Laureen Martina Cahill
**
** Description: set of functions that simplifies the handling of strings
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#include "../h/strlib.h"

#define NEW(type, size) malloc(sizeof(type) * size)

void swap(int *start, int *end)
{
    int copy = *start;
    *start = *end;
    *end = copy;
}

void strsubstr(char *source, char *dest, int start, int end)
{
    if(start > end) swap(&start, &end);

    if(start != end){
        start = (start < 0 ? 0 : start);
        end = (end < 0 ? 0 : (end > strlen(source) + 1 ? strlen(source) + 1 : end));

        strncpy(dest, source + start, end - start);
        dest[end - start] = '\0';
    } else {
        strcpy(dest, "");
    }
}

void strslice(char *source, char *dest, int start, int size)
{
    strsubstr(source, dest, start, start + size);
}

int strlastpos(char *source, char value, int end)
{
    int letter = (end < 0 ? 0 : (end >= strlen(source) ? strlen(source) : end));
    for( ; letter >= 0; letter--)
        if(source[letter] == value) return letter;

    return -1;
}

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
        // string = malloc(sizeof(char) * (strlen(copy) + strlen(value) + 1));
        string = NEW(char, strlen(copy) + strlen(value) + 1);
        strcpy(string, copy);
        strcat(string, value);
    }

    va_end(args);

    if(*dest != NULL) free(*dest);
    // *dest = malloc(sizeof(char) * (strlen(string) + 1));
    *dest = NEW(char, strlen(string) + 1);
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
