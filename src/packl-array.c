#include "headers/packl-array.h"

void packl_generate_array_item_size(PACKL_Compiler *c, PACKL_File *self, PACKL_Type item_type, size_t indent) {
    size_t item_size = packl_get_type_size(self, item_type);
    packl_generate_push(c, (int64_t)item_size, indent);
}

void packl_generate_array_size(PACKL_Compiler *c, PACKL_File *self, Array_Type type, size_t indent) {
    size_t size = type.size;
    packl_generate_array_item_size(c, self, *type.item_type, indent);
    packl_generate_push(c, (int64_t)size, indent);
    packl_generate_mul(c, indent);
}

void packl_generate_array_allocation_code(PACKL_Compiler *c, PACKL_File *self, Array_Type type, size_t indent) {
    packl_generate_array_size(c, self, type, indent);
    packl_generate_syscall(c, 2, indent); // the alloc syscall
}
