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
    "use",
    "as",
};

Node packl_init_node(Node_Kind kind, Location loc) {
    return (Node) { .kind = kind, .loc = loc };
}

PACKL_Type packl_parser_parse_type(PACKL_File *self);
Expression packl_parser_parse_comparative_expr(PACKL_File *self);
Expression packl_parser_parse_expr(PACKL_File *self);
Node packl_parser_parse_statement(PACKL_File *self);

Token packl_parser_peek(PACKL_File *self, size_t ahead) {
    return self->tokens.items[self->parser.current + ahead];
}

void packl_parser_advance(PACKL_File *self) {
    self->parser.current++;
}

// get the number of tokens that can be consumed
size_t packl_get_tokens_number(PACKL_File *self) {
    return self->tokens.count - self->parser.current;
}

bool packl_parser_eot(PACKL_File *self) {
    return ppeek(self).kind == TOKEN_KIND_END;
}

Operator packl_get_operator(PACKL_File *self, Token token) {
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

void packl_parser_expect(PACKL_File *self, Token_Kind kind, Token *token) {
    if (peot(self)) { PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "expected token of kind `%s` but end found", token_kinds_str[kind]); }
        

    Token current = ppeek(self);
    if (current.kind != kind) {
        PACKL_ERROR_LOC(self->filename, current.loc, "expected token of kind `%s` but `" SV_FMT "` found", token_kinds_str[kind], SV_UNWRAP(current.text));
    }

    if (token) *token = current;
    padv(self);
}

PACKL_Arg packl_parser_parse_arg(PACKL_File *self) {
    PACKL_Arg arg = {0};
    arg.expr = packl_parser_parse_expr(self);
    return arg;
}


PACKL_Args packl_parser_parse_args(PACKL_File *self) {
    PACKL_Args args = {0};
    DA_INIT(&args, sizeof(PACKL_Arg));

    if (ppeek(self).kind == TOKEN_KIND_CLOSE_PARENT) { return args; }
    
    while (!peot(self)) {
        PACKL_Arg arg = packl_parser_parse_arg(self);
        DA_APPEND(&args, arg);

        Token token = ppeek(self);

        if (token.kind == TOKEN_KIND_COMMA) { padv(self); continue; }
        else if (token.kind == TOKEN_KIND_CLOSE_PARENT) { break; }
        else { PACKL_ERROR_LOC(self->filename, token.loc, "unexpected token found `" SV_FMT "`", SV_UNWRAP(token.text)); }
    }

    return args;
}

Func_Call packl_parser_parse_func_call(PACKL_File *self) {
    Func_Call func_call = {0};
    Token token = ppeek(self);

    func_call.name = token.text;
    
    // consume the function name
    padv(self);

    // parsing arguments
    pexp(self, TOKEN_KIND_OPEN_PARENT, NULL);
    func_call.args = packl_parser_parse_args(self);
    pexp(self, TOKEN_KIND_CLOSE_PARENT, NULL);

    return func_call;
}

Mod_Call packl_parser_parse_module_call(PACKL_File *self) {
    Mod_Call mod_call = {0};

    Token token = {0};
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    mod_call.name = token.text;

    pexp(self, TOKEN_KIND_COLON, NULL);

    if (packl_get_tokens_number(self) < 2) {
        PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "expected more tokens after module call `" SV_FMT "`", SV_UNWRAP(mod_call.name));
    }

    if (ppeek(self).kind != TOKEN_KIND_IDENTIFIER) {
        PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "expected identifier after module call `" SV_FMT "`", SV_UNWRAP(mod_call.name));
    }
    
    if (ppeek_(self, 1).kind == TOKEN_KIND_OPEN_PARENT) {
        mod_call.kind = MODULE_CALL_FUNC_CALL;
        mod_call.as.func_call = packl_parser_parse_func_call(self);
        return mod_call;
    }

    mod_call.kind = MODULE_CALL_VARIABLE;
    mod_call.as.var_name = ppeek(self).text;
    padv(self);

    return mod_call;
}



Parameter packl_parser_parse_param(PACKL_File *self) {
    Parameter param = {0};

    Token token = {0};

    // for the param name
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    param.name = token.text;

    pexp(self, TOKEN_KIND_COLON, NULL);

    param.type = packl_parser_parse_type(self);

    return param;
}

Parameters packl_parser_parse_params(PACKL_File *self) {
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

AST packl_parser_parse_body(PACKL_File *self) {
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

Proc_Def packl_parser_parse_proc_def(PACKL_File *self) {
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


Array_Type packl_parser_parse_array_type(PACKL_File *self) {
    Array_Type arr_type = {0};

    pexp(self, TOKEN_KIND_ARRAY, NULL);
    
    pexp(self, TOKEN_KIND_OPEN_PARENT, NULL);

    arr_type.type = malloc(sizeof(PACKL_Type));
    *arr_type.type = packl_parser_parse_type(self);
    
    pexp(self, TOKEN_KIND_COMMA, NULL);
    
    arr_type.size = packl_parser_parse_expr(self);
    
    pexp(self, TOKEN_KIND_CLOSE_PARENT, NULL);

    return arr_type;
}

PACKL_Type packl_parser_parse_type(PACKL_File *self) {
    PACKL_Type type = {0};

    Token token = ppeek(self);

    if(token.kind == TOKEN_KIND_ARRAY) {
        type.kind = PACKL_TYPE_ARRAY; 
        type.as.array = packl_parser_parse_array_type(self);
        return type;
    }

    if (token.kind == TOKEN_KIND_INT_TYPE) {
        padv(self);
        type.kind = PACKL_TYPE_BASIC;
        type.as.basic = PACKL_TYPE_INT;
        return type;
    } 

    if(token.kind == TOKEN_KIND_STR_TYPE) {
        padv(self);
        type.kind = PACKL_TYPE_BASIC;
        type.as.basic = PACKL_TYPE_STR;
        return type;
    }

    PACKL_ERROR_LOC(self->filename, token.loc, "unknown type `" SV_FMT "`", SV_UNWRAP(token.text));
}

Expr_Arr packl_parser_parse_array_initialization(PACKL_File *self) {
    Expr_Arr arr = {0};
    DA_INIT(&arr, sizeof(Expression));

    if (ppeek(self).kind == TOKEN_KIND_CLOSE_CURLY_BRACE) { return arr; }
    
    while (!peot(self)) {
        Expression expr = packl_parser_parse_expr(self);
        DA_APPEND(&arr, expr);

        Token token = ppeek(self);

        if (token.kind == TOKEN_KIND_COMMA) { padv(self); continue; }
        else if (token.kind == TOKEN_KIND_CLOSE_CURLY_BRACE) { break; }
        else { PACKL_ERROR_LOC(self->filename, token.loc, "unexpected token found `" SV_FMT "`", SV_UNWRAP(token.text)); }
    }

    return arr;
}

Var_Declaration packl_parser_parse_var_dec(PACKL_File *self) {
    Var_Declaration var_dec = {0};

    // consuming the `var` keyword    
    padv(self);

    Token token = {0};
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    var_dec.name = token.text;

    pexp(self, TOKEN_KIND_COLON, NULL);
    
    var_dec.type = packl_parser_parse_type(self);

    pexp(self, TOKEN_KIND_EQUAL, NULL);

    if(ppeek(self).kind == TOKEN_KIND_OPEN_CURLY_BRACE) {
        padv(self);
        var_dec.value.kind = EXPR_KIND_ARRAY;
        var_dec.value.as.arr = packl_parser_parse_array_initialization(self);
        pexp(self, TOKEN_KIND_CLOSE_CURLY_BRACE, NULL);
    } else {
        var_dec.value = packl_parser_parse_expr(self);
    }

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

    expr.loc = lhs.loc;
    
    return expr;
}

Expression get_func_call_expr(Func_Call func) {
    Expression expr = {0};
    expr.kind = EXPR_KIND_FUNC_CALL;
    expr.as.func = malloc(sizeof(Func_Call));
    *expr.as.func = func;
    return expr;
}

Expression get_mod_call_expr(Mod_Call mod) {
    Expression expr = {0};
    expr.kind = EXPR_KIND_MOD_CALL;
    expr.as.func = malloc(sizeof(Mod_Call));
    *expr.as.mod = mod;
    return expr;
}

Expression get_arr_index_expr(Expr_Arr_Index arr_index) {
    Expression expr = {0};
    expr.kind = EXPR_KIND_ARRAY_INDEXING;
    expr.as.arr_index = arr_index;
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

Expr_Arr_Index packl_parser_parse_array_indexing(PACKL_File *self) {
    Expr_Arr_Index arr_index = {0};

    Token token = {0};
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    arr_index.name = token.text;

    pexp(self, TOKEN_KIND_OPEN_BRACKET, NULL);
    arr_index.index = malloc(sizeof(Expression));
    *arr_index.index = packl_parser_parse_expr(self);
    pexp(self, TOKEN_KIND_CLOSE_BRACKET, NULL);

    return arr_index;
}

Expression packl_parser_init_expr(Location loc, Expr_Kind kind, Expr_As as) {
    Expression expr = {0};
    expr.loc = loc;
    expr.as = as;
    expr.kind = kind;
    return expr;
}

Expression packl_parser_parse_primary_expr(PACKL_File *self) {
    if (peot(self)) { PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "expected more tokens for the expression evaluation but end found"); }

    Token token = ppeek(self);
    Location loc = token.loc;
    Expr_As as = {0};

    if (token.kind == TOKEN_KIND_INTEGER_LIT) {
        padv(self);
        as.integer = integer_from_sv(token.text);
        return packl_parser_init_expr(loc, EXPR_KIND_INTEGER, as);
    }

    if (token.kind == TOKEN_KIND_STRING_LIT) {
        padv(self);
        as.value = token.text;
        return packl_parser_init_expr(loc, EXPR_KIND_STRING, as);
    }

    if (token.kind == TOKEN_KIND_IDENTIFIER) {
        if (packl_get_tokens_number(self) < 2) { PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "expected a `;` but end of tokens found"); }

        if (ppeek_(self, 1).kind == TOKEN_KIND_OPEN_PARENT)  {
            as.func = malloc(sizeof(Func_Call));
            *as.func = packl_parser_parse_func_call(self);
            return packl_parser_init_expr(loc, EXPR_KIND_FUNC_CALL, as);
        }

        if (ppeek_(self, 1).kind == TOKEN_KIND_COLON) {
            as.mod = malloc(sizeof(Mod_Call));
            *as.mod = packl_parser_parse_module_call(self);
            return packl_parser_init_expr(loc, EXPR_KIND_MOD_CALL, as);
        }

        if (ppeek_(self, 1).kind == TOKEN_KIND_OPEN_BRACKET) {
            as.arr_index = packl_parser_parse_array_indexing(self);
            return packl_parser_init_expr(loc, EXPR_KIND_ARRAY_INDEXING, as);
        }

        padv(self);
        as.value = token.text;
        return packl_parser_init_expr(loc, EXPR_KIND_ID, as);
    }

    if (token.kind == TOKEN_KIND_OPEN_PARENT) {
        padv(self);
        Expression expr = packl_parser_parse_comparative_expr(self);
        expr.loc = loc;
        pexp(self, TOKEN_KIND_CLOSE_PARENT, NULL);
        return expr;
    }

    if (token.kind == TOKEN_KIND_NATIVE) {
        if (ppeek_(self, 1).kind == TOKEN_KIND_OPEN_PARENT)  {
            as.func = malloc(sizeof(Func_Call));
            *as.func = packl_parser_parse_func_call(self);
            return packl_parser_init_expr(loc, EXPR_KIND_NATIVE_CALL, as);
        }

        PACKL_ERROR_LOC(self->filename, token.loc, "expected `(` after the native `" SV_FMT "`", SV_UNWRAP(token.text));
    }

    PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "unexpected token found when parsing expression `" SV_FMT "`", SV_UNWRAP(token.text));
}

Expression packl_parser_parse_multiplicative_expr(PACKL_File *self) {
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

Expression packl_parser_parse_additive_expr(PACKL_File *self) {
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

Expression packl_parser_parse_comparative_expr(PACKL_File *self) {
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

Expression packl_parser_parse_expr(PACKL_File *self) {
    if (peot(self)) { PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "expected an expresssion but end found"); }
    return packl_parser_parse_comparative_expr(self);
}

Var_Reassign packl_parser_parse_var_reassign(PACKL_File *self) {
    Var_Reassign var = {0};

    Token token = {0};
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    var.name = token.text;

    if (ppeek(self).kind == TOKEN_KIND_OPEN_BRACKET) {
        var.kind = PACKL_TYPE_ARRAY;
        padv(self);
        var.index = packl_parser_parse_expr(self);
        pexp(self, TOKEN_KIND_CLOSE_BRACKET, NULL);
    } else {
        var.kind = PACKL_TYPE_BASIC;
    }

    pexp(self, TOKEN_KIND_EQUAL, NULL);

    var.expr = packl_parser_parse_expr(self);

    return var;
}

Node packl_parser_parse_identifier(PACKL_File *self) {
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

    if (ppeek_(self, 1).kind == TOKEN_KIND_OPEN_BRACKET) {
        node.kind = NODE_KIND_VAR_REASSIGN;
        node.as.var = packl_parser_parse_var_reassign(self);
        return node;
    }

    if (ppeek_(self, 1).kind == TOKEN_KIND_COLON) {
        node.kind = NODE_KIND_MOD_CALL;
        node.as.mod_call = packl_parser_parse_module_call(self);
        return node;
    }

    if (ppeek_(self, 1).kind == TOKEN_KIND_EQUAL) {
        node.kind = NODE_KIND_VAR_REASSIGN;
        node.as.var = packl_parser_parse_var_reassign(self);
        return node;
    }

    PACKL_ERROR(self->filename, "unexpected token found `" SV_FMT "`", SV_UNWRAP(ppeek(self).text));
}

Node packl_parser_parse_native_call(PACKL_File *self) {
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

If_Statement packl_parser_parse_if_statement(PACKL_File *self) {
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

While_Statement packl_parser_parse_while_statement(PACKL_File *self) {
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

Func_Def packl_parser_parse_func_def(PACKL_File *self) {
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
    func.return_type = packl_parser_parse_type(self);

    pexp(self, TOKEN_KIND_OPEN_CURLY_BRACE, &token);

    func.body = malloc(sizeof(AST));
    *func.body = packl_parser_parse_body(self);

    pexp(self, TOKEN_KIND_CLOSE_CURLY_BRACE, &token);

    return func;
}

For_Statement packl_parser_parser_for(PACKL_File *self) {
    For_Statement rof = {0};

    pexp(self, TOKEN_KIND_FOR, NULL);

    Token token = {0};
    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    rof.iter = token.text;

    pexp(self, TOKEN_KIND_COLON, NULL);
    
    rof.iter_type = packl_parser_parse_type(self);

    pexp(self, TOKEN_KIND_IN, NULL);

    pexp(self, TOKEN_KIND_OPEN_PARENT, NULL);
    rof.args = packl_parser_parse_args(self);
    pexp(self, TOKEN_KIND_CLOSE_PARENT, NULL);

    pexp(self, TOKEN_KIND_OPEN_CURLY_BRACE, NULL);
    rof.body = malloc(sizeof(AST));
    *rof.body = packl_parser_parse_body(self);
    pexp(self, TOKEN_KIND_CLOSE_CURLY_BRACE, NULL);

    return rof;
}

Expression packl_parser_parse_return(PACKL_File *self) {
    pexp(self, TOKEN_KIND_RETURN, NULL);
    return packl_parser_parse_expr(self);
}

Use packl_parser_parse_module_use(PACKL_File *self) {
    Use use = {0};

    pexp(self, TOKEN_KIND_USE, NULL);   

    Token token = {0};
    pexp(self, TOKEN_KIND_STRING_LIT, &token);
    use.filename = token.text;

    pexp(self, TOKEN_KIND_AS, NULL);

    pexp(self, TOKEN_KIND_IDENTIFIER, &token);
    use.alias = token.text;
    use.has_alias = 1;

    return use;
}

Node packl_parser_parse_proc_def_node(PACKL_File *self) {
    Node node = packl_init_node(NODE_KIND_PROC_DEF, ppeek(self).loc);
    node.as.proc_def = packl_parser_parse_proc_def(self);
    return node;
}

Node packl_parser_parse_var_dec_node(PACKL_File *self) {
    Node node = packl_init_node(NODE_KIND_VAR_DECLARATION, ppeek(self).loc);
    node.as.var_dec = packl_parser_parse_var_dec(self);
    return node;
}

Node packl_parser_parse_if_node(PACKL_File *self) {
    Node node = packl_init_node(NODE_KIND_IF, ppeek(self).loc);
    node.as.fi = packl_parser_parse_if_statement(self);
    return node;
}

Node packl_parser_parse_while_node(PACKL_File *self) {
    Node node = packl_init_node(NODE_KIND_WHILE, ppeek(self).loc);
    node.as.hwile = packl_parser_parse_while_statement(self);
    return node;
}

Node packl_parser_parse_func_def_node(PACKL_File *self) {
    Node node = packl_init_node(NODE_KIND_FUNC_DEF, ppeek(self).loc);
    node.as.func_def = packl_parser_parse_func_def(self);
    return node;
}

Node packl_parser_parse_return_node(PACKL_File *self) {
    Node node = packl_init_node(NODE_KIND_RETURN, ppeek(self).loc);
    node.as.ret = packl_parser_parse_return(self);
    return node;
}

Node packl_parser_parse_for_node(PACKL_File *self) {
    Node node = packl_init_node(NODE_KIND_FOR, ppeek(self).loc);
    node.as.rof = packl_parser_parser_for(self);
    return node;
}

Node packl_parser_parse_use_node(PACKL_File *self) {
    Node node = packl_init_node(NODE_KIND_USE, ppeek(self).loc);
    node.as.use = packl_parser_parse_module_use(self);
    return node;
}

Node packl_parser_parse_statement(PACKL_File *self) {
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
        case TOKEN_KIND_FOR:
            return packl_parser_parse_for_node(self);
        case TOKEN_KIND_USE:
            return packl_parser_parse_use_node(self);
        default:
            PACKL_ERROR_LOC(self->filename, ppeek(self).loc, "unexpected token found at the beginning of a statement `" SV_FMT "`", SV_UNWRAP(ppeek(self).text));
    }
}

AST packl_parser_parse_statements(PACKL_File *self) {
    AST ast = {0};
    DA_INIT(&ast, sizeof(Node));

    while (!peot(self)) {
        Node node = pstmt(self);
        DA_APPEND(&ast, node);
        if (ppeek(self).kind == TOKEN_KIND_SEMI_COLON) { padv(self); }
    }

    return ast;
}

void packl_parser_parse(PACKL_File *self) {
    self->ast = packl_parser_parse_statements(self);
}
