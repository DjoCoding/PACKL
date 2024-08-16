#include "packl.h"

PACKL packl_init(char *input, char *output) {
    PACKL packl = {0};
    packl.filename = input;
    packl.output = output;
    return packl;
} 

void packl_destroy_ast(AST ast);

void packl_destroy_func_call(Func_Call call);

void packl_destroy_expr(Expression expr) {
    if (expr.kind == EXPR_KIND_BIN_OP) {
        packl_destroy_expr(*expr.as.bin.lhs);
        packl_destroy_expr(*expr.as.bin.rhs);
        free(expr.as.bin.lhs);
        free(expr.as.bin.rhs);
    } else if (expr.kind == EXPR_KIND_FUNC_CALL) {
        packl_destroy_func_call(*expr.as.func);
        free(expr.as.func);
    } 
}

void packl_destroy_native_call(Func_Call call) {
    packl_destroy_func_call(call);
}

void packl_destroy_func_call(Func_Call call) {
    for(size_t i = 0; i < call.args.count; ++i) {
        Expression arg = call.args.items[i].expr;
        packl_destroy_expr(arg);
    }
    free(call.args.items);
}

void packl_destroy_proc_def(Proc_Def proc) {
    free(proc.params.items);
    packl_destroy_ast(*proc.body);
    free(proc.body);
}

void packl_destroy_var_dec(Var_Declaration var) {
    packl_destroy_expr(var.value);
}

void packl_destroy_if(If_Statement fi) {
    packl_destroy_expr(fi.condition);

    packl_destroy_ast(*fi.body);
    free(fi.body);

    if(fi.esle) { packl_destroy_ast(*fi.esle); free(fi.esle); }
}

void packl_destroy_node(Node node) {
    switch(node.kind) {
        case NODE_KIND_NATIVE_CALL:
            packl_destroy_native_call(node.as.func_call);
            break;
        case NODE_KIND_FUNC_CALL:
            packl_destroy_func_call(node.as.func_call);
            break;
        case NODE_KIND_PROC_DEF:
            packl_destroy_proc_def(node.as.proc_def);
            break;
        case NODE_KIND_VAR_DECLARATION:
            packl_destroy_var_dec(node.as.var_dec);
            break;
        case NODE_KIND_IF:
            packl_destroy_if(node.as.fi);
            break;
        default:
            ASSERT(false, "unreachable");
    }
}

void packl_destroy_ast(AST ast) {
    for (size_t i = 0; i < ast.count; ++i) {
        Node node = ast.items[i];
        packl_destroy_node(node);
    }
    if (ast.size != 0)  free(ast.items);
}

void packl_destroy(PACKL *self) {
    if (self->tokens.size != 0)        free(self->tokens.items);
    if (self->lexer.source.count != 0) free(self->lexer.source.content);    
    packl_destroy_ast(self->ast);    
}

void packl_load_file(PACKL *self) {
    self->lexer.source = sv_from_file(self->filename);
    self->lexer.current = 0;
    self->lexer.loc = (Location) {1, 1};
} 