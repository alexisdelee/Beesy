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
** Syntax: strpos(strA, strB)
** <strA> original string
** <strB> string to transfer
*/
int str(char **, const char *);

/*
** Description: dynamically string concatenation to a character pointer
**
** Syntax: strconcat(strA, intA, ...)
** <strA> original string
** <intA> number of arguments after this parameter
** <...> string to concatenate
*/
int strconcat(char **, int, ...);

/*
** Description: dynamically free character pointers
**
** Syntax: strfree(intA, intB, ...)
** <intA> value to return
** <intB> number of arguments after this parameter
** <...> string to free
*/
int strfree(int, int, ...);

char *strtrim(char *);
int strsplit(char *, const char *, int *, char ***);

#endif // STRLIB_H_INCLUDED
