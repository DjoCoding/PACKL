#ifndef PACKL_CONTEXT_H
#define PACKL_CONTEXT_H

#include "packl-defs.h"

void packl_init_contexts(PACKL_File *self);
void packl_push_new_context(PACKL_File *self);
Context packl_get_current_context(PACKL_File *self);
Context_Item *packl_get_context_item_in_current_context(PACKL_File *self, String_View id);
Context_Item *packl_get_context_item_in_all_contexts(PACKL_File *self, String_View id);
void packl_push_item_in_current_context(PACKL_File *self, Context_Item item);
char *packl_get_context_item_type_as_cstr(Context_Item_Type type);
void packl_pop_context(PACKL_File *self);
void packl_remove_contexts(PACKL_File *self);


Context_Item packl_init_var_context_item(String_View name, String_View type, size_t pos);
Context_Item packl_init_func_context_item(String_View name, String_View return_type, Parameters params, size_t label_value);
Context_Item packl_init_proc_context_item(String_View name, Parameters params, size_t label_value);
Context_Item packl_init_module_context_item(String_View name, char *filename);

void packl_find_item_and_report_error_if_found(PACKL_File *self, String_View name, Location loc);
void packl_find_item_in_current_context_and_report_error_if_found(PACKL_File *self, String_View name, Location loc);

Context_Item *packl_lookup_function_or_procedure(PACKL_File *self, String_View name);

Variable packl_find_variable(PACKL_File *self, String_View name, Location loc);
Context_Item *packl_find_function_or_procedure(PACKL_File *self, String_View name, Location loc);
Module packl_find_module(PACKL_File *self, String_View name, Location loc);
PACKL_File packl_find_used_file(PACKL_File *self, char *filename);

#endif 