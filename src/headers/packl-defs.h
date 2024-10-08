#ifndef PACKL_DEFS_H
#define PACKL_DEFS_H

#include "packl-error.h"
#include "../../tools/sv.h"

typedef struct {
    size_t line;
    size_t col;
} Location;

typedef struct {
    char **items;
    size_t count;
    size_t size;
} Strings;

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

// types
typedef enum Type Type;
typedef struct Array_Type Array_Type;
typedef enum PACKL_Type_Kind PACKL_Type_Kind;
typedef union PACKL_Type_As PACKL_Type_As;
typedef struct PACKL_Type PACKL_Type;

// parameters
typedef struct Parameter Parameter;
typedef struct Parameters Parameters;

// function definition
typedef struct Func_Def Func_Def;

// procedure defintion
typedef struct Proc_Def Proc_Def;

// module 
typedef struct Use Use;

// expression
typedef struct Expr_Attribute Expr_Attribute;
typedef struct Expr_Method_Call Expr_Method_Call;
typedef enum Expr_Operator_Kind Expr_Operator_Kind;
typedef enum Expr_Operator_Input_Kind Expr_Operator_Input_Kind;
typedef union Expr_Operator_Input_As Expr_Operator_Input_As;
typedef struct Expr_Operator_Input Expr_Operator_Input;
typedef struct Expr_Operator Expr_Operator;
typedef struct Expr_SizeOf Expr_SizeOf;
typedef struct Expr_Attribute Expr_Attribute;
typedef enum Operator Operator;
typedef struct Expr_Unary_Op Expr_Unary_Op;
typedef struct Expr_Bin_Op Expr_Bin_Op;
typedef union Expr_As Expr_As;
typedef enum Expr_Kind Expr_Kind;
typedef struct Expression Expression;
typedef struct Expr_Arr Expr_Arr;
typedef struct Expr_Arr_Index Expr_Arr_Index;

// While Statement
typedef struct While_Statement While_Statement;

// For Statement
typedef struct For_Statement For_Statement;

// If Statement
typedef struct If_Statement If_Statement;

// function call 
typedef enum PACKL_Arg_Type PACKL_Arg_Type;
typedef union PACKL_Arg_As PACKL_Arg_As;
typedef struct PACKL_Arg PACKL_Arg;
typedef struct PACKL_Args PACKL_Args;
typedef struct Func_Call Func_Call;

// Variable 
typedef union Variable_Format_As Variable_Format_As;
typedef enum Variable_Format_Kind Variable_Format_Kind;
typedef struct Variable_Format Variable_Format;

// variable declaration
typedef struct Var_Declaration Var_Declaration;

// variable reassignement
typedef struct Var_Reassign Var_Reassign;

// module call
typedef enum Mod_Call_Kind Mod_Call_Kind;
typedef union Mod_Call_As Mod_Call_As;
typedef struct Mod_Call Mod_Call;

// class
typedef struct Class_Def Class_Def;
typedef struct Attribute Attribute;
typedef struct Attributes Attributes;
typedef union Method_Def_As Method_Def_As;
typedef struct Method_Def Method_Def;
typedef struct Method_Call Method_Call;

// Nodes 
typedef enum Node_Kind Node_Kind;
typedef union Node_As Node_As;
typedef struct Node Node;

typedef struct AST AST;

// Context 
typedef enum Method_Kind Method_Kind;
typedef union Method_As Method_As;
typedef struct Method Method;
typedef struct Methods Methods;
typedef struct Class Class;
typedef struct Module Module;
typedef struct Function Function;
typedef struct Variable Variable;
typedef struct Procedure Procedure;
typedef enum Context_Item_Type Context_Item_Type;
typedef union Context_Item_As Context_Item_As;
typedef struct Context_Item Context_Item;
typedef struct Context Context;
typedef struct Contexts Contexts;

// Files 
typedef struct PACKL_File PACKL_File;
typedef struct PACKL_External_Files PACKL_External_Files;
typedef struct PACKL_Compiler PACKL_Compiler;

enum Token_Kind {
    TOKEN_KIND_IDENTIFIER = 0,
    
    TOKEN_KIND_NATIVE,
    
    TOKEN_KIND_STRING_LIT,
    TOKEN_KIND_INTEGER_LIT,
    
    TOKEN_KIND_SEMI_COLON,
    TOKEN_KIND_COLON,
    TOKEN_KIND_COMMA,
    TOKEN_KIND_DOT,

    TOKEN_KIND_OPEN_BRACKET,
    TOKEN_KIND_CLOSE_BRACKET,
    TOKEN_KIND_OPEN_PARENT, 
    TOKEN_KIND_CLOSE_PARENT,
    TOKEN_KIND_OPEN_CURLY_BRACE,
    TOKEN_KIND_CLOSE_CURLY_BRACE,
    
    TOKEN_KIND_EQUAL,

    TOKEN_KIND_PLUS,
    TOKEN_KIND_DOUBLE_PLUS,
    TOKEN_KIND_MINUS,
    TOKEN_KIND_DOUBLE_MINUS,
    TOKEN_KIND_STAR,
    TOKEN_KIND_SLASH,
    TOKEN_KIND_MOD,

    TOKEN_KIND_DOUBLE_EQUAL,
    TOKEN_KIND_LESS,
    TOKEN_KIND_GREATER,
    TOKEN_KIND_LESS_OR_EQUAL,
    TOKEN_KIND_GREATER_OR_EQUAL,
    TOKEN_KIND_NOT,
    TOKEN_KIND_NOT_EQUAL,

    TOKEN_KIND_PROC,
    TOKEN_KIND_FUNC,
    TOKEN_KIND_RETURN,
    TOKEN_KIND_VAR,
    TOKEN_KIND_IF,
    TOKEN_KIND_ELSE,
    TOKEN_KIND_WHILE,
    TOKEN_KIND_FOR,
    TOKEN_KIND_IN,
    TOKEN_KIND_USE,
    TOKEN_KIND_AS,
    TOKEN_KIND_CLASS,

    TOKEN_KIND_OR,
    TOKEN_KIND_AND,
    TOKEN_KIND_XOR,

    TOKEN_KIND_ARRAY,
    TOKEN_KIND_INT_TYPE,
    TOKEN_KIND_STR_TYPE,
    TOKEN_KIND_PTR_TYPE,

    TOKEN_KIND_OPERATOR,

    TOKEN_KIND_END,

    COUNT_TOKEN_KINDS,
};

struct Token { 
    Location loc;
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
    NODE_KIND_METHOD_CALL,
    NODE_KIND_PROC_DEF,
    NODE_KIND_FUNC_DEF,
    NODE_KIND_METHOD_DEF,
    NODE_KIND_RETURN,
    NODE_KIND_VAR_DECLARATION,
    NODE_KIND_VAR_REASSIGN,
    NODE_KIND_EXPR,
    NODE_KIND_IF,
    NODE_KIND_WHILE,
    NODE_KIND_FOR,
    NODE_KIND_USE,
    NODE_KIND_MOD_CALL,
    NODE_KIND_CLASS,
};


enum Type {
    PACKL_TYPE_INT = 0,
    PACKL_TYPE_STR,
    PACKL_TYPE_PTR,
    PACKL_TYPE_VOID,
    COUNT_PACKL_TYPES,
};

struct Array_Type {
    PACKL_Type *item_type;
    size_t size;
};

enum PACKL_Type_Kind {
    PACKL_TYPE_BASIC,
    PACKL_TYPE_ARRAY,
    PACKL_TYPE_USER_DEFINED,
};

union PACKL_Type_As {
    Type basic;
    Array_Type array;
    String_View user_defined;
};

struct PACKL_Type {
    PACKL_Type_Kind kind;
    PACKL_Type_As as;
};  

enum Expr_Kind {
    EXPR_KIND_BIN_OP = 0,
    EXPR_KIND_PRE_UNARY_OP,
    EXPR_KIND_POST_UNARY_OP,
    EXPR_KIND_INTEGER,
    EXPR_KIND_STRING,
    EXPR_KIND_ID,
    EXPR_KIND_FUNC_CALL,
    EXPR_KIND_NATIVE_CALL,
    EXPR_KIND_MOD_CALL,
    EXPR_KIND_ARRAY,
    EXPR_KIND_ARRAY_INDEXING,
    EXPR_KIND_OBJECT_ATTRIBUTE,
    EXPR_KIND_OBJECT_METHOD,
    EXPR_KIND_NOT_INITIALIZED,
    EXPR_KIND_OPERATOR,
};

enum Operator {
    OP_PLUS = 0,
    OP_MINUS,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_L,
    OP_G,
    OP_LE,
    OP_GE,
    OP_EQ,
    OP_INC,
    OP_DEC,
    OP_AND,
    OP_XOR,
    OP_OR,
    OP_NOT,
    OP_NE,
};  

struct Expr_Arr {
    Expression *items;
    size_t count;
    size_t size;
};

struct Expr_Arr_Index {
    String_View name;
    Expression *index;
};

struct Expr_Bin_Op {
    Expression *lhs, *rhs;
    Operator op;
};

struct Expr_Unary_Op {
    Expression *operand;
    Operator op;
};

struct Expr_Attribute {
    String_View obj_name;
    String_View attr;
};

enum Expr_Operator_Input_Kind {
    INPUT_KIND_TYPE = 0,
    INPUT_KIND_ID,
};

union Expr_Operator_Input_As {
    PACKL_Type type;
    String_View identifier;
};

enum Expr_Operator_Kind {
    NEW_OPERATOR = 0,
    SIZEOF_OPERATOR,
};

struct Expr_Operator_Input {
    Expr_Operator_Input_Kind kind;
    Expr_Operator_Input_As as;
};

struct Expr_Operator {
    Expr_Operator_Kind op;
    Expr_Operator_Input input;
};

union Expr_As {
    Expr_Bin_Op bin;
    Expr_Unary_Op unary;
    
    String_View value;
    
    int64_t integer;
    Func_Call *func;
    Mod_Call *mod;

    Method_Call *method;
    Expr_Attribute attr;
    
    Expr_Arr arr;
    Expr_Arr_Index arr_index;
    
    Expr_Operator operator;
};

struct Expression {
    Expr_Kind kind;
    Expr_As as;
    Location loc;
};  

struct PACKL_Arg {
    Expression expr;
};

struct PACKL_Args {
    PACKL_Arg *items;
    size_t count;
    size_t size;
};

struct Attribute {
    PACKL_Type type;
    size_t type_size;    // used later when making class contexts
    String_View name;
    size_t offset;
};

struct Attributes {
    Attribute *items;
    size_t count;
    size_t size;
};

struct Parameter {
    PACKL_Type type;
    String_View name;
};  

struct Parameters { 
    Parameter *items;
    size_t count;
    size_t size;
};

struct Func_Def {
    String_View name;
    Parameters params;
    AST *body;
    PACKL_Type return_type;
};

struct Proc_Def {
    String_View name;
    Parameters params;
    AST *body;
};

enum Variable_Format_Kind {
    VARIABLE_FORMAT_BASIC,
    VARIABLE_FORMAT_ARRAY,
    VARIABLE_FORMAT_CLASS,
};

union Variable_Format_As {
    Expression index;
    String_View attr;
};

struct Variable_Format {
    String_View name;
    Variable_Format_Kind kind;
    Variable_Format_As as;
};

struct Var_Reassign {
    Variable_Format format;
    Expression value;
};

struct While_Statement {
    Expression condition;
    AST *body;
};

struct If_Statement {
    Expression condition;
    AST *body;
    AST *esle;                    // the else part of the if statement, will be set to NULL if none is provided
};

struct For_Statement {
    String_View iter;
    PACKL_Type iter_type;
    PACKL_Args args;
    AST *body;
};

struct Var_Declaration {
    String_View name;
    PACKL_Type type;
    Expression value;
};

enum Method_Kind {
    METHOD_KIND_FUNCTION = 0,
    METHOD_KIND_PROCEDURE,
};

union Method_Def_As {
    Func_Def func;
    Proc_Def proc;
};

struct Method_Def {
    Method_Kind kind;
    String_View class_name;
    Method_Def_As as;
};

struct Func_Call {
    String_View name;
    PACKL_Args args;
};

struct Method_Call {
    String_View object_name;
    Func_Call func;
};

struct Use {
    String_View filename;
    String_View alias;
    int has_alias;
};

enum Mod_Call_Kind {
    MODULE_CALL_FUNC_CALL = 0,
    MODULE_CALL_VARIABLE,
};

union Mod_Call_As {
    Func_Call func_call;
    String_View var_name;
};

struct Mod_Call {
    String_View name;
    Mod_Call_Kind kind;
    Mod_Call_As as;
};

struct Class_Def {
    String_View name;
    Attributes attrs;
};

union Node_As {
    Func_Call func_call;
    Method_Call method_call;

    Proc_Def proc_def;
    Func_Def func_def;
    Method_Def method_def;

    Expression expr;
    
    If_Statement fi;
    While_Statement hwile;
    For_Statement rof;
    
    Var_Declaration var_dec;
    Var_Reassign var;
    Expression ret;
    
    Use use;
    Mod_Call mod_call;
    Class_Def class;
};

struct Node {
    Node_Kind kind;
    Node_As as;
    Location loc;
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

struct Variable {
    PACKL_Type type;
    size_t stack_pos;  
};

struct Function {
    size_t label_value;
    Parameters params;
    PACKL_Type return_type;
};

struct Module {
    char *filename;
};

union Method_As {
    Function func;
    Procedure proc;
};

struct Method {
    String_View name;
    Method_Kind kind;
    Method_As as;
};

struct Methods {
    Method *items;
    size_t count;
    size_t size;
};   

struct Class {
    String_View name;
    size_t attrs_size;
    Attributes attrs;
    Methods methods;
};

enum Context_Item_Type {
    CONTEXT_ITEM_TYPE_PROCEDURE = 0,
    CONTEXT_ITEM_TYPE_VARIABLE,
    CONTEXT_ITEM_TYPE_FUNCTION,
    CONTEXT_ITEM_TYPE_MODULE,
    CONTEXT_ITEM_TYPE_CLASS,
    COUNT_CONTEXT_ITEM_TYPES,
};

union Context_Item_As {
    Procedure proc;
    Variable variable;
    Function func;
    Module module;
    Class class;
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
    size_t stack_size;
};

struct Contexts {
    Context *items;
    size_t count;
    size_t size;
};

struct PACKL_External_Files {
    PACKL_File *items;
    size_t count;
    size_t size;
};

struct PACKL_File {
    char *filename;
    char *fullpath;

    Tokens tokens;
    AST ast;

    Lexer lexer;
    Parser parser;

    Contexts contexts;

    Strings root_files;
    PACKL_External_Files used_files;
};

struct PACKL_Compiler {
    char *output;    
    FILE *f;
    char *entry_file_path;
    bool has_entry;
    PACKL_File root_file;
    size_t label_value;
    size_t stack_size;
};


#endif