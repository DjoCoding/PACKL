#include "packl-printer.h"

char *token_kinds[] = {
    "id",
    "native",

    "str",
    "int",
    
    ";",
    ":",
    ",",

    "[",
    "]",
    "(",
    ")",
    "{",
    "}",

    "=",

    "+",
    "++",
    "-",
    "--",
    "*",
    "/",
    "%",
    
    "==",
    "<",
    ">",
    "<=",
    ">=",
    "!",
    "!=",

    "proc",
    
    "func",
    "return",
    
    "var",
    
    "if",
    "else",
    
    "while",
    
    "for",
    "in",

    "use",
    "as",
    
    "or",
    "and",
    "xor",
    
    "array",
    "type-int",
    "type-str",
    "type-ptr",

    "end",
};

void packl_print_expr(Expression expr, size_t indent);
void packl_print_mod_call(Mod_Call mod_call, size_t indent);
void packl_print_func_call(Func_Call func_call, size_t indent);
void packl_print_ast_nodes(AST ast, size_t indent);

void print_indent(size_t indent) {
    for (size_t i = 0; i < indent; ++i) { printf("  "); }
}

void packl_print_operator(Operator op, size_t indent) {    
    switch(op) {
        case OP_PLUS:
            printf("+\n");
            break;
        case OP_MINUS:
            printf("-\n");
            break;
        case OP_MUL:
            printf("*\n");
            break;
        case OP_DIV: 
            printf("/\n");
            break;
        case OP_MOD: 
            printf("mod\n");
            break;
        case OP_L:
            printf("<\n");
            break;
        case OP_G:
            printf(">\n");
            break;
        case OP_LE:
            printf("<=\n");
            break;
        case OP_GE:
            printf(">=\n");
            break;
        case OP_EQ:
            printf("==\n");
            break;
        case OP_AND:
            printf("and\n");
            break;
        case OP_OR:
            printf("or\n");
            break;
        case OP_XOR:
            printf("xor\n");
            break;
        case OP_NOT:
            printf("!\n");
            break;
        case OP_NE:
            printf("!=");
            break;
        case OP_INC:
            printf("++\n");
            break;
        case OP_DEC:
            printf("--\n");
            break;
        default:
            ASSERT(false, "unreachable"); 
    }
}

char *packl_token_kind_stringify(Token_Kind kind) {
    return token_kinds[kind];
} 

void packl_print_token(Token tok) {
    char *token_kind_str = packl_token_kind_stringify(tok.kind);

    printf("%s\t", token_kind_str);
    if (tok.text.count == 0) { printf("\n"); return; }
    printf(SV_FMT "\n", SV_UNWRAP(tok.text));
}

void packl_print_tokens(Tokens toks) {
    for (size_t i = 0; i < toks.count; ++i) {
        packl_print_token(toks.items[i]);
    }
}

void packl_print_basic_type(Type type, size_t indent) {
    print_indent(indent);
    if(type == PACKL_TYPE_INT) {
        printf("int-type\n");
        return;
    }

    if (type == PACKL_TYPE_STR) {
        printf("str-type\n");
        return;
    }
    
    if (type == PACKL_TYPE_PTR) {
        printf("ptr-type\n");
        return;
    }

    ASSERT(false, "unreachable");
}

void packl_print_type(PACKL_Type type, size_t indent);

void packl_print_array_type(Array_Type type, size_t indent) {
    print_indent(indent);
    printf("type: array\n");
    packl_print_type(*type.type, indent + 1);
    print_indent(indent);
    printf("size:\n");
    packl_print_expr(type.size, indent + 1);
}

void packl_print_type(PACKL_Type type, size_t indent) {
    if(type.kind == PACKL_TYPE_BASIC) {
        packl_print_basic_type(type.as.basic, indent);
        return;
    }


    if (type.kind == PACKL_TYPE_ARRAY) {
        packl_print_array_type(type.as.array, indent);
        return;
    }

    ASSERT(false, "unreachable");
}

void packl_print_expr_arr(Expr_Arr exprs, size_t indent) {
    print_indent(indent);
    printf("array:\n");
    for(size_t i = 0; i < exprs.count; ++i) {
        packl_print_expr(exprs.items[i], indent + 1);
    }
}

void packl_print_expr_arr_index(Expr_Arr_Index arr_index, size_t indent) {
    print_indent(indent);
    printf("array indexing:\n");

    print_indent(indent + 1);
    printf("array name: `" SV_FMT "`\n", SV_UNWRAP(arr_index.name));

    packl_print_expr(*arr_index.index, indent + 1);
}

void packl_print_string_expr(String_View value, size_t indent) {
    print_indent(indent);
    printf("string: \"" SV_FMT "\"\n", SV_UNWRAP(value));
}

void packl_print_integer_expr(int64_t value, size_t indent) {
    print_indent(indent);
    printf("integer: %ld\n", value);
}

void packl_print_id_expr(String_View value, size_t indent) {
    print_indent(indent);
    printf("identifier: `" SV_FMT "`\n", SV_UNWRAP(value));
}

void packl_print_binop_expr(Expression expr, size_t indent) {
    print_indent(indent);
    packl_print_operator(expr.as.bin.op, indent + 1);
    packl_print_expr(*expr.as.bin.lhs, indent + 2);
    packl_print_expr(*expr.as.bin.rhs, indent + 2);
}

void packl_print_preunary_expr(Expr_Unary_Op unary, size_t indent) {
    print_indent(indent);
    printf("pre-unary:\n");

    print_indent(indent + 1);
    packl_print_operator(unary.op, indent + 1);

    if (unary.operand) {
        packl_print_expr(*unary.operand, indent + 2);
    }
}

void packl_print_postunary_expr(Expr_Unary_Op unary, size_t indent) {
    print_indent(indent);
    printf("post-unary:\n");

    print_indent(indent + 1);
    packl_print_operator(unary.op, indent + 1);

    if (unary.operand) {
        packl_print_expr(*unary.operand, indent + 2);
    }
}

void packl_print_expr(Expression expr, size_t indent) {
    switch(expr.kind) {
        case EXPR_KIND_STRING:
            return packl_print_string_expr(expr.as.value, indent);
        case EXPR_KIND_INTEGER:
            return packl_print_integer_expr(expr.as.integer, indent);
        case EXPR_KIND_ID:
            return packl_print_id_expr(expr.as.value, indent);
        case EXPR_KIND_FUNC_CALL:
            return packl_print_func_call(*expr.as.func, indent);
        case EXPR_KIND_BIN_OP:
            return packl_print_binop_expr(expr, indent);
        case EXPR_KIND_NATIVE_CALL:
            return packl_print_func_call(*expr.as.func, indent);
        case EXPR_KIND_MOD_CALL:
            return packl_print_mod_call(*expr.as.mod, indent);
        case EXPR_KIND_ARRAY:
            return packl_print_expr_arr(expr.as.arr, indent);
        case EXPR_KIND_ARRAY_INDEXING:
            return packl_print_expr_arr_index(expr.as.arr_index, indent);
        case EXPR_KIND_PRE_UNARY_OP:
            return packl_print_preunary_expr(expr.as.unary, indent);
        case EXPR_KIND_POST_UNARY_OP:
            return packl_print_postunary_expr(expr.as.unary, indent);
        default:
            ASSERT(false, "unreachable");
    }        
}

void packl_print_arg(PACKL_Arg arg, size_t indent) {
    packl_print_expr(arg.expr, indent);
}

void packl_print_args(PACKL_Args args, size_t indent) {
    print_indent(indent);
    printf("begin\n");

    for(size_t i = 0; i < args.count; ++i) {
        packl_print_arg(args.items[i], indent + 1);
    }

    print_indent(indent);
    printf("end\n");
}

void packl_print_func_call(Func_Call func_call, size_t indent) {
    print_indent(indent);
    printf("func name: `"SV_FMT"`\n", SV_UNWRAP(func_call.name));

    print_indent(indent + 1);
    printf("args:\n");
    packl_print_args(func_call.args, indent + 2);
}

void packl_print_param(Parameter param, size_t indent) {
    print_indent(indent);
    printf("name: `" SV_FMT "`", SV_UNWRAP(param.name));
    print_indent(indent);
    printf("type:\n");
    packl_print_type(param.type, indent);
}

void packl_print_params(Parameters params, size_t indent) {
    print_indent(indent);
    printf("begin\n");

    for(size_t i = 0; i < params.count; ++i) {
        packl_print_param(params.items[i], indent + 1);
    }

    print_indent(indent);
    printf("end\n");
}

void packl_print_body(AST body, size_t indent) {
    print_indent(indent);
    printf("body:");
    packl_print_ast_nodes(body, indent + 2);
}

void packl_print_proc_def(Proc_Def proc_def, size_t indent) {
    print_indent(indent);
    printf("proc name: `"SV_FMT"`\n", SV_UNWRAP(proc_def.name));

    print_indent(indent + 1);
    printf("params:\n");
    packl_print_params(proc_def.params, indent + 2);

    packl_print_body(*proc_def.body, indent + 1);
}

void packl_print_var_dec(Var_Declaration var_dec, size_t indent) {
    print_indent(indent);
    printf("variable name: `" SV_FMT "`\n", SV_UNWRAP(var_dec.name));

    print_indent(indent);
    printf("variable type:\n");
    packl_print_type(var_dec.type, indent + 1);

    print_indent(indent);
    printf("value:\n");

    packl_print_expr(var_dec.value, indent + 1);
}

void packl_print_else(AST ast, size_t indent) {
    print_indent(indent);
    printf("else:\n");
    packl_print_body(ast, indent + 1);
}

void packl_print_if(If_Statement fi, size_t indent) {
    print_indent(indent);
    printf("condition:\n");
    packl_print_expr(fi.condition, indent + 1);

    packl_print_body(*fi.body, indent + 1);

    if (fi.esle) {
        packl_print_else(*fi.esle, indent);
    }
}

void packl_print_while(While_Statement hwile, size_t indent) {
    print_indent(indent);
    printf("condition:\n");
    packl_print_expr(hwile.condition, indent + 1);
    packl_print_body(*hwile.body, indent + 1);
}

void packl_print_for(For_Statement rof, size_t indent) {
    print_indent(indent);
    printf("iterator: `" SV_FMT "`",  SV_UNWRAP(rof.iter));
    print_indent(indent);
    printf("type:\n");
    packl_print_type(rof.iter_type, indent);
    packl_print_args(rof.args, indent + 1);
    packl_print_body(*rof.body, indent + 1);
}

void packl_print_use(Use use, size_t indent) {
    print_indent(indent);
    printf("file `" SV_FMT "`\n", SV_UNWRAP(use.filename));
    print_indent(indent);
    printf("alias: `" SV_FMT "`\n", SV_UNWRAP(use.alias));
}

void packl_print_mod_call(Mod_Call mod_call, size_t indent) {
    print_indent(indent);
    printf("alias `" SV_FMT "`\n", SV_UNWRAP(mod_call.name));
    if (mod_call.kind == MODULE_CALL_FUNC_CALL) {
        packl_print_func_call(mod_call.as.func_call, indent + 1);
    } else {
        print_indent(indent + 1);
        printf("kind: variable, name: `" SV_FMT "`\n", SV_UNWRAP(mod_call.as.var_name));
    }
}

void packl_print_var_reassign(Var_Reassign var, size_t indent) {
    print_indent(indent);
    printf("variable name: `" SV_FMT "`\n", SV_UNWRAP(var.name));

    if(var.kind == PACKL_TYPE_ARRAY) {
        print_indent(indent);
        printf("type: array\n");
        print_indent(indent);
        printf("index:\n");
        packl_print_expr(var.index, indent + 1);
    } else {
        print_indent(indent);
        printf("type: basic\n");
    }

    print_indent(indent);
    printf("value:\n");
    packl_print_expr(var.expr, indent + 1);
}

void packl_print_func_def(Func_Def func, size_t indent) {
    print_indent(indent);
    printf("function name: `" SV_FMT "`\n", SV_UNWRAP(func.name));

    print_indent(indent);
    printf("return type\n");
    packl_print_type(func.return_type, indent);
    packl_print_body(*func.body, indent + 1);
}

void packl_print_native_call_node(Node node, size_t indent) {
    print_indent(indent);
    printf("node kind: native call\n");

    packl_print_func_call(node.as.func_call, indent + 1);
}

void packl_print_func_call_node(Node node, size_t indent) {
    print_indent(indent);
    printf("node kind: function call\n");
    packl_print_func_call(node.as.func_call, indent + 1);
}

void packl_print_proc_def_node(Node node, size_t indent) {
    print_indent(indent);
    printf("node kind: procedure defintion\n");
    packl_print_proc_def(node.as.proc_def, indent + 1);
}

void packl_print_var_dec_node(Node node, size_t indent) {
    print_indent(indent);
    printf("node kind: variable declaration\n");
    packl_print_var_dec(node.as.var_dec, indent + 1);
}

void packl_print_if_node(Node node, size_t indent) {
    print_indent(indent);
    printf("node kind: if statement\n");
    packl_print_if(node.as.fi, indent + 1);
}

void packl_print_while_node(Node node, size_t indent) {
    print_indent(indent);
    printf("node kind: while statement\n");
    packl_print_while(node.as.hwile, indent + 1);
}

void packl_print_var_reassign_node(Node node, size_t indent) {
    print_indent(indent);
    printf("node kind: variable reassignement\n");
    packl_print_var_reassign(node.as.var, indent + 1);
}

void packl_print_func_def_node(Node node, size_t indent) {
    print_indent(indent);
    printf("node kind: function definition\n");
    packl_print_func_def(node.as.func_def, indent + 1);
}

void packl_print_return_node(Node node, size_t indent) {
    print_indent(indent);
    printf("return:\n");
    packl_print_expr(node.as.ret, indent + 1);
}

void packl_print_for_node(Node node, size_t indent) {
    print_indent(indent);
    printf("node kind: for statement\n");
    packl_print_for(node.as.rof, indent + 1);
}

void packl_print_use_node(Node node, size_t indent) {
    print_indent(indent);
    printf("node kind: use\n");
    packl_print_use(node.as.use, indent + 1);
}

void packl_print_mod_call_node(Node node, size_t indent) {
    print_indent(indent);
    printf("node kind: module call\n");
    packl_print_mod_call(node.as.mod_call, indent + 1);
}

void packl_print_ast_node(Node node, size_t indent) {
    switch(node.kind) {
        case NODE_KIND_NATIVE_CALL:
            return packl_print_native_call_node(node, indent);
        case NODE_KIND_FUNC_CALL:
            return packl_print_func_call_node(node, indent);
        case NODE_KIND_PROC_DEF:
            return packl_print_proc_def_node(node, indent);
        case NODE_KIND_VAR_DECLARATION:
            return packl_print_var_dec_node(node, indent);
        case NODE_KIND_IF:
            return packl_print_if_node(node, indent);
        case NODE_KIND_WHILE:
            return packl_print_while_node(node, indent);
        case NODE_KIND_VAR_REASSIGN:
            return packl_print_var_reassign_node(node, indent);
        case NODE_KIND_FUNC_DEF:
            return packl_print_func_def_node(node, indent);
        case NODE_KIND_RETURN:
            return packl_print_return_node(node, indent);
        case NODE_KIND_FOR:
            return packl_print_for_node(node, indent);
        case NODE_KIND_USE:
            return packl_print_use_node(node, indent);
        case NODE_KIND_MOD_CALL:
            return packl_print_mod_call_node(node, indent);
        default:
            ASSERT(false, "unreachable");
    }
}

void packl_print_ast_nodes(AST ast, size_t indent) {
    for (size_t i = 0; i < ast.count; ++i) {
        printf("\n");
        packl_print_ast_node(ast.items[i], indent);
    }
}

void packl_print_ast(AST ast) {
    printf("Program: \n");
    packl_print_ast_nodes(ast, 0); 
}