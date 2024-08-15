#include "packl-parser.h"

char *token_kinds_str[] = {
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


Node packl_parser_parse_statement(PACKL *self);

Token packl_parser_peek(PACKL *self, size_t ahead) {
    return self->tokens.items[self->parser.current + ahead];
}

void packl_parser_advance(PACKL *self) {
    self->parser.current++;
}

// get the number of tokens that can be consumed
size_t packl_get_tokens_number(PACKL *self) {
    return self->tokens.count - self->parser.current;
}

bool packl_parser_eot(PACKL *self) {
    return (self->tokens.count <= self->parser.current);
}

void packl_parser_expect(PACKL *self, Token_Kind kind, Token *token) {
    if (peot(self)) { PACKL_ERROR(self->filename, "expected token of kind %s but end of tokens found", token_kinds_str[kind]); }
        
    if (ppeek(self).kind != kind) {
        PACKL_ERROR(self->filename, "expected token of kind %s but " SV_FMT "found", token_kinds_str[kind], SV_UNWRAP(ppeek(self).text));
    }

    if (token) *token = ppeek(self);
    padv(self);
}

Func_Call_Arg packl_parser_parse_func_arg(PACKL *self) {
    Func_Call_Arg arg = {0};
    
    Token token = ppeek(self);
    padv(self);
    
    arg.value = token.text;

    if (token.kind == TOKEN_KIND_IDENTIFIER) {
        arg.type = ARG_TYPE_ID;
        return arg;
    }

    if (token.kind == TOKEN_KIND_STRING_LIT) {
        arg.type = ARG_TYPE_STRING_LIT;
        return arg;
    }

    if (token.kind == TOKEN_KIND_INTEGER_LIT) {
        arg.type = ARG_TYPE_INT_LIT;
        return arg;
    }

    PACKL_ERROR(self->filename, "expected a function argument but token of kind %s found", token_kinds_str[token.kind]);
}


Func_Call_Args packl_parser_parse_func_args(PACKL *self) {
    Func_Call_Args args = {0};
    DA_INIT(&args, sizeof(Func_Call_Arg));

    while (!peot(self)) {
        if (ppeek(self).kind == TOKEN_KIND_CLOSE_PARENT) { break; }
        Func_Call_Arg arg = packl_parser_parse_func_arg(self);
        DA_APPEND(&args, arg);
        // TODO: handle more than just one argument in the func call args
    }

    return args;
}

Func_Call packl_parser_parse_func_call(PACKL *self) {
    Func_Call func_call = {0};
    Token token = ppeek(self);

    func_call.name = token.text;
    
    // consume the function name
    padv(self);

    // parsing arguments
    pexp(self, TOKEN_KIND_OPEN_PARENT, NULL);
    func_call.args = packl_parser_parse_func_args(self);
    pexp(self, TOKEN_KIND_CLOSE_PARENT, NULL);

    return func_call;
}

Parameter packl_parser_parse_param(PACKL *self) {
    Parameter param = {0};

    Token token = {0};

    // for the param name
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    param.name = token.text;

    pexp(self, TOKEN_KIND_COLON, NULL);

    // for the type
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    param.type = token.text;

    return param;
}

Parameters packl_parser_parse_params(PACKL *self) {
    Parameters params = {0};
    DA_INIT(&params, sizeof(Parameter));

    while (!peot(self)) {
        if (ppeek(self).kind == TOKEN_KIND_CLOSE_PARENT) { break; }
        Parameter param = packl_parser_parse_param(self);
        DA_APPEND(&params, param);
        if (ppeek(self).kind == TOKEN_KIND_COMMA) { padv(self); }
    }

    return params;
}

AST packl_parser_parse_body(PACKL *self) {
    AST ast = {0};
    DA_INIT(&ast, sizeof(Node));

    while (!peot(self)) {
        if (ppeek(self).kind == TOKEN_KIND_CLOSE_CURLY_BRACE) { break; }
        Node node = pstmt(self);
        DA_APPEND(&ast, node);
        if (ppeek(self).kind == TOKEN_KIND_SEMI_COLON) { padv(self); }
    }

    return ast;
}

Proc_Def packl_parser_parse_proc_def(PACKL *self) {
    Proc_Def proc = {0};
    
    // consuming the `proc` token 
    padv(self);

    Token token = {0};
    
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    proc.name = token.text;

    if (ppeek(self).kind == TOKEN_KIND_OPEN_PARENT) {
        padv(self);
        proc.params = packl_parser_parse_params(self);
        pexp(self, TOKEN_KIND_CLOSE_PARENT, NULL);
    }

    pexp(self, TOKEN_KIND_OPEN_CURLY_BRACE, &token);

    proc.body = malloc(sizeof(AST));
    *proc.body = packl_parser_parse_body(self);
    
    pexp(self, TOKEN_KIND_CLOSE_CURLY_BRACE, &token);

    return proc;
}

Node packl_parser_parse_statement(PACKL *self) {
    Node node = {0};

    Token_Kind kind = ppeek(self).kind;

    if (kind == TOKEN_KIND_NATIVE) {
        if (packl_get_tokens_number(self) < 2) {
            PACKL_ERROR(self->filename, "expected more tokens after " SV_FMT, SV_UNWRAP(ppeek(self).text));
        }
            
        if (ppeek_(self, 1).kind == TOKEN_KIND_OPEN_PARENT) {
            node.kind = NODE_KIND_NATIVE_CALL;
            node.as.func_call =  packl_parser_parse_func_call(self);
            return node;
        }

        PACKL_ERROR(self->filename, "expected `(` after the native call " SV_FMT "\n", SV_UNWRAP(ppeek(self).text));
    }

    if (kind == TOKEN_KIND_IDENTIFIER) {
        if (packl_get_tokens_number(self) < 2) {
            PACKL_ERROR(self->filename, "expected more tokens after " SV_FMT, SV_UNWRAP(ppeek(self).text));
        }
            
        if (ppeek_(self, 1).kind == TOKEN_KIND_OPEN_PARENT) {
            node.kind = NODE_KIND_FUNC_CALL;
            node.as.func_call = packl_parser_parse_func_call(self);
            return node;
        }

        ASSERT(false, "unreachable");
    }

    if (kind == TOKEN_KIND_PROC) {
        node.kind = NODE_KIND_PROC_DEF;
        node.as.proc_def = packl_parser_parse_proc_def(self);
        return node;
    }

    ASSERT(false, "unreachable");
}

AST packl_parser_parse_statements(PACKL *self) {
    AST ast = {0};
    DA_INIT(&ast, sizeof(Node));

    while (!peot(self)) {
        Node node = pstmt(self);
        DA_APPEND(&ast, node);
        if (ppeek(self).kind == TOKEN_KIND_SEMI_COLON) { padv(self); }
    }

    return ast;
}

void packl_parser_parse(PACKL *self) {
    self->ast = packl_parser_parse_statements(self);
}
