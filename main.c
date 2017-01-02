#include <stdlib.h>
#include <stdio.h>

#include "sources/h/control.h"

int main(int argc, char **argv)
{
    Request request;
    int index, value = 20;

    if(beesy_connect_database("esgi", "P2wV7UUQ") == 1){
        if(beesy_search_document("initial", INTEGER|EQUAL|UPPER|SORT, "age", &value, &request) != 1){
            printf("Research problem\n");
            exit(-1);
        } else {
            for(index = 0; index < request.length; index++){
                printf("%s\n", request.document[index]);
                if(request.document[index] != NULL) free(request.document[index]);
            }
            if(request.document != NULL) free(request.document);
        }

        // beesy_drop_document("initial", INTEGER|EQUAL, "age", &value);
        beesy_close_database("P2wV7UUQ");
    } else {
        printf("Connexion problem\n");
        exit(-1);
    }

    return 0;
}
