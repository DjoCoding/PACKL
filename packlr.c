#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    char *program = *argv++;
    argc--;

    if(argc == 0) {
        fprintf(stderr, "expected a file to compile");
        exit(1);
    }

    int donext = 1;
    char *filepath = *argv++;
    
    char command[4000] = {0};
    sprintf(command, "./bin/packlc %s -code -out file.pasm\n", filepath);
    if (system(command)) {
        fprintf(stderr, "compilation failed\n");
        donext = 0;
    }


    sprintf(command, "./bin/pasm file.pasm\n");
    if (donext && system(command)) {
        fprintf(stderr, "assembling failed\n");
        donext = 0;
    }

    if (donext) {
        sprintf(command, "./bin/pvmr file.pvm\n");
        system(command);
    }

    sprintf(command, "rm -f file.pasm file.pvm\n");
    system(command);
    
    return 0;
}
