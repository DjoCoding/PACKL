#include "packl-codegen.h"

void packl_generate_expr_code(FILE *f, PACKL *self, Expression expr, size_t indent);
void packl_generate_func_call_node(FILE *f, PACKL *self, Node caller, Function func, size_t indent);
void packl_generate_native_call_node(FILE *f, PACKL *self, Node node, size_t indent);
void packl_generate_statements(FILE *f, PACKL *self, AST nodes, size_t indent);

size_t number_of_popped_values[] = {
    3,       // write  
    1,       // read: 2 for reading and 1 for push
    0,       // alloc: 1 for the size and 1 for the push
    1,       // free
    1,       // open: 2 for the filepath and the mode and one of the file pointer (push) 
    1,       // close 
    1,       // exit
};


void fprintfln(FILE *f) {
    fprintf(f, "\n");
}

void fprint_indent(FILE *f, size_t indent) {
    for (size_t i = 0; i < indent; ++i) {
        fprintf(f, "  ");
    }
}

#define PACKL_COMMENT(f, indent, ...) { fprint_indent(f, indent); fprintf(f, "; " __VA_ARGS__); fprintf(f, "\n"); }

void packl_generate_label(FILE *f, size_t label_value, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "#label_%zu:\n", label_value);
}

void packl_generate_push(FILE *f, PACKL *self, int64_t value, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "push %ld\n", value);
    self->stack_size++;    
}

void packl_generate_pushs(FILE *f, PACKL *self, String_View value, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "pushs \"" SV_FMT "\"\n", SV_UNWRAP(value));
    self->stack_size++;    
}

void packl_generate_nop(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "nop\n");
}

void packl_generate_halt(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "halt\n");    
}

void packl_generate_pop(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "pop\n");
    self->stack_size--;
}

void packl_generate_add(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "add\n");
    self->stack_size -= 1;
}

void packl_generate_sub(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "sub\n");
    self->stack_size -= 1;
}

void packl_generate_mul(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "mul\n");
    self->stack_size -= 1;
}

void packl_generate_div(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "div\n");
    self->stack_size -= 1;
}

void packl_generate_cmpl(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "cmpl\n");
    self->stack_size -= 1;
}

void packl_generate_cmpg(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "cmpg\n");
    self->stack_size -= 1;
}

void packl_generate_mod(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "mod\n");
    self->stack_size -= 1;
}

void packl_generate_swap(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "swap\n");
}

void packl_generate_dup(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "dup\n");
    self->stack_size++;
}

void packl_generate_inswap(FILE *f, PACKL *self, int64_t value, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "inswap %ld\n", value);
}

void packl_generate_indup(FILE *f, PACKL *self, int64_t value, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "indup %ld\n", value);
    self->stack_size++;
} 

void packl_generate_syscall(FILE *f, PACKL *self, int64_t value, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "syscall %ld\n", value);
    self->stack_size -= number_of_popped_values[value];
}

void packl_generate_jmp(FILE *f, PACKL *self, size_t value, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "jmp $label_%zu\n", value);
}

void packl_generate_cmp(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "cmp\n");
    self->stack_size -= 1;
}

void packl_generate_jz(FILE *f, PACKL *self, size_t value, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "jz $label_%zu\n", value);
    self->stack_size--;
}


void packl_generate_jle(FILE *f, PACKL *self, size_t value, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "jle $label_%zu\n", value);
    self->stack_size--;
}


void packl_generate_jl(FILE *f, PACKL *self, size_t value, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "jl $label_%zu\n", value);
    self->stack_size--;
}


void packl_generate_jge(FILE *f, PACKL *self, size_t value, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "jge $label_%zu\n", value);
    self->stack_size--;
}


void packl_generate_jg(FILE *f, PACKL *self, size_t value, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "jg $label_%zu\n", value);
    self->stack_size--;
}

void packl_generate_putc(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "putc\n");
    self->stack_size--; // TODO: see if you can change this line, i think it's not correct
}

void packl_generate_call(FILE *f, PACKL *self, size_t value, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "call $label_%zu\n", value);
}

void packl_generate_ret(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "ret\n");
}

void packl_generate_smem(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "smem\n");
    self->stack_size -= 3;
}

void packl_generate_gmem(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "gmem\n");
    self->stack_size -= 2;
}

void packl_generate_readc(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "readc\n");
    self->stack_size++;
}

void packl_generate_loadb(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "loadb\n");
}

void packl_generate_strb(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "strb\n");
    self->stack_size -= 2;
}

void packl_generate_ssp(FILE *f, PACKL *self, size_t indent) {
    fprint_indent(f, indent);
    fprintf(f, "ssp\n");
    self->stack_size -= 1;
}

void packl_setup_proc_param(PACKL *self, Parameter param, size_t pos) {
    Context_Item new_var = packl_init_var_context_item(param.name, param.type, pos);
    packl_push_item_in_current_context(self, new_var);
}

void packl_setup_proc_params(PACKL *self, Parameters params) {
    for (size_t i = 0; i < params.count; ++i) {
        Parameter param = params.items[params.count - i - 1];
        size_t pos = self->stack_size - i - 1;
        packl_setup_proc_param(self, param, pos);
    }
}

void packl_generate_integer_expr_code(FILE *f, PACKL *self, int64_t value, size_t indent) {
    packl_generate_push(f, self, value, indent);
}

void packl_generate_identifier_expr_code(FILE *f, PACKL *self, String_View name, size_t indent) {
    Variable var = packl_find_variable(self, name, (Location) {0, 0});
    size_t var_pos = self->stack_size - var.stack_pos - 1;

    packl_generate_indup(f, self, var_pos, indent);
}

void packl_generate_func_call_expr_code(FILE *f, PACKL *self, Func_Call func, size_t indent) {
    Context_Item *item = packl_find_function_or_procedure(self, func.name, (Location) { 0, 0 });
    if (item->type != CONTEXT_ITEM_TYPE_FUNCTION) { 
        PACKL_ERROR(self->filename, "procedure call returns void in an expression");
    }

    Function function = item->as.func;
    Node caller = { .as.func_call = func, .kind = NODE_KIND_FUNC_CALL };
    packl_generate_func_call_node(f, self, caller, function, indent);
}

void packl_generate_operation_code(FILE *f, PACKL *self, Operator op, size_t indent) {
    switch(op) {
        case OP_PLUS:
            return packl_generate_add(f, self, indent);
        case OP_MINUS:
            return packl_generate_sub(f, self, indent);
        case OP_MUL:
            return packl_generate_mul(f,self, indent);
        case OP_DIV:
            return packl_generate_div(f, self, indent);
        case OP_MOD:
            return packl_generate_mod(f, self, indent);
        case OP_LESS:
            return packl_generate_cmpl(f, self, indent);
        case OP_GREATER:
            return packl_generate_cmpg(f, self, indent);
        default:
            ASSERT(false, "unreachable");
    }
}

void packl_generate_binop_call_expr_code(FILE *f, PACKL *self, Expr_Bin_Op binop, size_t indent) {
    packl_generate_expr_code(f, self, *binop.lhs, indent);
    packl_generate_expr_code(f, self, *binop.rhs, indent);
    packl_generate_operation_code(f, self, binop.op, indent);
}

void packl_generate_string_expr_code(FILE *f, PACKL *self, String_View value, size_t indent) {
    packl_generate_pushs(f, self, value, indent);
} 

void packl_generate_native_call_code(FILE *f, PACKL *self, Func_Call native, size_t indent) {
    Node node = { .kind = NODE_KIND_NATIVE_CALL, .as.func_call = native, .loc = (Location) { 0, 0 } };
    packl_generate_native_call_node(f, self, node, indent);
}

void packl_generate_expr_code(FILE *f, PACKL *self, Expression expr, size_t indent) {
    switch(expr.kind) {
        case EXPR_KIND_INTEGER:
            return packl_generate_integer_expr_code(f, self, integer_from_sv(expr.as.value), indent);
        case EXPR_KIND_ID:
            return packl_generate_identifier_expr_code(f, self, expr.as.value, indent);
        case EXPR_KIND_FUNC_CALL:
            return packl_generate_func_call_expr_code(f, self, *expr.as.func, indent);
        case EXPR_KIND_BIN_OP:
            return packl_generate_binop_call_expr_code(f, self, expr.as.bin, indent);
        case EXPR_KIND_STRING:
            return packl_generate_string_expr_code(f, self, expr.as.value, indent);
        case EXPR_KIND_NATIVE_CALL:
            return packl_generate_native_call_code(f, self, *expr.as.func, indent);
        default:
            ASSERT(false, "unreachable");
    }
}

void packl_pop_proc_scope(FILE *f, PACKL *self, size_t stack_pos, size_t indent) {
    while(self->stack_size > stack_pos) {
        packl_generate_pop(f, self, indent);
    }
}

void packl_generate_proc_def_code(FILE *f, PACKL *self, Node proc_def_node, size_t indent) {
    Proc_Def proc_def = proc_def_node.as.proc_def;
    packl_find_item_and_report_error_if_found(self, proc_def.name, proc_def_node.loc);

    size_t label = self->label_value++;

    Context_Item new_proc = packl_init_proc_context_item(proc_def.name, proc_def.params, label);
    packl_push_item_in_current_context(self, new_proc);
    
    packl_push_new_context(self);

    size_t stack_size = self->stack_size;
    self->stack_size = proc_def.params.count;

    packl_setup_proc_params(self, proc_def.params);

    packl_generate_label(f, label, indent);

    packl_generate_statements(f, self, *proc_def.body, indent + 1);

    packl_pop_proc_scope(f, self, proc_def.params.count, indent + 1);
    self->stack_size = stack_size;

    packl_pop_context(self);
    packl_generate_ret(f, self, indent + 1);
}

void packl_generate_var_dec_node(FILE *f, PACKL *self, Node var_dec_node, size_t indent) {
    Var_Declaration var_dec = var_dec_node.as.var_dec;
    packl_find_item_in_current_context_and_report_error_if_found(self, var_dec.name, var_dec_node.loc);

    Context_Item new_var = packl_init_var_context_item(var_dec.name, var_dec.type, self->stack_size);
    packl_push_item_in_current_context(self, new_var);
    
    packl_generate_expr_code(f, self, var_dec.value, indent);
}

void packl_generate_var_reassign_node(FILE *f, PACKL *self, Node var_reassign_node, size_t indent) {
    Var_Reassign var_reassign = var_reassign_node.as.var;
    Variable var = packl_find_variable(self, var_reassign.name, var_reassign_node.loc);

    packl_generate_expr_code(f, self, var_reassign.expr, indent);
    
    size_t var_pos = self->stack_size - var.stack_pos - 1;

    packl_generate_inswap(f, self, var_pos, indent);

    packl_generate_pop(f, self, indent);
}

void packl_check_caller_arity(FILE *f, PACKL *self, Node caller, size_t params_count) {
    if (caller.as.func_call.args.count < params_count) {
        PACKL_ERROR_LOC(self->filename, caller.loc, "too few arguemnts for `" SV_FMT "` call, expected %zu got %zu", SV_UNWRAP(caller.as.func_call.name), params_count, caller.as.func_call.args.count);
    }

    if (caller.as.func_call.args.count > params_count) {
        PACKL_ERROR_LOC(self->filename, caller.loc, "too many arguemnts for `" SV_FMT "` call, expected %zu got %zu", SV_UNWRAP(caller.as.func_call.name), params_count, caller.as.func_call.args.count);
    }

    // Add type checking for this 
}

void packl_push_params(FILE *f, PACKL *self, Func_Call_Args args, size_t indent) {
    for(size_t i = 0; i < args.count; ++i) {
        Func_Call_Arg arg = args.items[i];
        packl_generate_expr_code(f, self, arg.expr, indent);
    }
}


void packl_pop_arguments(FILE *f, PACKL *self, size_t params_number, size_t indent) {
    // pop the arguments pushed to the stack
    while(params_number != 0) {
        packl_generate_pop(f, self, indent);
        params_number--;
    }
}

void packl_generate_proc_call_node(FILE *f, PACKL *self, Node caller, Procedure proc, size_t indent) {
    packl_check_caller_arity(f, self, caller, proc.params.count);

    packl_push_params(f, self, caller.as.func_call.args, indent);

    packl_generate_call(f, self, proc.label_value, indent);

    packl_pop_arguments(f, self, proc.params.count, indent);
}

void packl_generate_func_call_node(FILE *f, PACKL *self, Node caller, Function func, size_t indent) {
    packl_check_caller_arity(f, self, caller, func.params.count);

    // this is for the return value
    PACKL_COMMENT(f, indent, "this is for the return value");
    packl_generate_push(f, self, 0, indent);

    packl_push_params(f, self, caller.as.func_call.args, indent);

    packl_generate_call(f, self, func.label_value, indent);
    
    packl_pop_arguments(f, self, func.params.count, indent);
}

// the call node can be a function or procedure call
void packl_generate_call_node(FILE *f, PACKL *self, Node call_node, size_t indent) {
    Func_Call func_call = call_node.as.func_call;
    Context_Item *context_item = packl_find_function_or_procedure(self, func_call.name, call_node.loc);
    
    if (context_item->type == CONTEXT_ITEM_TYPE_FUNCTION) {
        return packl_generate_func_call_node(f, self, call_node, context_item->as.func, indent);
    }

    if (context_item->type == CONTEXT_ITEM_TYPE_PROCEDURE) {
        return packl_generate_proc_call_node(f, self, call_node, context_item->as.proc, indent);
    }

    ASSERT(false, "unreachable");
}

void packl_pop_func_scope(FILE *f, PACKL *self, size_t stack_pos, size_t indent) {
    while(self->stack_size != stack_pos) {
        packl_generate_pop(f, self, indent);
    }
}

void packl_setup_func_params(PACKL *self, Func_Def func, Parameters params) {
    // set the function parameters
    packl_setup_proc_params(self, params);

    // set the function return value 
    size_t pos = self->stack_size - params.count - 1;
    Context_Item func_ret_var = packl_init_var_context_item(func.name, func.return_type, pos);
    packl_push_item_in_current_context(self, func_ret_var);
}

void packl_generate_func_def_code(FILE *f, PACKL *self, Node func_def_node, size_t indent) {
    Func_Def func_def = func_def_node.as.func_def;
    packl_find_item_and_report_error_if_found(self, func_def.name, func_def_node.loc);

    size_t label = self->label_value++;

    Context_Item new_func = packl_init_func_context_item(func_def.name, func_def.return_type, func_def.params, label);
    packl_push_item_in_current_context(self, new_func);
    
    size_t stack_size = self->stack_size;
    self->stack_size = func_def.params.count + 1;          // + 1 for the return value

    packl_push_new_context(self);

    // this will setup the function return value also
    packl_setup_func_params(self, func_def, func_def.params);

    packl_generate_label(f, label, indent);

    packl_generate_statements(f, self, *func_def.body, indent + 1);

    packl_pop_proc_scope(f, self, func_def.params.count + 1, indent + 1);
    self->stack_size = stack_size;

    packl_pop_context(self);

    packl_generate_ret(f, self, indent + 1);
}

void packl_generate_native_write_code(FILE *f, PACKL *self, Node caller, size_t indent) {
    Func_Call native_write = caller.as.func_call;

    packl_check_caller_arity(f, self, caller, 3);
    
    packl_push_params(f, self, native_write.args, indent);

    packl_generate_syscall(f, self, 0, indent);
}

void packl_generate_native_exit_code(FILE *f, PACKL *self, Node caller, size_t indent) {
    Func_Call native_exit = caller.as.func_call;

    packl_check_caller_arity(f, self, caller, 1);
    
    packl_push_params(f, self, native_exit.args, indent);

    packl_generate_syscall(f, self, 6, indent);
}

void packl_generate_native_gb_code(FILE *f, PACKL *self, Node caller, size_t indent) {
    Func_Call native_gb = caller.as.func_call;

    packl_check_caller_arity(f, self, caller, 2);
    
    packl_push_params(f, self, native_gb.args, indent);

    packl_generate_add(f, self, indent);
    packl_generate_loadb(f, self, indent);
}

void packl_generate_native_call_node(FILE *f, PACKL *self, Node node, size_t indent) {
    Func_Call native = node.as.func_call;

    if (sv_eq(native.name, SV("write"))) {
        return packl_generate_native_write_code(f, self, node, indent);
    }

    if (sv_eq(native.name, SV("exit"))) {
        return packl_generate_native_exit_code(f, self, node, indent);
    }

    if (sv_eq(native.name, SV("get_byte"))) {
        return packl_generate_native_gb_code(f, self, node, indent);
    }

    ASSERT(false, "unreachable");
}

void packl_generate_if_node(FILE *f, PACKL *self, Node if_node, size_t indent) {
    If_Statement fi = if_node.as.fi;

    size_t label = self->label_value;

    self->label_value += 2;             // we reserve two labels for this if statement
    
    size_t stack_size = self->stack_size;
    packl_generate_expr_code(f, self, fi.condition, indent);

    packl_push_new_context(self);

    packl_generate_jz(f, self, label, indent);         // for the else of quit part
    
    // if body
    packl_generate_statements(f, self, *fi.body, indent);
    
    packl_generate_jmp(f, self, label + 1, indent);    // for the quit part
    
    if (fi.esle) {
        packl_generate_label(f, label, indent);
        
        packl_generate_statements(f, self, *fi.esle, indent);
    }

    packl_generate_label(f, label + 1, indent);


    self->stack_size = stack_size;
    packl_pop_context(self);
}


void packl_generate_while_node(FILE *f, PACKL *self, Node while_node, size_t indent) {
    While_Statement hwile = while_node.as.hwile;
    
    size_t label = self->label_value;

    self->label_value += 2;             // we reserve two labels for this while statement
    
    size_t stack_size = self->stack_size;

    packl_generate_label(f, label, indent);

    packl_generate_expr_code(f, self, hwile.condition, indent);

    packl_push_new_context(self);

    packl_generate_jz(f, self, label + 1, indent);         // for the exit part
    
    // while body
    packl_generate_statements(f, self, *hwile.body, indent);
    
    packl_generate_jmp(f, self, label, indent);    

    packl_generate_label(f, label + 1, indent);

    self->stack_size = stack_size;

    packl_pop_context(self);
}

void packl_generate_statement(FILE *f, PACKL *self, Node node, size_t indent) {
    switch(node.kind) {
        case NODE_KIND_PROC_DEF:
            return packl_generate_proc_def_code(f, self, node, indent);
        case NODE_KIND_VAR_DECLARATION:
            return packl_generate_var_dec_node(f, self, node, indent);
        case NODE_KIND_VAR_REASSIGN:
            return packl_generate_var_reassign_node(f, self, node, indent);
        case NODE_KIND_FUNC_CALL:
            return packl_generate_call_node(f, self, node, indent);
        case NODE_KIND_FUNC_DEF:
            return packl_generate_func_def_code(f, self, node, indent);
        case NODE_KIND_NATIVE_CALL:
            return packl_generate_native_call_node(f, self, node, indent);
        case NODE_KIND_IF:
            return packl_generate_if_node(f, self, node, indent);
        case NODE_KIND_WHILE:   
            return packl_generate_while_node(f, self, node, indent);
        default:
            ASSERT(false, "unreachable");
    }
}


void packl_generate_statements(FILE *f, PACKL *self, AST nodes, size_t indent) {
    for (size_t i = 0; i < nodes.count; ++i) {
        Node node = nodes.items[i];
        packl_generate_statement(f, self, node, indent);
    }
}

void packl_generate_entry_point(FILE *f, PACKL *self, Context_Item item) {
    if (item.type == CONTEXT_ITEM_TYPE_PROCEDURE) {
        fprintf(f, "#entry: $label_%zu\n", item.as.proc.label_value);
        return;
    }

    if (item.type == CONTEXT_ITEM_TYPE_FUNCTION) {
        fprintf(f, "#entry: $label_%zu\n", item.as.func.label_value);
        return;   
    }

    ASSERT(false, "unreachable");
}

void packl_generate_code(PACKL *self) {
    FILE *f = fopen(self->output, "w");
    if(!f) {
        PACKL_ERROR(self->filename, "could not open the output file for writing");
    }

    packl_init_contexts(self);
    packl_push_new_context(self);

    packl_generate_statements(f, self, self->ast, 0);

    Context_Item *item = packl_find_function_or_procedure(self, SV("main"), (Location) { 0, 0 });

    packl_generate_entry_point(f, self, *item);

    packl_pop_context(self);
    packl_remove_contexts(self);


    fclose(f);
}

