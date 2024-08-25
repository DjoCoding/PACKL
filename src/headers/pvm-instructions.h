#ifndef PVM_INSTRUCTIONS_H
#define PVM_INSTRUCTIONS_H

#include "packl-defs.h"

#define PACKL_COMMENT(f, indent, ...) { fprint_indent(f, indent); fprintf(f, "; " __VA_ARGS__); fprintf(f, "\n"); }

void fprintfln(FILE *f);
void fprint_indent(FILE *f, size_t indent);
void packl_generate_label(PACKL_Compiler *c, size_t label_value, size_t indent);
void packl_generate_push(PACKL_Compiler *c, int64_t value, size_t indent);
void packl_generate_pushs(PACKL_Compiler *c, String_View value, size_t indent);
void packl_generate_nop(PACKL_Compiler *c, size_t indent);
void packl_generate_halt(PACKL_Compiler *c, size_t indent);
void packl_generate_pop(PACKL_Compiler *c, size_t indent);
void packl_generate_add(PACKL_Compiler *c, size_t indent);
void packl_generate_sub(PACKL_Compiler *c, size_t indent);
void packl_generate_mul(PACKL_Compiler *c, size_t indent);
void packl_generate_div(PACKL_Compiler *c, size_t indent);
void packl_generate_mod(PACKL_Compiler *c, size_t indent);
void packl_generate_cmpl(PACKL_Compiler *c, size_t indent);
void packl_generate_cmpg(PACKL_Compiler *c, size_t indent);
void packl_generate_cmpge(PACKL_Compiler *c, size_t indent);
void packl_generate_cmple(PACKL_Compiler *c, size_t indent);
void packl_generate_cmpe(PACKL_Compiler *c, size_t indent);
void packl_generate_cmpne(PACKL_Compiler *c, size_t indent);
void packl_generate_swap(PACKL_Compiler *c, size_t indent);
void packl_generate_dup(PACKL_Compiler *c, size_t indent);
void packl_generate_inswap(PACKL_Compiler *c, int64_t value, size_t indent);
void packl_generate_indup(PACKL_Compiler *c, int64_t value, size_t indent);
void packl_generate_syscall(PACKL_Compiler *c, int64_t value, size_t indent);
void packl_generate_writei(PACKL_Compiler *c, size_t indent);
void packl_generate_jmp(PACKL_Compiler *c, size_t value, size_t indent);
void packl_generate_cmp(PACKL_Compiler *c, size_t indent);
void packl_generate_jz(PACKL_Compiler *c, size_t value, size_t indent);
void packl_generate_jle(PACKL_Compiler *c, size_t value, size_t indent);
void packl_generate_jl(PACKL_Compiler *c, size_t value, size_t indent);
void packl_generate_jge(PACKL_Compiler *c, size_t value, size_t indent);
void packl_generate_jg(PACKL_Compiler *c, size_t value, size_t indent);
void packl_generate_putc(PACKL_Compiler *c, size_t indent);
void packl_generate_call(PACKL_Compiler *c, size_t value, size_t indent);
void packl_generate_ret(PACKL_Compiler *c, size_t indent);
void packl_generate_store(PACKL_Compiler *c, size_t indent);
void packl_generate_load(PACKL_Compiler *c, size_t indent);
void packl_generate_readc(PACKL_Compiler *c, size_t indent);
void packl_generate_loadb(PACKL_Compiler *c, size_t indent);
void packl_generate_storeb(PACKL_Compiler *c, size_t indent);
void packl_generate_ssp(PACKL_Compiler *c, size_t indent);
void packl_generate_not(PACKL_Compiler *c, size_t indent);
void packl_generate_and(PACKL_Compiler *c, size_t indent);
void packl_generate_or(PACKL_Compiler *c, size_t indent);
void packl_generate_xor(PACKL_Compiler *c, size_t indent);

#endif 