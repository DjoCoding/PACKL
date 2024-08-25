#include "src/headers/packl.h"
#include <time.h>

typedef struct {
    char **value;
    int count;
} Args;

char *args_shift(Args *args) {
    args->count--;
    char *value = *args->value;
    args->value++;
    return value;
}

int args_end(Args *args) {
    return (args->count == 0);
}

void usage(char *program) {
    printf("usage: %s <filepath> [OPTIONS]\n", program);
    printf("OPTIONS:\n");
    printf("\thelp:     display the usage\n");
    printf("\tlex:      lex the PACKL file and display the tokens\n");
    printf("\tparse:    parse the PACKL file and display the AST\n");
    printf("\tcode:     generate the PASM file\n");
    printf("\tout:      specify the PASM output file name\n");
}

typedef enum {
    LEX = 0,
    PARSE, 
    CODE,
    OUT,
    COUNT_FLAGS,
} Config_Flags;

char *flags[] = {
    "lex",
    "parse",
    "code",
    "out",
};

uint8_t config[COUNT_FLAGS] = {0};

void handle_flags(Args *args, char *program, char **output) {
    while (!args_end(args)) {
        char *flag = args_shift(args);
        if (*flag == '-') {
            flag++;
            for (size_t i = 0; i < ARR_SIZE(flags); ++i) {
                if (strcmp(flag, flags[i]) == 0) { if (config[i]) { usage(program); THROW_ERROR("flag %s is already enabled", flag); } else { config[i] = 1; break; } }
                if (strcmp(flag, "out")) { continue; }
                if (args_end(args)) { usage(program); THROW_ERROR("no output file path provided"); }
                *output = args_shift(args);
                break;
            }
            
        } else { THROW_ERROR("invalid flag %s", flag); }   
    }
}

int main(int argc, char **argv) {
    srand(time(NULL));

    Args args = { argv, argc };

    char *program = args_shift(&args);
    char *input_filepath  = NULL;
    char *output_filepath = NULL;


    if (args_end(&args)) { usage(program); THROW_ERROR("no input file path provided"); }
    char *flag = args_shift(&args);

    if(strcmp(flag, "-help") == 0) { usage(program); exit(0); }
    else {
        input_filepath = flag;
    }

    handle_flags(&args, program, &output_filepath);

    if (!output_filepath) { output_filepath = "a.out"; }

    PACKL_Compiler c = packl_init(input_filepath, output_filepath);

    if (config[LEX]) {
        lex(&c.root_file);
        packl_print_tokens(c.root_file.tokens);
    } else if (config[PARSE]) {
        lex(&c.root_file);
        parse(&c.root_file);
        packl_print_ast(c.root_file.ast);
    } else if (config[CODE]) {
        compile(&c);
    }

    packl_destroy(&c);
    return 0;
}