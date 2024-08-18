#include "packl-context.h"

char *context_item_as_cstr[] = {
    "procedure",
    "variable",
};

Context packl_get_current_context(PACKL *self) {
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

void packl_pop_context(PACKL *self) {
    Context current = packl_get_current_context(self);
    packl_destroy_context(current);
    self->contexts.count--;
}   

void packl_push_new_context(PACKL *self) {
    Context context = {0};
    context.stack_size = self->stack_size;

    DA_INIT(&context, sizeof(Context_Item));
    DA_APPEND(&self->contexts, context);
}

void packl_init_contexts(PACKL *self) {
    DA_INIT(&self->contexts, sizeof(Context));
}

void packl_remove_contexts(PACKL *self) {
    free(self->contexts.items);
}

Context_Item *packl_get_context_item_in_context(Context context, String_View id) {
    for (size_t i = 0; i < context.count; ++i) {
        if (sv_eq(context.items[i].name, id)) { return &context.items[i]; }
    }
    return NULL;
}

Context_Item *packl_get_context_item_in_current_context(PACKL *self, String_View id) {
    return packl_get_context_item_in_context(packl_get_current_context(self), id);
}

Context_Item *packl_get_context_item_in_all_contexts(PACKL *self, String_View id) {
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

void packl_push_item_in_current_context(PACKL *self, Context_Item item) {
    DA_APPEND(&self->contexts.items[self->contexts.count - 1], item);
}

Parameters packl_copy_params(Parameters params) {
    Parameters out = {0};
    out.items = malloc(sizeof(Parameter) * params.count);
    out.count = out.size = params.count;
    memcpy(out.items, params.items, sizeof(Parameter) * params.count);
    return out;
}

Variable packl_init_context_variable(String_View type, size_t pos) {
    Variable var = {0};
    var.stack_pos = pos;
    var.type = type;
    return var;
}

Function packl_init_context_function(String_View return_type, Parameters params, size_t label_value) {
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

Context_Item packl_init_var_context_item(String_View name, String_View type, size_t pos) {
    Variable var = packl_init_context_variable(type,pos);
    return (Context_Item) { .name = name, .type = CONTEXT_ITEM_TYPE_VARIABLE, .as.variable = var };
} 

Context_Item packl_init_func_context_item(String_View name, String_View return_type, Parameters params, size_t label_value) {
    Function func = packl_init_context_function(return_type, params, label_value);
    return (Context_Item) { .name = name, .type = CONTEXT_ITEM_TYPE_FUNCTION, .as.func = func };
}

Context_Item packl_init_proc_context_item(String_View name, Parameters params, size_t label_value) {
    Procedure proc = packl_init_context_procedure(params, label_value);
    return (Context_Item) { .name = name, .type = CONTEXT_ITEM_TYPE_PROCEDURE, .as.proc = proc };
}
