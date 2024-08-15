#ifndef PACKL_H
#define PACKL_H


#include "packl-defs.h"
#include "packl-lexer.h"
#include "packl-parser.h"
#include "packl-codegen.h"
#include "packl-context.h"
#include "packl-printer.h"

PACKL packl_init(char *input, char *output);
void packl_load_file(PACKL *self);
void packl_destroy(PACKL *self);


#endif 