#ifndef PACKL_CONTEXT_H
#define PACKL_CONTEXT_H

#include "packl-defs.h"

extern size_t data_type_size[COUNT_PACKL_TYPES];
extern char *context_item_as_cstr[COUNT_CONTEXT_ITEM_TYPES];

void packl_init_contexts(PACKL_File *self);
void packl_remove_contexts(PACKL_File *self);

void packl_push_new_context(PACKL_File *self);
void packl_pop_context(PACKL_File *self);

Context packl_get_current_context(PACKL_File *self);
Context_Item *packl_get_context_item_in_current_context(PACKL_File *self, String_View id);
Context_Item *packl_get_context_item_in_all_contexts(PACKL_File *self, String_View id);
void packl_push_item_in_current_context(PACKL_File *self, Context_Item item);
char *packl_get_context_item_type_as_cstr(Context_Item_Type type);


Context_Item packl_init_var_context_item(String_View name, PACKL_Type type, size_t pos);
Context_Item packl_init_func_context_item(String_View name, PACKL_Type return_type, Parameters params, size_t label_value);
Context_Item packl_init_proc_context_item(String_View name, Parameters params, size_t label_value);
Context_Item packl_init_module_context_item(String_View name, char *filename);
Context_Item packl_init_record_context_item(PACKL_File *self, String_View name, Fields fields);

void packl_find_item_and_report_error_if_found(PACKL_File *self, String_View name, Location loc);
void packl_find_item_in_current_context_and_report_error_if_found(PACKL_File *self, String_View name, Location loc);

Context_Item *packl_lookup_function_or_procedure(PACKL_File *self, String_View name);

Variable packl_find_variable(PACKL_File *self, String_View name, Location loc);
Context_Item *packl_find_function_or_procedure(PACKL_File *self, String_View name, Location loc);
Module packl_find_module(PACKL_File *self, String_View name, Location loc);
PACKL_File packl_find_used_file(PACKL_File *self, char *filename);
Record packl_find_record(PACKL_File *self, String_View name, Location loc);

size_t packl_get_record_field_offset(PACKL_File *self, String_View rec_name, String_View field_name);
Field packl_get_record_field(PACKL_File *self, String_View rec_name, String_View field_name);

size_t packl_get_type_size(PACKL_File *self, PACKL_Type type);

int packl_on_global_context(PACKL_File *self);
Context_Item *packl_get_context_item_in_context(Context context, String_View id);
Function *packl_find_function_in_previous_scopes(PACKL_File *self, String_View name);

#endif 
