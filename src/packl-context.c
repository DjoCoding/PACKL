#include "packl-context.h"

size_t data_type_size[COUNT_PACKL_TYPES] = {8, 8, 8};

char *context_item_as_cstr[COUNT_CONTEXT_ITEM_TYPES] = {
    "procedure",
    "variable",
    "function",
    "record",
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

size_t packl_get_array_size(PACKL_File *self, Array_Type arr_type) {
    size_t size = arr_type.size;
    size_t item_size = 0;

    PACKL_Type item_type = *arr_type.item_type;

    switch(item_type.kind) {
        case PACKL_TYPE_ARRAY:
            item_size = packl_get_array_size(self,item_type.as.array);
            break;
        default:
            item_size = packl_get_type_size(self, item_type);
    }        

    return size * item_size;
}

size_t packl_get_user_defined_type(PACKL_File *self, String_View type_name) {
    Context_Item *item = packl_get_context_item_in_all_contexts(self, type_name);
    if (!item) {
        PACKL_ERROR(self->filename, "`" SV_FMT "` type not declared yet", SV_UNWRAP(type_name));
    }

    if (item->type != CONTEXT_ITEM_TYPE_RECORD) {
        PACKL_ERROR(self->filename, "`" SV_FMT "` is not a type", SV_UNWRAP(type_name));
    }

    return item->as.record.size;
}

size_t packl_get_type_size(PACKL_File *self, PACKL_Type type) {
    switch(type.kind) {
        case PACKL_TYPE_BASIC:
            return data_type_size[type.as.basic];
        case PACKL_TYPE_ARRAY:
            return packl_get_array_size(self, type.as.array);
        case PACKL_TYPE_USER_DEFINED:   
            return packl_get_user_defined_type(self, type.as.user_defined);
        default:
            ASSERT(false, "unreachable");
    }
}

size_t packl_get_record_size(PACKL_File *self, String_View name, Fields fields) {
    size_t size = 0;

    for(size_t i = 0; i < fields.count; ++i) {
        Field field = fields.items[i];
        size += packl_get_type_size(self, field.type);
    }

    return size;    
}

Record packl_init_context_record(PACKL_File *self, Fields fields) {
    Record rec = {0};
    rec.fields = fields;
    return rec;
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

Context_Item packl_init_record_context_item(PACKL_File *self, String_View name, Fields fields) {
    Record record = packl_init_context_record(self, fields);
    record.size = packl_get_record_size(self, name, record.fields);
    return (Context_Item) { .name = name, .type = CONTEXT_ITEM_TYPE_RECORD, .as.record = record };
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

Record packl_find_record(PACKL_File *self, String_View name, Location loc) {
    Context_Item *item = packl_get_context_item_in_all_contexts(self, name);
    if (!item) { PACKL_ERROR_LOC(self->filename, loc, "record `" SV_FMT "` referenced before usage", SV_UNWRAP(name)); }
    if (item->type != CONTEXT_ITEM_TYPE_RECORD) { PACKL_ERROR_LOC(self->filename, loc, "expected `" SV_FMT "` to be a record but found as %s", SV_UNWRAP(name), context_item_as_cstr[item->type]); }
    return item->as.record; 
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


size_t packl_get_record_field_offset(PACKL_File *self, String_View rec_name, String_View field_name) {
    Record record = packl_find_record(self, rec_name, (Location){0,0});
    size_t offset = 0;

    for(size_t i = 0; i < record.fields.count; ++i) {
        Field field = record.fields.items[i];
        if (sv_eq(field_name, field.name)) { return offset; }
        offset += packl_get_type_size(self, field.type);
    }

    PACKL_ERROR(self->filename, "no field `" SV_FMT "` for the record `" SV_FMT "`", SV_UNWRAP(field_name), SV_UNWRAP(rec_name));
}

Field packl_get_record_field(PACKL_File *self, String_View rec_name, String_View field_name) {
    Record record = packl_find_record(self, rec_name, (Location){0,0});

    for(size_t i = 0; i < record.fields.count; ++i) {
        Field field = record.fields.items[i];
        if (sv_eq(field_name, field.name)) { return field; }
    }

    PACKL_ERROR(self->filename, "no field `" SV_FMT "` for the record `" SV_FMT "`", SV_UNWRAP(field_name), SV_UNWRAP(rec_name));
}