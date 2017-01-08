/*
** Filename: shell.c
**
** Made by: Alexis Delée and Laureen Martina Cahill
**
** Description: shell creation and administration
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "../h/strlib.h"
#include "../h/shell.h"
#include "../h/interne.h"

void beesy_error(int error)
{
    if(error == EILSEQ)
        printf("fatal error: ");
    else
        printf("error: ");

    switch(error){
        case EPERM : printf("operation not permitted\n"); break; // 1
        case ENOENT: printf("no such collection or database\n"); break; // 2
        case EINTR : printf("interrupted function\n"); break; // 4
        case E2BIG : printf("argument list too long\n"); break; // 7
        case EAGAIN: printf("too few arguments\n"); break; // 11
        case ENOMEM: printf("not enough memory\n"); break; // 12
        case EACCES: printf("permission denied\n"); break; // 13
        case EINVAL: printf("invalid argument\n"); break; // 22
        case ENOTTY: printf("inappropriate input control operation\n"); break; // 25
        case EILSEQ: printf("illegal byte sequence\n"); break; // 42
    }
}

int beesy_argv(char *command, Terminal *terminal)
{
    char *pcommand = NULL;
    char *_command = NULL;
    int status;

    if(str(&pcommand, command)) return ENOMEM;

    _command = strtrim(pcommand);

    status = strsplit(_command, " ", &terminal->argc, &terminal->argv);
    if(status)
        return strfree(status, 2, &pcommand, &_command);

    return 0;
}

int beesy_detect(Settings *settings, Terminal *terminal, char (*commands)[2][300])
{
    int cursor = 0;
    int helped = 0;

    for( ; cursor < 11; cursor++){
        if(!strcmp("help", terminal->argv[0])){
            printf("%s %s\n", commands[cursor][0], commands[cursor][1]);
            helped = 1;
        } else if(!strcmp(commands[cursor][0], terminal->argv[0])){
            return beesy_run(settings, terminal, cursor);
        }
    }

    if(!helped) return EPERM;
    else return 0;
}

int beesy_run(Settings *settings, Terminal *terminal, int id)
{
    int status;

    if(settings->permission & UNINITIATED){
        status = beesy_boot(settings);
        if(status){
            return status;
        } else if(id == 9){
            printf("logged in as domain administrator...\n");
            return 0;
        }
    }

    switch(id){
        case 1:
            if(terminal->argc < 3) return EAGAIN;
            if(terminal->argc > 3) return E2BIG;

            status = check(64, 2, &terminal->argv[1], &terminal->argv[2]);
            if(status)
                return status;

            status = beesy_connect_database(settings, terminal->argv[1], terminal->argv[2]);
            if(status){
                return status;
            }

            printf("connect to the database \"%s\"...\n", terminal->argv[1]);
            sprintf(terminal->header, "%s", terminal->argv[1]);

            break;
        case 4:
            if(terminal->argc < 2) return EAGAIN;

            status = check(64, 1, &terminal->argv[1]);
            if(status)
                return status;

            if(!strcmp("--collection", terminal->argv[1])
               || !strcmp("-c", terminal->argv[1])){

                if(terminal->argc < 3) return EAGAIN;
                if(terminal->argc > 3) return E2BIG;

                status = check(64, 1, &terminal->argv[2]);
                if(status)
                    return status;

                status = beesy_drop_collection(settings, terminal->argv[2]);
                if(status)
                    return status;

                printf("deleting the collection \"%s\"...\n", terminal->argv[2]);
                break;
            } else {
                return EPERM;
            }
        case 5:
            if(terminal->argc > 1) return E2BIG;

            status = beesy_close_database(settings);
            if(status)
                return status;

            printf("disconnect to the current database...\n");
            str(&terminal->header, "");

            break;
        case 6:
            if(terminal->argc > 1) return E2BIG;

            status = beesy_commit(settings);
            if(status)
                return status;

            break;
        case 7:
            if(terminal->argc < 2) return EAGAIN;
            if(terminal->argc > 2) return E2BIG;

            status = beesy_rollback(settings, terminal->argv[1]);
            if(status)
                return status;

            break;
        case 8:
            if(terminal->argc > 1) return E2BIG;

            status = beesy_log(settings);
            if(status)
                return status;

            break;
        case 9:
            if(terminal->argc < 2) return EAGAIN;
            if(terminal->argc > 2) return E2BIG;

            if(settings->permission & ROOT) return EPERM;

            status = check(64, 1, &terminal->argv[1]);
            if(status)
                return status;

            status = beesy_connect_root(settings, terminal->argv[1]);
            if(status)
                return status;

            printf("logged in as domain administrator...\n");

            break;
        case 10:
            if(terminal->argc > 1) return E2BIG;

            if(settings->permission & ROOT){
                settings->permission ^= ROOT; // reset root mode without altering other permissions
                printf("logged in as user...\n");
            } else {
                exit(1);
            }

            break;
    }

    return 0;
}

int beesy_analyze(Settings *settings, Terminal *terminal)
{
    char commands[11][2][300] = {
        {"help", "\t\thelp on database methods"},
        {"connect", "[path] [password]\n\t\tconnexion to the database"},
        {"search", "\t\tsearch for documents in collection"},
        {"insert", "\t\tinsert/update documents in collection"},
        {"drop", "[option] [path] [password] [Options: -db, --database, -c, --collection, -doc, --document]\n\t\tdelete document/collection/database"},
        {"disconnect", "\tclose current database"},
        {"commit", "\t\tcreate a global back up"},
        {"reset", "[id]\n\t\tback to an old version"},
        {"log", "\t\tdisplay commits history"},
        {"sudo", "[password]\n\t\tswitch to root mode"},
        {"exit", "\t\tend of the current process"}
    };
    int status;

    if(terminal->argc < 1) return ENOTTY;
    status = beesy_detect(settings, terminal, commands);
    if(status)
        return status;

    return 0;
}
