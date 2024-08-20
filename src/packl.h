#ifndef PACKL_H
#define PACKL_H


#include "packl-defs.h"
#include "packl-lexer.h"
#include "packl-parser.h"
#include "packl-codegen.h"
#include "packl-context.h"
#include "packl-printer.h"

#define compile packl_compile

PACKL_Compiler packl_init(char *input, char *output);
void packl_destroy(PACKL_Compiler *c);
void packl_compile(PACKL_Compiler *c);

#endif 