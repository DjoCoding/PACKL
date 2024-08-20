#ifndef PACKL_CODEGEN_H
#define PACKL_CODEGEN_H

#include "packl-defs.h"
#include "packl-error.h"
#include "packl-context.h"
#include "packl-lexer.h"
#include "packl-parser.h"

#define codegen packl_generate_file_code

PACKL_File packl_init_file(char *filename);
void packl_generate_file_code(PACKL_Compiler *c, PACKL_File *self);

#endif 