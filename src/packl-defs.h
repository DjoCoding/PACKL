#ifndef PACKL_DEFS_H
#define PACKL_DEFS_H

#include "packl-error.h"
#include "../tools/sv.h"

typedef struct {
    size_t line;
    size_t col;
} Location;

#define LOC_FMT          "(%zu, %zu)"
#define LOC_UNWRAP(loc)  loc.line, loc.col

#define ARR_SIZE(arr)    (sizeof(arr)/sizeof((arr)[0]))

// LEXING
typedef enum Token_Kind Token_Kind;
typedef struct Token Token;
typedef struct Tokens Tokens;
typedef struct Lexer Lexer;


// PARSING
typedef struct Parser Parser;

// parameters
typedef struct Parameter Parameter;
typedef struct Parameters Parameters;

// procedure defintion
typedef struct Proc_Def Proc_Def;

// function call 
typedef enum Func_Call_Arg_Type Func_Call_Arg_Type;
typedef union Func_Call_Arg_As Func_Call_Arg_As;
typedef struct Func_Call_Arg Func_Call_Arg;
typedef struct Func_Call_Args Func_Call_Args;
typedef struct Func_Call Func_Call;

typedef enum Node_Kind Node_Kind;
typedef union Node_As Node_As;
typedef struct Node Node;

typedef struct AST AST;

// Context 
typedef struct Procedure Procedure;
typedef enum Context_Item_Type Context_Item_Type;
typedef union Context_Item_As Context_Item_As;
typedef struct Context_Item Context_Item;
typedef struct Context Context;
typedef struct Contexts Contexts;


typedef struct PACKL PACKL;

enum Token_Kind {
    TOKEN_KIND_IDENTIFIER = 0,
    
    TOKEN_KIND_NATIVE,
    
    TOKEN_KIND_STRING_LIT,
    TOKEN_KIND_INTEGER_LIT,
    
    TOKEN_KIND_SEMI_COLON,
    TOKEN_KIND_COLON,
    TOKEN_KIND_COMMA,

    TOKEN_KIND_OPEN_PARENT, 
    TOKEN_KIND_CLOSE_PARENT,
    TOKEN_KIND_OPEN_CURLY_BRACE,
    TOKEN_KIND_CLOSE_CURLY_BRACE,
    TOKEN_KIND_PROC,

    COUNT_TOKEN_KINDS,
};

struct Token { 
    Token_Kind kind;
    String_View text;
};

struct Tokens {
    Token *items;
    size_t count;
    size_t size;
};

struct Lexer {
    String_View source;
    Location loc;
    size_t current;
};

enum Node_Kind {
    NODE_KIND_FUNC_CALL = 0,
    NODE_KIND_NATIVE_CALL,
    NODE_KIND_PROC_DEF,
};

enum PACKL_Type {
    PACKL_TYPE_STRING = 0,
    PACKL_TYPE_INTEGER,
};

struct Parameter {
    String_View type;
    String_View name;
};  

struct Parameters { 
    Parameter *items;
    size_t count;
    size_t size;
};

struct Proc_Def {
    String_View name;
    Parameters params;
    AST *body;
};

enum Func_Call_Arg_Type {
    ARG_TYPE_STRING_LIT,
    ARG_TYPE_INT_LIT,
    ARG_TYPE_ID,
};

struct Func_Call_Arg {
    Func_Call_Arg_Type type;
    String_View value;
};

struct Func_Call_Args {
    Func_Call_Arg *items;
    size_t count;
    size_t size;
};

struct Func_Call {
    String_View name;
    Func_Call_Args args;
};

union Node_As {
    Func_Call func_call;
    Proc_Def proc_def;
};

struct Node {
    Node_Kind kind;
    Node_As as;
};

struct AST {
    Node *items;
    size_t count;
    size_t size;
};

struct Parser {
    size_t current;
};

struct Procedure {
    Parameters params;
    size_t label_value;
};

enum Context_Item_Type {
    CONTEXT_ITEM_TYPE_PROCEDURE = 0,
};

union Context_Item_As {
    Procedure proc;
};

struct Context_Item {
    String_View name;
    Context_Item_Type type;
    Context_Item_As as;
};

struct Context {
    Context_Item *items;
    size_t count;
    size_t size;
};

struct Contexts {
    Context *items;
    size_t count;
    size_t size;
};

struct PACKL {
    char *filename;
    char *output;
    
    Tokens tokens;
    AST ast;

    Lexer lexer;
    Parser parser;

    Contexts contexts;

    size_t label_value;
};

#endif