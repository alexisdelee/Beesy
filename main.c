#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>

#include "sources/h/strlib.h"
#include "sources/h/shell.h"
#include "sources/h/control.h"
#include "sources/h/interne.h"

int main(int argc, char **argv)
{
    char command[300] = "";
    int status;
    int index;
    Settings settings;
    Terminal terminal;
    Stack stack;

    stack.size = 0; // initialize the stack at 0

    printf("Beesy shell version: 1.0.0\
           \nWelcome to the Beesy shell.\
           \nFor interactive help, type \"help\".\n\n");

    // configure of the application from the file beesy.inc
    if(beesy_settings(&settings)){
        beesy_error(EILSEQ);
        exit(-1);
    }

    terminal.header = extend(1, "");
    if(terminal.header == NULL){
        beesy_error(ENOMEM);
        exit(-1);
    }

    // start the creation of the files / folders necessary for the application
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
        status = beesy_analyze(&settings, &terminal, &stack);
        if(status)
            beesy_error(status);

        // remember to free memory
        for(index = 0; index < terminal.argc; index++)
            strfree(0, 1, &terminal.argv[index]);
        if(terminal.argv == NULL) free(terminal.argv);
    } while(1);

    return strfree(0, 1, &terminal.header);
}
