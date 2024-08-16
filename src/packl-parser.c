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
    "=",
    "+",
    "-",
    "*",
    "/",
    "%",

    "proc",
    "var",
};


Expression packl_parser_parse_additive_expr(PACKL *self);
Expression packl_parser_parse_expr(PACKL *self);
Node packl_parser_parse_statement(PACKL *self);

Operator packl_get_operator(PACKL *self, Token_Kind kind) {
    if (kind == TOKEN_KIND_PLUS) { return OP_PLUS; }
    if (kind == TOKEN_KIND_MINUS) { return OP_MINUS; }
    if (kind == TOKEN_KIND_STAR) { return OP_MUL; }
    if (kind == TOKEN_KIND_SLASH) { return OP_DIV; }
    if (kind == TOKEN_KIND_MOD) { return OP_MOD; }
    PACKL_ERROR(self->filename, "expected operator type but `%s` found", token_kinds_str[kind]);
}

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
    arg.expr = packl_parser_parse_expr(self);
    return arg;
}


Func_Call_Args packl_parser_parse_func_args(PACKL *self) {
    Func_Call_Args args = {0};
    DA_INIT(&args, sizeof(Func_Call_Arg));

    if (ppeek(self).kind == TOKEN_KIND_CLOSE_PARENT) { return args; }
    
    while (!peot(self)) {
        Func_Call_Arg arg = packl_parser_parse_func_arg(self);
        DA_APPEND(&args, arg);

        if (ppeek(self).kind == TOKEN_KIND_COMMA) { padv(self); continue; }
        else if (ppeek(self).kind == TOKEN_KIND_CLOSE_PARENT) { break; }
        else { PACKL_ERROR(self->filename, "unexpected token found `" SV_FMT "`", SV_UNWRAP(ppeek(self).text)); }
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
        
        if (node.kind == NODE_KIND_PROC_DEF) { PACKL_ERROR(self->filename, "can not define a procedure inside one"); }
        
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

Var_Declaration packl_parser_parse_var_dec(PACKL *self) {
    Var_Declaration var_dec = {0};
    
    // consuming the `var` token
    padv(self);

    Token token = {0};
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    var_dec.name = token.text;

    pexp(self, TOKEN_KIND_COLON, NULL);
    
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    var_dec.type = token.text;

    pexp(self, TOKEN_KIND_EQUAL, NULL);

    var_dec.value = packl_parser_parse_expr(self);

    return var_dec;
}

Expression get_bin_operation(Expression lhs, Expression rhs, Operator op) {
    Expression expr = {0};
    expr.kind = EXPR_KIND_BIN_OP;
    
    expr.as.bin.lhs = malloc(sizeof(Expression));
    expr.as.bin.rhs = malloc(sizeof(Expression));
    
    *expr.as.bin.lhs = lhs;
    *expr.as.bin.rhs = rhs;
    expr.as.bin.op = op;
    
    return expr;
}

Expression get_func_call_expr(Func_Call func) {
    Expression expr = {0};
    expr.kind = EXPR_KIND_FUNC_CALL;
    expr.as.func = malloc(sizeof(Func_Call));
    *expr.as.func = func;
    return expr;
}

int ismultiplicative(Token_Kind kind) {
    return (kind == TOKEN_KIND_STAR || kind == TOKEN_KIND_SLASH || kind == TOKEN_KIND_MOD);
}

int isadditive(Token_Kind kind) {
    return (kind == TOKEN_KIND_PLUS || kind == TOKEN_KIND_MINUS);
}

Expression packl_parser_parse_primary_expr(PACKL *self) {
    if (peot(self)) { PACKL_ERROR(self->filename, "expected more tokens for the expression"); }

    Token token = ppeek(self);
    
    if (token.kind == TOKEN_KIND_INTEGER_LIT) {
        padv(self);
        return (Expression) { .kind = EXPR_KIND_INTEGER, .as.value = token.text };
    }

    if (token.kind == TOKEN_KIND_STRING_LIT) {
        padv(self);
        return (Expression) { .kind = EXPR_KIND_STRING, .as.value = token.text };
    }

    if (token.kind == TOKEN_KIND_IDENTIFIER) {
        if (packl_get_tokens_number(self) < 2) { PACKL_ERROR(self->filename, "expected a `;` but end of tokens found"); }

        if (ppeek_(self, 1).kind == TOKEN_KIND_OPEN_PARENT)  {
            Func_Call func = packl_parser_parse_func_call(self);
            return get_func_call_expr(func);
        }

        padv(self);
        return (Expression) { .kind = EXPR_KIND_ID, .as.value = token.text };
    }

    PACKL_ERROR(self->filename, "unexpected token found when parsing expression: " SV_FMT, SV_UNWRAP(token.text));
}

Expression packl_parser_parse_multiplicative_expr(PACKL *self) {
    Expression lhs = packl_parser_parse_primary_expr(self);

    while (!peot(self)) {
        Token token = ppeek(self);
        if (!ismultiplicative(token.kind)) { break; }
    
        Operator op = packl_get_operator(self, token.kind);
        padv(self);

        Expression rhs = packl_parser_parse_multiplicative_expr(self);

        Expression bin = get_bin_operation(lhs, rhs, op);

        lhs = bin;
    }

    return lhs;
}

Expression packl_parser_parse_additive_expr(PACKL *self) {
    Expression lhs = packl_parser_parse_multiplicative_expr(self);

    while (!peot(self)) {
        Token token = ppeek(self);
        if (!isadditive(token.kind)) { break; }

        Operator op = packl_get_operator(self, token.kind);
        padv(self);

        Expression rhs = packl_parser_parse_additive_expr(self);

        Expression bin = get_bin_operation(lhs, rhs, op);

        lhs = bin;
    }

    return lhs;
}

Expression packl_parser_parse_expr(PACKL *self) {
    return packl_parser_parse_additive_expr(self);
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

    if (kind == TOKEN_KIND_VAR) {
        node.kind = NODE_KIND_VAR_DECLARATION;
        node.as.var_dec = packl_parser_parse_var_dec(self);
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