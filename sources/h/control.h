/*
** Filename: control.h
**
** Made by: Alexis Delée and Laureen Martina Cahill
**
** Description: prototypes from control.c
*/

#ifndef CONTROL_H_INCLUDED
#define CONTROL_H_INCLUDED

#define INTEGER 0x001
#define REAL 0x002
#define STRING 0x004
#define ARRAY 0x008

#define EQUAL 0x010
#define DIFFERENT 0x020
#define LOWER 0x040
#define UPPER 0x080
#define INCLUDE 0x100
#define SORT 0x200

typedef struct {
    int length;
    char **document;
} Request;

/*
** Description: commit
*/
int beesy_commit();

/*
** Description: allows you to return to an old version of the database
**
** Syntax: beesy_rollback(strA)
** <strA> string storing the hash
*/
int beesy_rollback(const char *);

/*
** Description: display commits history
*/
int beesy_log();

/*
** Description: connexion to the database (creation if none exists)
**
** Syntax: beesy_connect_database(strA, strB)
** <strA> link to the file
** <strB> string storing the password
*/
int beesy_connect_database(const char *, const char *);

/*
** Description: take care of the correct closing to the database
**
** Syntax: beesy_close_database(strA)
** <strA> string storing the password
*/
int beesy_close_database(const char *);

/*
** Description: search for documents in a collection
**
** Syntax: beesy_search_document(strA, intA, strB, void, structA)
** <strA> name of the collection
** <intA> type of data to process
** <strB> search criterion
** <void> data
** <structA> structure of type Request containing the results
*/
int beesy_search_document(const char *, int, const char *, void *, Request *);

/*
** Description: deleting documents in a collection
**
** Syntax: beesy_search_document(strA, intA, strB, void)
** <strA> name of the collection
** <intA> type of data to process
** <strB> search criterion
** <void> data
*/
int beesy_drop_document(const char *, int, const char *, void *);

#endif // CONTROL_H_INCLUDED
