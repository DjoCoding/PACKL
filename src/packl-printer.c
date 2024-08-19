#include "packl-printer.h"

char *token_kinds[] = {
    "id",
    "native",

    "str",
    "int",
    
    ";",
    ":",
    ",",

    "(",
    ")",
    "{",
    "}",
    "=",
    "+",
    "-",
    "*",
    "/",
    "%",
    "<",
    ">",

    "proc",
    "func",
    "return",
    "var",
    "if",
    "else",
    "while",
    "for",
    "in",

    "end",
};

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
        case OP_LESS:
            printf("<\n");
            break;
        case OP_GREATER:
            printf(">\n");
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

void packl_print_expr(Expression expr, size_t indent) {
    if (expr.kind == EXPR_KIND_STRING) {
        print_indent(indent);
        printf("string: \"" SV_FMT "\"\n", SV_UNWRAP(expr.as.value));
    } else if (expr.kind == EXPR_KIND_INTEGER) {
        print_indent(indent);
        printf("integer: " SV_FMT "\n", SV_UNWRAP(expr.as.value));
    } else if (expr.kind == EXPR_KIND_ID) {
        print_indent(indent);
        printf("identifier: `" SV_FMT "`\n", SV_UNWRAP(expr.as.value));
    } else if (expr.kind == EXPR_KIND_FUNC_CALL) {
        packl_print_func_call(*expr.as.func, indent);
    } else if (expr.kind == EXPR_KIND_BIN_OP) {
        print_indent(indent);
        packl_print_operator(expr.as.bin.op, indent + 1);
        packl_print_expr(*expr.as.bin.lhs, indent + 2);
        packl_print_expr(*expr.as.bin.rhs, indent + 2);
    } else if (expr.kind == EXPR_KIND_NATIVE_CALL) {
        packl_print_func_call(*expr.as.func, indent);
    } else { ASSERT(false, "unreachable"); }
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
    printf(", type: `" SV_FMT "` \n", SV_UNWRAP(param.type));
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
    printf("variable type: `" SV_FMT "`\n", SV_UNWRAP(var_dec.type));

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
    printf("iterator: `" SV_FMT "`, type: `" SV_FMT "`\n", SV_UNWRAP(rof.iter), SV_UNWRAP(rof.iter_type));
    packl_print_args(rof.args, indent + 1);
    packl_print_body(*rof.body, indent + 1);
}

void packl_print_var_reassign(Var_Reassign var, size_t indent) {
    print_indent(indent);
    printf("variable name: `" SV_FMT "`\n", SV_UNWRAP(var.name));

    print_indent(indent);
    printf("value:\n");
    packl_print_expr(var.expr, indent + 1);
}

void packl_print_func_def(Func_Def func, size_t indent) {
    print_indent(indent);
    printf("function name: `" SV_FMT "`\n", SV_UNWRAP(func.name));

    print_indent(indent);
    printf("return type: `" SV_FMT "`\n", SV_UNWRAP(func.return_type));

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