#include "headers/packl-codegen.h"

PACKL_File packl_init_file(char *filename, char *fullpath);
void packl_compile_file(PACKL_Compiler *c, PACKL_File *self);

PACKL_Type packl_generate_expr_code(PACKL_Compiler *c, PACKL_File *self, Expression expr, size_t indent);
void packl_generate_func_call_node(PACKL_Compiler *c, PACKL_File *self, Node caller, Function func, size_t indent);
void packl_generate_native_call_node(PACKL_Compiler *c, PACKL_File *self, Node node, size_t indent);
void packl_generate_statements(PACKL_Compiler *c, PACKL_File *self, AST nodes, size_t indent);
void packl_push_arguments(PACKL_Compiler *c, PACKL_File *self, PACKL_Args args, size_t indent);

// write sub-functions
void packl_generate_write_arg_by_type_code(PACKL_Compiler *c, PACKL_File *self, PACKL_Type type, size_t indent);

#define PACKL_TYPE_INTEGER    ((PACKL_Type) { .kind = PACKL_TYPE_BASIC, .as.basic = PACKL_TYPE_INT })
#define PACKL_TYPE_POINTER    ((PACKL_Type) { .kind = PACKL_TYPE_BASIC, .as.basic = PACKL_TYPE_PTR })
#define PACKL_TYPE_NONE       ((PACKL_Type) { .kind = PACKL_TYPE_BASIC, .as.basic = PACKL_TYPE_VOID })
#define PACKL_TYPE_STRING     ((PACKL_Type) { .kind = PACKL_TYPE_BASIC, .as.basic = PACKL_TYPE_STR  })


void packl_check_if_type_and_operator_fits_together(PACKL_File *self, PACKL_Type type, Operator op) {
    if(type.kind == PACKL_TYPE_ARRAY) {
        PACKL_ERROR(self->filename, "no arithmetic or logic operators used with arrays");
    }

    if (type.kind == PACKL_TYPE_USER_DEFINED) {
        PACKL_ERROR(self->filename, "no arithmetic or logic operators used with classs");
    }

    if (type.as.basic == PACKL_TYPE_STR) {
        PACKL_ERROR(self->filename, "no arithmetic or logic operators used with strings");
    }

    if (type.as.basic == PACKL_TYPE_VOID) {
        PACKL_ERROR(self->filename, "no arithmetic or logic operators used with void typed data");
    }
}

int packl_check_type_equality(PACKL_File *self, PACKL_Type type1, PACKL_Type type2) {
    if (type1.kind != type2.kind) { return 0; }
    if (type1.kind == PACKL_TYPE_BASIC) {
        if ((type1.as.basic == PACKL_TYPE_INT && type2.as.basic == PACKL_TYPE_PTR)
                ||
            (type1.as.basic == PACKL_TYPE_PTR && type2.as.basic == PACKL_TYPE_INT))
            { return 1; }

        if(type1.as.basic != type2.as.basic) { return 0; }
        return 1;
    }

    if (type1.kind == PACKL_TYPE_USER_DEFINED) {
        packl_find_class(self, type1.as.user_defined, (Location){0,0});
        packl_find_class(self, type2.as.user_defined, (Location){0,0});
        if (!sv_eq(type1.as.user_defined, type2.as.user_defined)) { return 0; }
        return 1;
    }

    return packl_check_type_equality(self, *type1.as.array.item_type, *type2.as.array.item_type);
}

PACKL_Type packl_type_check(PACKL_File *self, PACKL_Type lhs_type, PACKL_Type rhs_type, Operator op) {
    packl_check_if_type_and_operator_fits_together(self, lhs_type, op);
    packl_check_if_type_and_operator_fits_together(self, rhs_type, op);


    if (lhs_type.as.basic == PACKL_TYPE_INT) {
        if (rhs_type.as.basic == PACKL_TYPE_PTR) {
            if (op == OP_PLUS) { return rhs_type; }
            PACKL_ERROR(self->filename, "unexpected operator between an integer number and a pointer");
        }
        return lhs_type;
    }

    if (lhs_type.as.basic == PACKL_TYPE_PTR) {
        if (rhs_type.as.basic == PACKL_TYPE_INT) {
            if (op == OP_PLUS) { return lhs_type; }
            if (op == OP_MINUS) { return lhs_type; }
            PACKL_ERROR(self->filename, "unexpected operator between an integer number and a pointer");
        }
        PACKL_ERROR(self->filename, "can not make operations between two pointers");
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

void packl_pop_scope(PACKL_Compiler *c, size_t stack_pos, size_t indent) {
    while(c->stack_size > stack_pos) {
        packl_generate_pop(c, indent);
    }
}


void packl_print_expr_type(PACKL_Type type) {
    if(type.kind == PACKL_TYPE_BASIC) {
        switch(type.as.basic) {
            case PACKL_TYPE_INT:  fprintf(stderr, "int\n"); break;
            case PACKL_TYPE_STR:  fprintf(stderr, "str\n"); break;
            case PACKL_TYPE_PTR:  fprintf(stderr, "ptr\n"); break;
            case PACKL_TYPE_VOID: fprintf(stderr, "void\n"); break;
            default:
                ASSERT(false, "unreachable");
        }
    } else if (type.kind == PACKL_TYPE_USER_DEFINED) {
        fprintf(stderr, SV_FMT"\n", SV_UNWRAP(type.as.user_defined));
    } else {
        fprintf(stderr, "array of ");
        packl_print_expr_type(*type.as.array.item_type);
    }
}

void packl_expect_type(PACKL_File *self, Location loc, PACKL_Type expected, PACKL_Type target) {
    if(packl_check_type_equality(self, expected, target)) {
        return;
    }
    PACKL_ERROR_LOC(self->filename, loc, "type mismatch");
}

PACKL_Type packl_generate_integer_expr_code(PACKL_Compiler *c, int64_t value, size_t indent) {
    packl_generate_push(c, value, indent);
    return PACKL_TYPE_INTEGER;
}

PACKL_Type packl_generate_identifier_expr_code(PACKL_Compiler *c, PACKL_File *self, String_View name, size_t indent) {
    Variable var = packl_find_variable(self, name, (Location) {0, 0});
    size_t var_pos = c->stack_size - var.stack_pos - 1;
    packl_generate_indup(c, var_pos, indent);
    return var.type;
}

void packl_generate_method_call(PACKL_Compiler *c, PACKL_File *self, Variable object, Method method, PACKL_Args args, size_t indent) {

    size_t stack_size = c->stack_size;

    // push the current object
    packl_generate_indup(c, c->stack_size - object.stack_pos - 1, indent);

    // push the function args
    packl_push_arguments(c, self, args, indent);

    // generate the call
    size_t label = 0;

    if (method.kind == METHOD_KIND_FUNCTION) {
        label = method.as.func.label_value;
    } else if (method.kind == METHOD_KIND_PROCEDURE) {
        label = method.as.proc.label_value;
    } else {
        ASSERT(false, "`packl_generate_method_call` failed to get the label value to call the method");
    }

    packl_generate_call(c, label, indent);

    packl_pop_scope(c, stack_size, indent);
}

PACKL_Type packl_generate_method_call_expr_code(PACKL_Compiler *c, PACKL_File *self, Method_Call method_call, size_t indent) {
    Variable object = packl_find_variable(self, method_call.object_name, (Location){0,0});

    if (object.type.kind != PACKL_TYPE_USER_DEFINED) {
        PACKL_ERROR(self->filename, "`" SV_FMT "` is not an object, it has no methods", SV_UNWRAP(method_call.object_name));
    }

    Class *class = packl_find_class(self, object.type.as.user_defined, (Location){0, 0});
    Method *isfound = packl_find_class_method(self, *class, method_call.func.name);

    if (!isfound) {
        PACKL_ERROR(self->filename, "object `" SV_FMT "` of class `" SV_FMT "` has not method `" SV_FMT "`", SV_UNWRAP(method_call.object_name), SV_UNWRAP(class->name), SV_UNWRAP(method_call.func.name));
    }

    Method method = *isfound;
    if (method.kind == METHOD_KIND_PROCEDURE) {
        PACKL_ERROR(self->filename, "method `" SV_FMT "` of class returns void in an expression", SV_UNWRAP(class->name));
    }

    ASSERT(method.kind == METHOD_KIND_FUNCTION, "`packl_generate_method_call_expr_code` failed, not handled method kind");


    // for the function return value
    packl_generate_push(c, 0, indent);

    packl_generate_method_call(c, self, object, method, method_call.func.args, indent);

    return method.as.func.return_type;
}

PACKL_Type packl_generate_func_call_expr_code(PACKL_Compiler *c, PACKL_File *self, Func_Call func, size_t indent) {
    Context_Item *item = packl_get_context_item_in_all_contexts(self, func.name);
    if (!item) {
        PACKL_ERROR(self->filename, "`" SV_FMT "` called but never declared", SV_UNWRAP(func.name));
    }

    Function function = {0};
    Function *isfound = NULL;


    if (item->type == CONTEXT_ITEM_TYPE_VARIABLE) {
        // maybe it's a recursive call
        isfound = packl_find_function_in_previous_scopes(self, func.name);
        if (!isfound) {
            PACKL_ERROR(self->filename, "`" SV_FMT "` expected to be a function, but found as a variable", SV_UNWRAP(func.name));
        }
        function = *isfound;
    } else if (item->type != CONTEXT_ITEM_TYPE_FUNCTION) {
        if (item->type == CONTEXT_ITEM_TYPE_PROCEDURE) {
            PACKL_ERROR(self->filename, "`" SV_FMT "` found as procedure, procedures return void in an expression", SV_UNWRAP(func.name));
        }

        PACKL_ERROR(self->filename, "`" SV_FMT "` expected to be a function, but found as a %s", SV_UNWRAP(func.name), context_item_as_cstr[item->type]);
    }

    if (item->type == CONTEXT_ITEM_TYPE_FUNCTION) {
        function = item->as.func;
    }

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
        case OP_L:
            return packl_generate_cmpl(c, indent);
        case OP_G:
            return packl_generate_cmpg(c, indent);
        case OP_GE:
            return packl_generate_cmpge(c, indent);
        case OP_LE:
            return packl_generate_cmple(c, indent);
        case OP_EQ:
            return packl_generate_cmpe(c, indent);
        case OP_NE:
            return packl_generate_cmpne(c, indent);
        case OP_AND:
            return packl_generate_and(c, indent);
        case OP_OR:
            return packl_generate_or(c, indent);
        case OP_XOR:
            return packl_generate_xor(c, indent);
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
    return PACKL_TYPE_STRING;
}

PACKL_Type packl_get_native_return_type(String_View native_name) {
    if (sv_eq(native_name, SV("malloc"))) {
        return PACKL_TYPE_POINTER;
    }

    if (sv_eq(native_name, SV("mload"))) {
        return PACKL_TYPE_INTEGER;
    }

    return PACKL_TYPE_NONE;
}

PACKL_Type packl_generate_native_call_code(PACKL_Compiler *c, PACKL_File *self, Func_Call native, size_t indent) {
    Node node = { .kind = NODE_KIND_NATIVE_CALL, .as.func_call = native, .loc = (Location) { 0, 0 } };
    packl_generate_native_call_node(c, self, node, indent);
    return packl_get_native_return_type(native.name);
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

void packl_generate_push_object_attr_code(PACKL_Compiler *c, size_t offset, size_t size_to_load, size_t indent) {
    packl_generate_push(c, (int64_t)offset, indent);
    packl_generate_add(c, indent);
    packl_generate_push(c, (int64_t)size_to_load, indent);
    packl_generate_load(c, indent);
}

PACKL_Type packl_generate_attr_expr_code(PACKL_Compiler *c, PACKL_File *self, Expr_Attribute expr, size_t indent) {
    Variable var = packl_find_variable(self, expr.obj_name, (Location){0,0});


    if (var.type.kind != PACKL_TYPE_USER_DEFINED) {
        PACKL_ERROR(self->filename, SV_FMT " is not an class", SV_UNWRAP(expr.obj_name));
    }

    Class *class = packl_find_class(self, var.type.as.user_defined, (Location){0,0});

    Attribute *found = packl_find_class_attr(self, *class, expr.attr);
    if (!found) {
        PACKL_ERROR(self->filename, "no attr called `" SV_FMT "` for the class `" SV_FMT "`", SV_UNWRAP(expr.attr), SV_UNWRAP(class->name));
    }

    Attribute attr = *found;

    size_t offset = attr.offset;
    size_t size = packl_get_type_size(self, attr.type);

    PACKL_COMMENT(c->f, indent, "this is for the structure attr");

    packl_generate_indup(c, c->stack_size - var.stack_pos - 1, indent);
    packl_generate_push_object_attr_code(c, offset, size, indent);

    return attr.type;
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
    packl_generate_array_item_size(c, self, *var.type.as.array.item_type, indent);

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

    return *var.type.as.array.item_type;
}

PACKL_Type packl_generate_not_unary_expr_code(PACKL_Compiler *c, PACKL_File *self, Expression expr, size_t indent) {
    PACKL_Type type = packl_generate_expr_code(c, self, expr, indent);
    packl_generate_not(c, indent);
    return type;
}

PACKL_Type packl_generate_postunary_expr_code(PACKL_Compiler *c, PACKL_File *self, Expr_Unary_Op unary_op, size_t indent) {
    Expression operand = *unary_op.operand;

    if (operand.kind != EXPR_KIND_ID) {
        PACKL_ERROR_LOC(self->filename, operand.loc, "unsupported operation, expected an identifier");
    }

    Variable var = packl_find_variable(self, operand.as.value, operand.loc);


    if (var.type.kind != PACKL_TYPE_BASIC) {
        PACKL_ERROR_LOC(self->filename, operand.loc, "variable must be of an integer type");
    }

    if (var.type.as.basic != PACKL_TYPE_INT) {
        PACKL_ERROR_LOC(self->filename, operand.loc, "variable must be of an integer type");
    }

    // duplicate the value of the variable
    packl_generate_indup(c, c->stack_size - var.stack_pos - 1, indent);

    packl_generate_dup(c, indent);

    packl_generate_push(c, 1, indent);

    if (unary_op.op == OP_INC) {
        packl_generate_add(c, indent);
    } else {
        packl_generate_sub(c, indent);
    }

    packl_generate_inswap(c, c->stack_size - var.stack_pos - 1, indent);

    packl_generate_pop(c, indent);

    return var.type;
}

PACKL_Type packl_generate_preunary_expr_code(PACKL_Compiler *c, PACKL_File *self, Expr_Unary_Op unary_op, size_t indent) {
    // for the pre unary operators, only not aka ! is supported
    if(unary_op.op == OP_NOT) {
        return packl_generate_not_unary_expr_code(c, self, *unary_op.operand, indent);
    }
    ASSERT(false, "unreachable");
}

void packl_generate_sizeof_code(PACKL_Compiler *c, PACKL_File *self, Expr_Operator_Input input, size_t indent) {
    size_t size = packl_get_operator_input_size(c, self, input);
    packl_generate_push(c, (int64_t)size, indent);
}

PACKL_Type packl_generate_new_code(PACKL_Compiler *c, PACKL_File *self, Expr_Operator_Input input, size_t indent) {
    packl_generate_sizeof_code(c, self, input, indent);
    packl_generate_syscall(c, 2, indent);

    if (input.kind == INPUT_KIND_TYPE) { return PACKL_TYPE_POINTER; }

    Class *class = packl_find_class(self, input.as.identifier, (Location){0, 0});
    return (PACKL_Type) { .as.user_defined = class->name, .kind = PACKL_TYPE_USER_DEFINED };
}


PACKL_Type packl_generate_operator_expr_code(PACKL_Compiler *c, PACKL_File *self, Expr_Operator operator, size_t indent) {
    switch(operator.op) {
        case SIZEOF_OPERATOR:
            packl_generate_sizeof_code(c, self, operator.input, indent);
            return PACKL_TYPE_INTEGER;
        case NEW_OPERATOR:
            return packl_generate_new_code(c, self, operator.input, indent);
        default:
            ASSERT(false, "`packl_generate_operator_expr_code` failed to generate the new operator code");
    }
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
        case EXPR_KIND_PRE_UNARY_OP:
            return packl_generate_preunary_expr_code(c, self, expr.as.unary, indent);
        case EXPR_KIND_POST_UNARY_OP:
            return packl_generate_postunary_expr_code(c, self, expr.as.unary, indent);
        case EXPR_KIND_OBJECT_ATTRIBUTE:
            return packl_generate_attr_expr_code(c, self, expr.as.attr, indent);
        case EXPR_KIND_OBJECT_METHOD:
            return packl_generate_method_call_expr_code(c, self, *expr.as.method, indent);
        case EXPR_KIND_OPERATOR:
            return packl_generate_operator_expr_code(c, self, expr.as.operator, indent);
        default:
            ASSERT(false, "unreachable");
    }
}

void packl_generate_proc_body_code(PACKL_Compiler *c, PACKL_File *self, Proc_Def proc, size_t indent) {
    packl_setup_proc_params(c, self, proc.params);

    packl_generate_label(c, c->label_value++, indent);

    packl_generate_statements(c, self, *proc.body, indent + 1);
}

void packl_generate_proc_def_code(PACKL_Compiler *c, PACKL_File *self, Node proc_def_node, size_t indent) {
    Proc_Def proc_def = proc_def_node.as.proc_def;
    packl_find_item_and_report_error_if_found(self, proc_def.name, proc_def_node.loc);

    Context_Item new_proc = packl_init_proc_context_item(proc_def.name, proc_def.params, c->label_value);
    packl_push_item_in_current_context(self, new_proc);

    packl_push_new_context(self);

    // size_t stack_size = c->stack_size;
    // c->stack_size = proc_def.params.count;

    size_t initial_stack_size = c->stack_size;

    c->stack_size += proc_def.params.count;
    size_t after_args_push_stack_size = c->stack_size;


    packl_generate_proc_body_code(c, self, proc_def, indent);

    packl_pop_scope(c, after_args_push_stack_size, indent + 1);
    c->stack_size = initial_stack_size;

    packl_pop_context(self);
    packl_generate_ret(c, indent + 1);
}


void packl_reassign_array_item_code(PACKL_Compiler *c, PACKL_File *self, PACKL_Type arr_type, Expression value, Expression index, size_t indent) {
    packl_generate_dup(c, indent);
    packl_generate_array_item_size(c, self, *arr_type.as.array.item_type, indent);
    packl_generate_swap(c, indent);
    packl_generate_indup(c, 1, indent);
    packl_expect_type(self, index.loc, packl_generate_expr_code(c, self, index, indent), PACKL_TYPE_INTEGER);
    packl_generate_mul(c, indent);
    packl_generate_add(c, indent);
    packl_generate_swap(c, indent);

    PACKL_Type expected_type = *arr_type.as.array.item_type;
    packl_expect_type(self, value.loc, expected_type, packl_generate_expr_code(c, self, value, indent));

    packl_generate_swap(c, indent);
    packl_generate_store(c, indent);
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
        if (var_dec.value.kind == EXPR_KIND_NOT_INITIALIZED) {
            packl_generate_push(c, 0, indent);
            return;
        }

        if (!packl_check_type_equality(self, packl_generate_expr_code(c, self, var_dec.value, indent), var_dec.type)) {
            PACKL_ERROR_LOC(self->filename, var_dec_node.loc, "type mismatch");
        }

        return;
    }

    if (var_dec.type.kind == PACKL_TYPE_ARRAY) {
        return packl_handle_array_var_dec(c, self, var_dec, indent);
    }

    if (var_dec.type.kind == PACKL_TYPE_USER_DEFINED) {
        // this will throw an error if no class is found
        packl_find_class(self, var_dec.type.as.user_defined, var_dec_node.loc);

        PACKL_Type type = packl_generate_expr_code(c, self, var_dec.value, indent);
        if (!packl_check_type_equality(self, var_dec.type, type)) {
            PACKL_ERROR_LOC(self->filename, var_dec_node.loc, "type mismatch");
        }
        return;
    }

    ASSERT(false, "unreachable");
}

void packl_reassign_str_char_code(PACKL_Compiler *c, PACKL_File *self, Expression value, Expression index, size_t indent) {
    packl_generate_dup(c, indent);

    PACKL_Type index_type = packl_generate_expr_code(c, self, index, indent);
    packl_expect_type(self, index.loc, PACKL_TYPE_INTEGER, index_type);

    packl_generate_add(c, indent);

    PACKL_Type value_type = packl_generate_expr_code(c, self, value, indent);
    packl_expect_type(self, value.loc, PACKL_TYPE_INTEGER, value_type);

    packl_generate_storeb(c, indent);
}

void packl_reassign_class_attr(PACKL_Compiler *c, PACKL_File *self, Variable var, Variable_Format fmt, Expression value, size_t indent) {
    packl_generate_indup(c, c->stack_size - var.stack_pos - 1, indent);

    Class *class = packl_find_class(self, var.type.as.user_defined, (Location){0, 0});
    Attribute *found = packl_find_class_attr(self, *class, fmt.as.attr);

    if (!found) {
        PACKL_ERROR(self->filename, "no attr called `" SV_FMT "` for the class `" SV_FMT "`", SV_UNWRAP(fmt.as.attr), SV_UNWRAP(class->name));
    }

    Attribute attr = *found;

    size_t offset = attr.offset;

    packl_generate_push(c, offset, indent);
    packl_generate_add(c, indent);

    PACKL_Type expr_type = packl_generate_expr_code(c, self, value, indent);
    if (!packl_check_type_equality(self, expr_type, attr.type)) {
        PACKL_ERROR(self->filename, "type mismatch");
    }

    size_t size = packl_get_type_size(self, expr_type);

    packl_generate_push(c, size, indent);
    packl_generate_store(c, indent);
}

void packl_generate_var_reassign_node(PACKL_Compiler *c, PACKL_File *self, Node var_reassign_node, size_t indent) {
    Var_Reassign var_reassign = var_reassign_node.as.var;
    Variable_Format fmt = var_reassign.format;

    Variable var = packl_find_variable(self, fmt.name, var_reassign_node.loc);

    if (fmt.kind == VARIABLE_FORMAT_ARRAY) {
        packl_generate_indup(c, c->stack_size - var.stack_pos - 1, indent);

        if (var.type.kind == PACKL_TYPE_BASIC && var.type.as.basic == PACKL_TYPE_STR) {
            packl_reassign_str_char_code(c, self, var_reassign.value, fmt.as.index, indent);
        } else if (var.type.kind == PACKL_TYPE_ARRAY){
            packl_reassign_array_item_code(c, self, var.type, var_reassign.value, fmt.as.index, indent);
        } else {
            PACKL_ERROR_LOC(self->filename, var_reassign_node.loc, "`" SV_FMT "` is not an array nor a str", SV_UNWRAP(var_reassign.format.name));
        }

        return packl_generate_pop(c, indent);
    }

    if (fmt.kind == VARIABLE_FORMAT_CLASS) {
        return packl_reassign_class_attr(c, self, var, fmt, var_reassign.value, indent);
    }

    if (var_reassign.value.kind == EXPR_KIND_POST_UNARY_OP) {
        packl_generate_postunary_expr_code(c, self, var_reassign.value.as.unary, indent);
        packl_generate_pop(c, indent);
    }

    PACKL_Type expected_type = var.type;
    packl_expect_type(self, var_reassign.value.loc, expected_type, packl_generate_expr_code(c, self, var_reassign.value, indent));

    size_t var_pos = c->stack_size - var.stack_pos - 1;
    packl_generate_inswap(c, var_pos, indent);

    packl_generate_pop(c, indent);
}

void packl_check_caller_arity(PACKL_File *self, Node caller, size_t params_count) {
    if (caller.as.func_call.args.count < params_count) {
        PACKL_ERROR_LOC(self->filename, caller.loc, "too few arguments for `" SV_FMT "` call, expected %zu got %zu", SV_UNWRAP(caller.as.func_call.name), params_count, caller.as.func_call.args.count);
    }

    if (caller.as.func_call.args.count > params_count) {
        PACKL_ERROR_LOC(self->filename, caller.loc, "too many arguments for `" SV_FMT "` call, expected %zu got %zu", SV_UNWRAP(caller.as.func_call.name), params_count, caller.as.func_call.args.count);
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
    PACKL_COMMENT(c->f, indent, "this is for the function return value");
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

void packl_setup_func_params(PACKL_Compiler *c, PACKL_File *self, Func_Def func, Parameters params) {
    // set the function parameters
    packl_setup_proc_params(c, self, params);

    // set the function return value
    size_t pos = c->stack_size - params.count - 1;
    Context_Item func_ret_var = packl_init_var_context_item(func.name, func.return_type, pos);
    packl_push_item_in_current_context(self, func_ret_var);
}

void packl_generate_func_body_code(PACKL_Compiler *c, PACKL_File *self, Func_Def func, size_t indent) {
    // this will setup the function return value also
    packl_setup_func_params(c, self, func, func.params);

    packl_generate_label(c, c->label_value++, indent);

    packl_generate_statements(c, self, *func.body, indent + 1);
}

void packl_generate_func_def_code(PACKL_Compiler *c, PACKL_File *self, Node func_def_node, size_t indent) {
    Func_Def func_def = func_def_node.as.func_def;
    packl_find_item_and_report_error_if_found(self, func_def.name, func_def_node.loc);

    Context_Item new_func = packl_init_func_context_item(func_def.name, func_def.return_type, func_def.params, c->label_value);
    packl_push_item_in_current_context(self, new_func);

    // push a new context for the function local variables
    packl_push_new_context(self);

    size_t stack_size = c->stack_size;
    // we're kinda gonna pretend that only the function parameters exist on the stack
    c->stack_size = func_def.params.count + 1;          // + 1 for the return value

    packl_generate_func_body_code(c, self, func_def, indent);

    packl_pop_scope(c, func_def.params.count + 1, indent + 1);

    // getting back the stack to where it was at
    c->stack_size = stack_size;
    packl_generate_ret(c, indent + 1);

    // pop the function scope
    packl_pop_context(self);
}

void packl_generate_write_to_stdout(PACKL_Compiler *c, String_View view, size_t indent) {
    packl_generate_push(c, 0, indent);
    packl_generate_pushs(c, view, indent);
    packl_generate_syscall(c, 0, indent);
}

void packl_generate_write_object_code(PACKL_Compiler *c, PACKL_File *self, Class class, size_t indent) {
    packl_generate_swap(c, indent);

    packl_generate_write_to_stdout(c, SV("{ "), indent);

    for (size_t i = 0; i < class.attrs.count; ++i) {
        Attribute attr = class.attrs.items[i];
        packl_generate_dup(c, indent);
        packl_generate_indup(c, 2, indent);
        packl_generate_write_to_stdout(c, attr.name, indent);
        packl_generate_write_to_stdout(c, SV(": "), indent);
        packl_generate_push_object_attr_code(c, attr.offset, attr.type_size, indent);
        packl_generate_write_arg_by_type_code(c, self, attr.type, indent);
        if (i != class.attrs.count - 1) {    
            packl_generate_write_to_stdout(c, SV(", "), indent);
        }
    }


    packl_generate_write_to_stdout(c, SV(" }"), indent);


    packl_generate_pop(c, indent); // pop the object
    packl_generate_pop(c, indent); // pop the file stream
}


void packl_generate_write_basic_code(PACKL_Compiler *c, PACKL_File *self, PACKL_Type type, size_t indent) {
    if (type.as.basic == PACKL_TYPE_INT) {
        return packl_generate_writei(c, indent);
    }

    if (type.as.basic == PACKL_TYPE_STR) {
        return packl_generate_syscall(c, 0, indent);
    }

    if (type.as.basic == PACKL_TYPE_VOID) {
        packl_generate_pop(c, indent);
    }

    if (type.as.basic == PACKL_TYPE_PTR) {
        TODO("`packl_generate_write_basic_code`, add a way in PVM to write pointers to the screen");
    }

    TODO("`packl_generate_write_basic_code` failed, unknown basic type");
}

void packl_generate_write_arg_by_type_code(PACKL_Compiler *c, PACKL_File *self, PACKL_Type type, size_t indent) {
    if (type.kind == PACKL_TYPE_USER_DEFINED) {
        Class *class = packl_find_class(self, type.as.user_defined, (Location) { 0, 0 });
        return packl_generate_write_object_code(c, self, *class, indent);
    }

    if (type.kind == PACKL_TYPE_BASIC) {
        return packl_generate_write_basic_code(c, self, type, indent);
    }

    if (type.kind == PACKL_TYPE_ARRAY) {
        TODO("`packl_generate_write_arg_code` failed, implement array printing");
    }

    ASSERT(false, "`packl_generate_write_arg_code` failed, unkown type kind");
}

void packl_generate_write_arg_code(PACKL_Compiler *c, PACKL_File *self, PACKL_Arg arg, size_t indent) {
    PACKL_Type type = packl_generate_expr_code(c, self, arg.expr, indent);
    packl_generate_write_arg_by_type_code(c, self, type, indent);
}

void packl_generate_native_write_code(PACKL_Compiler *c, PACKL_File *self, Node caller, size_t indent) {
    Func_Call native_write = caller.as.func_call;

    if (caller.as.func_call.args.count < 2) {
        PACKL_ERROR_LOC(self->filename, caller.loc, "too few arguments for `" SV_FMT "` call, expected at least %d got %zu", SV_UNWRAP(caller.as.func_call.name), 2, caller.as.func_call.args.count);
    }

    Expression file_stream = native_write.args.items[0].expr;
    if (file_stream.kind != EXPR_KIND_INTEGER) {
        PACKL_ERROR_LOC(self->filename, file_stream.loc, "expected the file stream to be an integer");
    }

    packl_generate_expr_code(c, self, file_stream, indent);

    for (size_t i = 1; i < native_write.args.count; ++i) {
        PACKL_Arg arg = native_write.args.items[i];
        packl_generate_dup(c, indent);
        packl_generate_write_arg_code(c, self, arg, indent);
    }

    // pop the file stream
    packl_generate_pop(c, indent);
}

void packl_generate_native_exit_code(PACKL_Compiler *c, PACKL_File *self, Node caller, size_t indent) {
    Func_Call native_exit = caller.as.func_call;

    packl_check_caller_arity(self, caller, 1);

    Expression arg = native_exit.args.items[0].expr;

    PACKL_Type type = packl_generate_expr_code(c, self, arg, indent);
    packl_expect_type(self, arg.loc, PACKL_TYPE_INTEGER, type);

    packl_generate_syscall(c, 6, indent);
}

void packl_generate_native_malloc_code(PACKL_Compiler *c, PACKL_File *self, Node caller, size_t indent) {
    Func_Call native_malloc = caller.as.func_call;

    packl_check_caller_arity(self, caller, 1);

    Expression arg = native_malloc.args.items[0].expr;

    PACKL_Type type = packl_generate_expr_code(c, self, arg, indent);
    packl_expect_type(self, arg.loc, PACKL_TYPE_INTEGER, type);

    packl_generate_syscall(c, 2, indent);
}

void packl_generate_native_mdealloc_code(PACKL_Compiler *c, PACKL_File *self, Node caller, size_t indent) {
    Func_Call native_mdealloc = caller.as.func_call;

    packl_check_caller_arity(self, caller, 1);

    Expression arg = native_mdealloc.args.items[0].expr;

    PACKL_Type type = packl_generate_expr_code(c, self, arg, indent);
    packl_expect_type(self, arg.loc, PACKL_TYPE_POINTER, type);

    packl_generate_syscall(c, 3, indent);
}


void packl_generate_native_mload_code(PACKL_Compiler *c, PACKL_File *self, Node caller, size_t indent) {
    Func_Call native_mload = caller.as.func_call;

    packl_check_caller_arity(self, caller, 2);

    Expression ptr_arg = native_mload.args.items[0].expr;
    Expression size_arg = native_mload.args.items[1].expr;

    PACKL_Type type = packl_generate_expr_code(c, self, ptr_arg, indent);
    packl_expect_type(self, ptr_arg.loc, PACKL_TYPE_POINTER, type);

    type = packl_generate_expr_code(c, self, size_arg, indent);
    packl_expect_type(self, ptr_arg.loc, PACKL_TYPE_INTEGER, type);

    packl_generate_load(c, indent);
}

void packl_generate_native_mset_code(PACKL_Compiler *c, PACKL_File *self, Node caller, size_t indent) {
    Func_Call native_mset = caller.as.func_call;

    packl_check_caller_arity(self, caller, 3);

    Expression ptr_arg = native_mset.args.items[0].expr;
    Expression data_arg = native_mset.args.items[1].expr;
    Expression size_arg = native_mset.args.items[2].expr;

    PACKL_Type type = packl_generate_expr_code(c, self, ptr_arg, indent);
    packl_expect_type(self, ptr_arg.loc, PACKL_TYPE_POINTER, type);

    type = packl_generate_expr_code(c, self, data_arg, indent);
    packl_expect_type(self, ptr_arg.loc, PACKL_TYPE_INTEGER, type);

    type = packl_generate_expr_code(c, self, size_arg, indent);
    packl_expect_type(self, ptr_arg.loc, PACKL_TYPE_INTEGER, type);

    packl_generate_store(c, indent);
}

void packl_generate_native_call_node(PACKL_Compiler *c, PACKL_File *self, Node node, size_t indent) {
    Func_Call native = node.as.func_call;

    if (sv_eq(native.name, SV("write"))) {
        return packl_generate_native_write_code(c, self, node, indent);
    }

    if (sv_eq(native.name, SV("exit"))) {
        return packl_generate_native_exit_code(c, self, node, indent);
    }

    if (sv_eq(native.name, SV("malloc"))) {
        return packl_generate_native_malloc_code(c, self, node, indent);
    }

    if (sv_eq(native.name, SV("mdealloc"))) {
        return packl_generate_native_mdealloc_code(c, self, node, indent);
    }

    if (sv_eq(native.name, SV("mload"))) {
        return packl_generate_native_mload_code(c, self, node, indent);
    }

    if (sv_eq(native.name, SV("mset"))) {
        return packl_generate_native_mset_code(c, self, node, indent);
    }


    ASSERT(false, "unreachable");
}

void packl_generate_if_node(PACKL_Compiler *c, PACKL_File *self, Node if_node, size_t indent) {
    If_Statement fi = if_node.as.fi;

    size_t label = c->label_value;

    c->label_value += 2;             // we reserve two labels for this if statement

    size_t stack_size = c->stack_size;

    packl_generate_expr_code(c, self, fi.condition, indent);

    // for the if scope
    packl_push_new_context(self);

    packl_generate_jz(c, label, indent);         // for the else of quit part

    // if body
    packl_generate_statements(c, self, *fi.body, indent);

    // pop the if scope
    packl_pop_scope(c, stack_size, indent);

    // pop the if context
    packl_pop_context(self);

    packl_generate_jmp(c, label + 1, indent);    // for the quit part

    packl_generate_label(c, label, indent);
    if (fi.esle) {
        // for the else scope
        packl_push_new_context(self);

        packl_generate_statements(c, self, *fi.esle, indent);
        // pop the else scope
        packl_pop_scope(c, stack_size, indent);

        packl_pop_context(self);
    }

    packl_generate_label(c, label + 1, indent);


    c->stack_size = stack_size;
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

    packl_pop_scope(c, stack_size, indent);

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

    packl_pop_scope(c, stack_size + 1, indent); // let only the for iterator

    packl_generate_jmp(c, label, indent + 1);

    packl_generate_label(c, label + 1, indent);

    // pop the for iterator
    packl_generate_pop(c, indent);

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


void packl_handle_class_definition(PACKL_Compiler *c, PACKL_File *self, Node node, size_t indent) {
    Class_Def class = node.as.class;
    String_View class_name = class.name;
    packl_find_item_and_report_error_if_found(self, class_name, node.loc);

    Context_Item context_item = packl_init_class_context_item(self, class_name, class.attrs);
    packl_push_item_in_current_context(self, context_item);
}

void packl_generate_proc_method_code(PACKL_Compiler *c, PACKL_File *self, Class class, Proc_Def proc, size_t indent) {
    size_t stack_size = c->stack_size;
    c->stack_size = proc.params.count + 1;   // for the `this` object

    // push a new context for the procedure local variables
    packl_push_new_context(self);

    // setup the parameters
    packl_setup_proc_params(c, self, proc.params);

    // setup the this object
    PACKL_Type object_type = { .kind = PACKL_TYPE_USER_DEFINED, .as.user_defined = class.name };
    Context_Item this = packl_init_var_context_item(SV("this"), object_type, c->stack_size - proc.params.count - 1);
    packl_push_item_in_current_context(self, this);

    packl_generate_proc_body_code(c, self, proc, indent);

    // let the function parameters and `this` and the function return value
    packl_pop_scope(c, proc.params.count + 1, indent);

    c->stack_size = stack_size;

    // pop the function scope
    packl_pop_context(self);
}

void packl_generate_func_method_code(PACKL_Compiler *c, PACKL_File *self, Class class, Func_Def func, size_t indent) {
    size_t stack_size = c->stack_size;
    c->stack_size = func.params.count + 2;   // one for the return value and another one for the `this` object

    // push a new context for the function local variables
    packl_push_new_context(self);

    // setup the parameters
    packl_setup_proc_params(c, self, func.params);

    // setup the this object
    PACKL_Type object_type = { .kind = PACKL_TYPE_USER_DEFINED, .as.user_defined = class.name };
    Context_Item this = packl_init_var_context_item(SV("this"), object_type, c->stack_size - func.params.count - 1);
    packl_push_item_in_current_context(self, this);


    // setup the function return value
    Context_Item func_return_value = packl_init_var_context_item(func.name, func.return_type, c->stack_size - func.params.count - 2);
    packl_push_item_in_current_context(self, func_return_value);

    packl_generate_func_body_code(c, self, func, indent);

    // let the function parameters and `this` and the function return value
    packl_pop_scope(c, func.params.count + 2, indent);

    c->stack_size = stack_size;

    // pop the function scope
    packl_pop_context(self);
}

void packl_generate_method_def_code(PACKL_Compiler *c, PACKL_File *self, Node node, size_t indent) {
    Method_Def method = node.as.method_def;

    Class *class = packl_find_class(self, method.class_name, node.loc);

    String_View method_name;
    if (method.kind == METHOD_KIND_FUNCTION) {
        method_name = method.as.func.name;
    } else if (method.kind == METHOD_KIND_PROCEDURE) {
        method_name = method.as.proc.name;
    } else {
        ASSERT(false, "`packl_generate_method_def_code` failed to get the name of the method");
    }

    Method *isfound = packl_find_class_method(self, *class, method_name);
    if (isfound) {
        PACKL_ERROR_LOC(self->filename, node.loc, "method `" SV_FMT "` already declared in the `" SV_FMT "` class", SV_UNWRAP(method_name), SV_UNWRAP(class->name));
    }

    Method new_method = { .name = method_name, .kind = method.kind };

    if (method.kind == METHOD_KIND_FUNCTION) {
        Func_Def as_func = method.as.func;
        new_method.as.func = packl_init_context_function(as_func.return_type, as_func.params, c->label_value);

        // generate the function
        packl_generate_func_method_code(c, self, *class, as_func, indent);
        DA_APPEND(&class->methods, new_method);
    } else if (method.kind == METHOD_KIND_PROCEDURE) {
        Proc_Def as_proc = method.as.proc;
        new_method.as.proc = packl_init_context_procedure(as_proc.params, c->label_value);

        // generate the procedure
        packl_generate_proc_method_code(c, self, *class, as_proc, indent);
        DA_APPEND(&class->methods, new_method);
    }

    packl_generate_ret(c, indent + 1);
}

void packl_generate_method_call_node(PACKL_Compiler *c, PACKL_File *self, Node node, size_t indent) {
    Method_Call method_call = node.as.method_call;

    Variable object = packl_find_variable(self, method_call.object_name, (Location){0,0});

    if (object.type.kind != PACKL_TYPE_USER_DEFINED) {
        PACKL_ERROR(self->filename, "`" SV_FMT "` is not an object, it has no methods", SV_UNWRAP(method_call.object_name));
    }

    Class *class = packl_find_class(self, object.type.as.user_defined, (Location){0, 0});
    Method *isfound = packl_find_class_method(self, *class, method_call.func.name);

    if (!isfound) {
        PACKL_ERROR(self->filename, "object `" SV_FMT "` of class `" SV_FMT "` has not method `" SV_FMT "`", SV_UNWRAP(method_call.object_name), SV_UNWRAP(class->name), SV_UNWRAP(method_call.func.name));
    }

    Method method = *isfound;

    if (method.kind == METHOD_KIND_FUNCTION) { packl_generate_push(c, 0, indent); } // for the function return value;

    packl_generate_method_call(c, self, object, method, method_call.func.args, indent);
}

void packl_generate_statement(PACKL_Compiler *c, PACKL_File *self, Node node, size_t indent) {
    switch(node.kind) {
        case NODE_KIND_VAR_DECLARATION:
            return packl_generate_var_dec_node(c, self, node, indent);
        case NODE_KIND_VAR_REASSIGN:
            return packl_generate_var_reassign_node(c, self, node, indent);
        case NODE_KIND_FUNC_CALL:
            return packl_generate_call_node(c, self, node, indent);
        case NODE_KIND_NATIVE_CALL:
            return packl_generate_native_call_node(c, self, node, indent);
        case NODE_KIND_IF:
            return packl_generate_if_node(c, self, node, indent);
        case NODE_KIND_WHILE:
            return packl_generate_while_node(c, self, node, indent);
        case NODE_KIND_FOR:
            return packl_generate_for_node(c, self, node, indent);
        case NODE_KIND_MOD_CALL:
            return packl_generate_mod_call_node(c, self, node, indent);
        case NODE_KIND_METHOD_CALL:
            return packl_generate_method_call_node(c, self, node, indent);
        default:
            PACKL_ERROR_LOC(self->filename, node.loc, "unexpected token found");
    }
}

void packl_generate_global_statement(PACKL_Compiler *c, PACKL_File *self, Node node, size_t indent) {
    switch(node.kind) {
        case NODE_KIND_USE:
            return packl_generate_use_node(c, self, node, indent);
        case NODE_KIND_VAR_DECLARATION:
            return packl_generate_var_dec_node(c, self, node, indent);
        case NODE_KIND_PROC_DEF:
            return packl_generate_proc_def_code(c, self, node, indent);
        case NODE_KIND_FUNC_DEF:
            return packl_generate_func_def_code(c, self, node, indent);
        case NODE_KIND_METHOD_DEF:
            return packl_generate_method_def_code(c, self, node, indent);
        case NODE_KIND_CLASS:
            return packl_handle_class_definition(c, self, node, indent);
        default:
            PACKL_ERROR_LOC(self->filename, node.loc, "unexpected token, consider starting a definition by a procedure, a function, a global variable declarations");
    }
}


void packl_generate_statements(PACKL_Compiler *c, PACKL_File *self, AST nodes, size_t indent) {
    for (size_t i = 0; i < nodes.count; ++i) {
        Node node = nodes.items[i];
        packl_generate_statement(c, self, node, indent);
    }
}

void packl_generate_global_statements(PACKL_Compiler *c, PACKL_File *self, AST nodes, size_t indent) {
    for (size_t i = 0; i < nodes.count; ++i) {
        Node node = nodes.items[i];
        packl_generate_global_statement(c, self, node, indent);
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

    packl_generate_global_statements(c, self, self->ast, 0);

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
