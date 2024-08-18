#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    char *program = *argv++;
    argc--;

    if(argc == 0) {
        fprintf(stderr, "expected a file to compile");
        exit(1);
    }

    char *filepath = *argv++;
    
    char command[4000] = {0};
    sprintf(command, "./bin/packlc %s -code -out file.pasm\n", filepath);
    if (system(command)) {
        fprintf(stderr, "compilation failed\n");
        exit(1);
    }


    sprintf(command, "./bin/pasm file.pasm\n");
    if (system(command)) {
        fprintf(stderr, "assembling failed\n");
        exit(1);
    }
    
    sprintf(command, "./bin/pvmr file.pvm\n");
    if (system(command)) {
        fprintf(stderr, "execution failed\n");
        exit(1);
    }

    return 0;
}
