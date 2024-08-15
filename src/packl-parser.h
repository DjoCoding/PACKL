#ifndef PACKL_PARSER_H
#define PACKL_PARSER_H

#include "packl-defs.h"
#include "packl-error.h"
#include <stdbool.h>

#define ppeek(self)                     packl_parser_peek(self, 0)
#define ppeek_(self, ahead)             packl_parser_peek(self, ahead)
#define padv                            packl_parser_advance
#define peot                            packl_parser_eot
#define pexp                            packl_parser_expect
#define pstmt                           packl_parser_parse_statement
#define parse                           packl_parser_parse

void packl_parser_parse(PACKL *self);


#endif 