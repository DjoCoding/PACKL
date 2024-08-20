#include "packl-context.h"

char *context_item_as_cstr[] = {
    "procedure",
    "variable",
    "function",
};

Context packl_get_current_context(PACKL_File *self) {
    return self->contexts.items[self->contexts.count - 1];
}

void packl_destroy_context(Context context) {
    for (size_t i = 0; i < context.count; ++i) {
        Context_Item item = context.items[i];
        if (context.items[i].type == CONTEXT_ITEM_TYPE_PROCEDURE) {
            free(item.as.proc.params.items);
            item.as.proc.params.size = 0;
        }
    }
}

void packl_pop_context(PACKL_File *self) {
    Context current = packl_get_current_context(self);
    packl_destroy_context(current);
    self->contexts.count--;
}   

void packl_push_new_context(PACKL_File *self) {
    Context context = {0};
    DA_INIT(&context, sizeof(Context_Item));
    DA_APPEND(&self->contexts, context);
}

void packl_init_contexts(PACKL_File *self) {
    DA_INIT(&self->contexts, sizeof(Context));
}

void packl_remove_contexts(PACKL_File *self) {
    free(self->contexts.items);
}

Context_Item *packl_get_context_item_in_context(Context context, String_View id) {
    for (size_t i = 0; i < context.count; ++i) {
        if (sv_eq(context.items[i].name, id)) { return &context.items[i]; }
    }
    return NULL;
}

Context_Item *packl_get_context_item_in_current_context(PACKL_File *self, String_View id) {
    return packl_get_context_item_in_context(packl_get_current_context(self), id);
}

Context_Item *packl_get_context_item_in_all_contexts(PACKL_File *self, String_View id) {
    for (size_t i = self->contexts.count - 1;; --i) {
        Context context = self->contexts.items[i];
        Context_Item *item = packl_get_context_item_in_context(context, id);
        if (item) { return item; }
        if (i == 0) { break; }
    } 
    return NULL;
}

char *packl_get_context_item_type_as_cstr(Context_Item_Type type) {
    return context_item_as_cstr[type];
}

void packl_push_item_in_current_context(PACKL_File *self, Context_Item item) {
    DA_APPEND(&self->contexts.items[self->contexts.count - 1], item);
}

Parameters packl_copy_params(Parameters params) {
    Parameters out = {0};
    out.items = malloc(sizeof(Parameter) * params.count);
    out.count = out.size = params.count;
    memcpy(out.items, params.items, sizeof(Parameter) * params.count);
    return out;
}

Variable packl_init_context_variable(PACKL_Type type, size_t pos) {
    Variable var = {0};
    var.stack_pos = pos;
    var.type = type;
    return var;
}

Function packl_init_context_function(PACKL_Type return_type, Parameters params, size_t label_value) {
    Function func = {0};
    func.label_value = label_value;
    func.params = packl_copy_params(params);
    func.return_type = return_type;
    return func;
}

Procedure packl_init_context_procedure(Parameters params, size_t label_value) {
    Procedure proc = {0};
    proc.label_value = label_value;
    proc.params = packl_copy_params(params);
    return proc;
}

Module packl_init_context_module(char *filename) {
    Module module = {0};
    module.filename = filename;
    return module;
}

Context_Item packl_init_var_context_item(String_View name, PACKL_Type type, size_t pos) {
    Variable var = packl_init_context_variable(type, pos);
    return (Context_Item) { .name = name, .type = CONTEXT_ITEM_TYPE_VARIABLE, .as.variable = var };
} 

Context_Item packl_init_func_context_item(String_View name, PACKL_Type return_type, Parameters params, size_t label_value) {
    Function func = packl_init_context_function(return_type, params, label_value);
    return (Context_Item) { .name = name, .type = CONTEXT_ITEM_TYPE_FUNCTION, .as.func = func };
}

Context_Item packl_init_proc_context_item(String_View name, Parameters params, size_t label_value) {
    Procedure proc = packl_init_context_procedure(params, label_value);
    return (Context_Item) { .name = name, .type = CONTEXT_ITEM_TYPE_PROCEDURE, .as.proc = proc };
}

Context_Item packl_init_module_context_item(String_View name, char *filename) {
    Module module = packl_init_context_module(filename);
    return (Context_Item) { .name = name, .type = CONTEXT_ITEM_TYPE_MODULE, .as.module = module };
}

void packl_find_item_and_report_error_if_found(PACKL_File *self, String_View name, Location loc) {
    Context_Item *item = packl_get_context_item_in_all_contexts(self, name);
    if (!item) { return; }
    PACKL_ERROR_LOC(self->filename, loc, SV_FMT " declared twice, first declared as %s", SV_UNWRAP(name), context_item_as_cstr[item->type]);
}

void packl_find_item_in_current_context_and_report_error_if_found(PACKL_File *self, String_View name, Location loc) {
    Context_Item *item = packl_get_context_item_in_current_context(self, name);
    if (!item) { return; }
    PACKL_ERROR_LOC(self->filename, loc, SV_FMT " declared twice, first declared as %s", SV_UNWRAP(name), context_item_as_cstr[item->type]);
}

Variable packl_find_variable(PACKL_File *self, String_View name, Location loc) {
    Context_Item *item = packl_get_context_item_in_all_contexts(self, name);
    if (!item) { PACKL_ERROR_LOC(self->filename, loc, "variable `" SV_FMT "` referenced before declaration", SV_UNWRAP(name)); }
    if (item->type != CONTEXT_ITEM_TYPE_VARIABLE) { PACKL_ERROR_LOC(self->filename, loc, "expected `" SV_FMT "` to be a variable but found as %s", SV_UNWRAP(name), context_item_as_cstr[item->type]); }
    return item->as.variable;
}

Module packl_find_module(PACKL_File *self, String_View name, Location loc) {
    Context_Item *item = packl_get_context_item_in_all_contexts(self, name);
    if (!item) { PACKL_ERROR_LOC(self->filename, loc, "module `" SV_FMT "` referenced before usage", SV_UNWRAP(name)); }
    if (item->type != CONTEXT_ITEM_TYPE_MODULE) { PACKL_ERROR_LOC(self->filename, loc, "expected `" SV_FMT "` to be a module but found as %s", SV_UNWRAP(name), context_item_as_cstr[item->type]); }
    return item->as.module;
}

Context_Item *packl_lookup_function_or_procedure(PACKL_File *self, String_View name) {
    Context_Item *item = packl_get_context_item_in_all_contexts(self, name);
    if (!item) { return NULL; }
    if (item->type != CONTEXT_ITEM_TYPE_FUNCTION && item->type != CONTEXT_ITEM_TYPE_PROCEDURE) { PACKL_ERROR(self->filename, "expected `" SV_FMT "` to be a function or a procedure but found as %s", SV_UNWRAP(name), context_item_as_cstr[item->type]); }
    return item;
}


Context_Item *packl_find_function_or_procedure(PACKL_File *self, String_View name, Location loc) {
    Context_Item *item = packl_get_context_item_in_all_contexts(self, name);
    if (!item) { PACKL_ERROR_LOC(self->filename, loc, "function `" SV_FMT "` called before declaration", SV_UNWRAP(name)); }
    if (item->type != CONTEXT_ITEM_TYPE_FUNCTION && item->type != CONTEXT_ITEM_TYPE_PROCEDURE) { PACKL_ERROR_LOC(self->filename, loc, "expected `" SV_FMT "` to be a function or a procedure but found as %s", SV_UNWRAP(name), context_item_as_cstr[item->type]); }
    return item;
}

PACKL_File packl_find_used_file(PACKL_File *self, char *filename) {
    for(size_t i = 0; i < self->used_files.count; ++i) {
        PACKL_File file = self->used_files.items[i];
        if (strcmp(file.filename, filename) == 0) {
            return file;
        }
    }
    ASSERT(false, "unreachable");
}