/*
** Filename: control.c
**
** Made by: Alexis Delée and Laureen Martina Cahill
**
** Description: set of functions used in the management of the database
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <direct.h>
#include <dirent.h>
#include <limits.h>
#include <errno.h>

#include "../h/strlib.h"
#include "../h/interne.h"
#include "../h/control.h"

#define NEW(type, size) malloc(sizeof(type) * (size))
#define ERR_DEBUG(err) __FILE__, __LINE__ + err
#define ENCRYPT 0
#define DECRYPT 1

int beesy_settings(Settings *settings)
{
    FILE *fconfiguration = NULL;
    char content[200];
    char *configuration = NULL;
    char *endptr = NULL;

    fconfiguration = fopen("beesy.inc", "r");
    if(fconfiguration != NULL){
        while(fgets(content, 200, fconfiguration) != NULL){
            configuration = strtrim(content);
            if(configuration[0] != '#' && strlen(configuration) >= 2){
                if(strstr(configuration, "basedir") != NULL){
                    strncpy(settings->baseDir, strchr(configuration, '=') + 1, 64);
                } else if(strstr(configuration, "initialization_commit") != NULL) {
                    if(!strcmp("false", strchr(configuration, '=') + 1))
                        settings->initializationCommit = 0;
                    else
                        settings->initializationCommit = 1;
                } else if(strstr(configuration, "size_line") != NULL){
                    settings->sizeLine = strtol(strchr(configuration, '=') + 1, &endptr, 10) + 1;

                    if((errno == ERANGE && (settings->sizeLine == LONG_MAX || settings->sizeLine == LONG_MIN))
                       || (errno != 0 && settings->sizeLine == 0)
                       || (endptr == strchr(configuration, '=') + 1)){
                        settings->sizeLine = 1001;
                    }
                }
            }

            strfree(0, 1, &configuration);
        }

        fclose(fconfiguration);
    } else {
        return -1;
    }

    // if(checkPath(settings->sizePath, 1, &settings->baseDir) == -1) return -1;

    settings->permission = WAIT;

    return 1;
}

int beesy_commit(Settings *settings)
{
    FILE *frefs = NULL;
    char *databasePath = NULL;
    char *frefsPath = NULL;
    char seed[12] = "";
    char uniqID[41] = "";
    char *command = NULL;

    sprintf(seed, "%ld", time(NULL));
    sha1(uniqID, seed);

    // create directory
    strconcat(&databasePath, 2, settings->baseDir, "tags");
    _mkdir(databasePath);

    strconcat(&databasePath, 3, settings->baseDir, "tags/", uniqID);
    _mkdir(databasePath);

    // copy database and collections
    strconcat(&command, 5, "robocopy ", settings->baseDir, "current ", databasePath, " *.* /e > nul");
    system(command);

    // add/modification refs file
    strconcat(&frefsPath, 2, settings->baseDir, "refs");
    frefs = fopen(frefsPath, "ab");
    if(frefs != NULL){
        fwrite(uniqID, sizeof(char), 40, frefs);
        fclose(frefs);

        return strfree(1, 3, &databasePath, &frefsPath, &system);
    } else {
        return strfree(-1, 3, &databasePath, &frefsPath, &system);
    }
}

int beesy_rollback(Settings settings, const char *hash)
{
    char *command = NULL;

    if(strlen(hash) != 40) return -1;

    strconcat(&command, 3, "rd \"", settings.baseDir, "current\" /s /q > nul");
    system(command);

    strconcat(&command, 2, settings.baseDir, "current");
    _mkdir(command);

    strconcat(&command, 7, "robocopy ", settings.baseDir, "tags/", hash, " ", settings.baseDir, "current/ *.* /e > nul");
    system(command);

    return strfree(1, 1, &command);
}

int beesy_log(Settings settings)
{
    FILE *frefs = NULL;
    char *frefsPath = NULL;
    char hash[41] = "";

    if(strlen(settings.baseDir) < 2) return -1;

    strconcat(&frefsPath, 2, settings.baseDir, "refs");
    frefs = fopen(frefsPath, "rb");
    if(frefs != NULL){
        while(fread(hash, sizeof(char), 40, frefs), !feof(frefs)){
            printf("%s\n", hash);
        }
        fclose(frefs);
    } else {
        return strfree(-1, 1, &frefsPath);
    }

    return strfree(1, 1, &frefsPath);
}

int beesy_connect_database(Settings *settings, const char *database, const char *password)
{
    char *databasePath = NULL;
    int status;

    if(!(settings->permission & WAIT)) return EACCES;

    status = strconcat(&databasePath, 2, settings->baseDir, "current/");
    if(status)
        return status;

    _mkdir(databasePath);

    status = strconcat(&databasePath, 3, settings->baseDir, "current/", database);
    if(status)
        return status;

    if(!_mkdir(databasePath) && settings->initializationCommit){
        beesy_commit(settings);
    }

    strcpy(settings->currentDatabase, databasePath);
    status = confidential(databasePath, password, DECRYPT);
    if(status)
        return strfree(status, 1, &databasePath);

    settings->permission = ADVANCED;
    return strfree(0, 1, &databasePath);
}

int beesy_close_database(Settings *settings, const char *password)
{
    if(!(settings->permission & (ADVANCED|ROOT))) return EACCES;

    settings->permission = WAIT;
    return confidential(settings->currentDatabase, password, ENCRYPT);
}

int beesy_search_document(Settings settings, const char *collection, int mode, const char *criteria, void *value, Request *request)
{
    FILE *fcollection = NULL;
    char *collectionPath = NULL;
    int index;

    request->length = 0;
    if(!settings.permission) return -1;

    strconcat(&collectionPath, 3, settings.currentDatabase, "/~$", collection);
    fcollection = fopen(collectionPath, "r");
    if(fcollection != NULL){
        Result result;

        if(mode & INTEGER){
            if(readJson(settings.sizeLine, fcollection, criteria, INTEGER, &result) == -1) return strfree(-1, 1, &collectionPath);

            if(mode & SORT){
                quickSort(result._integer, 0, result._size - 1, mode, result._matches);
            }

            request->document = NEW(char *, result._size);
            if(request->document == NULL) return strfree(-1, 1, &collectionPath);

            for(index = 0; index < result._size; index++){
                if((mode & DIFFERENT && result._integer[index] != *(long *)value)
                   || (mode & EQUAL && result._integer[index] == *(long *)value)){
                    request->document[request->length] = extend(strlen(result._matches[index]), result._matches[index]);
                    if(request->document[request->length] == NULL) return strfree(-1, 1, &collectionPath);
                    request->length++;
                }

                if((mode & LOWER && !(mode & DIFFERENT) && result._integer[index] < *(long *)value)
                   ||(mode & UPPER && !(mode & DIFFERENT) && result._integer[index] > *(long *)value)){
                    request->document[request->length] = extend(strlen(result._matches[index]), result._matches[index]);
                    if(request->document[request->length] == NULL) return strfree(-1, 1, &collectionPath);
                    request->length++;
                }
            }
        } else if(mode & REAL){
            if(readJson(settings.sizeLine, fcollection, criteria, REAL, &result) == -1) return strfree(-1, 1, &collectionPath);

            if(mode & SORT){
                quickSort(result._real, 0, result._size - 1, mode, result._matches);
            }

            request->document = NEW(char *, result._size);
            if(request->document == NULL) return strfree(-1, 1, &collectionPath);

            for(index = 0; index < result._size; index++){
                if((mode & DIFFERENT && result._real[index] != *(double *)value)
                   || (mode & EQUAL && result._real[index] == *(double *)value)){
                    request->document[request->length] = extend(strlen(result._matches[index]), result._matches[index]);
                    if(request->document[request->length] == NULL) return strfree(-1, 1, &collectionPath);
                    request->length++;
                }

                if((mode & LOWER && !(mode & DIFFERENT) && result._real[index] < *(double *)value)
                   ||(mode & UPPER && !(mode & DIFFERENT) && result._real[index] > *(double *)value)){
                    request->document[request->length] = extend(strlen(result._matches[index]), result._matches[index]);
                    if(request->document[request->length] == NULL) return strfree(-1, 1, &collectionPath);
                    request->length++;
                }
            }
        } else if(mode & STRING){
            if(readJson(settings.sizeLine, fcollection, criteria, STRING, &result) == -1) return strfree(-1, 1, &collectionPath);

            if(mode & SORT){
                quickSort(result._string, 0, result._size - 1, mode, result._matches);
            }

            request->document = NEW(char *, result._size);
            if(request->document == NULL) return strfree(-1, 1, &collectionPath);

            for(index = 0; index < result._size; index++){
                if((mode & DIFFERENT && strcmp(result._string[index], *(char **)value))
                   || (mode & EQUAL && !strcmp(result._string[index], *(char **)value))
                   || (mode & INCLUDE && strstr(result._string[index], *(char **)value) != NULL)){
                    request->document[request->length] = extend(strlen(result._matches[index]), result._matches[index]);
                    if(request->document[request->length] == NULL) return strfree(-1, 1, &collectionPath);
                    request->length++;
                }
            }
        } else {
            return strfree(-1, 1, &collectionPath);
        }

        if(result._match != NULL) free(result._match);
        if(result._integer != NULL) free(result._integer);
        if(result._real != NULL) free(result._real);
        if(mode & STRING){
            for(index = 0; index < result._size; index++){
                if(result._string[index] != NULL) free(result._string[index]);
                if(result._matches[index] != NULL) free(result._matches[index]);
            }
            if(result._string != NULL) free(result._string);
            if(result._matches != NULL) free(result._matches);
        }

        fclose(fcollection);
    } else {
        return strfree(-1, 1, &collectionPath);
    }

    return strfree(1, 1, &collectionPath);
}

int beesy_drop_document(Settings settings, const char *collection, int mode, const char *criteria, void *value)
{
    FILE *fcollection = NULL;
    FILE *fcopy = NULL;
    char *collectionPath = NULL;
    char *copyPath = NULL;
    char *temporaire = NULL;
    char *content = NULL;
    Request request;
    int index, limit = 0;

    if(!settings.permission) return -1;

    strconcat(&collectionPath, 3, settings.currentDatabase, "/~$", collection);
    fcollection = fopen(collectionPath, "r");
    if(fcollection != NULL){
        if(beesy_search_document(settings, collection, mode, criteria, value, &request) == -1) return -1;

        strconcat(&copyPath, 3, settings.currentDatabase, "/_", collection);
        fcopy = fopen(copyPath, "w");
        if(fcopy != NULL){
            content = NEW(char *, settings.sizeLine);
            if(content == NULL) return strfree(-1, 2, &collectionPath, &copyPath);
            while(fgets(content, settings.sizeLine, fcollection) != NULL){
                temporaire = extend(strlen(content) - 1, content);
                if(temporaire == NULL) return strfree(-1, 2, &collectionPath, &copyPath);

                for(index = 0; index < request.length; index++){
                    if(strstr(temporaire, request.document[index]) != NULL){
                        limit = 1;
                        break;
                    }
                }

                if(limit){
                    limit = 0;
                    if(temporaire != NULL) free(temporaire);
                    continue;
                }

                printf("%s\n", temporaire);
                fprintf(fcopy, "%s\n", temporaire);

                strfree(0, 2, &content, &temporaire);
                content = NEW(char *, settings.sizeLine);
                if(content == NULL) return strfree(-1, 2, &collectionPath, &copyPath);
            }

            fclose(fcopy);
        } else {
            fclose(fcollection);
            return strfree(-1, 2, &collectionPath, &copyPath);
        }

        fclose(fcollection);

        remove(collectionPath);
        rename(copyPath, collectionPath);
    } else {
        return strfree(-1, 3, &collectionPath, &copyPath, &content);
    }

    for(index = 0; index < request.length; index++)
        if(request.document[index] != NULL) free(request.document[index]);
    if(request.document != NULL) free(request.document);

    return strfree(1, 2, &collectionPath, &copyPath);
}

int beesy_drop_collection(Settings settings, const char *collection)
{
    char *collectionPath = NULL;

    if(!settings.permission) return -1;
    if(check(1, &collection) == -1) return -1;

    strconcat(&collectionPath, 3, settings.currentDatabase, "/", collection);
    if(remove(collectionPath)) return strfree(-1, 1, &collectionPath);

    strconcat(&collectionPath, 3, settings.currentDatabase, "/~$", collection);
    if(remove(collectionPath)) return strfree(-1, 1, &collectionPath);

    return strfree(1, 1, &collectionPath);
}

void beesy_error(int error, int log)
{
    if(error == EILSEQ)
        printf("fatal error: ");
    else
        printf("error: ");

    switch(error){
        case EPERM : printf("operation not permitted\n"); break; // 1
        case ENOENT: printf("no such directory\n"); break;
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

int beesy_detect(Settings *settings, Terminal terminal, char (*commands)[2][100], char **header)
{
    int cursor = 0;
    int helped = 0;

    for( ; cursor < 9; cursor++){
        if(!strcmp("help", terminal.argv[0])){
            printf("[%s] %s\n", commands[cursor][0], commands[cursor][1]);
            helped = 1;
        } else if(!strcmp(commands[cursor][0], terminal.argv[0])){
            return beesy_run(settings, terminal, cursor, header);
        }
    }

    if(!helped) return EPERM;
    else return 0;
}

int beesy_run(Settings *settings, Terminal terminal, int id, char **header)
{
    int status;

    switch(id){
        case 1:
            if(terminal.argc < 3) return EAGAIN;
            if(terminal.argc > 3) return E2BIG;

            status = check(2, &terminal.argv[1], &terminal.argv[2]);
            if(status)
                return status;

            status = beesy_connect_database(settings, terminal.argv[1], terminal.argv[2]);
            if(status){
                return status;
            }

            printf("connect to the database \"%s\"...\n", terminal.argv[1]);
            sprintf(*header, "%s>", terminal.argv[1]);

            break;
        case 5:
            if(terminal.argc < 2) return EAGAIN;
            if(terminal.argc > 2) return E2BIG;

            status = check(1, &terminal.argv[1]);
            if(status)
                return status;

            status = beesy_close_database(settings, terminal.argv[1]);
            if(status)
                return status;

            printf("disconnect to the current database...\n");
            sprintf(*header, ">");

            break;
    }

    return 0;
}

int beesy_analyze(Settings *settings, Terminal terminal, char **header)
{
    char commands[9][2][100] = {
        {"help", "help on database methods"},
        {"connect", "connexion to the database"},
        {"search", "search for documents in collection"},
        {"insert", "insert/update documents in collection"},
        {"drop", "delete document/collection/database"},
        {"disconnect", "close current database"},
        {"commit", "back up"},
        {"rollback", "back to an old version"},
        {"log", "display commits history"}
    };
    int status;

    if(terminal.argc < 1) return ENOTTY;
    status = beesy_detect(settings, terminal, commands, header);
    if(status)
        return status;

    return 0;
}
