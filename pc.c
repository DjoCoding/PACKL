#include "src/packl.h"

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
    fprintf(stderr, "usage: %s <filepath> [flags]\n", program);
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

int main(int argc, char **argv) {
    Args args = { argv, argc };

    char *program = args_shift(&args);
    char *input_filepath  = NULL;
    char *output_filepath = NULL;


    if (args_end(&args)) { usage(program); THROW_ERROR("no input file path provided"); }
    input_filepath = args_shift(&args);

    while (!args_end(&args)) {
        char *flag = args_shift(&args);
        if (*flag == '-') {
            flag++;
            for (size_t i = 0; i < ARR_SIZE(flags); ++i) {
                if (strcmp(flag, flags[i]) == 0) { if (config[i]) { usage(program); THROW_ERROR("flag %s is already enabled", flag); } else { config[i] = 1; break; } }
                if (strcmp(flag, "out")) { continue; }
                if (args_end(&args)) { usage(program); THROW_ERROR("no output file path provided"); }
                output_filepath = args_shift(&args);
                break;
            }
            
        } else { THROW_ERROR("invalid flag %s", flag); }   
    }


    if (!output_filepath) { output_filepath = "a.out"; }

    PACKL packl = packl_init(input_filepath, output_filepath);
    packl_load_file(&packl);

    if (config[LEX]) {
        lex(&packl);
        packl_print_tokens(packl.tokens);
    } else if (config[PARSE]) {
        lex(&packl);
        parse(&packl);
        packl_print_ast(packl.ast);
    } else if (config[CODE]) {
        lex(&packl);
        parse(&packl);
        codegen(&packl);
    }

    packl_destroy(&packl);
    return 0;
}