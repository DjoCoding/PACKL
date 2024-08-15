#include "packl-codegen.h"

void packl_generate_statements_code(FILE *f, PACKL *self, AST ast, size_t indent);

void fprint_indent(FILE *f, size_t indent) {
    for (size_t i = 0; i < indent; ++i) {
        fprintf(f, "  ");
    }
}

void packl_generate_push(FILE *f, String_View view, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "push " SV_FMT "\n", SV_UNWRAP(view));
}

void packl_generate_write(FILE *f,  String_View view, size_t indent) {
    // push the stdout for the moment
    fprint_indent(f, indent);
    fprintf(f, "push 0\n");

    fprint_indent(f, indent);
    fprintf(f, "pushs \"" SV_FMT "\"\n", SV_UNWRAP(view));
    
    fprint_indent(f, indent);
    fprintf(f, "push %zu\n", view.count);
    
    fprint_indent(f, indent);
    fprintf(f, "syscall 0\n");
}

void packl_generate_exit(FILE *f, String_View view, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "push " SV_FMT "\n", SV_UNWRAP(view));

    fprint_indent(f, indent);
    fprintf(f, "syscall 6\n");
}

void packl_generate_call(FILE *f, size_t label, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "call %zu\n", label);
}

void packl_generate_push_arg_code(FILE *f, PACKL *self, Func_Call_Arg arg, size_t indent) {
    // TODO: add the argument id check
    packl_generate_push(f, arg.value, indent);
}

void packl_generate_push_args_code(FILE *f, PACKL *self, Func_Call_Args args, size_t indent) {
    for (size_t i = 0; i < args.count; ++i) {
        packl_generate_push_arg_code(f, self, args.items[i], indent);
    }
}

void packl_generate_func_call_code(FILE *f, PACKL *self, Func_Call func_call, size_t indent) {
    Context_Item *item = packl_get_context_item_in_all_contexts(self, func_call.name);
    if (!item) { PACKL_ERROR(self->filename, "function " SV_FMT " called but not decalred", SV_UNWRAP(func_call.name)); }
    
    packl_generate_push_args_code(f, self, func_call.args, indent);

    packl_generate_call(f, item->as.proc.label_value, indent);
}

void packl_generate_native_call_code(FILE *f, PACKL *self, Func_Call func_call, size_t indent) {
    if (sv_eq(func_call.name, SV("write"))) {
        packl_generate_write(f, func_call.args.items[0].value, indent);
    } else if (sv_eq(func_call.name, SV("exit"))) {
        packl_generate_exit(f, func_call.args.items[0].value, indent);
    } else { ASSERT(false, "unreachable"); }
}

void packl_generate_proc_def_code(FILE *f, PACKL *self, Proc_Def proc_def, size_t indent) {
    fprint_indent(f, indent);

    // check if the procedure is already defined in the current context
    Context_Item *found = packl_get_context_item_in_current_context(self, proc_def.name);

    if (found) { PACKL_ERROR(self->filename, "procedure " SV_FMT " already declared in the current scope as %s", SV_UNWRAP(proc_def.name), packl_get_context_item_type_as_cstr(found->type)); }

    // add the procedure to the current context
    Procedure proc = {0};
    
    proc.params.items = malloc(sizeof(Parameter) * proc_def.params.count); 
    proc.params.count = proc.params.size = proc_def.params.count;
    memcpy(proc.params.items, proc_def.params.items, sizeof(Parameter) * proc_def.params.count);
    
    proc.label_value = self->label_value;

    Context_Item item = {0};
    item.name = proc_def.name;
    item.type = CONTEXT_ITEM_TYPE_PROCEDURE;
    item.as.proc = proc;

    packl_push_item_in_current_context(self, item);
    
    fprintf(f, "#label_%zu:\n", self->label_value++);

    // push a new context for the procedure definition
    packl_push_new_context(self);
    
    // generate the code in that context 
    packl_generate_statements_code(f, self, *proc_def.body, indent + 1);

    // pop the context added 
    packl_pop_context(self);

    fprint_indent(f, indent + 1);
    fprintf(f, "ret\n\n");
} 

void packl_generate_statement_code(FILE *f, PACKL *self, Node node, size_t indent) {
    if (node.kind == NODE_KIND_NATIVE_CALL) {
        packl_generate_native_call_code(f, self, node.as.func_call, indent);
    } else if (node.kind == NODE_KIND_FUNC_CALL) {
        packl_generate_func_call_code(f, self, node.as.func_call, indent);
    } else if (node.kind == NODE_KIND_PROC_DEF) {
        packl_generate_proc_def_code(f, self, node.as.proc_def, indent);
    } else { ASSERT(false, "unreachable"); }
}

void packl_generate_statements_code(FILE *f, PACKL *self, AST ast, size_t indent) {
    for (size_t i = 0; i < ast.count; ++i) {
        packl_generate_statement_code(f, self, ast.items[i], indent);
    }
}

void packl_generate_entry_code(FILE *f, PACKL *self) {
    Context_Item *item = packl_get_context_item_in_current_context(self, SV("main"));
    
    if (!item) { PACKL_ERROR(self->filename, "no `main` entry point provided"); }
    if (item->type != CONTEXT_ITEM_TYPE_PROCEDURE) { PACKL_ERROR(self->filename, "found `main` defintion but as %s, consider adding the `main` entry point", packl_get_context_item_type_as_cstr(item->type)); }

    fprintf(f, "#entry: $label_%zu\n", item->as.proc.label_value);
}

void packl_generate_code(PACKL *self) {
    FILE *f = fopen(self->output, "w");
    if (!f) { PACKL_ERROR(self->filename, "could not open the file for the code generation"); }

    // init the contexts    
    packl_init_contexts(self);

    // push the global context
    packl_push_new_context(self);

    packl_generate_statements_code(f, self, self->ast, 0);
    
    // generate the entry point code    
    packl_generate_entry_code(f, self);

    // pop the global context
    packl_pop_context(self);

    // remove all the contexts
    packl_remove_contexts(self);

    fclose(f);
}