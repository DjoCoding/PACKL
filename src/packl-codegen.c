#include "packl-codegen.h"

size_t number_of_popped_values[] = {
    3,       // write  
    1,       // read: 2 for reading and 1 for push
    0,       // alloc: 1 for the size and 1 for the push
    1,       // free
    1,       // open: 2 for the filepath and the mode and one of the file pointer (push) 
    1,       // close 
    1,       // exit
};

void packl_generate_expr_code(FILE *f, PACKL *self, Expression expr, size_t indent);
void packl_generate_statements_code(FILE *f, PACKL *self, AST ast, size_t indent);

void fprint_indent(FILE *f, size_t indent) {
    for (size_t i = 0; i < indent; ++i) {
        fprintf(f, "  ");
    }
}

void packl_generate_label(FILE *f, size_t label_value) {
    fprintf(f, "#label_%zu:\n", label_value);
}

void packl_generate_push(FILE *f, PACKL *self, int64_t value) {
    fprintf(f, "push %ld\n", value);
    self->stack_size++;    
}

void packl_generate_pushs(FILE *f, PACKL *self, String_View value) {
    fprintf(f, "pushs \"" SV_FMT "\"\n", SV_UNWRAP(value));
    self->stack_size++;    
}

void packl_generate_nop(FILE *f, PACKL *self) {
    fprintf(f, "nop\n");
}

void packl_generate_halt(FILE *f, PACKL *self) {
    fprintf(f, "halt\n");    
}

void packl_generate_pop(FILE *f, PACKL *self) {
    fprintf(f, "pop\n");
    self->stack_size--;
}

void packl_generate_add(FILE *f, PACKL *self) {
    fprintf(f, "add\n");
    self->stack_size -= 1;
}

void packl_generate_sub(FILE *f, PACKL *self) {
    fprintf(f, "sub\n");
    self->stack_size -= 1;
}

void packl_generate_mul(FILE *f, PACKL *self) {
    fprintf(f, "mul\n");
    self->stack_size -= 1;
}

void packl_generate_div(FILE *f, PACKL *self) {
    fprintf(f, "div\n");
    self->stack_size -= 1;
}

void packl_generate_mod(FILE *f, PACKL *self) {
    fprintf(f, "mod\n");
    self->stack_size -= 1;
}

void packl_generate_swap(FILE *f, PACKL *self) {
    fprintf(f, "swap\n");
}

void packl_generate_dup(FILE *f, PACKL *self) {
    fprintf(f, "dup\n");
    self->stack_size++;
}

void packl_generate_inswap(FILE *f, PACKL *self, int64_t value) {
    fprintf(f, "inswap %ld\n", value);
}

void packl_generate_indup(FILE *f, PACKL *self, int64_t value) {
    fprintf(f, "indup %ld\n", value);
    self->stack_size++;
} 

void packl_generate_syscall(FILE *f, PACKL *self, int64_t value) {
    fprintf(f, "syscall %ld\n", value);
    self->stack_size -= number_of_popped_values[value];
}

void packl_generate_jmp(FILE *f, PACKL *self, size_t value) {
    fprintf(f, "jmp $label_%zu\n", value);
}

void packl_generate_cmp(FILE *f, PACKL *self) {
    fprintf(f, "cmp\n");
    self->stack_size -= 1;
}

void packl_generate_jz(FILE *f, PACKL *self, size_t value) {
    fprintf(f, "jz $label_%zu\n", value);
    self->stack_size--;
}


void packl_generate_jle(FILE *f, PACKL *self, size_t value) {
    fprintf(f, "jle $label_%zu\n", value);
    self->stack_size--;
}


void packl_generate_jl(FILE *f, PACKL *self, size_t value) {
    fprintf(f, "jl $label_%zu\n", value);
    self->stack_size--;
}


void packl_generate_jge(FILE *f, PACKL *self, size_t value) {
    fprintf(f, "jge $label_%zu\n", value);
    self->stack_size--;
}


void packl_generate_jg(FILE *f, PACKL *self, size_t value) {
    fprintf(f, "jg $label_%zu\n", value);
    self->stack_size--;
}

void packl_generate_putc(FILE *f, PACKL *self) {
    fprintf(f, "putc\n");
    self->stack_size--; // TODO: see if you can change this line, i think it's not correct
}

void packl_generate_call(FILE *f, PACKL *self, size_t value) {
    fprintf(f, "call $label_%zu\n", value);
}

void packl_generate_ret(FILE *f, PACKL *self) {
    fprintf(f, "ret\n");
}

void packl_generate_smem(FILE *f, PACKL *self) {
    fprintf(f, "smem\n");
    self->stack_size -= 3;
}

void packl_generate_gmem(FILE *f, PACKL *self) {
    fprintf(f, "gmem\n");
    self->stack_size -= 2;
}

void packl_generate_readc(FILE *f, PACKL *self) {
    fprintf(f, "readc\n");
    self->stack_size++;
}

void packl_generate_getc(FILE *f, PACKL *self) {
    fprintf(f, "getc\n");
}

void packl_generate_setc(FILE *f, PACKL *self) {
    fprintf(f, "setc\n");
    self->stack_size--;
}

void packl_generate_body_code(FILE *f, PACKL *self, AST body, size_t indent) {
    // push a new context for the if body
    packl_push_new_context(self);
    
    // generate the code in that context 
    packl_generate_statements_code(f, self, body, indent);

    // pop the context added 
    packl_pop_context(self);
}

void packl_generate_operation(FILE *f, PACKL *self, Operator op, size_t indent) {
    fprint_indent(f, indent);
    switch(op) {
        case OP_PLUS:
            packl_generate_add(f, self);            
            break;
        case OP_MINUS:
            packl_generate_sub(f, self);            
            break;
        case OP_MUL:
            packl_generate_mul(f, self);            
            break;
        case OP_DIV:
            packl_generate_div(f, self);            
            break;
        case OP_MOD:
            packl_generate_mod(f, self);            
            break;
        default:
            ASSERT(false, "unreachable");
    }
}

void packl_generate_variable_push_code(FILE *f, PACKL *self, String_View name, size_t indent) {
    Context_Item *item = packl_get_context_item_in_all_contexts(self, name);
    if (!item) { PACKL_ERROR(self->filename, "variable `" SV_FMT "` not defined yet", SV_UNWRAP(name)); }
    
    if (item->type == CONTEXT_ITEM_TYPE_PROCEDURE) {
        PACKL_ERROR(self->filename, "expected `" SV_FMT "` to be a variable but found as a procedure", SV_UNWRAP(name));
    }

    fprint_indent(f, indent);
    packl_generate_indup(f, self, self->stack_size - item->as.variable.stack_pos - 1);
    
    if (sv_eq(item->as.variable.type, SV("str"))) {
        fprint_indent(f, indent);
        packl_generate_indup(f, self, self->stack_size - item->as.variable.stack_pos - 2);
    }
}

void packl_generate_expr_push_int_code(FILE *f, PACKL *self, int64_t value, size_t indent) {
    fprint_indent(f, indent);
    packl_generate_push(f, self, value);    
}

void packl_generate_expr_push_string_code(FILE *f, PACKL *self,  String_View value, size_t indent) {
    fprint_indent(f, indent);
    packl_generate_pushs(f, self, value);
    packl_generate_expr_push_int_code(f, self, (int64_t)value.count, indent);
}

void packl_generate_expr_bin_op_code(FILE *f, PACKL *self, Expr_Bin_Op bin, size_t indent) {
    packl_generate_expr_code(f, self, *bin.lhs, indent);
    packl_generate_expr_code(f, self, *bin.rhs, indent);
    packl_generate_operation(f, self, bin.op, indent);
}

void packl_generate_expr_code(FILE *f, PACKL *self, Expression expr, size_t indent) {
    switch(expr.kind) {
        case EXPR_KIND_INTEGER:
            packl_generate_expr_push_int_code(f, self, integer_from_sv(expr.as.value), indent);
            break;
        case EXPR_KIND_STRING:
            packl_generate_expr_push_string_code(f, self, expr.as.value, indent);
            break;
        case EXPR_KIND_ID:
            packl_generate_variable_push_code(f, self, expr.as.value, indent);
            break;
        case EXPR_KIND_BIN_OP:
            packl_generate_expr_bin_op_code(f, self, expr.as.bin, indent);
            break;
        default:
            TODO("not implemented yet, consider handling more primary types");
    }
}

void packl_generate_write(FILE *f, PACKL *self, Expression expr, size_t indent) {
    // push the stdout for the moment
    fprint_indent(f, indent);
    packl_generate_push(f, self, 0);

    packl_generate_expr_code(f, self, expr, indent);
    
    fprint_indent(f, indent);
    packl_generate_syscall(f, self, 0);
}


void packl_generate_exit(FILE *f, PACKL *self, Expression expr, size_t indent) {
    packl_generate_expr_code(f, self, expr, indent);

    fprint_indent(f, indent);
    packl_generate_syscall(f, self, 6);
}

void packl_generate_push_arg_code(FILE *f, PACKL *self, Func_Call_Arg arg, size_t indent) {
    (void)f;
    (void)self;
    (void)arg;
    (void)indent;
    TODO("add arguments push to the stack");
}

void packl_generate_push_args_code(FILE *f, PACKL *self, Func_Call_Args args, size_t indent) {
    for (size_t i = 0; i < args.count; ++i) {
        packl_generate_push_arg_code(f, self, args.items[i], indent);
    }
}

void packl_generate_func_call_code(FILE *f, PACKL *self, Func_Call func_call, size_t indent) {
    Context_Item *item = packl_get_context_item_in_all_contexts(self, func_call.name);
    if (!item) { PACKL_ERROR(self->filename, "function `" SV_FMT "` called but not decalred", SV_UNWRAP(func_call.name)); }
    
    packl_generate_push_args_code(f, self, func_call.args, indent);

    fprint_indent(f, indent);
    packl_generate_call(f, self, item->as.proc.label_value);
}

void packl_generate_native_call_code(FILE *f, PACKL *self, Func_Call func_call, size_t indent) {
    if (sv_eq(func_call.name, SV("write"))) {
        packl_generate_write(f, self, func_call.args.items[0].expr, indent);
    } else if (sv_eq(func_call.name, SV("exit"))) {
        packl_generate_exit(f, self, func_call.args.items[0].expr, indent);
    } else { ASSERT(false, "unreachable"); }
}

void packl_generate_proc_def_code(FILE *f, PACKL *self, Proc_Def proc_def, size_t indent) {
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
    
    fprint_indent(f, indent);
    packl_generate_label(f, self->label_value++);

    packl_generate_body_code(f, self, *proc_def.body, indent + 1);

    fprint_indent(f, indent + 1);
    packl_generate_ret(f, self);
} 

void packl_generate_var_dec_code(FILE *f, PACKL *self, Var_Declaration var_dec, size_t indent) {
    // check if the variable is already defined in the current context
    Context_Item *found = packl_get_context_item_in_current_context(self, var_dec.name);
    if (found) { PACKL_ERROR(self->filename, "variable " SV_FMT " already declared in the current scope as %s", SV_UNWRAP(var_dec.name), packl_get_context_item_type_as_cstr(found->type)); }

    Variable variable = {0};
    variable.type = var_dec.type;
    variable.stack_pos = self->stack_size;

    Context_Item item = {0};
    item.name = var_dec.name;
    item.type = CONTEXT_ITEM_TYPE_VARIABLE;
    item.as.variable = variable;

    packl_push_item_in_current_context(self, item);

    // support for only integer variables
    packl_generate_expr_code(f, self, var_dec.value, indent);
}

// void packl_generate_else_code(FILE *f, PACKL *self, AST ast, size_t indent) {
//     // generate label for the else block
//     packl_generate_label(f, self->label_value);
//     packl_generate_body_code(f, self, ast, indent + 1);
// }

void packl_generate_if_code(FILE *f, PACKL *self, If_Statement fi, size_t indent) {
    // evaluate the condition
    packl_generate_expr_code(f, self, fi.condition, indent);

    size_t label = self->label_value;
    
    // generate a jump if the the top of the stack is 0 to the else block or quit the current block
    fprint_indent(f, indent);
    packl_generate_jz(f, self, label);

    self->label_value += 2;      // we already reserved two labels 

    // generate the body code 
    packl_generate_body_code(f, self, *fi.body, indent);

    // generate an unconditional jump to then end of the if statement
    fprint_indent(f, indent);
    packl_generate_jmp(f, self, label + 1);

    // check for the else block
    if (fi.esle) {
        // generate the else label
        packl_generate_label(f, label);

        // generate the body code of the else
        packl_generate_body_code(f, self, *fi.esle, indent);
    } else {
        // generate a label to quit the current block
        packl_generate_label(f, label);
    }

    // generate the end of the if statement label
    packl_generate_label(f, label + 1);
}

void packl_generate_while_code(FILE *f, PACKL *self, While_Statement hwile, size_t indent) {
    size_t label = self->label_value;
    self->label_value += 3;                // we need three labels for the while code generation

    // make a label for the loop
    packl_generate_label(f, label);

    // evaluate the expression
    packl_generate_expr_code(f, self, hwile.condition, indent);

    // handle the cases
    fprint_indent(f, indent);
    packl_generate_jz(f, self, label + 1);  // if the condition is false
    
    fprint_indent(f, indent);
    packl_generate_jmp(f, self, label + 2); // if the condition is true

    // generate the labels 
    
    // for the while body
    packl_generate_label(f, label + 2); 
    packl_generate_body_code(f, self, *hwile.body, indent);

    // make a jump to initial while label
    fprint_indent(f, indent);
    packl_generate_jmp(f, self, label);  

    // to exit from the while loop
    packl_generate_label(f, label + 1);
}

void packl_generate_var_reassign_code(FILE *f, PACKL *self, Var_Reassign var, size_t indent) {
    Context_Item *item = packl_get_context_item_in_all_contexts(self, var.name);
    if (!item) { PACKL_ERROR(self->filename, "variable `" SV_FMT "` not defined yet", SV_UNWRAP(var.name)); }
    
    if (item->type == CONTEXT_ITEM_TYPE_PROCEDURE) {
        PACKL_ERROR(self->filename, "expected `" SV_FMT "` to be a variable but found as a procedure", SV_UNWRAP(var.name));
    }

    packl_generate_expr_code(f, self, var.expr, indent);

    fprint_indent(f, indent);
    packl_generate_inswap(f, self, self->stack_size - item->as.variable.stack_pos - 1);

    fprint_indent(f, indent);
    packl_generate_pop(f, self);
}


void packl_generate_statement_code(FILE *f, PACKL *self, Node node, size_t indent) {
    switch(node.kind) {
        case NODE_KIND_NATIVE_CALL:
            packl_generate_native_call_code(f, self, node.as.func_call, indent);
            break;
        case NODE_KIND_FUNC_CALL:
            packl_generate_func_call_code(f, self, node.as.func_call, indent);
            break;
        case NODE_KIND_PROC_DEF:
            packl_generate_proc_def_code(f, self, node.as.proc_def, indent);
            break;
        case NODE_KIND_VAR_DECLARATION:
            packl_generate_var_dec_code(f, self, node.as.var_dec, indent);
            break;
        case NODE_KIND_IF:
            packl_generate_if_code(f, self, node.as.fi, indent);
            break;
        case NODE_KIND_WHILE: 
            packl_generate_while_code(f, self, node.as.hwile, indent);
            break;
        case NODE_KIND_VAR_REASSIGN:
            packl_generate_var_reassign_code(f, self, node.as.var, indent);
            break;
        default:
            ASSERT(false, "unreachable");
    }
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