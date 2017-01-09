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
#include <limits.h>
#include <math.h>

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

int beesy_analyze_argv(Terminal *terminal, int param)
{
    int status;
    int index;

    if(param > 1 && terminal->argc < param) return EAGAIN;
    if(terminal->argc > param) return E2BIG;

    for(index = 1; index < param; index++){
        status = check(64, 1, &terminal->argv[index]);
        if(status)
            return status;
    }

    return 0;
}

int beesy_analyze_symbol(int type, const char *symbol)
{
    int mode = 0;
    int size = strlen(symbol);

    if(size > 2)
        return 0;

    if(symbol[0] == '!'){
        mode |= DIFFERENT;
    } else if(symbol[0] == '='){
        mode |= EQUAL;
    } else if((type & INTEGER || type & REAL) && (symbol[0] == '<')){
        mode |= LOWER;
    } else if((type & INTEGER || type & REAL) && (symbol[0] == '>')){
        mode |= UPPER;
    } else if((type & STRING) && (symbol[0] == '|')){
        mode |= INCLUDE;
    } else {
        return 0;
    }

    if(size == 2){
        if((type & INTEGER || type & REAL) && (symbol[1] == '=') && (mode & LOWER || mode & UPPER)){
            mode |= EQUAL;
        } else {
            return 0;
        }
    }

    if(mode == 0) return 0;
    return mode | type;
}

int beesy_run_search(Settings *settings, Terminal *terminal)
{
    Request request;
    char *endptr = NULL;
    long valueInteger;
    double valueReal;
    int mode;
    int index;
    int status;

    if(!strcmp("--integer", terminal->argv[1])
       || !strcmp("-i", terminal->argv[1])){
        valueInteger = strtol(terminal->argv[5], &endptr, 10);

        if((errno == ERANGE && (valueInteger == LONG_MAX || valueInteger == LONG_MIN))
           || (errno != 0 && valueInteger == 0)
           || (endptr == terminal->argv[5])){
            return EINTR;
        }

        mode = beesy_analyze_symbol(INTEGER, terminal->argv[4]);
        if(!mode) return EINVAL;

        status = beesy_search_document(settings, terminal->argv[2], mode, terminal->argv[3], &valueInteger, &request);
    } else if(!strcmp("--real", terminal->argv[1])
              || !strcmp("-r", terminal->argv[1])){
        valueReal = strtod(terminal->argv[5], &endptr);

        if((errno == ERANGE && (valueReal == HUGE_VAL || valueReal == -HUGE_VAL))
           || (errno != 0 && valueReal == 0)
           || (endptr == terminal->argv[5])){
            return EINTR;
        }

        mode = beesy_analyze_symbol(REAL, terminal->argv[4]);
        if(!mode) return EINVAL;

        status = beesy_search_document(settings, terminal->argv[2], mode, terminal->argv[3], &valueReal, &request);
    } else if(!strcmp("--string", terminal->argv[1])
              || !strcmp("-s", terminal->argv[1])){
        mode = beesy_analyze_symbol(STRING, terminal->argv[4]);
        if(!mode) return EINVAL;

        status = beesy_search_document(settings, terminal->argv[2], mode, terminal->argv[3], &terminal->argv[5], &request);
    } else {
        return EINVAL;
    }

    if(status)
        return status;

    for(index = 0; index < request.length; index++){
        printf("%s\n", request.document[index]);
        strfree(0, 1, &request.document[index]);
    }
    if(request.document != NULL) free(request.document);

    return 0;
}

int beesy_run_drop(Settings *settings, Terminal *terminal)
{
    char *command = NULL;
    int status;

    if(!strcmp("--collection", terminal->argv[1])
       || !strcmp("-c", terminal->argv[1])){
        status = beesy_analyze_argv(terminal, 3);
        if(status)
            return status;

        status = beesy_drop_collection(settings, terminal->argv[2]);
        if(status)
            return status;

        printf("deleting the collection \"%s\"...\n", terminal->argv[2]);
    } else if(!strcmp("--database", terminal->argv[1])
              || !strcmp("-db", terminal->argv[1])){
        status = beesy_analyze_argv(terminal, 4);
        if(status)
            return status;

        status = beesy_connect_database(settings, terminal->argv[2], terminal->argv[3]);
        if(status)
            return status;

        settings->permission ^= ADVANCED;
        settings->permission |= WAIT;

        status = strconcat(&command, 5, "rd \"", settings->baseDir, "current/", terminal->argv[2], "\" /s /q > nul");
        if(status)
            return status;

        system(command);
        strfree(0, 1, &command);

        printf("deleting the database \"%s\"...\n", terminal->argv[2]);
    } else {
        return EINVAL;
    }

    return 0;
}

void beesy_run_exit(Settings *settings)
{
    if(settings->permission & ROOT){
        settings->permission ^= ROOT; // reset root mode without altering other permissions
        printf("logged in as user...\n");
    } else {
        exit(1);
    }
}

int beesy_security_mode(Settings *settings)
{
    int status;

    if(settings->permission & SECURITY){
        settings->permission |= ROOT;
        status = beesy_commit(settings);
        settings->permission ^= ROOT;
        if(status)
            return status;
    }

    return 0;
}

int beesy_run(Settings *settings, Terminal *terminal, int id)
{
    int status;

    switch(id){
        case 1:
            status = beesy_analyze_argv(terminal, 3);
            if(status)
                return status;

            status = beesy_connect_database(settings, terminal->argv[1], terminal->argv[2]);
            if(status)
                return status;

            printf("connect to the database \"%s\"...\n", terminal->argv[1]);
            sprintf(terminal->header, "%s", terminal->argv[1]);

            break;
        case 2:
            status = beesy_analyze_argv(terminal, 6);
            if(status)
                return status;

            status = beesy_run_search(settings, terminal);
            if(status)
                return status;

            status = beesy_security_mode(settings);
            if(status)
                return status;

            break;
        case 4:
            if(terminal->argc < 2) return EAGAIN;

            status = beesy_run_drop(settings, terminal);
            if(status)
                return status;

            status = beesy_security_mode(settings);
            if(status)
                return status;

            break;
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
            status = beesy_analyze_argv(terminal, 2);
            if(status)
                return status;

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
            status = beesy_analyze_argv(terminal, 2);
            if(status)
                return status;

            status = beesy_connect_root(settings, terminal->argv[1]);
            if(status)
                return status;

            printf("logged in as domain administrator...\n");
            break;
        case 10:
            if(terminal->argc > 1) return E2BIG;

            beesy_run_exit(settings);
            break;
    }

    return 0;
}

int beesy_analyze(Settings *settings, Terminal *terminal)
{
    char commands[11][2][300] = {
        {"help", "\t\thelp on database methods"},
        {"connect", "[path] [password]\n\t\tconnexion to the database"},
        {"search", "[option] [path] [keyword] [symbol] [value] [Options: -i, --integer, -r, --real, -s, --string]\n\t\tsearch for documents in collection"},
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
