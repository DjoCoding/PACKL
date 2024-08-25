#include "headers/packl-operator.h"

size_t packl_get_operator_input_size(PACKL_Compiler *c, PACKL_File *self, Expr_Operator_Input input) {
    if (input.kind == INPUT_KIND_TYPE) {
        return packl_get_type_size(self, input.as.type);
    } 
    
    if (input.kind == INPUT_KIND_ID) {
        Context_Item *item = packl_get_context_item_in_all_contexts(self, input.as.identifier);
        if (!item) {
            PACKL_ERROR(self->filename, "`" SV_FMT "` not declared yet", SV_UNWRAP(input.as.identifier));
        }

        if (item->type == CONTEXT_ITEM_TYPE_CLASS) {
            return item->as.class.attrs_size;
        }
        
        PACKL_ERROR(self->filename, "expected `"SV_FMT"` to be a class type but found as %s", SV_UNWRAP(item->name), context_item_as_cstr[item->type]);
    } 
    
    ASSERT(false, "`packl_generate_native_sizeof_code` failed to generate the code for the sizeof operator input kind");   
}