/*
** Filename: control.c
**
** Made by: Alexis Del�e and Laureen Martina Cahill
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
#include <math.h>

#include "../h/strlib.h"
#include "../h/interne.h"
#include "../h/control.h"

#define NEW(type, size) malloc(sizeof(type) * (size))
#define ENCRYPT 0
#define DECRYPT 1

int beesy_settings(Settings *settings)
{
    FILE *fconfiguration = NULL;
    char content[200];
    int status;

    settings->permission = 0;

    fconfiguration = fopen("beesy.inc", "r");
    if(fconfiguration != NULL){
        while(fgets(content, 200, fconfiguration) != NULL){
            if(content[0] == '#'){
                continue;
            } else if(strstr(content, "basedir") != NULL
               || strstr(content, "security") != NULL){ // only the "basedir" and "security" options can be modified
                status = parseString(settings, content);
            } else {
                continue;
            }

            if(status)
                return status;
        }

        fclose(fconfiguration);
    } else {
        return ENOENT;
    }

    printf("\n");

    settings->permission |= UNINITIATED;
    return 0;
}

int beesy_connect_root(Settings *settings, const char *password)
{
    FILE *faccess = NULL;
    char secret[41] = "";
    char *accessPath = NULL;
    char hash[41] = "";
    int status;

    status = strconcat(&accessPath, 2, settings->baseDir, "root");
    if(status)
        return status;

    faccess = fopen(accessPath, "r");
    if(faccess != NULL){
        fgets(secret, 41, faccess);
        sha1(hash, password);

        fclose(faccess);
    } else {
        strfree(EINTR, 1, &accessPath);
        exit(-1);
    }

    if(strcmp(hash, secret))
        return strfree(EACCES, 1, &accessPath);

    settings->permission |= ROOT;
    return strfree(0, 1, &accessPath);
}

int beesy_init_root(Settings *settings)
{
    FILE *faccess = NULL;
    char password[65] = "";
    char *strim = NULL;
    char *accessPath = NULL;
    char hash[41] = "";
    int status;

    printf("first connection, set root password: ");
    fgets(password, 65, stdin); // enter the root password for the first time

    strim = strtrim(password);

    status = strconcat(&accessPath, 2, settings->baseDir, "root");
    if(status)
        return status;

    faccess = fopen(accessPath, "w");
    if(faccess != NULL){
        sha1(hash, strim);
        fprintf(faccess, "%s", hash);
        fclose(faccess);
    } else {
        strfree(EINTR, 1, &accessPath);
        exit(-1);
    }

    settings->permission |= ROOT; // we attribute it the "root"
    return strfree(0, 1, &accessPath);
}

int configurationOfMarker(Settings *settings)
{
    FILE *fconfiguration = NULL;

    fconfiguration = fopen("marker", "r");
    if(fconfiguration == NULL){
        fconfiguration = fopen("marker", "w");
        if(fconfiguration == NULL){
            return EINTR;
        }
    } else {
        settings->permission |= WAIT; // if the file exists, it gets good permission
    }

    fclose(fconfiguration);
    return 0;
}

int beesy_boot(Settings *settings)
{
    FILE *frefs = NULL;
    char *frefsPath = NULL;
    char *databasePath = NULL;
    int status;

    status = configurationOfMarker(settings);
    if(status)
        return status;

    if(settings->permission & WAIT)
        return 0;

    status = beesy_init_root(settings);
    if(status)
        return status;

    // create "current" directory
    status = strconcat(&databasePath, 2, settings->baseDir, "current");
    if(status)
        return status;
    _mkdir(databasePath);

    // create "tags" directory
    status = strconcat(&databasePath, 2, settings->baseDir, "tags");
    if(status)
        return status;
    _mkdir(databasePath);

    // modification "refs" file
    status = strconcat(&frefsPath, 2, settings->baseDir, "refs");
    if(status)
        return status;

    frefs = fopen(frefsPath, "wb");
    if(frefs != NULL)
        fclose(frefs);
    else
        return strfree(EINTR, 2, &databasePath, &frefsPath);

    settings->permission ^= UNINITIATED; // delete this flag
    settings->permission |= WAIT;

    printf("logged in as domain administrator...\n");

    return strfree(0, 2, &databasePath, &frefsPath);
}

int beesy_commit(Settings *settings)
{
    FILE *frefs = NULL;
    char *databasePath = NULL;
    char *frefsPath = NULL;
    char seed[12] = "";
    char uniqID[41] = "";
    char *command = NULL;
    int status;

    if(!(settings->permission & ROOT)) return EACCES;

    sprintf(seed, "%ld", time(NULL));
    sha1(uniqID, seed); // backups are generated from the time hash

    status = strconcat(&databasePath, 3, settings->baseDir, "tags/", uniqID);
    if(status)
        return status;
    _mkdir(databasePath);

    // copy databases and collections
    status = strconcat(&command, 5, "robocopy ", settings->baseDir, "current ", databasePath, " *.* /e > nul");
    if(status)
        return status;
    system(command);

    // modification refs file
    status = strconcat(&frefsPath, 2, settings->baseDir, "refs");
    if(status)
        return status;
    frefs = fopen(frefsPath, "ab");
    if(frefs != NULL){
        fwrite(uniqID, sizeof(char), 40, frefs);
        fclose(frefs);

        printf("commit %s\n", uniqID);

        return strfree(0, 3, &databasePath, &frefsPath, &system);
    } else {
        return strfree(EINTR, 3, &databasePath, &frefsPath, &system);
    }
}

int beesy_rollback(Settings *settings, const char *hash)
{
    char *command = NULL;
    int status;

    if(!(settings->permission & ROOT) || !(settings->permission & WAIT)) return EACCES;
    if(strlen(hash) != 40) return EINVAL;

    status = strconcat(&command, 3, "rd \"", settings->baseDir, "current\" /s /q > nul");
    if(status)
        return status;
    system(command);

    status = strconcat(&command, 2, settings->baseDir, "current");
    if(status)
        return status;
    _mkdir(command);

    status = strconcat(&command, 7, "robocopy ", settings->baseDir, "tags/", hash, " ", settings->baseDir, "current/ *.* /e > nul");
    if(status)
        return status;
    system(command);

    printf("databases are now at %s\n", hash);

    return strfree(0, 1, &command);
}

int beesy_log(Settings *settings)
{
    FILE *frefs = NULL;
    char *frefsPath = NULL;
    char hash[41] = "";
    int status;

    if(strlen(settings->baseDir) < 2) return -1;

    status = strconcat(&frefsPath, 2, settings->baseDir, "refs");
    if(status)
        return status;

    frefs = fopen(frefsPath, "rb");
    if(frefs != NULL){
        while(fread(hash, sizeof(char), 40, frefs), !feof(frefs)){
            printf("commit %s\n", hash);
        }
        fclose(frefs);
    } else {
        return strfree(EINTR, 1, &frefsPath);
    }

    return strfree(0, 1, &frefsPath);
}

int initPasswd(Settings *settings, const char *database)
{
    FILE *froot = NULL;
    FILE *passwd = NULL;
    char *path = NULL;
    char content[41] = "";
    char fileHash[41] = "";
    char rootHash[41] = "";
    char hash[41] = "";
    int status;
    int index;

    status = strconcat(&path, 2, settings->baseDir, "root");
    if(status)
        return status;

    froot = fopen(path, "r");
    if(froot != NULL){
        fgets(content, 41, froot);
        sprintf(content, "%.40s", content);
        fclose(froot);
    } else {
        return strfree(EINTR, 1, &path);
    }

    sha1(fileHash, database);
    sha1(rootHash, content);

    for(index = 0; index < 40; index++){
        hash[index] = (fileHash[index] ^ rootHash[index]) ^ settings->passdbHash[index]; // (name of the database) xor (hash of content in root) xor (password)
    }

    status = strconcat(&path, 4, settings->baseDir, "current/", database, "/passwd");
    if(status)
        return status;

    passwd = fopen(path, "w");
    if(passwd != NULL){
        fprintf(passwd, "%s", hash);
        fclose(passwd);
    } else {
        return strfree(EINTR, 1, &path);
    }

    return strfree(0, 1, &path);
}

int idPasswd(Settings *settings, const char *database)
{
    FILE *froot = NULL;
    FILE *passwd = NULL;
    char *path = NULL;
    char content[41] = "";
    char fileHash[41] = "";
    char rootHash[41] = "";
    char hash[41] = "";
    char dbHash[41] = "";
    int status;
    int index;

    status = strconcat(&path, 2, settings->baseDir, "root");
    if(status)
        return status;

    froot = fopen(path, "r");
    if(froot != NULL){
        fgets(content, 41, froot);
        sprintf(content, "%.40s", content);
        fclose(froot);
    } else {
        return strfree(EINTR, 1, &path);
    }

    status = strconcat(&path, 4, settings->baseDir, "current/", database, "/passwd");
    if(status)
        return status;

    passwd = fopen(path, "r");
    if(passwd != NULL){
        fgets(fileHash, 41, passwd);
        fclose(passwd);
    } else {
        return strfree(EINTR, 1, &path);
    }

    sha1(rootHash, content);
    sha1(dbHash, database);

    for(index = 0; index < 40; index++)
        hash[index] = (fileHash[index] ^ settings->passdbHash[index]) ^ rootHash[index]; // operation identical to the creation

    if(!strcmp(hash, dbHash)) // if the hash obtained is identical to the hash of the name of the database, then the password is valid
        return strfree(0, 1, &path);
    else
        return strfree(EACCES, 1, &path);
}

int beesy_connect_database(Settings *settings, const char *database, const char *password)
{
    char *databasePath = NULL;
    int status;

    if(!(settings->permission & WAIT)) return EACCES;

    settings->passdbHash[0] = '\0';
    sha1(settings->passdbHash, password);

    status = strconcat(&databasePath, 3, settings->baseDir, "current/", database);
    if(status)
        return status;

    if(!_mkdir(databasePath)){
        status = initPasswd(settings, database); // initialization of the password when creating the database
        if(status)
            return status;

        printf("creation of the database \"%s\"...\n", database);

        if(settings->permission & SECURITY){
            settings->permission |= ROOT;
            status = beesy_commit(settings);
            settings->permission ^= ROOT;
            if(status)
                return status;
        }
    } else {
        status = idPasswd(settings, database);
        if(status)
            return status;
    }

    strcpy(settings->currentDatabase, databasePath);
    status = confidential(databasePath, settings->passdbHash, DECRYPT);
    if(status)
        return strfree(status, 1, &databasePath);

    settings->permission = ((settings->permission >> 29) << 29) | ADVANCED; // reset permissions without altering the root mode
    return strfree(0, 1, &databasePath);
}

int beesy_close_database(Settings *settings)
{
    if(!(settings->permission & (ADVANCED|STAGE))) return EACCES;

    settings->permission = ((settings->permission >> 29) << 29) | WAIT; // reset permissions without altering the root mode
    return confidential(settings->currentDatabase, settings->passdbHash, ENCRYPT);
}

int beesy_comparison(Request *request, Result result, int mode, void *value)
{
    int index;

    if(mode & INTEGER){
        for(index = 0; index < result._size; index++){
            if((mode & DIFFERENT && result._integer[index] != *(long *)value)
               || (mode & EQUAL && result._integer[index] == *(long *)value)){
                request->document[request->length] = extend(strlen(result._matches[index]), result._matches[index]);
                if(request->document[request->length] == NULL) return ENOMEM;
                request->length++;
            }

            if((mode & LOWER && !(mode & DIFFERENT) && result._integer[index] < *(long *)value)
               ||(mode & UPPER && !(mode & DIFFERENT) && result._integer[index] > *(long *)value)){
                request->document[request->length] = extend(strlen(result._matches[index]), result._matches[index]);
                if(request->document[request->length] == NULL) return ENOMEM;
                request->length++;
            }
        }
    } else if(mode & REAL){
        for(index = 0; index < result._size; index++){
            if((mode & DIFFERENT && result._real[index] != *(double *)value)
               || (mode & EQUAL && result._real[index] == *(double *)value)){
                request->document[request->length] = extend(strlen(result._matches[index]), result._matches[index]);
                if(request->document[request->length] == NULL) return ENOMEM;
                request->length++;
            }

            if((mode & LOWER && !(mode & DIFFERENT) && result._real[index] < *(double *)value)
               ||(mode & UPPER && !(mode & DIFFERENT) && result._real[index] > *(double *)value)){
                request->document[request->length] = extend(strlen(result._matches[index]), result._matches[index]);
                if(request->document[request->length] == NULL) return ENOMEM;
                request->length++;
            }
        }
    } else if(mode & STRING){
        for(index = 0; index < result._size; index++){
            if((mode & DIFFERENT && strcmp(result._string[index], *(char **)value))
               || (mode & EQUAL && !strcmp(result._string[index], *(char **)value))
               || (mode & INCLUDE && strstr(result._string[index], *(char **)value) != NULL)){
                request->document[request->length] = extend(strlen(result._matches[index]), result._matches[index]);
                if(request->document[request->length] == NULL) return ENOMEM;
                request->length++;
            }
        }
    }

    return 0;
}

int beesy_search_document(Settings *settings, const char *collection, int mode, const char *criteria, void *value, Request *request)
{
    Result result;
    FILE *fcollection = NULL;
    char *collectionPath = NULL;
    int index;
    int status;

    request->length = 0;
    if(!(settings->permission & (ADVANCED|STAGE))) return EACCES;

    status = strconcat(&collectionPath, 3, settings->currentDatabase, "/~$", collection);
    if(status)
        return status;

    fcollection = fopen(collectionPath, "r");
    if(fcollection != NULL){
        if(readJson(6632, fcollection, criteria, mode & (INTEGER|REAL|STRING), &result)){
            fclose(fcollection);
            return strfree(EPERM, 1, &collectionPath);
        }

        if(mode & INTEGER){
            if(mode & SORT){
                quickSort(result._integer, 0, result._size - 1, mode, result._matches);
            }
        } else if(mode & REAL){
            if(mode & SORT){
                quickSort(result._real, 0, result._size - 1, mode, result._matches);
            }
        } else if(mode & STRING){
            if(mode & SORT){
                quickSort(result._string, 0, result._size - 1, mode, result._matches);
            }
        } else {
            fclose(fcollection);
            return strfree(EINTR, 1, &collectionPath);
        }

        request->document = NEW(char *, result._size);
        if(request->document == NULL) return strfree(ENOMEM, 1, &collectionPath);

        status = beesy_comparison(request, result, mode, value);
        if(status){
            fclose(fcollection);
            return strfree(status, 1, &collectionPath);
        }

        strfree(0, 1, &result._match);
        if(result._integer != NULL) free(result._integer);
        if(result._real != NULL) free(result._real);
        if(mode & STRING){
            for(index = 0; index < result._size; index++){
                strfree(0, 2, &result._string[index], &result._matches[index]);
            }
            if(result._string != NULL) free(result._string);
            if(result._matches != NULL) free(result._matches);
        }

        fclose(fcollection);
    } else {
        return strfree(EINTR, 1, &collectionPath);
    }

    return strfree(0, 1, &collectionPath);
}

int beesy_drop_document(Settings *settings, const char *collection, int mode, const char *criteria, void *value)
{
    Request request;
    char *collectionPath = NULL;
    char *copyPath = NULL;
    int index;
    int status;

    if(!(settings->permission & (ADVANCED|STAGE))) return EACCES;

    status = strconcat(&collectionPath, 3, settings->currentDatabase, "/~$", collection);
    if(status)
        return status;

    status = strconcat(&copyPath, 3, settings->currentDatabase, "/_", collection);
    if(status)
        return status;

    status = beesy_search_document(settings, collection, mode, criteria, value, &request);
    if(status)
        return status;

    status = createCopy(settings, collectionPath, copyPath, request);
    if(status)
        return status;

    for(index = 0; index < request.length; index++)
        if(request.document[index] != NULL) free(request.document[index]);
    if(request.document != NULL) free(request.document);

    return strfree(0, 2, &collectionPath, &copyPath);
}

int beesy_drop_collection(Settings *settings, const char *collection)
{
    char *collectionPath = NULL;
    int status;

    if(!(settings->permission & (ADVANCED|STAGE))) return EACCES;
    if(!strcmp("passwd", collection)) return EACCES;

    status = strconcat(&collectionPath, 3, settings->currentDatabase, "/", collection);
    if(status)
        return status;
    remove(collectionPath);

    status = strconcat(&collectionPath, 3, settings->currentDatabase, "/~$", collection);
    if(status)
        return status;
    remove(collectionPath);

    if(settings->permission & SECURITY){
        settings->permission |= ROOT;
        status = beesy_commit(settings);
        settings->permission ^= ROOT;
        if(status)
            return status;
    }

    printf("deleting the collection \"%s\"...\n", collection);
    return strfree(0, 1, &collectionPath);
}

int beesy_drop_database(Settings *settings, const char *collection, const char *password)
{
    char *command = NULL;
    int status;

    if(!(settings->permission & WAIT)) return EACCES;

    status = beesy_connect_database(settings, collection, password);
    if(status)
        return status;

    settings->permission ^= ADVANCED;
    settings->permission |= WAIT;

    status = strconcat(&command, 5, "rd \"", settings->baseDir, "current/", collection, "\" /s /q > nul");
    if(status)
        return status;

    system(command);

    printf("deleting the database \"%s\"...\n", collection);
    return strfree(0, 1, &command);
}

int beesy_integrity(Stack *stack, const char *criteria)
{
    int index;

    if(!strcmp("_id", criteria)){
        return EACCES;
    }

    for(index = 0; index < stack->size; index++){
        if(!strcmp(criteria, stack->key[index])){
            return EINVAL;
        }
    }

    return 0;
}

int beesy_push(Settings *settings, Stack *stack, const char *criteria, const char *value, int mode)
{
    int status;
    int valueInteger;
    double valueReal;
    char *endptr = NULL;

    if(!(settings->permission & (ADVANCED|STAGE))) return EACCES;

    if(stack->size + 1 > 50){ // if the maximum number of elements in the stack has been reached
        printf("maximum stack size reached: please \"pull\"...\n");
        return EACCES;
    }

    status = beesy_integrity(stack, criteria);
    if(status)
        return status;

    if(mode & INTEGER){
        valueInteger = strtol(value, &endptr, 10);

        if((errno == ERANGE && (valueInteger == LONG_MAX || valueInteger == LONG_MIN))
           || (errno != 0 && valueInteger == 0)
           || (endptr == value)){
            return EINVAL;
        }
    } else if(mode & REAL){
        valueReal = strtod(value, &endptr);

        if((errno == ERANGE && (valueReal == HUGE_VAL || valueReal == -HUGE_VAL))
           || (errno != 0 && valueReal == 0)
           || (endptr == value)){
            return EINVAL;
        }
    }

    strcpy(stack->key[stack->size], criteria);
    stack->key[stack->size][strlen(criteria)] = '\0';

    strcpy(stack->value[stack->size], value);
    stack->value[stack->size][strlen(value)] = '\0';

    stack->type[stack->size] = mode & (INTEGER|REAL|STRING);
    stack->size++;

    printf("A new token has been added to the stack...\n");

    settings->permission = ((settings->permission >> 29) << 29) | STAGE; // reset permissions without altering the root mode
    return 0;
}

int beesy_pull(Settings *settings, Stack *stack, const char *collection)
{
    FILE *fcollection = NULL;
    int index;
    int status;
    char *json_string = NULL;
    char seed[12] = "";
    char id[5] = "";
    char *path = NULL;

    sprintf(seed, "%ld", time(NULL));
    sprintf(id, "%04x", bsd(seed, strlen(seed)));

    status = writeJson(&json_string, "_id", id, STRING, 0, stack->size + 1);
    if(status)
        return status;

    for(index = 0; index < stack->size; index++){
        status = writeJson(&json_string, stack->key[index], stack->value[index], stack->type[index], index + 1, stack->size + 1);
        if(status)
            return status;
    }

    status = strconcat(&path, 3, settings->currentDatabase, "/~$", collection);
    if(status)
        return status;

    fcollection = fopen(path, "a");
    if(fcollection != NULL){
        fprintf(fcollection, "%s\n", json_string);
        printf("%s\n", json_string);
        fclose(fcollection);
    } else {
        return strfree(EINTR , 2, &json_string, &path);
    }

    return strfree(0 , 2, &json_string, &path);
}

int beesy_insert_document(Settings *settings, Stack *stack, const char *collection, int mode)
{
    int status;

    if(!(settings->permission & STAGE)) return EACCES;

    if(mode & PULL){
        status = beesy_pull(settings, stack, collection);
        if(status)
            return status;
    }

    settings->permission = ((settings->permission >> 29) << 29) | ADVANCED; // reset permissions without altering the root mode
    return 0;
}
