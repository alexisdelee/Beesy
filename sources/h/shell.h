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

void beesy_error(int);
int beesy_argv(char *, Terminal *);
int beesy_detect(Settings *, Terminal *, char (*)[2][300]);
int beesy_analyze_argv(Terminal *, int);
int beesy_analyze_symbol(int, const char *);
int beesy_run_search(Settings *, Terminal *);
int beesy_run_drop(Settings *, Terminal *);
void beesy_run_exit(Settings *);
int beesy_security_mode(Settings *);
int beesy_run(Settings *, Terminal *, int);
int beesy_analyze(Settings *, Terminal *);

#endif // SHELL_H_INCLUDED
