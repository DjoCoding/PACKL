#ifndef PACKL_LEXER_H
#define PACKL_LEXER_H

#include "packl-defs.h"


extern char *builtin_operators[];

#define lpeek    packl_lexer_peek
#define ladv     packl_lexer_advance
#define leof     packl_lexer_eof 
#define lread    packl_lexer_read_token
#define lex      packl_lexer_lex

void packl_lexer_lex(PACKL_File *self);

#endif 