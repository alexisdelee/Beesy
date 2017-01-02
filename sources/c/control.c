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

#include "../h/strlib.h"
#include "../h/interne.h"
#include "../h/control.h"

#define NEW(type, size) malloc(sizeof(type) * (size))
#define ERR_DEBUG(err) __FILE__, __LINE__ + err
#define ENCRYPT 0
#define DECRYPT 1

char hostDir[65] = "database/";
char currentDatabase[65] = "";
short _status = 0;

int beesy_commit()
{
    FILE *frefs = NULL;
    char *databasePath = NULL, *frefsPath = NULL;
    char seed[12] = "", uniqID[41] = "", *command = NULL;

    sprintf(seed, "%ld", time(NULL));
    sha1(uniqID, seed);

    // create directory
    strconcat(&databasePath, 2, hostDir, "tags");
    _mkdir(databasePath);

    strconcat(&databasePath, 3, hostDir, "tags/", uniqID);
    _mkdir(databasePath);

    // copy database and collections
    strconcat(&command, 5, "robocopy ", hostDir, "current ", databasePath, " *.* /e > nul");
    system(command);

    // add/modification refs file
    strconcat(&frefsPath, 2, hostDir, "refs");
    frefs = fopen(frefsPath, "ab");
    if(frefs != NULL){
        fwrite(uniqID, sizeof(char), 40, frefs);
        fclose(frefs);

        return strfree(1, 3, &databasePath, &frefsPath, &system);
    } else {
        return strfree(-1, 3, &databasePath, &frefsPath, &system);
    }
}

int beesy_rollback(const char *hash)
{
    char *command = NULL;

    if(strlen(hash) != 40) return -1;

    strconcat(&command, 3, "rd \"", hostDir, "current\" /s /q > nul");
    system(command);

    strconcat(&command, 2, hostDir, "current");
    _mkdir(command);

    strconcat(&command, 7, "robocopy ", hostDir, "tags/", hash, " ", hostDir, "current/ *.* /e > nul");
    system(command);

    return strfree(1, 1, &command);
}

int beesy_log()
{
    FILE *frefs = NULL;
    char *frefsPath = NULL, hash[41] = "";

    if(strlen(hostDir) < 2) return -1;

    strconcat(&frefsPath, 2, hostDir, "refs");
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

int beesy_connect_database(const char *database, const char *password)
{
    char *databasePath = NULL;

    if(_status) return -1;

    strconcat(&databasePath, 2, hostDir, "current/");
    _mkdir(databasePath);

    strconcat(&databasePath, 3, hostDir, "current/", database);
    if(!_mkdir(databasePath)){
        beesy_commit();
    }

    strcpy(currentDatabase, databasePath);
    if(confidential(databasePath, password, DECRYPT) == -1) return strfree(-1, 1, &databasePath);

    _status = 1;
    return strfree(1, 1, &databasePath);
}

int beesy_close_database(const char *password)
{
    if(!_status) return -1;

    _status = 0;
    return confidential(currentDatabase, password, ENCRYPT);
}

int beesy_search_document(const char *collection, int mode, const char *criteria, void *value, Request *request)
{
    FILE *fcollection = NULL;
    char *collectionPath = NULL;
    int index;

    request->length = 0;
    if(!_status) return -1;

    strconcat(&collectionPath, 3, currentDatabase, "/~$", collection);
    fcollection = fopen(collectionPath, "r");
    if(fcollection != NULL){
        Result result;

        if(mode & INTEGER){
            if(readJson(fcollection, criteria, INTEGER, &result) == -1) return strfree(-1, 1, &collectionPath);

            if(mode & SORT){
                quickSort(result._integer, 0, result._size - 1, mode, result._matches);
            }

            request->document = NEW(char *, result._size);
            if(request->document == NULL) return strfree(-1, 1, &collectionPath);

            for(index = 0; index < result._size; index++){
                if((mode & DIFFERENT && result._integer[index] != *(long int *)value)
                   || (mode & EQUAL && result._integer[index] == *(long int *)value)){
                    request->document[request->length] = extend(strlen(result._matches[index]), result._matches[index]);
                    if(request->document[request->length] == NULL) return strfree(-1, 1, &collectionPath);
                    request->length++;
                }

                if((mode & LOWER && !(mode & DIFFERENT) && result._integer[index] < *(long int *)value)
                   ||(mode & UPPER && !(mode & DIFFERENT) && result._integer[index] > *(long int *)value)){
                    request->document[request->length] = extend(strlen(result._matches[index]), result._matches[index]);
                    if(request->document[request->length] == NULL) return strfree(-1, 1, &collectionPath);
                    request->length++;
                }
            }
        } else if(mode & REAL){
            if(readJson(fcollection, criteria, REAL, &result) == -1) return strfree(-1, 1, &collectionPath);

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
            if(readJson(fcollection, criteria, STRING, &result) == -1) return strfree(-1, 1, &collectionPath);

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

int beesy_drop_document(const char *collection, int mode, const char *criteria, void *value)
{
    FILE *fcollection = NULL;
    FILE *fcopy = NULL;
    char *collectionPath = NULL, *copyPath = NULL, *temporaire = NULL, content[MAXSIZE] = "";
    Request request;
    int index, limit = 0;

    if(!_status) return -1;

    strconcat(&collectionPath, 3, currentDatabase, "/~$", collection);
    fcollection = fopen(collectionPath, "r");
    if(fcollection != NULL){
        if(beesy_search_document(collection, mode, criteria, value, &request) == -1) return -1;

        strconcat(&copyPath, 3, currentDatabase, "/_", collection);
        fcopy = fopen(copyPath, "w");
        if(fcopy != NULL){
            while(fgets(content, MAXSIZE, fcollection) != NULL){
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

                if(temporaire != NULL) free(temporaire);
            }

            fclose(fcopy);
        } else {
            fclose(fcollection);
            return strfree(-1, 1, &collectionPath);
        }

        fclose(fcollection);

        remove(collectionPath);
        rename(copyPath, collectionPath);
    } else {
        return strfree(-1, 2, &collectionPath, &copyPath);
    }

    for(index = 0; index < request.length; index++)
        if(request.document[index] != NULL) free(request.document[index]);
    if(request.document != NULL) free(request.document);

    return strfree(1, 2, &collectionPath, &copyPath);
}
