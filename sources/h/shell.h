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
int beesy_analyze(Settings *, Terminal *);
int beesy_detect(Settings *, Terminal *, char (*)[2][300]);
int beesy_run(Settings *, Terminal *, int);

#endif // SHELL_H_INCLUDED
