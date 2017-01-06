/*
** Filename: strlib.h
**
** Made by: Alexis Delée and Laureen Martina Cahill
**
** Description: prototypes from strlib.c
*/

#ifndef STRLIB_H_INCLUDED
#define STRLIB_H_INCLUDED

/*
** Description: dynamically assigning a string to a character pointer
**
** Syntax: strpos(str, string)
** <str> original string
** <string> string to transfer
*/
int str(char **, const char *);

/*
** Description: dynamically string concatenation to a character pointer
**
** Syntax: strconcat(str, indexA, ...)
** <str> original string
** <indexA> number of arguments after this parameter
** <...> string to concatenate
*/
int strconcat(char **, int, ...);

/*
** Description: dynamically free character pointers
**
** Syntax: strfree(indexA, indexB, ...)
** <indexA> value to return
** <indexB> number of arguments after this parameter
** <...> string to free
*/
int strfree(int, int, ...);

char *strtrim(char *);

int strsplit(char *, const char *, int *, char ***);

#endif // STRLIB_H_INCLUDED
