#ifndef PACKL_CONTEXT_H
#define PACKL_CONTEXT_H

#include "packl-defs.h"

void packl_init_contexts(PACKL *self);
void packl_push_new_context(PACKL *self);
Context packl_get_current_context(PACKL *self);
Context_Item *packl_get_context_item_in_current_context(PACKL *self, String_View id);
Context_Item *packl_get_context_item_in_all_contexts(PACKL *self, String_View id);
void packl_push_item_in_current_context(PACKL *self, Context_Item item);
char *packl_get_context_item_type_as_cstr(Context_Item_Type type);
void packl_pop_context(PACKL *self);
void packl_remove_contexts(PACKL *self);


Context_Item packl_init_var_context_item(String_View name, String_View type, size_t pos);
Context_Item packl_init_func_context_item(String_View name, String_View return_type, Parameters params, size_t label_value);
Context_Item packl_init_proc_context_item(String_View name, Parameters params, size_t label_value);

void packl_find_item_and_report_error_if_found(PACKL *self, String_View name, Location loc);
void packl_find_item_in_current_context_and_report_error_if_found(PACKL *self, String_View name, Location loc);

Variable packl_find_variable(PACKL *self, String_View name, Location loc);
Context_Item *packl_find_function_or_procedure(PACKL *self, String_View name, Location loc);

#endif 