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
#define STAGE       0x008
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
#define PUSH        0x400
#define PULL        0x800

typedef struct {
    char baseDir[65];
    char currentDatabase[65];
    char passdbHash[41];
    int permission;
    int root;
    long sizeLine;
} Settings;

typedef struct {
    int size;
    /* int length;
    char **elements; */
    char key[50][64];
    char value[50][64];
    int type[50];
} Stack;

typedef struct {
    int length;
    char **document;
} Request;

typedef struct {
    char *_match;
    char **_matches;
    int _size;
    long *_integer;
    double *_real;
    char **_string;
} Result;

/*
** Description: parse the file beesy.inc and passes the result in the structure of type Settings
**
** Syntax; beesy_settings(structA)
** <structA> configuration options
*/
int beesy_settings(Settings *);

/*
** Description: compare the entered and stored root password
**
** Syntax: beesy_connect_root(structA, strA)
** <structA> configuration options
** <strA> password
*/
int beesy_connect_root(Settings *, const char *);

/*
** Description: initialize the root password
**
** Syntax: beesy_init_root(structA)
** <structA> configuration options
*/
int beesy_init_root(Settings *);

/*
** Description: indicate whether the application should install the basic files / folders
**
** Syntax: configurationOfMarker(structA)
** <structA> configuration options
*/
int configurationOfMarker(Settings *);

/*
** Description: creation of files / folders necessary for the application
**
** Syntax: beesy_boot(structA)
** <structA> configuration options
*/
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

/*
** Description: initialize the password of a database
**
** Syntax: initPasswd(structA, strA)
** <structA> configuration options
** <strA> name of the database
*/
int initPasswd(Settings *, const char *);

/*
** Description: compare the entered and stored password
**
** Syntax: idPasswd(structA, strA)
** <structA> configuration options
** <strA> name of the database
*/
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
** Description: performing mathematical operations on queries
**
** Syntax: beesy_comparison(structA, structB, intA, void)
** <structA> structure containing queries
** <structB> structure containing queries after transformation
** <intA> type of data to process
** <void> data
*/
int beesy_comparison(Request *, Result, int, void *);

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

/*
** Description: deleting collection in database
**
** Syntax: beesy_drop_collection(structA, strA)
** <structA> configuration options
** <strA> name of the collection
*/
int beesy_drop_collection(Settings *, const char *);

/*
** Description: deleting database
**
** Syntax: beesy_drop_database(structA, strA, strB)
** <structA> configuration options
** <strA> name of the database
** <strB> password
*/
int beesy_drop_database(Settings *, const char *, const char *);

int beesy_integrity(Stack *, const char *);
int beesy_push(Settings *, Stack *, const char *, const char *, int);
int beesy_pull(Settings *, Stack *, const char *);
int beesy_insert_document(Settings *, Stack *, const char *, int);

#endif // CONTROL_H_INCLUDED
