#include "packl-lexer.h"

char *symbols[] = {
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
};

Token_Kind symbol_token_kinds[] = {
    TOKEN_KIND_SEMI_COLON,
    TOKEN_KIND_COLON,
    TOKEN_KIND_COMMA,
    TOKEN_KIND_OPEN_PARENT,
    TOKEN_KIND_CLOSE_PARENT,
    TOKEN_KIND_OPEN_CURLY_BRACE,
    TOKEN_KIND_CLOSE_CURLY_BRACE,
    TOKEN_KIND_EQUAL,
    TOKEN_KIND_PLUS,
    TOKEN_KIND_MINUS,
    TOKEN_KIND_STAR,
    TOKEN_KIND_SLASH,
    TOKEN_KIND_MOD,
    TOKEN_KIND_LESS,
    TOKEN_KIND_GREATER,
};

char *keywords[] = {
    "proc",
    "func",
    "return",
    "var",
    "if",
    "else",
    "while",
};

char *natives[] = {
    "write",
    "exit",
    "get_byte",
};

Token_Kind keyword_token_kinds[] = {
    TOKEN_KIND_PROC,
    TOKEN_KIND_FUNC,
    TOKEN_KIND_RETURN,
    TOKEN_KIND_VAR,
    TOKEN_KIND_IF,
    TOKEN_KIND_ELSE,
    TOKEN_KIND_WHILE,
};

char packl_lexer_peek(PACKL *self) {
    return (self->lexer.source.content[self->lexer.current]);
}

void packl_lexer_advance(PACKL *self) {
    char current = lpeek(self);

    if (current == '\n') { self->lexer.loc.line++; self->lexer.loc.col = 1; }
    else { self->lexer.loc.col++; }

    self->lexer.current++;
}

bool packl_lexer_eof(PACKL *self) {
    return (self->lexer.source.count <= self->lexer.current);
}

Token packl_lexer_lex_string(PACKL *self) {
    Token token = {0};
    token.loc = self->lexer.loc;

    char *begin = &self->lexer.source.content[self->lexer.current];
    size_t size = 0;

    // consuming the `"` char
    ladv(self); 

    // consuming the quotes
    begin++;
    
    char current = 0;

    while (!leof(self)) {
        current = lpeek(self);
        ladv(self);
        if (current == '"') { break; }
        size++;
    }

    token.kind = TOKEN_KIND_STRING_LIT;
    token.text = SV_GET(begin, size);

    return token;
}

Token packl_lexer_lex_id(PACKL *self) {
    Token token = {0};
    token.loc = self->lexer.loc;

    char *begin = &self->lexer.source.content[self->lexer.current];
    size_t size = 0;


    while (!leof(self)) {
        if(!isalnum(lpeek(self)) && lpeek(self) != '_') { break; }
        ladv(self);
        size++;
    }

    String_View id = SV_GET(begin, size);

    // check for the natives
    for (size_t i = 0; i < ARR_SIZE(natives); ++i) {
        if (sv_eq(id, SV(natives[i]))) {
            token.text = SV_GET(begin, size);
            token.kind = TOKEN_KIND_NATIVE;
            return token;
        }
    }

    // check for keywords
    for (size_t i = 0; i < ARR_SIZE(keywords); ++i) {
        if (sv_eq(id, SV(keywords[i]))) { 
            token.kind = keyword_token_kinds[i];
            return token;
        }
    }

    token.text = id;
    token.kind = TOKEN_KIND_IDENTIFIER;
    return token;
}

Token packl_lexer_lex_number(PACKL *self) {
    Token token = {0};
    token.loc = self->lexer.loc;

    char *begin = &self->lexer.source.content[self->lexer.current];
    size_t size = 0;


    while (!leof(self)) {
        if (!isdigit(lpeek(self))) { break; }
        ladv(self);
        size++;
    }

    token.text = SV_GET(begin, size);
    token.kind = TOKEN_KIND_INTEGER_LIT;
    return token;
}

Token packl_lexer_read_token(PACKL *self) {
    Token token = {0};
    token.loc = self->lexer.loc;

    char *current = &self->lexer.source.content[self->lexer.current];

    String_View current_view = SV_GET(current, 1);

    for (size_t i = 0; i < ARR_SIZE(symbols); ++i) {
        if (sv_eq(SV(symbols[i]), current_view)) {
            ladv(self);
            token.kind = symbol_token_kinds[i];
            token.text = current_view;
            return token;
        }
    }

    if (*current == '"') { return packl_lexer_lex_string(self); }

    if (isdigit(*current)) { return packl_lexer_lex_number(self); }

    if (isalpha(*current)) { return packl_lexer_lex_id(self); }

    if (*current == '-') {
        TODO("handle negative numbers");
    }

    PACKL_ERROR_LOC(self->filename, token.loc, "failed to identify the char `%c`", *current);
}

void packl_lexer_lex(PACKL *self) {
    DA_INIT(&self->tokens, sizeof(Token));
    while (!leof(self)) {
        if (isspace(lpeek(self))) { ladv(self); continue; }
        Token token = lread(self);
        DA_APPEND(&self->tokens, token);
    }
    DA_APPEND(&self->tokens, ((Token) { .kind = TOKEN_KIND_END, .loc = self->lexer.loc }));
}