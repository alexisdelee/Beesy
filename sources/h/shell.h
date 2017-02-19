/*
** Filename: shell.h
**
** Made by: Alexis Delée and Laureen Martina Cahill
**
** Description: prototypes from shell.c
*/

#ifndef SHELL_H_INCLUDED
#define SHELL_H_INCLUDED

#include "control.h"

typedef struct {
    int argc;
    char **argv;
    char *header;
} Terminal;

/*
** Description: display error message based on error
**
** Syntax: beesy_error(intA)
** <intA> error
*/
void beesy_error(int);

/*
** Description: remove unwanted characters in the input command and split in array all arguments
**
** Syntax: beesy_argv(strA, structA)
** <strA> command
** <structA> structure containing information about the input command
*/
int beesy_argv(char *, Terminal *);

/*
** Description: detecting whether a command is known
**
** Syntax: beesy_detect(structA, structB, arr::strA)
** <structA> structure containing the configurations
** <structB> structure containing information about the input command
** <arr:strA> all recognized commands
*/
int beesy_detect(Settings *, Terminal *, Stack *, char (*)[2][300]);

/*
** Description: check if the parameters don't contain forbidden characters and don't exceed 64 characters
**
** Syntax: beesy_analyze_argv(structA, intA)
** <structA> structure containing information about the input command
** <intA> number of parameters to be analyzed
*/
int beesy_analyze_argv(Terminal *, int);

/*
** Description: execute the insert function
*/
int beesy_run_insert(Settings *, Terminal *, Stack *);

/*
** Description: check if the mathematical operations are accepted on the requested type
**
** Syntax: beesy_analyze_symbol(intA, strA)
** <intA> type
** <structA> symbol
*/
int beesy_analyze_symbol(int, const char *);

/*
** Description: activate the flag sort
*/
int beesy_run_search_options(int, Settings *, const char *, int, const char *, void *, Request *);

/*
** Description: analyze the symbol according to the type
**
** Syntax: beesy_analyze_type(strA, strB)
** <strA> type
** <strB> symbol
*/
int beesy_analyze_type(const char *, const char *);

/*
** Description: execute the search function
*/
int beesy_run_search(int, Settings *, Terminal *, const char *, const char *, const char *, const char *, const char *);

/*
** Description: execute the drop functions
*/
int beesy_run_drop(Settings *, Terminal *);

/*
** Description: execute the quit function
*/
void beesy_run_exit(Settings *);

/*
** Description: launch security mode
**
** Syntax: beesy_security_mode(structA)
** <structA> structure containing the configurations
*/
int beesy_security_mode(Settings *);

/*
** Description: empty the stack
**
** Syntax: popStack(struct)
** <struct> stack
*/
void popStack(Stack *);

/*
** Description: control center for the functions to be executed
**
** Syntax: beesy_run(structA, structB, intA)
** <structA> structure containing the configurations
** <structB> structure containing information about the input command
** <intA> id of the command
*/
int beesy_run(Settings *, Terminal *, Stack *, int);

/*
** Description: control center for parameter analysis
**
** Syntax: beesy_analyze(structA, structB)
** <structA> structure containing the configurations
** <structB> structure containing information about the input command
*/
int beesy_analyze(Settings *, Terminal *, Stack *);

#endif // SHELL_H_INCLUDED
