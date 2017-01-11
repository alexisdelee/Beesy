#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "sources/h/strlib.h"
#include "sources/h/shell.h"
#include "sources/h/control.h"

int main(int argc, char **argv)
{
    /* Settings settings;
    [X] beesy_settings(&settings);
    [X] beesy_connect_database(&settings, "esgi", "P2wV7UUQ")
    [X] beesy_search_document(settings, "initial", INTEGER|EQUAL|UPPER|SORT, "age", &value, &request)
    [ ] beesy_insert_document(settings, "initial", ...)
    [X] beesy_drop_document(settings, "initial", INTEGER|EQUAL", "age", &value)
    [X] beesy_close_database(settings, "P2wV7UUQ")
    [X] beesy_drop_collection(settings, "initial")
    [ ] beesy_drop_database(settings, "esgi") */

    /* Settings settings;
    if(beesy_settings(&settings) == -1) exit(-1);

    if(beesy_connect_database(&settings, "esgi", "P2wV7UUQ") == 1){
        if(beesy_search_document(settings, "initial", INTEGER|EQUAL|UPPER|SORT, "age", &value, &request) != 1){
            printf("Research problem\n");
            exit(-1);
        } else {
            for(index = 0; index < request.length; index++){
                printf("%s\n", request.document[index]);
                if(request.document[index] != NULL) free(request.document[index]);
            }
            if(request.document != NULL) free(request.document);
        }

        // beesy_drop_document(settings, "initial", INTEGER|EQUAL, "age", &value);
        // beesy_drop_collection(settings, "test");
        beesy_close_database(&settings, "P2wV7UUQ");
    } else {
        printf("Connexion problem\n");
        exit(-1);
    } */

    /* <shell> */
    char command[300] = "";
    int status;
    int index;
    Settings settings;
    Terminal terminal;

    printf("Beesy shell version: 1.0.0\
           \nWelcome to the Beesy shell.\
           \nFor interactive help, type \"help\".\n\n");

    // configure of the application from the file beesy.inc
    if(beesy_settings(&settings)){
        beesy_error(EILSEQ);
        exit(-1);
    }

    if(str(&terminal.header, "")){
        beesy_error(ENOMEM);
        exit(-1);
    }

    // initialization
    /* if(settings.permission & UNINITIATED){
        status = beesy_boot(&settings);
        if(status)
            return status;
    } */
    status = beesy_boot(&settings);
    if(status)
        return status;

    do {
        // initialization of the Terminal structure
        terminal.argc = 0;
        terminal.argv = NULL;

        // display of the header
        printf("%s%c ", terminal.header, settings.permission & ROOT ? '#' : '$');

        // input of the command
        fflush(stdin);
        fgets(command, 300, stdin);

        // cleaning and splitting of the input variable
        status = beesy_argv(command, &terminal);
        if(status){
            beesy_error(status);
            exit(-1);
        }

        // analysis of the input variable
        status = beesy_analyze(&settings, &terminal);
        if(status)
            beesy_error(status);

        // remember to free memory
        for(index = 0; index < terminal.argc; index++)
            strfree(0, 1, &terminal.argv[index]);
        if(terminal.argv == NULL) free(terminal.argv);
    } while(1);
    /* </shell> */

    return strfree(0, 1, &terminal.header);
}
