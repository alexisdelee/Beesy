/*
** Filename: strlib.h
**
** Made by: Alexis Delee and Laureen Martina Cahill
**
** Description: prototypes from strlib.c
*/

#ifndef STRLIB_H_INCLUDED
#define STRLIB_H_INCLUDED

/*
** Description: get a string from a start index and an end index - 1
**
** Syntax: strsubstr(str, destination, indexA, indexB)
** <str> original string
** <destination> altered string
** <indexA> index between 0 and the length of the string
** <indexB> index between 0 and the length of the string
**
** Rules:
** indexA == indexB ==> destination = ""
** indexA < 0 || indexB < 0 ==> indexA = 0 || indexB = 0
** indexB >= strlen(str) ==> indexB = strlen(str)
*/
void strsubstr(char *, char *, int, int);

/*
** Description: get a string from a start index and a size
**
** Syntax: strslice(str, destination, indexA, size)
** <str> original string
** <destination> altered string
** <indexA> index between 0 and the length of the string
** <size> size
**
** Rules:
** size == 0 ==> destination = ""
** indexA < 0 ==> indexA = 0
** (indexA + size) < 0 ==> (indexA + size) = 0
** (indexA + size) >= strlen(str) ==> (indexA + size) = strlen(str)
*/
void strslice(char *, char *, int, int);

/*
** Description: return an index with the first occurrence of the request value starting from the end of the string
**
** Syntax: strpos(str, value, indexA)
** <str> original string
** <value> desired character
** <indexA> start index between 0 and the length of the string
**
** Rules:
** indexA < 0 ==> indexA = 0
** indexA >= strlen(source) ==> indexA = strlen(source)
** return n if the value is found or -1
*/
int strlastpos(char *, char, int);

/*
** Description: dynamically assigning a string to a character pointer
**
** Syntax: strpos(str, string)
** <str> original string
** <string> string to transfer
*/
void str(char **, const char *);

/*
** Description: dynamically string concatenation to a character pointer
**
** Syntax: strconcat(str, indexA, ...)
** <str> original string
** <indexA> number of arguments after this parameter
** <...> string to concatenate
*/
void strconcat(char **, int, ...);

/*
** Description: dynamically free character pointers
**
** Syntax: strfree(indexA, indexB, ...)
** <indexA> value to return
** <indexB> number of arguments after this parameter
** <...> string to free
*/
int strfree(int, int, ...);

#endif // STRLIB_H_INCLUDED
