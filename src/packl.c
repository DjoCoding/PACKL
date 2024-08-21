#include "packl.h"

PACKL_Compiler packl_init(char *input, char *output) {
    PACKL_Compiler c = {0};
    c.output = output;
    char *fullpath = malloc(sizeof(char) * (FILENAME_MAX));
    
    if (!realpath(input, fullpath)) {
        fprintf(stderr, "could not find the file %s\n", input);
        exit(1);
    }

    c.root_file = packl_init_file(input, fullpath);
    return c;
} 

void packl_generate_code(PACKL_Compiler *c) {
    c->f = fopen(c->output, "w");
    if(!c->f) {
        PACKL_ERROR(c->root_file.filename, "could not open the output file for writing");
    }    
    codegen(c, &c->root_file);
    fclose(c->f);
}

void packl_compile(PACKL_Compiler *c) {
    lex(&c->root_file);
    parse(&c->root_file);
    packl_generate_code(c);
}

void packl_destroy_ast(AST ast);

void packl_destroy_func_call(Func_Call call);


void packl_destroy_type(PACKL_Type type) {
    if(type.kind == PACKL_TYPE_BASIC) { return; }
    if(type.kind == PACKL_TYPE_ARRAY) {
        packl_destroy_type(*type.as.array.type);
        free(type.as.array.type);
        return;
    }
    ASSERT(false, "unreachable");
}

void packl_destroy_expr(Expression expr) {
    if (expr.kind == EXPR_KIND_BIN_OP) {
        packl_destroy_expr(*expr.as.bin.lhs);
        packl_destroy_expr(*expr.as.bin.rhs);
        free(expr.as.bin.lhs);
        free(expr.as.bin.rhs);
    } else if (expr.kind == EXPR_KIND_FUNC_CALL) {
        packl_destroy_func_call(*expr.as.func);
        free(expr.as.func);
    } else if (expr.kind == EXPR_KIND_ARRAY) {
        for(size_t i = 0; i < expr.as.arr.count; ++i) {
            packl_destroy_expr(expr.as.arr.items[i]);
        }
        free(expr.as.arr.items);
    }  else if (expr.kind == EXPR_KIND_ARRAY_INDEXING) {
        packl_destroy_expr(*expr.as.arr_index.index);
        free(expr.as.arr_index.index);
    } else if (expr.kind == EXPR_KIND_ARRAY) {
        for(size_t i = 0; i < expr.as.arr.count; ++i) {
            packl_destroy_expr(expr.as.arr.items[i]);
        }
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
    packl_destroy_ast(*proc.body);
    free(proc.params.items);
    free(proc.body);
}

void packl_destroy_var_dec(Var_Declaration var) {
    packl_destroy_expr(var.value);
    packl_destroy_type(var.type);
}

void packl_destroy_if(If_Statement fi) {
    packl_destroy_expr(fi.condition);

    packl_destroy_ast(*fi.body);
    free(fi.body);

    if(fi.esle) { packl_destroy_ast(*fi.esle); free(fi.esle); }
}

void packl_destroy_while(While_Statement hwile) {
    packl_destroy_expr(hwile.condition);

    packl_destroy_ast(*hwile.body);
    free(hwile.body);
}

void packl_destroy_var_reassign(Var_Reassign var) {
    if (var.kind == PACKL_TYPE_ARRAY) {
        packl_destroy_expr(var.index);
    }
    
    packl_destroy_expr(var.expr);
}

void packl_destroy_func_def(Func_Def func) {
    packl_destroy_ast(*func.body);
    free(func.params.items);
    free(func.body);
}

void packl_destroy_for(For_Statement rof) {
    packl_destroy_ast(*rof.body);
    free(rof.args.items);
    free(rof.body);
} 

void packl_destroy_mod_call(Mod_Call mod_call) {
    if (mod_call.kind == MODULE_CALL_FUNC_CALL) {
        packl_destroy_func_call(mod_call.as.func_call);
    }
}

void packl_destroy_node(Node node) {
    switch(node.kind) {
        case NODE_KIND_NATIVE_CALL:
            return packl_destroy_native_call(node.as.func_call);
        case NODE_KIND_FUNC_CALL:
            return packl_destroy_func_call(node.as.func_call);
        case NODE_KIND_PROC_DEF:
            return packl_destroy_proc_def(node.as.proc_def);
        case NODE_KIND_VAR_DECLARATION:
            return packl_destroy_var_dec(node.as.var_dec);
        case NODE_KIND_IF:
            return packl_destroy_if(node.as.fi);
        case NODE_KIND_WHILE:
            return packl_destroy_while(node.as.hwile);
        case NODE_KIND_VAR_REASSIGN:
            return packl_destroy_var_reassign(node.as.var);
        case NODE_KIND_FUNC_DEF:
            return packl_destroy_func_def(node.as.func_def);
        case NODE_KIND_RETURN:
            return packl_destroy_expr(node.as.ret);
        case NODE_KIND_FOR:
            return packl_destroy_for(node.as.rof);
        case NODE_KIND_USE:
            return;
        case NODE_KIND_MOD_CALL:
            return packl_destroy_mod_call(node.as.mod_call);
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

void packl_file_destroy(PACKL_File *self) {
    if (self->tokens.size != 0)        free(self->tokens.items);
    if (self->lexer.source.count != 0) free(self->lexer.source.content);    
    packl_destroy_ast(self->ast);    
    
    // pop the global context
    if (self->contexts.count) { packl_pop_context(self); }

    if (self->contexts.size) {
        packl_remove_contexts(self);
    }
    
    for(size_t i = 0; i < self->used_files.count; ++i) {
        packl_file_destroy(&self->used_files.items[i]);
        free(self->used_files.items[i].filename);
    }
}

void packl_destroy(PACKL_Compiler *c) {
    packl_file_destroy(&c->root_file);
}