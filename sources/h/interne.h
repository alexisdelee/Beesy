/*
** Filename: interne.h
**
** Made by: Alexis Delee and Laureen Martina Cahill
**
** Description: prototypes from interne.c
*/

#ifndef INTERNE_H_INCLUDED
#define INTERNE_H_INCLUDED

#include "../../ext/jsmn/include/jsmn.h"

#define MAXSIZE 1000

typedef struct {
    char *_match;
    char **_matches;
    int _size;
    long int *_integer;
    double *_real;
    char **_string;
} Result;

/*
** Description: modify the variable "hostDir" from the folder name stored in the configuration file
**
** Syntax: hostLocation(strA, strB)
** <strA> path to the configuration file
** <strB> string storing the folder to the databases
*/
int hostLocation(const char *, char *);

/*
** Description: return the size of a file
**
** Syntax: fsize(source)
** <source> file
*/
int fsize(FILE *);

/*
** Description: get a hash of a string from the hash algorithm sha1
**
** Syntax: sha1(strA, strB)
** <strA> string storing the hash
** <strB> string to hash
*/
int sha1(char *, const char *);

/*
** Description: get a hash of a file from the function sha1
**
** Syntax: fsha1(strA, strB)
** <strA> string storing the hash
** <strB> path to the file to hash
*/
int fsha1(char *, const char *);

/*
** Description: encryption / decryption by xor method
**
** Syntax: XOR(strA, strB, strC, indexA)
** <strA> string storing the password
** <strB> path to the current file
** <strC> path to the new file
** <indexA> O to encrypt or 1 to decipher
*/
int XOR(const char *, const char *, const char *, int);

char *extend(int, const char *);

int jsoneq(const char *, jsmntok_t *tok, const char *);

int readJson(FILE *, const char *, int, Result *);

void quickSort(void *, int, int, int, char **);

#endif // INTERNE_H_INCLUDED
