/*
** Filename: control.h
**
** Made by: Alexis Delée and Laureen Martina Cahill
**
** Description: prototypes from control.c
*/

#ifndef CONTROL_H_INCLUDED
#define CONTROL_H_INCLUDED

#define UNINITIATED 0x001
#define WAIT        0x002
#define ADVANCED    0x004
#define SECURITY    0x20000000
#define ROOT        0x40000000

#define INTEGER     0x001
#define REAL        0x002
#define STRING      0x004

#define EQUAL       0x010
#define DIFFERENT   0x020
#define LOWER       0x040
#define UPPER       0x080
#define INCLUDE     0x100
#define SORT        0x200

typedef struct {
    char baseDir[65];
    char currentDatabase[65];
    char passdbHash[41];
    int permission;
    int root;
    long sizeLine;
} Settings;

typedef struct {
    int length;
    char **document;
} Request;

/*
** Description: parse the file beesy.inc and passes the result in the structure of type Settings
**
** Syntax; beesy_settings(structA)
** <structA> configuration options
*/
int beesy_settings(Settings *);

int beesy_connect_root(Settings *, const char *);
int beesy_init_root(Settings *);
int configurationOfMarker(Settings *);
int beesy_boot(Settings *);

/*
** Description: commit
**
** beesy_commit(structA)
** <structA> configuration options
*/
int beesy_commit(Settings *);

/*
** Description: allows you to return to an old version of the database
**
** Syntax: beesy_rollback(structA, strA)
** <structA> configuration options
** <strA> string storing the hash
*/
int beesy_rollback(Settings *, const char *);

/*
** Description: display commits history
**
** Syntax: beesy_log(structA)
** <structA> configuration options
*/
int beesy_log(Settings *);

int initPasswd(Settings *, const char *);
int idPasswd(Settings *, const char *);

/*
** Description: connexion to the database (creation if none exists)
**
** Syntax: beesy_connect_database(structA, strA, strB)
** <structA> configuration options
** <strA> link to the file
** <strB> string storing the password
*/
int beesy_connect_database(Settings *, const char *, const char *);

/*
** Description: take care of the correct closing to the database
**
** Syntax: beesy_close_database(structA)
** <structA> configuration options
*/
int beesy_close_database(Settings *);

/*
** Description: search for documents in a collection
**
** Syntax: beesy_search_document(structA, strA, intA, strB, void, structB)
** <structA> configuration options
** <strA> name of the collection
** <intA> type of data to process
** <strB> search criterion
** <void> data
** <structB> structure of type Request containing the results
*/
int beesy_search_document(Settings *, const char *, int, const char *, void *, Request *);

/*
** Description: deleting documents in a collection
**
** Syntax: beesy_search_document(structA, strA, intA, strB, void)
** <structA> configuration options
** <strA> name of the collection
** <intA> type of data to process
** <strB> search criterion
** <void> data
*/
int beesy_drop_document(Settings *, const char *, int, const char *, void *);

int beesy_drop_collection(Settings *, const char *);

#endif // CONTROL_H_INCLUDED
