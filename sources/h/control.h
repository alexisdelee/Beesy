/*
** Filename: control.h
**
** Made by: Alexis Delee and Laureen Martina Cahill
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

int beesy_commit();
int beesy_rollback(const char *);
int beesy_log();
int beesy_confidential(const char *, const char *, short);
int beesy_connect_database(const char *, const char *);
int beesy_close_database(const char *);
int beesy_search_document(const char *, int, const char *, void *, Request *);
int beesy_drop_document(const char *, int, const char *, void *);

#endif // CONTROL_H_INCLUDED
