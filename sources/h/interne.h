/*
** Filename: interne.h
**
** Made by: Alexis Delée and Laureen Martina Cahill
**
** Description: prototypes from interne.c
*/

#ifndef INTERNE_H_INCLUDED
#define INTERNE_H_INCLUDED

#include "../../ext/jsmn/include/jsmn.h"
#include "control.h"

typedef struct {
    char *_match;
    char **_matches;
    int _size;
    long *_integer;
    double *_real;
    char **_string;
} Result;

/*
** Description: return the size of a file
**
** Syntax: fsize(source)
** <source> link to file
*/
long fsize(FILE *);

int prohibitedCharacters(const char *);
int check(int, int, ...);
int parseString(Settings *, char *);
int parseNumber(Settings *, char *);

/*
** Description: get a hash of a string from the hash algorithm sha1
**
** Syntax: sha1(strA, strB)
** <strA> string storing the hash
** <strB> string to hash
*/
void sha1(char *, const char *);

/*
** Description: encryption / decryption by xor method
**
** Syntax: XOR(strA, strB, strC, intA)
** <strA> string storing the password
** <strB> path to the current file
** <strC> path to the new file
** <intA> 0 to encrypt or 1 to decrypt
*/
int _xor(const char *, const char *, const char *, short);

/*
** Description: safety management
**
** Syntax: confidential(strA, strB, intA)
** <strA> path to the database folder
** <strB> string storing the password
** <intA> 0 to encrypt or 1 to decrypt (to use with the ENCRYPT and DECRYPT macros)
*/
int confidential(const char *, const char *, short);

/*
** Description: function of reallocation of string
**
** Syntax: extend(intA, strA)
** <intA> size of the string
** <strA> content of the string
*/
char *extend(int, const char *);

int _swap(int, void *, void *);

/*
** Description: parse string in JSON format
*/
int jsoneq(const char *, jsmntok_t *tok, const char *);

/*
** Description: management of the reading of a JSON file
**
** Syntax: readJson(long, source, strA, intB, structA)
** <intA> max size by line to read
** <source> link to the collection file
** <strA> search criterion
** <intB> type of data to process
** <structA> structure of type Result containing the results
*/
int readJson(long, FILE *, const char *, int, Result *);

/*
** Description: multiple quick sort function
**
** Syntax: quickSort(void, intA, intB, intC, arr::strA)
** <void> array of data
** <intA> start index
** <intB> last index
** <intC> type of data to process
** <arr::strA> string containing the results to sort
*/
void quickSort(void *, int, int, int, char **);

#endif // INTERNE_H_INCLUDED
