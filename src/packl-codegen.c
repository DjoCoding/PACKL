#include "packl-codegen.h"

size_t data_type_size[COUNT_PACKL_TYPES] = {4, 8};

PACKL_File packl_init_file(char *filename, char *fullpath);
void packl_compile_file(PACKL_Compiler *c, PACKL_File *self);

void packl_generate_array_item_size(PACKL_Compiler *c, PACKL_File *self, PACKL_Type type, size_t indent);
PACKL_Type packl_generate_expr_code(PACKL_Compiler *c, PACKL_File *self, Expression expr, size_t indent);
void packl_generate_func_call_node(PACKL_Compiler *c, PACKL_File *self, Node caller, Function func, size_t indent);
void packl_generate_native_call_node(PACKL_Compiler *c, PACKL_File *self, Node node, size_t indent);
void packl_generate_statements(PACKL_Compiler *c, PACKL_File *self, AST nodes, size_t indent);

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

void packl_generate_label(PACKL_Compiler *c, size_t label_value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "#label_%zu:\n", label_value);
}

void packl_generate_push(PACKL_Compiler *c, int64_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "push %ld\n", value);
    c->stack_size++;    
}

void packl_generate_pushs(PACKL_Compiler *c, String_View value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "pushs \"" SV_FMT "\"\n", SV_UNWRAP(value));
    c->stack_size++;    
}

void packl_generate_nop(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "nop\n");
}

void packl_generate_halt(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "halt\n");    
}

void packl_generate_pop(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "pop\n");
    c->stack_size--;
}

void packl_generate_add(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "add\n");
    c->stack_size -= 1;
}

void packl_generate_sub(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "sub\n");
    c->stack_size -= 1;
}

void packl_generate_mul(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "mul\n");
    c->stack_size -= 1;
}

void packl_generate_div(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "div\n");
    c->stack_size -= 1;
}

void packl_generate_cmpl(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "cmpl\n");
    c->stack_size -= 1;
}

void packl_generate_cmpg(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "cmpg\n");
    c->stack_size -= 1;
}

void packl_generate_mod(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "mod\n");
    c->stack_size -= 1;
}

void packl_generate_swap(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "swap\n");
}

void packl_generate_dup(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "dup\n");
    c->stack_size++;
}

void packl_generate_inswap(PACKL_Compiler *c, int64_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "inswap %ld\n", value);
}

void packl_generate_indup(PACKL_Compiler *c, int64_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "indup %ld\n", value);
    c->stack_size++;
} 

void packl_generate_syscall(PACKL_Compiler *c, int64_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "syscall %ld\n", value);
    c->stack_size -= number_of_popped_values[value];
}

void packl_generate_jmp(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "jmp $label_%zu\n", value);
}

void packl_generate_cmp(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "cmp\n");
    c->stack_size -= 1;
}

void packl_generate_jz(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "jz $label_%zu\n", value);
    c->stack_size--;
}


void packl_generate_jle(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "jle $label_%zu\n", value);
    c->stack_size--;
}


void packl_generate_jl(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "jl $label_%zu\n", value);
    c->stack_size--;
}


void packl_generate_jge(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "jge $label_%zu\n", value);
    c->stack_size--;
}


void packl_generate_jg(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "jg $label_%zu\n", value);
    c->stack_size--;
}

void packl_generate_putc(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "putc\n");
    c->stack_size--; // TODO: see if you can change this line, i think it's not correct
}

void packl_generate_call(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "call $label_%zu\n", value);
}

void packl_generate_ret(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "ret\n");
}

void packl_generate_store(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "store\n");
    c->stack_size -= 3;
}

void packl_generate_load(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "load\n");
    c->stack_size -= 1;
}

void packl_generate_readc(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "readc\n");
    c->stack_size++;
}

void packl_generate_loadb(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "loadb\n");
}

void packl_generate_storeb(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "strb\n");
    c->stack_size -= 2;
}

void packl_generate_ssp(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "ssp\n");
    c->stack_size -= 1;
}

void packl_check_if_type_and_operator_fits_together(PACKL_File *self, PACKL_Type type, Operator op) {
    if(type.kind == PACKL_TYPE_ARRAY) {
        PACKL_ERROR(self->filename, "no arithmetic or logic operators used with arrays");
    }
}

int packl_check_type_equality(PACKL_Type type1, PACKL_Type type2) {
    if (type1.kind != type2.kind) { return 0; }
    if (type1.kind == PACKL_TYPE_BASIC) {
        if(type1.as.basic != type2.as.basic) { return 0; }
        return 1;
    }
    return packl_check_type_equality(*type1.as.array.type, *type2.as.array.type);
}

PACKL_Type packl_type_check(PACKL_File *self, PACKL_Type lhs_type, PACKL_Type rhs_type, Operator op) {
    packl_check_if_type_and_operator_fits_together(self, lhs_type, op);
    packl_check_if_type_and_operator_fits_together(self, rhs_type, op);
    // for now only integers are supported
    if (lhs_type.as.basic == PACKL_TYPE_INT) {
        return lhs_type;
    }
    ASSERT(false, "unreachable");
}

void packl_setup_proc_param(PACKL_File *self, Parameter param, size_t pos) {
    Context_Item new_var = packl_init_var_context_item(param.name, param.type, pos);
    packl_push_item_in_current_context(self, new_var);
}

void packl_setup_proc_params(PACKL_Compiler *c, PACKL_File *self, Parameters params) {
    for (size_t i = 0; i < params.count; ++i) {
        Parameter param = params.items[params.count - i - 1];
        size_t pos = c->stack_size - i - 1;
        packl_setup_proc_param(self, param, pos);
    }
}

void packl_print_expr_type(PACKL_Type type) {
    if(type.kind == PACKL_TYPE_BASIC) {
        switch(type.as.basic) {
            case PACKL_TYPE_INT: fprintf(stderr, "int\n"); break;
            case PACKL_TYPE_STR: fprintf(stderr, "str\n"); break;
            default:
                ASSERT(false, "unreachable");
        }
    } else {
        fprintf(stderr, "array of ");
        packl_print_expr_type(*type.as.array.type);
    }
} 



void packl_expect_type(PACKL_File *self, Location loc, PACKL_Type expected, PACKL_Type target) {
    if(packl_check_type_equality(expected, target)) {
        return;
    }
    PACKL_ERROR_LOC(self->filename, loc, "type mismatch");
}


PACKL_Type packl_generate_integer_expr_code(PACKL_Compiler *c, int64_t value, size_t indent) {
    packl_generate_push(c, value, indent);
    return (PACKL_Type) { .kind = PACKL_TYPE_BASIC, .as.basic = PACKL_TYPE_INT };
}

PACKL_Type packl_generate_identifier_expr_code(PACKL_Compiler *c, PACKL_File *self, String_View name, size_t indent) {
    Variable var = packl_find_variable(self, name, (Location) {0, 0});
    size_t var_pos = c->stack_size - var.stack_pos - 1;
    packl_generate_indup(c, var_pos, indent);
    return var.type;
}

PACKL_Type packl_generate_func_call_expr_code(PACKL_Compiler *c, PACKL_File *self, Func_Call func, size_t indent) {
    Context_Item *item = packl_find_function_or_procedure(self, func.name, (Location) { 0, 0 });
    if (item->type != CONTEXT_ITEM_TYPE_FUNCTION) { 
        PACKL_ERROR(self->filename, "procedure call returns void in an expression");
    }

    Function function = item->as.func;
    Node caller = { .as.func_call = func, .kind = NODE_KIND_FUNC_CALL };
    packl_generate_func_call_node(c, self, caller, function, indent);

    return function.return_type;
}

void packl_generate_operation_code(PACKL_Compiler *c, Operator op, size_t indent) {
    switch(op) {
        case OP_PLUS:
            return packl_generate_add(c, indent);
        case OP_MINUS:
            return packl_generate_sub(c, indent);
        case OP_MUL:
            return packl_generate_mul(c, indent);
        case OP_DIV:
            return packl_generate_div(c, indent);
        case OP_MOD:
            return packl_generate_mod(c, indent);
        case OP_LESS:
            return packl_generate_cmpl(c, indent);
        case OP_GREATER:
            return packl_generate_cmpg(c, indent);
        default:
            ASSERT(false, "unreachable");
    }
}

PACKL_Type packl_generate_binop_call_expr_code(PACKL_Compiler *c, PACKL_File *self, Expr_Bin_Op binop, size_t indent) {
    PACKL_Type lhs_type = packl_generate_expr_code(c, self, *binop.lhs, indent);
    PACKL_Type rhs_type = packl_generate_expr_code(c, self, *binop.rhs, indent);
    PACKL_Type return_type = packl_type_check(self, lhs_type, rhs_type, binop.op);
    packl_generate_operation_code(c, binop.op, indent);
    return return_type;
}

PACKL_Type packl_generate_string_expr_code(PACKL_Compiler *c, PACKL_File *self, String_View value, size_t indent) {
    packl_generate_pushs(c, value, indent);
    return (PACKL_Type) { .kind = PACKL_TYPE_BASIC, .as.basic = PACKL_TYPE_STR };
} 

PACKL_Type packl_generate_native_call_code(PACKL_Compiler *c, PACKL_File *self, Func_Call native, size_t indent) {
    Node node = { .kind = NODE_KIND_NATIVE_CALL, .as.func_call = native, .loc = (Location) { 0, 0 } };
    packl_generate_native_call_node(c, self, node, indent);
    // return packl_get_native_return_type(native.name);
    return (PACKL_Type) {0};
}

PACKL_Type packl_generate_mod_call_expr_code(PACKL_Compiler *c, PACKL_File *self, Mod_Call mod, size_t indent) {
    Location loc = {0, 0};

    Module module = packl_find_module(self, mod.name, loc);
    PACKL_File target = packl_find_used_file(self, module.filename);
    
    if (mod.kind == MODULE_CALL_FUNC_CALL) {
        Context_Item *item = packl_find_function_or_procedure(&target, mod.as.func_call.name, loc);
        if (item->type != CONTEXT_ITEM_TYPE_FUNCTION) { 
            PACKL_ERROR(self->filename, "procedure call returns void in an expression");
        }

        Function func = item->as.func;
        Node caller = { .as.func_call = mod.as.func_call, .kind = NODE_KIND_FUNC_CALL };

        packl_generate_func_call_node(c, self, caller, func, indent);
        return func.return_type;
    }

    if (mod.kind == MODULE_CALL_VARIABLE) {
        return packl_generate_identifier_expr_code(c, &target, mod.as.var_name, indent);
    }

    ASSERT(false, "unreachable");
}

PACKL_Type packl_generate_string_index_code(PACKL_Compiler *c, PACKL_File *self, Variable var, Expression index, size_t indent) {
    PACKL_Type int_type = { .kind = PACKL_TYPE_BASIC, .as.basic = PACKL_TYPE_INT };
    
    packl_generate_indup(c, c->stack_size - var.stack_pos - 1, indent);
    
    PACKL_Type index_type = packl_generate_expr_code(c, self, index, indent);
    packl_expect_type(self, index.loc, int_type, index_type);

    packl_generate_add(c, indent);
    packl_generate_loadb(c, indent);

    return int_type;
}

PACKL_Type packl_generate_array_indexing_code(PACKL_Compiler *c, PACKL_File *self, Expr_Arr_Index arr_index, size_t indent) {
    Variable var = packl_find_variable(self, arr_index.name, (Location){0,0});
    
    if (var.type.kind == PACKL_TYPE_BASIC && var.type.as.basic == PACKL_TYPE_STR) {
        return packl_generate_string_index_code(c, self, var, *arr_index.index, indent);
    }

    if (var.type.kind != PACKL_TYPE_ARRAY) {
        PACKL_ERROR(self->filename, SV_FMT " is not an array", SV_UNWRAP(arr_index.name));
    }

    PACKL_COMMENT(c->f, indent, "this is for the array indexing");

    // push the item size
    packl_generate_array_item_size(c, self, *var.type.as.array.type, indent);

    // push the array
    packl_generate_indup(c, c->stack_size - var.stack_pos - 1, indent);

    // push the index
    packl_generate_expr_code(c, self, *arr_index.index, indent);

    // indup the item size
    packl_generate_indup(c, 2, indent);

    // multiply them together
    packl_generate_mul(c, indent);

    // add 
    packl_generate_add(c, indent);

    packl_generate_swap(c, indent);

    packl_generate_load(c, indent);

    return *var.type.as.array.type;
}

PACKL_Type packl_generate_expr_code(PACKL_Compiler *c, PACKL_File *self, Expression expr, size_t indent) {
    switch(expr.kind) {
        case EXPR_KIND_INTEGER:
            return packl_generate_integer_expr_code(c, expr.as.integer, indent);
        case EXPR_KIND_ID:
            return packl_generate_identifier_expr_code(c, self, expr.as.value, indent);
        case EXPR_KIND_FUNC_CALL:
            return packl_generate_func_call_expr_code(c, self, *expr.as.func, indent);
        case EXPR_KIND_BIN_OP:
            return packl_generate_binop_call_expr_code(c, self, expr.as.bin, indent);
        case EXPR_KIND_STRING:
            return packl_generate_string_expr_code(c, self, expr.as.value, indent);
        case EXPR_KIND_NATIVE_CALL:
            return packl_generate_native_call_code(c, self, *expr.as.func, indent);
        case EXPR_KIND_MOD_CALL:
            return packl_generate_mod_call_expr_code(c, self, *expr.as.mod, indent);
        case EXPR_KIND_ARRAY_INDEXING:
            return packl_generate_array_indexing_code(c, self, expr.as.arr_index, indent);
        default:
            ASSERT(false, "unreachable");
    }
}

void packl_pop_proc_scope(PACKL_Compiler *c, size_t stack_pos, size_t indent) {
    while(c->stack_size > stack_pos) {
        packl_generate_pop(c, indent);
    }
}

void packl_generate_proc_def_code(PACKL_Compiler *c, PACKL_File *self, Node proc_def_node, size_t indent) {
    Proc_Def proc_def = proc_def_node.as.proc_def;
    packl_find_item_and_report_error_if_found(self, proc_def.name, proc_def_node.loc);

    size_t label = c->label_value++;

    Context_Item new_proc = packl_init_proc_context_item(proc_def.name, proc_def.params, label);
    packl_push_item_in_current_context(self, new_proc);
    
    packl_push_new_context(self);

    size_t stack_size = c->stack_size;
    c->stack_size = proc_def.params.count;

    packl_setup_proc_params(c, self, proc_def.params);

    packl_generate_label(c, label, indent);

    packl_generate_statements(c, self, *proc_def.body, indent + 1);

    packl_pop_proc_scope(c, proc_def.params.count, indent + 1);
    c->stack_size = stack_size;

    packl_pop_context(self);
    packl_generate_ret(c, indent + 1);
}

void packl_generate_array_item_size(PACKL_Compiler *c, PACKL_File *self, PACKL_Type type, size_t indent) {
    if(type.kind == PACKL_TYPE_BASIC) {
        packl_generate_push(c, data_type_size[type.as.basic], indent);
        return;
    }

    if (type.kind == PACKL_TYPE_ARRAY) {
        // push the single item size
        packl_generate_array_item_size(c, self, *type.as.array.type, indent);
        // push the size of the array
        packl_generate_expr_code(c, self, type.as.array.size, indent);
        // multiply them
        packl_generate_mul(c, indent);
        return;
    }

    ASSERT(false, "unreachable");
}

void packl_generate_array_allocation_code(PACKL_Compiler *c, PACKL_File *self, Array_Type arr_type, size_t indent) {
    packl_generate_array_item_size(c, self, *arr_type.type, indent);

    PACKL_Type size_type = packl_generate_expr_code(c, self, arr_type.size, indent);
    PACKL_Type expected_type = (PACKL_Type) { .kind = PACKL_TYPE_BASIC, .as.basic = PACKL_TYPE_INT };
    packl_expect_type(self, arr_type.size.loc, expected_type, size_type);

    packl_generate_mul(c, indent);
    packl_generate_syscall(c, 2, indent); // the alloc syscall
}

void packl_reassign_array_item_code(PACKL_Compiler *c, PACKL_File *self, PACKL_Type arr_type, Expression value, Expression index, size_t indent) {   
    // example: arr[2] = 1; arr is a array of integers
    // stack: arr

    // duplicate the array pointer
    packl_generate_dup(c, indent);
    // stack: arr arr

    // generate the push of the data type size
    packl_generate_array_item_size(c, self, *arr_type.as.array.type, indent);
    // stack: arr arr sizeof(int)

    packl_generate_swap(c, indent);
    // stack: arr sizeof(int) arr

    // duplicate the array item size because we need it 
    packl_generate_indup(c, 1, indent);
    // stack: arr sizeof(int) arr sizeof(int)

    // generate the index
    PACKL_Type index_type = packl_generate_expr_code(c, self, index, indent);
    PACKL_Type expected_type = (PACKL_Type) { .kind = PACKL_TYPE_BASIC, .as.basic = PACKL_TYPE_INT };
    packl_expect_type(self, index.loc, expected_type, index_type);
    // stack: arr sizeof(int) arr sizeof(int) 2

    // multiply the index with the item_size 
    packl_generate_mul(c, indent);
    // stack: arr sizeof(int) arr (2*sizeof(int))

    // add the offset to the array pointer 
    packl_generate_add(c, indent);
    // stack: arr sizeof(int) (arr + 2 * sizeof(int))

    // generate a swap
    packl_generate_swap(c, indent);
    // stack: arr (arr + 2 * sizeof(int)) sizeof(int)

    // push the data 
    PACKL_Type value_type = packl_generate_expr_code(c, self, value, indent);
    expected_type = *arr_type.as.array.type;
    packl_expect_type(self, value.loc, expected_type, value_type);
    // stack: arr (arr + 2 * sizeof(int)) sizeof(int) 1

    // generate another swap
    packl_generate_swap(c, indent);
    // stack: arr (arr + 2 * sizeof(int)) 1 sizeof(int)

    // ready for the storing
    packl_generate_store(c, indent);
    // stack: arr 
}

void packl_handle_array_var_dec(PACKL_Compiler *c, PACKL_File *self, Var_Declaration var_dec, size_t indent) {
    PACKL_Type arr_type = var_dec.type;
    // allocate the memory for the array
    packl_generate_array_allocation_code(c, self, arr_type.as.array, indent);

    // write values to that memory
    Expr_Arr arr = var_dec.value.as.arr;
    for(size_t i = 0; i < arr.count; ++i) {
        // index for the push 
        Expression index = { .kind = EXPR_KIND_INTEGER, .as.integer = (int64_t)i };
        packl_reassign_array_item_code(c, self, arr_type, arr.items[i], index, indent);
    }
}

void packl_generate_var_dec_node(PACKL_Compiler *c, PACKL_File *self, Node var_dec_node, size_t indent) {
    Var_Declaration var_dec = var_dec_node.as.var_dec;
    packl_find_item_in_current_context_and_report_error_if_found(self, var_dec.name, var_dec_node.loc);

    Context_Item new_var = packl_init_var_context_item(var_dec.name, var_dec.type, c->stack_size);
    packl_push_item_in_current_context(self, new_var);
    
    if (var_dec.type.kind == PACKL_TYPE_BASIC) {
        PACKL_Type expr_type = packl_generate_expr_code(c, self, var_dec.value, indent);
        if (!packl_check_type_equality(expr_type, var_dec.type)) {
            PACKL_ERROR_LOC(self->filename, var_dec_node.loc, "type mismatch");
        }
        return;
    }

    if (var_dec.type.kind == PACKL_TYPE_ARRAY) {
        return packl_handle_array_var_dec(c, self, var_dec, indent);
    }


    ASSERT(false, "unreachable");
}

void packl_reassign_str_char_code(PACKL_Compiler *c, PACKL_File *self, Expression value, Expression index, size_t indent) {
    PACKL_Type expected_type = { .kind = PACKL_TYPE_BASIC, .as.basic = PACKL_TYPE_INT };    
    
    packl_generate_dup(c, indent);

    PACKL_Type index_type = packl_generate_expr_code(c, self, index, indent);
    packl_expect_type(self, index.loc, expected_type, index_type);

    packl_generate_add(c, indent);

    PACKL_Type value_type = packl_generate_expr_code(c, self, value, indent);
    packl_expect_type(self, value.loc, expected_type, value_type);

    packl_generate_storeb(c, indent);
}


void packl_generate_var_reassign_node(PACKL_Compiler *c, PACKL_File *self, Node var_reassign_node, size_t indent) {
    Var_Reassign var_reassign = var_reassign_node.as.var;
    Variable var = packl_find_variable(self, var_reassign.name, var_reassign_node.loc);


    if (var_reassign.kind == PACKL_TYPE_ARRAY) {
        packl_generate_indup(c, c->stack_size - var.stack_pos - 1, indent);

        if (var.type.kind == PACKL_TYPE_BASIC && var.type.as.basic == PACKL_TYPE_STR) {
            packl_reassign_str_char_code(c, self, var_reassign.expr, var_reassign.index, indent);
        } else {
            packl_reassign_array_item_code(c, self, var.type, var_reassign.expr, var_reassign.index, indent);
        }

        return packl_generate_pop(c, indent);
    }


    PACKL_Type expr_type = packl_generate_expr_code(c, self, var_reassign.expr, indent);
    PACKL_Type expected_type = var.type;
    packl_expect_type(self, var_reassign.expr.loc, expected_type, expr_type);

    size_t var_pos = c->stack_size - var.stack_pos - 1;

    packl_generate_inswap(c, var_pos, indent);

    packl_generate_pop(c, indent);
}

void packl_check_caller_arity(PACKL_File *self, Node caller, size_t params_count) {
    if (caller.as.func_call.args.count < params_count) {
        PACKL_ERROR_LOC(self->filename, caller.loc, "too few arguemnts for `" SV_FMT "` call, expected %zu got %zu", SV_UNWRAP(caller.as.func_call.name), params_count, caller.as.func_call.args.count);
    }

    if (caller.as.func_call.args.count > params_count) {
        PACKL_ERROR_LOC(self->filename, caller.loc, "too many arguemnts for `" SV_FMT "` call, expected %zu got %zu", SV_UNWRAP(caller.as.func_call.name), params_count, caller.as.func_call.args.count);
    }

    // Add type checking for this 
}

void packl_push_arguments(PACKL_Compiler *c, PACKL_File *self, PACKL_Args args, size_t indent) {
    for(size_t i = 0; i < args.count; ++i) {
        PACKL_Arg arg = args.items[i];
        packl_generate_expr_code(c, self, arg.expr, indent);
    }
}


void packl_pop_arguments(PACKL_Compiler *c, size_t params_number, size_t indent) {
    // pop the arguments pushed to the stack
    while(params_number != 0) {
        packl_generate_pop(c, indent);
        params_number--;
    }
}

void packl_generate_proc_call_node(PACKL_Compiler *c, PACKL_File *self, Node caller, Procedure proc, size_t indent) {
    packl_check_caller_arity(self, caller, proc.params.count);

    packl_push_arguments(c, self, caller.as.func_call.args, indent);

    packl_generate_call(c, proc.label_value, indent);

    packl_pop_arguments(c, proc.params.count, indent);
}

void packl_generate_func_call_node(PACKL_Compiler *c, PACKL_File *self, Node caller, Function func, size_t indent) {
    packl_check_caller_arity(self, caller, func.params.count);

    // this is for the return value
    PACKL_COMMENT(c->f, indent, "this is for the return value");
    packl_generate_push(c, 0, indent);

    packl_push_arguments(c, self, caller.as.func_call.args, indent);

    packl_generate_call(c, func.label_value, indent);
    
    packl_pop_arguments(c, func.params.count, indent);
}

// the call node can be a function or procedure call
void packl_generate_call_node(PACKL_Compiler *c, PACKL_File *self, Node call_node, size_t indent) {
    Func_Call func_call = call_node.as.func_call;
    Context_Item *context_item = packl_find_function_or_procedure(self, func_call.name, call_node.loc);
    
    if (context_item->type == CONTEXT_ITEM_TYPE_FUNCTION) {
        return packl_generate_func_call_node(c, self, call_node, context_item->as.func, indent);
    }

    if (context_item->type == CONTEXT_ITEM_TYPE_PROCEDURE) {
        return packl_generate_proc_call_node(c, self, call_node, context_item->as.proc, indent);
    }

    ASSERT(false, "unreachable");
}

void packl_pop_func_scope(PACKL_Compiler *c, size_t stack_pos, size_t indent) {
    while(c->stack_size != stack_pos) {
        packl_generate_pop(c, indent);
    }
}

void packl_setup_func_params(PACKL_Compiler *c, PACKL_File *self, Func_Def func, Parameters params) {
    // set the function parameters
    packl_setup_proc_params(c, self, params);

    // set the function return value 
    size_t pos = c->stack_size - params.count - 1;
    Context_Item func_ret_var = packl_init_var_context_item(func.name, func.return_type, pos);
    packl_push_item_in_current_context(self, func_ret_var);
}

void packl_generate_func_def_code(PACKL_Compiler *c, PACKL_File *self, Node func_def_node, size_t indent) {
    Func_Def func_def = func_def_node.as.func_def;
    packl_find_item_and_report_error_if_found(self, func_def.name, func_def_node.loc);

    size_t label = c->label_value++;

    Context_Item new_func = packl_init_func_context_item(func_def.name, func_def.return_type, func_def.params, label);
    packl_push_item_in_current_context(self, new_func);
    
    size_t stack_size = c->stack_size;
    c->stack_size = func_def.params.count + 1;          // + 1 for the return value

    packl_push_new_context(self);

    // this will setup the function return value also
    packl_setup_func_params(c, self, func_def, func_def.params);

    packl_generate_label(c, label, indent);

    packl_generate_statements(c, self, *func_def.body, indent + 1);

    packl_pop_proc_scope(c, func_def.params.count + 1, indent + 1);
    c->stack_size = stack_size;

    packl_pop_context(self);

    packl_generate_ret(c, indent + 1);
}

void packl_generate_native_write_code(PACKL_Compiler *c, PACKL_File *self, Node caller, size_t indent) {
    Func_Call native_write = caller.as.func_call;

    packl_check_caller_arity(self, caller, 3);
    
    packl_push_arguments(c, self, native_write.args, indent);

    packl_generate_syscall(c, 0, indent);
}

void packl_generate_native_exit_code(PACKL_Compiler *c, PACKL_File *self, Node caller, size_t indent) {
    Func_Call native_exit = caller.as.func_call;

    packl_check_caller_arity(self, caller, 1);
    
    packl_push_arguments(c, self, native_exit.args, indent);

    packl_generate_syscall(c, 6, indent);
}

void packl_generate_native_call_node(PACKL_Compiler *c, PACKL_File *self, Node node, size_t indent) {
    Func_Call native = node.as.func_call;

    if (sv_eq(native.name, SV("write"))) {
        return packl_generate_native_write_code(c, self, node, indent);
    }

    if (sv_eq(native.name, SV("exit"))) {
        return packl_generate_native_exit_code(c, self, node, indent);
    }

    ASSERT(false, "unreachable");
}

void packl_generate_if_node(PACKL_Compiler *c, PACKL_File *self, Node if_node, size_t indent) {
    If_Statement fi = if_node.as.fi;

    size_t label = c->label_value;

    c->label_value += 2;             // we reserve two labels for this if statement
    
    size_t stack_size = c->stack_size;
    packl_generate_expr_code(c, self, fi.condition, indent);

    packl_push_new_context(self);

    packl_generate_jz(c, label, indent);         // for the else of quit part
    
    // if body
    packl_generate_statements(c, self, *fi.body, indent);
    
    packl_generate_jmp(c, label + 1, indent);    // for the quit part
    
    packl_generate_label(c, label, indent);
    if (fi.esle) {        
        packl_generate_statements(c, self, *fi.esle, indent);
    } 

    packl_generate_label(c, label + 1, indent);


    c->stack_size = stack_size;
    packl_pop_context(self);
}


void packl_generate_while_node(PACKL_Compiler *c, PACKL_File *self, Node while_node, size_t indent) {
    While_Statement hwile = while_node.as.hwile;
    
    size_t label = c->label_value;

    c->label_value += 2;             // we reserve two labels for this while statement
    
    size_t stack_size = c->stack_size;

    packl_generate_label(c, label, indent);

    packl_generate_expr_code(c, self, hwile.condition, indent);

    packl_push_new_context(self);

    packl_generate_jz(c, label + 1, indent);         // for the exit part
    
    // while body
    packl_generate_statements(c, self, *hwile.body, indent);
    
    packl_generate_jmp(c, label, indent);    

    packl_generate_label(c, label + 1, indent);

    c->stack_size = stack_size;

    packl_pop_context(self);
}


void packl_check_for_arguments(PACKL_File *self, For_Statement rof) {
    if (rof.args.count < 2) {
        PACKL_ERROR(self->filename, "too few arguments for the for loop, expected 2 got %zu", rof.args.count);
    }

    if (rof.args.count > 2) {
        PACKL_ERROR(self->filename, "too many arguments for the for loop, expected 2 got %zu", rof.args.count);
    }
}

void packl_push_for_iterator(PACKL_Compiler *c, PACKL_File *self, For_Statement rof, size_t indent) {
    Context_Item iter = packl_init_var_context_item(rof.iter, rof.iter_type, c->stack_size);
    packl_push_item_in_current_context(self, iter);
    packl_generate_expr_code(c, self, rof.args.items[0].expr, indent);
}

void packl_reassign_for_iter(PACKL_Compiler *c, PACKL_File *self, For_Statement rof, size_t indent) {
    Variable iter_var = packl_find_variable(self, rof.iter, (Location) {0, 0});
    packl_generate_indup(c, c->stack_size - iter_var.stack_pos - 1, indent);
    packl_generate_push(c, 1, indent);
    packl_generate_add(c, indent);
    packl_generate_inswap(c, c->stack_size - iter_var.stack_pos - 1, indent);
    packl_generate_pop(c, indent);
}

void packl_generate_for_node(PACKL_Compiler *c, PACKL_File *self, Node for_node, size_t indent) {
    For_Statement rof = for_node.as.rof;

    size_t label = c->label_value;
    c->label_value += 2;      
        
    size_t stack_size = c->stack_size;

    packl_check_for_arguments(self, rof);
    packl_push_new_context(self);
    packl_push_for_iterator(c, self, rof, indent);

    packl_generate_label(c, label, indent);

    packl_generate_dup(c, indent + 1);
    packl_generate_expr_code(c, self, rof.args.items[1].expr, indent + 1);
    packl_generate_cmp(c, indent + 1);
    packl_generate_jg(c, label + 1, indent + 1);         // for the exit part
    
    // for body
    packl_generate_statements(c, self, *rof.body, indent + 1);

    packl_reassign_for_iter(c, self, rof, indent + 1);
    
    packl_generate_jmp(c, label, indent + 1);    

    packl_generate_label(c, label + 1, indent);

    packl_pop_proc_scope(c, stack_size, indent);
    // c->stack_size = stack_size;

    packl_pop_context(self);
}

void packl_throw_file_used_twice_error(PACKL_Compiler *c, PACKL_File *self, char *fullpath) {
    for(size_t i = 0; i < self->used_files.count; ++i) {
        char *file_fullpath = self->used_files.items[i].fullpath;
        if (strcmp(file_fullpath, fullpath) == 0) {
            PACKL_ERROR(self->filename, "file `%s` used twice", fullpath);
        }
    }
}

void packl_throw_circular_dependency_error(PACKL_File *self, char *new_file) {
    for(size_t i = 0; i < self->root_files.count; ++i) {
        char *root_file = self->root_files.items[i];
        if(strcmp(new_file, root_file) == 0) {
            PACKL_ERROR(self->filename, "circular dependecy found between `%s` and `%s`", self->filename, new_file);
        }
    }
}

void packl_set_root_files(PACKL_File *new_file, Strings root_files, char *user_file) {
    for(size_t i = 0; i < root_files.count; ++i) {
        DA_APPEND(&new_file->root_files, root_files.items[i]);
    }
    DA_APPEND(&new_file->root_files, user_file);
}

void packl_generate_use_node(PACKL_Compiler *c, PACKL_File *self, Node use_node, size_t indent) {
    Use use = use_node.as.use;
    
    char *filename = cstr_from_sv(use.filename);    
    char *fullpath = malloc(sizeof(char) * FILENAME_MAX);
    
    if (!realpath(filename, fullpath)) {
        PACKL_ERROR_LOC(self->filename, use_node.loc, "failed to use the file %s", filename);
    }
    
    packl_throw_file_used_twice_error(c, self, fullpath);
    packl_throw_circular_dependency_error(self, fullpath);

    Context_Item new_module = packl_init_module_context_item(use.alias, filename);
    packl_push_item_in_current_context(self, new_module);

    PACKL_File file = packl_init_file(filename, fullpath);

    packl_set_root_files(&file, self->root_files, self->fullpath);

    packl_compile_file(c, &file);

    DA_APPEND(&self->used_files, file);
}

void packl_generate_mod_call_node(PACKL_Compiler *c, PACKL_File *self, Node call_node, size_t indent) {
    Location loc = {0, 0};
    Mod_Call mod = call_node.as.mod_call;
    Module module = packl_find_module(self, mod.name, loc);
    PACKL_File target = packl_find_used_file(self, module.filename);
    
    // Here we don't execute the function in the other file context, but we just grab the function and then execute it in our context
    if (mod.kind == MODULE_CALL_FUNC_CALL) {
        Context_Item *item = packl_find_function_or_procedure(&target, mod.as.func_call.name, loc);
        if (item->type == CONTEXT_ITEM_TYPE_FUNCTION) {
            Function func = item->as.func;
            Node caller = { .as.func_call = mod.as.func_call, .kind = NODE_KIND_FUNC_CALL };
            packl_generate_func_call_node(c, self, caller, func, indent);
            return;
        }

        if (item->type == CONTEXT_ITEM_TYPE_PROCEDURE) {
            Procedure proc = item->as.proc;
            Node caller = { .as.func_call = mod.as.func_call, .kind = NODE_KIND_FUNC_CALL };
            packl_generate_proc_call_node(c, self, caller, proc, indent);
            return;
        }

        ASSERT(false, "unreachable");        
    }

    PACKL_ERROR_LOC(self->filename, call_node.loc, "cannot access external module variables outside expression");
}

void packl_generate_statement(PACKL_Compiler *c, PACKL_File *self, Node node, size_t indent) {
    switch(node.kind) {
        case NODE_KIND_PROC_DEF:
            return packl_generate_proc_def_code(c, self, node, indent);
        case NODE_KIND_VAR_DECLARATION:
            return packl_generate_var_dec_node(c, self, node, indent);
        case NODE_KIND_VAR_REASSIGN:
            return packl_generate_var_reassign_node(c, self, node, indent);
        case NODE_KIND_FUNC_CALL:
            return packl_generate_call_node(c, self, node, indent);
        case NODE_KIND_FUNC_DEF:
            return packl_generate_func_def_code(c, self, node, indent);
        case NODE_KIND_NATIVE_CALL:
            return packl_generate_native_call_node(c, self, node, indent);
        case NODE_KIND_IF:
            return packl_generate_if_node(c, self, node, indent);
        case NODE_KIND_WHILE:   
            return packl_generate_while_node(c, self, node, indent);
        case NODE_KIND_FOR:
            return packl_generate_for_node(c, self, node, indent);
        case NODE_KIND_USE:
            return packl_generate_use_node(c, self, node, indent);
        case NODE_KIND_MOD_CALL:
            return packl_generate_mod_call_node(c, self, node, indent);
        default:
            ASSERT(false, "unreachable");
    }
}


void packl_generate_statements(PACKL_Compiler *c, PACKL_File *self, AST nodes, size_t indent) {
    for (size_t i = 0; i < nodes.count; ++i) {
        Node node = nodes.items[i];
        packl_generate_statement(c, self, node, indent);
    }
}

void packl_generate_entry_point(PACKL_Compiler *c, PACKL_File *self, Context_Item item) {
    if (item.type == CONTEXT_ITEM_TYPE_PROCEDURE) {
        fprintf(c->f, "#entry: $label_%zu\n", item.as.proc.label_value);
        return;
    }

    if (item.type == CONTEXT_ITEM_TYPE_FUNCTION) {
        fprintf(c->f, "#entry: $label_%zu\n", item.as.func.label_value);
        return;   
    }

    ASSERT(false, "unreachable");
}

void packl_generate_file_code(PACKL_Compiler *c, PACKL_File *self) {
    packl_init_contexts(self);
    
    // push the global context
    packl_push_new_context(self);

    packl_generate_statements(c, self, self->ast, 0);

    if (c->has_entry) {
        PACKL_ERROR(self->filename, "found two main entry points, first defined here `%s`", c->entry_file_path);
    }

    Context_Item *item = packl_lookup_function_or_procedure(self, SV("main"));
    if (item) {
        c->has_entry = 1;
        c->entry_file_path = self->filename;
        packl_generate_entry_point(c, self, *item);
    }
}

void packl_load_file(PACKL_File *self) {
    self->lexer.source = sv_from_file(self->filename);
    self->lexer.current = 0;
    self->lexer.loc = (Location) {1, 1};
} 

PACKL_File packl_init_file(char *filename, char *fullpath) {
    PACKL_File file = {0};
    
    file.filename = filename;
    file.fullpath = fullpath;

    packl_load_file(&file);
    DA_INIT(&file.root_files, sizeof(char *));
    DA_INIT(&file.used_files, sizeof(PACKL_File));
    return file;
}

void packl_compile_file(PACKL_Compiler *c, PACKL_File *self) {
    lex(self);
    parse(self);
    codegen(c, self);
}

