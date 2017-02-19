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

/*
** Description: return the size of a file
**
** Syntax: fsize(source)
** <source> link to file
*/
long fsize(FILE *);

/*
** Description: check if a string doesn't contain a forbidden character
**
** Syntax: prohibitedCharacters(strA)
** <strA>: string to analyze
*/
int prohibitedCharacters(const char *);

/*
** Description: check if a string doesn't exceed n characters and start the function prohibitedCharacters()
**
** Syntax: check(intA, intB, ...)
** <intA> maximum size
** <intB> number of arguments after this parameter
** <...> string to analyze
*/
int check(int, int, ...);

/*
** Description: options recognized by beesy
*/
int parseString(Settings *, char *);

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

/*
** Description: invert two values of the same type
*/
int _swap(int, void *, void *);

/*
** Description: parse string in JSON format
*/
int jsoneq(const char *, jsmntok_t *tok, const char *);

/*
** Description: management of the reading of a JSON file
**
** Syntax: readJson(intA, source, strA, intB, structA)
** <intA> max size by line to read
** <source> link to the collection file
** <strA> search criterion
** <intB> type of data to process
** <structA> structure of type Result containing the results
*/
int readJson(long, FILE *, const char *, int, Result *);

/*
** Description: management of the writing of a JSON file
**
** Syntax: writeJson(ptr::str, strA, strB, intA, intB, intC)
** <ptr::str> where to store the JSON string
** <strA> key to insert
** <strB> value to insert
** <intA> type of the key
** <intB> current position
** <intC> number of elements to insert
*/
int writeJson(char **, const char *, const char *, int, int, int);

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

/*
** Description: delete the original file and rename the new file with the old name
**
** Syntax: replaceCopy(strA, strB)
** <strA> old path
** <strB> new path
*/
void replaceCopy(const char *, const char *);

/*
** Description: create a copy by deleting documents related to the search
**
** Syntax: createCopy(structA, strA, strB, structB)
** <structA> configuration options
** <strA> old path
** <strB> new path
** <structB> structure containing the documents to be deleted
*/
int createCopy(Settings *, const char *, const char *, Request);

/*
** Description: BSD checksum (used to generate a unique ID for each document)
**
** Syntax: bsd(strA, intA)
** <strA> string to hash
** <intA> size of the string
*/
int bsd(const char *seed, int);

#endif // INTERNE_H_INCLUDED
