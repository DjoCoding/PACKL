#include "pvm-instructions.h"

size_t number_of_popped_values[] = {
    2,       // write  
    1,       // read: 2 for reading and 1 for push
    0,       // alloc: 1 for the size and 1 for the push
    1,       // free
    1,       // open: 2 for the filepath and the mode and one of the file pointer (push) 
    1,       // close 
    1,       // exit
};

void fprintfln(FILE *f) {
    fprintf(f, "\n");
}

void fprint_indent(FILE *f, size_t indent) {
    for (size_t i = 0; i < indent; ++i) {
        fprintf(f, "  ");
    }
}

void packl_generate_label(PACKL_Compiler *c, size_t label_value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "#label_%zu:\n", label_value);
}

void packl_generate_push(PACKL_Compiler *c, int64_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "push %ld\n", value);
    c->stack_size++;    
}

void packl_generate_pushs(PACKL_Compiler *c, String_View value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "pushs \"" SV_FMT "\"\n", SV_UNWRAP(value));
    c->stack_size++;    
}

void packl_generate_nop(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "nop\n");
}

void packl_generate_halt(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "halt\n");    
}

void packl_generate_pop(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "pop\n");
    c->stack_size--;
}

void packl_generate_add(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "add\n");
    c->stack_size -= 1;
}

void packl_generate_sub(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "sub\n");
    c->stack_size -= 1;
}

void packl_generate_mul(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "mul\n");
    c->stack_size -= 1;
}

void packl_generate_div(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "div\n");
    c->stack_size -= 1;
}

void packl_generate_mod(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "mod\n");
    c->stack_size -= 1;
}

void packl_generate_cmpl(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "cmpl\n");
    c->stack_size -= 1;
}

void packl_generate_cmpg(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "cmpg\n");
    c->stack_size -= 1;
}

void packl_generate_cmpge(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "cmpge\n");
    c->stack_size -= 1;
}

void packl_generate_cmple(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "cmple\n");
    c->stack_size -= 1;
}

void packl_generate_cmpe(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "cmpe\n");
    c->stack_size -= 1;
}

void packl_generate_cmpne(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "cmpne\n");
    c->stack_size -= 1;
}

void packl_generate_swap(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "swap\n");
}

void packl_generate_dup(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "dup\n");
    c->stack_size++;
}

void packl_generate_inswap(PACKL_Compiler *c, int64_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "inswap %ld\n", value);
}

void packl_generate_indup(PACKL_Compiler *c, int64_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "indup %ld\n", value);
    c->stack_size++;
} 

void packl_generate_syscall(PACKL_Compiler *c, int64_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "syscall %ld\n", value);
    c->stack_size -= number_of_popped_values[value];
}

void packl_generate_writei(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "writei\n");
    c->stack_size -= 2;
}

void packl_generate_jmp(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "jmp $label_%zu\n", value);
}

void packl_generate_cmp(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "cmp\n");
    c->stack_size -= 1;
}

void packl_generate_jz(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "jz $label_%zu\n", value);
    c->stack_size--;
}


void packl_generate_jle(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "jle $label_%zu\n", value);
    c->stack_size--;
}


void packl_generate_jl(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "jl $label_%zu\n", value);
    c->stack_size--;
}


void packl_generate_jge(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "jge $label_%zu\n", value);
    c->stack_size--;
}


void packl_generate_jg(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "jg $label_%zu\n", value);
    c->stack_size--;
}

void packl_generate_putc(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "putc\n");
    c->stack_size--; // TODO: see if you can change this line, i think it's not correct
}

void packl_generate_call(PACKL_Compiler *c, size_t value, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "call $label_%zu\n", value);
}

void packl_generate_ret(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "ret\n");
}

void packl_generate_store(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "store\n");
    c->stack_size -= 3;
}

void packl_generate_load(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "load\n");
    c->stack_size -= 1;
}

void packl_generate_readc(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "readc\n");
    c->stack_size++;
}

void packl_generate_loadb(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "loadb\n");
}

void packl_generate_storeb(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "strb\n");
    c->stack_size -= 2;
}

void packl_generate_ssp(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "ssp\n");
    c->stack_size -= 1;
}

void packl_generate_not(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "not\n");
}

void packl_generate_and(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "and\n");
    c->stack_size -= 1;
}

void packl_generate_or(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "or\n");
    c->stack_size -= 1;
}

void packl_generate_xor(PACKL_Compiler *c, size_t indent) {
    fprint_indent(c->f, indent);
    fprintf(c->f, "xor\n");
    c->stack_size -= 1;
}
