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

    "proc",
};

void packl_print_ast_nodes(AST ast, size_t indent);

void print_indent(size_t indent) {
    for (size_t i = 0; i < indent; ++i) { printf("  "); }
}

char *packl_token_kind_stringify(Token_Kind kind) {
    return token_kinds[kind];
} 

void packl_print_token(Token tok) {
    ASSERT(COUNT_TOKEN_KINDS == 6, "you added more token kinds probably, update the `packl_print_token` function");
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

void packl_print_func_call_arg(Func_Call_Arg arg, size_t indent) {
    print_indent(indent);
    
    if (arg.type == ARG_TYPE_STRING_LIT) {
        printf("type: string");
    } else if (arg.type == ARG_TYPE_INT_LIT) {
        printf("type: integer");
    } else if (arg.type == ARG_TYPE_ID) {
        printf("type: identifer");
    } else { ASSERT(false, "unreachable"); }

    printf(", value: " SV_FMT "\n", SV_UNWRAP(arg.value));
}

void packl_print_func_call_args(Func_Call_Args args, size_t indent) {
    print_indent(indent);
    printf("begin\n");

    for(size_t i = 0; i < args.count; ++i) {
        packl_print_func_call_arg(args.items[i], indent + 1);
    }

    print_indent(indent);
    printf("end\n");
}

void packl_print_func_call(Func_Call func_call, size_t indent) {
    print_indent(indent);
    printf("func name: "SV_FMT"\n", SV_UNWRAP(func_call.name));
    print_indent(indent + 1);
    printf("args:\n");
    packl_print_func_call_args(func_call.args, indent + 2);
}

void packl_print_param(Parameter param, size_t indent) {
    print_indent(indent);
    printf("name: " SV_FMT, SV_UNWRAP(param.name));
    printf(", type: " SV_FMT "\n", SV_UNWRAP(param.type));
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
    printf("body: ");
    packl_print_ast_nodes(body, indent + 1);
}

void packl_print_proc_def(Proc_Def proc_def, size_t indent) {
    print_indent(indent);
    printf("proc name: "SV_FMT"\n", SV_UNWRAP(proc_def.name));

    print_indent(indent + 1);
    printf("params:\n");
    packl_print_params(proc_def.params, indent + 2);

    packl_print_body(*proc_def.body, indent + 1);
}

void packl_print_ast_node(Node node, size_t indent) {
    print_indent(indent);

    if (node.kind == NODE_KIND_NATIVE_CALL) {
        printf("node kind: native call\n");
        packl_print_func_call(node.as.func_call, indent + 1);
    } else if (node.kind == NODE_KIND_FUNC_CALL) {
        printf("node kind: function call\n");
        packl_print_func_call(node.as.func_call, indent + 1);
    } else if (node.kind == NODE_KIND_PROC_DEF){
        printf("node kind: procedure definition\n");
        packl_print_proc_def(node.as.proc_def, indent + 1);
    } else { ASSERT(false, "unreachable"); }
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