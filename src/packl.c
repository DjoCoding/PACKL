#include "packl.h"

PACKL packl_init(char *input, char *output) {
    PACKL packl = {0};
    packl.filename = input;
    packl.output = output;
    return packl;
} 

void packl_destroy_ast(AST ast);

void packl_destroy_node(Node node) {
    if (node.kind == NODE_KIND_FUNC_CALL) {
        free(node.as.func_call.args.items);
    } else if (node.kind == NODE_KIND_NATIVE_CALL) {
        free(node.as.func_call.args.items);
    } else if (node.kind == NODE_KIND_PROC_DEF) {
        if (node.as.proc_def.params.size != 0) free(node.as.proc_def.params.items);        
        packl_destroy_ast(*node.as.proc_def.body);
        free(node.as.proc_def.body);
    } else if (node.kind == NODE_KIND_VAR_DECLARATION) {
        
    } else { ASSERT(false, "unreachable"); }
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