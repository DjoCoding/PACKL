#ifndef PACKL_ARRAY_H
#define PACKL_ARRAY_H

#include "packl-defs.h"
#include "packl-context.h"
#include "pvm-instructions.h"

void packl_generate_array_item_size(PACKL_Compiler *c, PACKL_File *self, PACKL_Type item_type, size_t indent);
void packl_generate_array_size(PACKL_Compiler *c, PACKL_File *self, Array_Type type, size_t indent);
void packl_generate_array_allocation_code(PACKL_Compiler *c, PACKL_File *self, Array_Type type, size_t indent);

#endif 