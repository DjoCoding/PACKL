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
    "<",
    ">",

    "proc",
    "var",
    "if",
    "else",
    "while",
};

Node packl_init_node(Node_Kind kind, Location loc) {
    return (Node) { .kind = kind, .loc = loc };
}

Expression packl_parser_parse_comparative_expr(PACKL *self);
Expression packl_parser_parse_expr(PACKL *self);
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
    return ppeek(self).kind == TOKEN_KIND_END;
}

Operator packl_get_operator(PACKL *self, Token token) {
    switch(token.kind) {
        case TOKEN_KIND_PLUS:
            return OP_PLUS;
        case TOKEN_KIND_MINUS:
            return OP_MINUS;
        case TOKEN_KIND_STAR:
            return OP_MUL;
        case TOKEN_KIND_SLASH:
            return OP_DIV;
        case TOKEN_KIND_MOD:
            return OP_MOD;
        case TOKEN_KIND_LESS:
            return OP_LESS;
        case TOKEN_KIND_GREATER:
            return OP_GREATER;
        default:
            PACKL_ERROR_LOC(self->filename, token.loc, "expected operator type but `" SV_FMT "` found", SV_UNWRAP(ppeek(self).text));
    }
}

void packl_parser_expect(PACKL *self, Token_Kind kind, Token *token) {
    if (peot(self)) { PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "expected token of kind `%s` but end found", token_kinds_str[kind]); }
        

    Token current = ppeek(self);
    if (current.kind != kind) {
        PACKL_ERROR_LOC(self->filename, current.loc, "expected token of kind `%s` but `" SV_FMT "` found", token_kinds_str[kind], SV_UNWRAP(current.text));
    }

    if (token) *token = current;
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

        Token token = ppeek(self);

        if (token.kind == TOKEN_KIND_COMMA) { padv(self); continue; }
        else if (token.kind == TOKEN_KIND_CLOSE_PARENT) { break; }
        else { PACKL_ERROR_LOC(self->filename, token.loc, "unexpected token found `" SV_FMT "`", SV_UNWRAP(token.text)); }
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
        
        if (node.kind == NODE_KIND_PROC_DEF) { PACKL_ERROR_LOC(self->filename, node.loc, "procedure `" SV_FMT "` defined inside a body", SV_UNWRAP(node.as.proc_def.name)); }
        if (node.kind == NODE_KIND_FUNC_DEF) { PACKL_ERROR_LOC(self->filename, node.loc, "function `" SV_FMT "` defined inside a body", SV_UNWRAP(node.as.func_def.name)); }


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

Expression get_native_call_expr(Func_Call func) {
    Expression expr = {0};
    expr.kind = EXPR_KIND_NATIVE_CALL;
    expr.as.func = malloc(sizeof(Func_Call));
    *expr.as.func = func;
    return expr;
}

int ismultiplicative(Token_Kind kind) {
    Token_Kind kinds[] = {
        TOKEN_KIND_STAR,
        TOKEN_KIND_SLASH,
        TOKEN_KIND_MOD,
    };

    for (size_t i = 0; i < ARR_SIZE(kinds); ++i) {
        if (kinds[i] == kind) { return true; }
    }

    return false;
}

int isadditive(Token_Kind kind) {
    Token_Kind kinds[] = {
        TOKEN_KIND_PLUS,
        TOKEN_KIND_MINUS,
    };

    for (size_t i = 0; i < ARR_SIZE(kinds); ++i) {
        if (kinds[i] == kind) { return true; }
    }

    return false;
}

int iscomparative(Token_Kind kind) {
    Token_Kind kinds[] = {
        TOKEN_KIND_LESS,
        TOKEN_KIND_GREATER,
    };

    for (size_t i = 0; i < ARR_SIZE(kinds); ++i) {
        if (kinds[i] == kind) { return true; }
    }

    return false;
}

Expression packl_parser_parse_primary_expr(PACKL *self) {
    if (peot(self)) { PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "expected more tokens for the expression evaluation but end found"); }

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
        if (packl_get_tokens_number(self) < 2) { PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "expected a `;` but end of tokens found"); }

        if (ppeek_(self, 1).kind == TOKEN_KIND_OPEN_PARENT)  {
            Func_Call func = packl_parser_parse_func_call(self);
            return get_func_call_expr(func);
        }

        padv(self);
        return (Expression) { .kind = EXPR_KIND_ID, .as.value = token.text };
    }

    if (token.kind == TOKEN_KIND_OPEN_PARENT) {
        padv(self);
        Expression expr = packl_parser_parse_comparative_expr(self);
        pexp(self, TOKEN_KIND_CLOSE_PARENT, NULL);
        return expr;
    }

    if (token.kind == TOKEN_KIND_NATIVE) {
        if (ppeek_(self, 1).kind == TOKEN_KIND_OPEN_PARENT)  {
            Func_Call native = packl_parser_parse_func_call(self);
            return get_native_call_expr(native);
        }

        PACKL_ERROR_LOC(self->filename, token.loc, "expected `(` after the native `" SV_FMT "`", SV_UNWRAP(token.text));
    }

    PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "unexpected token found when parsing expression `" SV_FMT "`", SV_UNWRAP(token.text));
}

Expression packl_parser_parse_multiplicative_expr(PACKL *self) {
    Expression lhs = packl_parser_parse_primary_expr(self);

    while (!peot(self)) {
        Token token = ppeek(self);
        if (!ismultiplicative(token.kind)) { break; }
    
        Operator op = packl_get_operator(self, token);
        padv(self);

        Expression rhs = packl_parser_parse_primary_expr(self);

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

        Operator op = packl_get_operator(self, token);
        padv(self);

        Expression rhs = packl_parser_parse_multiplicative_expr(self);

        Expression bin = get_bin_operation(lhs, rhs, op);

        lhs = bin;
    }

    return lhs;
}

Expression packl_parser_parse_comparative_expr(PACKL *self) {
    Expression lhs = packl_parser_parse_additive_expr(self);

    while (!peot(self)) {
        Token token = ppeek(self);
        if (!iscomparative(token.kind)) { break; }

        Operator op = packl_get_operator(self, token);
        padv(self);

        Expression rhs = packl_parser_parse_additive_expr(self);

        Expression bin = get_bin_operation(lhs, rhs, op);

        lhs = bin;
    }

    return lhs;
}

Expression packl_parser_parse_expr(PACKL *self) {
    if (peot(self)) { PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "expected an expresssion but end found"); }
    return packl_parser_parse_comparative_expr(self);
}

Var_Reassign packl_parser_parse_var_reassign(PACKL *self) {
    Var_Reassign var = {0};

    Token token = {0};
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    var.name = token.text;

    pexp(self, TOKEN_KIND_EQUAL, NULL);

    var.expr = packl_parser_parse_expr(self);

    return var;
}

Node packl_parser_parse_identifier(PACKL *self) {
    Node node = {0};
    node.loc = ppeek(self).loc;

    if (packl_get_tokens_number(self) < 2) {
        PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "expected more tokens after `" SV_FMT "`", SV_UNWRAP(ppeek(self).text));
    }
            
    if (ppeek_(self, 1).kind == TOKEN_KIND_OPEN_PARENT) {
        node.kind = NODE_KIND_FUNC_CALL;
        node.as.func_call = packl_parser_parse_func_call(self);
        return node;
    }

    if (ppeek_(self, 1).kind == TOKEN_KIND_EQUAL) {
        node.kind = NODE_KIND_VAR_REASSIGN;
        node.as.var = packl_parser_parse_var_reassign(self);
        return node;
    }

    PACKL_ERROR(self->filename, "unexpected token found `" SV_FMT "`", SV_UNWRAP(ppeek(self).text));
}

Node packl_parser_parse_native_call(PACKL *self) {
    Node node = {0};
    node.loc = ppeek(self).loc;

    if (packl_get_tokens_number(self) < 2) {
        PACKL_ERROR_LOC(self->filename, node.loc, "expected more tokens after `" SV_FMT "`", SV_UNWRAP(ppeek(self).text));
    }
        
    if (ppeek_(self, 1).kind == TOKEN_KIND_OPEN_PARENT) {
        node.kind = NODE_KIND_NATIVE_CALL;
        node.as.func_call =  packl_parser_parse_func_call(self);
        return node;
    }

    PACKL_ERROR_LOC(self->filename, node.loc, "expected `(` after the native call `" SV_FMT "`", SV_UNWRAP(ppeek(self).text));
}

Node packl_get_node_from_if(If_Statement fi) {
    return (Node) { .kind = NODE_KIND_IF, .as.fi = fi };
}

If_Statement packl_parser_parse_if_statement(PACKL *self) {
    If_Statement fi = {0};

    // consume the `if` token 
    pexp(self, TOKEN_KIND_IF, NULL);

    if (peot(self)) { PACKL_ERROR(self->filename, "expected `if` condition but end found"); }

    if (ppeek(self).kind == TOKEN_KIND_OPEN_PARENT) {
        padv(self);
        fi.condition = packl_parser_parse_expr(self);
        pexp(self, TOKEN_KIND_CLOSE_PARENT, NULL);
        pexp(self, TOKEN_KIND_OPEN_CURLY_BRACE, NULL);
    } else {
        fi.condition = packl_parser_parse_expr(self);
    }

    fi.body = malloc(sizeof(AST));
    *fi.body = packl_parser_parse_body(self);
    pexp(self, TOKEN_KIND_CLOSE_CURLY_BRACE, NULL);

    if (peot(self)) { PACKL_ERROR(self->filename, "expected more tokens for the `if` statement but end found"); }

    if (ppeek(self).kind == TOKEN_KIND_ELSE) {
        padv(self);
        fi.esle = malloc(sizeof(AST));
        
        if (ppeek(self).kind == TOKEN_KIND_OPEN_CURLY_BRACE) {
            padv(self);
            *fi.esle = packl_parser_parse_body(self);
            pexp(self, TOKEN_KIND_CLOSE_CURLY_BRACE, NULL);
        } else {
            If_Statement elseif = packl_parser_parse_if_statement(self);
            Node node = packl_get_node_from_if(elseif);
            DA_INIT(fi.esle, sizeof(Node));
            DA_APPEND(fi.esle, node);
        }
    } 

    return fi;
}

While_Statement packl_parser_parse_while_statement(PACKL *self) {
    While_Statement hwile = {0};

    // consume the `while` token 
    pexp(self, TOKEN_KIND_WHILE, NULL);

    if (peot(self)) { PACKL_ERROR(self->filename, "expected `while` condition but end found"); }

    if (ppeek(self).kind == TOKEN_KIND_OPEN_PARENT) {
        padv(self);
        hwile.condition = packl_parser_parse_expr(self);
        pexp(self, TOKEN_KIND_CLOSE_PARENT, NULL);
        pexp(self, TOKEN_KIND_OPEN_CURLY_BRACE, NULL);
    } else {
        hwile.condition = packl_parser_parse_expr(self);
    }

    hwile.body = malloc(sizeof(AST));
    *hwile.body = packl_parser_parse_body(self);
    pexp(self, TOKEN_KIND_CLOSE_CURLY_BRACE, NULL);

    if (peot(self)) { PACKL_ERROR(self->filename, "expected more tokens for the `while` statement but end found"); }

    return hwile;
}

Func_Def packl_parser_parse_func_def(PACKL *self) {
    Func_Def func = {0};
    
    // consuming the `func` token 
    padv(self);

    Token token = {0};
    
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    func.name = token.text;

    if (ppeek(self).kind == TOKEN_KIND_OPEN_PARENT) {
        padv(self);
        func.params = packl_parser_parse_params(self);
        pexp(self, TOKEN_KIND_CLOSE_PARENT, NULL);
    }

    // for the return type
    pexp(self, TOKEN_KIND_COLON, NULL);
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    func.return_type = token.text;

    pexp(self, TOKEN_KIND_OPEN_CURLY_BRACE, &token);

    func.body = malloc(sizeof(AST));
    *func.body = packl_parser_parse_body(self);

    pexp(self, TOKEN_KIND_CLOSE_CURLY_BRACE, &token);

    return func;
}

Expression packl_parser_parse_return(PACKL *self) {
    pexp(self, TOKEN_KIND_RETURN, NULL);
    return packl_parser_parse_expr(self);
}

Node packl_parser_parse_proc_def_node(PACKL *self) {
    Node node = packl_init_node(NODE_KIND_PROC_DEF, ppeek(self).loc);
    node.as.proc_def = packl_parser_parse_proc_def(self);
    return node;
}

Node packl_parser_parse_var_dec_node(PACKL *self) {
    Node node = packl_init_node(NODE_KIND_VAR_DECLARATION, ppeek(self).loc);
    node.as.var_dec = packl_parser_parse_var_dec(self);
    return node;
}

Node packl_parser_parse_if_node(PACKL *self) {
    Node node = packl_init_node(NODE_KIND_IF, ppeek(self).loc);
    node.as.fi = packl_parser_parse_if_statement(self);
    return node;
}

Node packl_parser_parse_while_node(PACKL *self) {
    Node node = packl_init_node(NODE_KIND_WHILE, ppeek(self).loc);
    node.as.hwile = packl_parser_parse_while_statement(self);
    return node;
}

Node packl_parser_parse_func_def_node(PACKL *self) {
    Node node = packl_init_node(NODE_KIND_FUNC_DEF, ppeek(self).loc);
    node.as.func_def = packl_parser_parse_func_def(self);
    return node;
}

Node packl_parser_parse_return_node(PACKL *self) {
    Node node = packl_init_node(NODE_KIND_RETURN, ppeek(self).loc);
    node.as.ret = packl_parser_parse_return(self);
    return node;
}

Node packl_parser_parse_statement(PACKL *self) {
    switch(ppeek(self).kind) {
        case TOKEN_KIND_NATIVE:
            return packl_parser_parse_native_call(self);
        case TOKEN_KIND_IDENTIFIER:
            return packl_parser_parse_identifier(self);
        case TOKEN_KIND_PROC:
            return packl_parser_parse_proc_def_node(self);
        case TOKEN_KIND_VAR:
            return packl_parser_parse_var_dec_node(self);
        case TOKEN_KIND_IF:
            return packl_parser_parse_if_node(self);
        case TOKEN_KIND_WHILE:
            return packl_parser_parse_while_node(self);
        case TOKEN_KIND_FUNC:
            return packl_parser_parse_func_def_node(self);
        case TOKEN_KIND_RETURN:
            return packl_parser_parse_return_node(self);
        default:
            PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "unexpected token found at the beginning of a statement `" SV_FMT "`", SV_UNWRAP(ppeek(self).text));
    }
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
