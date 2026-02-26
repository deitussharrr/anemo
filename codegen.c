#include "codegen.h"

#include "utils.h"

#include <stdio.h>
#include <string.h>

static const char *arg_reg64(int i) {
    static const char *regs[] = {"%rdi", "%rsi", "%rdx", "%rcx", "%r8", "%r9"};
    return regs[i];
}

static const char *label_for_fn(const char *name) {
    if (strcmp(name, "main") == 0) {
        return "main";
    }
    static char buf[256];
    snprintf(buf, sizeof(buf), "anemo_%s", name);
    return buf;
}

static int stack_slot_offset(int slot_index) {
    return -8 * (slot_index + 1);
}

static int temp_slot(const IRFunction *fn, int temp_id) {
    return (int)fn->vars.len + temp_id;
}

static void emit_escape_cstr(FILE *out, const char *s) {
    fputc('"', out);
    for (const unsigned char *p = (const unsigned char *)s; *p; p++) {
        unsigned char c = *p;
        switch (c) {
            case '\n': fputs("\\n", out); break;
            case '\t': fputs("\\t", out); break;
            case '\r': fputs("\\r", out); break;
            case '\\': fputs("\\\\", out); break;
            case '"': fputs("\\\"", out); break;
            default:
                if (c < 32 || c > 126) {
                    fprintf(out, "\\x%02x", c);
                } else {
                    fputc((int)c, out);
                }
                break;
        }
    }
    fputc('"', out);
}

static void emit_rodata(FILE *out, const IRProgram *ir) {
    fprintf(out, ".section .rodata\n");
    fprintf(out, ".LC_fmt_int:\n  .string \"%%ld\\n\"\n");
    fprintf(out, ".LC_fmt_str:\n  .string \"%%s\\n\"\n");
    fprintf(out, ".LC_bool_yes:\n  .string \"yes\"\n");
    fprintf(out, ".LC_bool_no:\n  .string \"no\"\n");

    for (size_t i = 0; i < ir->strings.len; i++) {
        fprintf(out, ".LC_str_%d:\n  .string ", ir->strings.items[i].id);
        emit_escape_cstr(out, ir->strings.items[i].value);
        fputc('\n', out);
    }
    fputc('\n', out);
}

static void load_slot(FILE *out, int offset, const char *reg) {
    fprintf(out, "  movq %d(%%rbp), %s\n", offset, reg);
}

static void store_slot(FILE *out, int offset, const char *reg) {
    fprintf(out, "  movq %s, %d(%%rbp)\n", reg, offset);
}

static void load_temp(FILE *out, const IRFunction *fn, int temp, const char *reg) {
    int off = stack_slot_offset(temp_slot(fn, temp));
    load_slot(out, off, reg);
}

static void store_temp(FILE *out, const IRFunction *fn, int temp, const char *reg) {
    int off = stack_slot_offset(temp_slot(fn, temp));
    store_slot(out, off, reg);
}

static void emit_binop(FILE *out, const IRFunction *fn, const IRInstr *in) {
    load_temp(out, fn, in->src1, "%rax");
    load_temp(out, fn, in->src2, "%rbx");

    switch (in->binop) {
        case IRBIN_ADD:
            fprintf(out, "  addq %%rbx, %%rax\n");
            break;
        case IRBIN_SUB:
            fprintf(out, "  subq %%rbx, %%rax\n");
            break;
        case IRBIN_MUL:
            fprintf(out, "  imulq %%rbx, %%rax\n");
            break;
        case IRBIN_DIV:
            fprintf(out, "  cqto\n");
            fprintf(out, "  idivq %%rbx\n");
            break;
        case IRBIN_BOTH:
            fprintf(out, "  andq %%rbx, %%rax\n");
            fprintf(out, "  cmpq $0, %%rax\n");
            fprintf(out, "  setne %%al\n");
            fprintf(out, "  movzbq %%al, %%rax\n");
            break;
        case IRBIN_EITHER:
            fprintf(out, "  orq %%rbx, %%rax\n");
            fprintf(out, "  cmpq $0, %%rax\n");
            fprintf(out, "  setne %%al\n");
            fprintf(out, "  movzbq %%al, %%rax\n");
            break;
        case IRBIN_SAME:
            fprintf(out, "  cmpq %%rbx, %%rax\n");
            fprintf(out, "  sete %%al\n");
            fprintf(out, "  movzbq %%al, %%rax\n");
            break;
        case IRBIN_DIFF:
            fprintf(out, "  cmpq %%rbx, %%rax\n");
            fprintf(out, "  setne %%al\n");
            fprintf(out, "  movzbq %%al, %%rax\n");
            break;
        case IRBIN_LESS:
            fprintf(out, "  cmpq %%rbx, %%rax\n");
            fprintf(out, "  setl %%al\n");
            fprintf(out, "  movzbq %%al, %%rax\n");
            break;
        case IRBIN_MORE:
            fprintf(out, "  cmpq %%rbx, %%rax\n");
            fprintf(out, "  setg %%al\n");
            fprintf(out, "  movzbq %%al, %%rax\n");
            break;
        case IRBIN_ATMOST:
            fprintf(out, "  cmpq %%rbx, %%rax\n");
            fprintf(out, "  setle %%al\n");
            fprintf(out, "  movzbq %%al, %%rax\n");
            break;
        case IRBIN_ATLEAST:
            fprintf(out, "  cmpq %%rbx, %%rax\n");
            fprintf(out, "  setge %%al\n");
            fprintf(out, "  movzbq %%al, %%rax\n");
            break;
    }

    store_temp(out, fn, in->dst, "%rax");
}

static void emit_unop(FILE *out, const IRFunction *fn, const IRInstr *in) {
    load_temp(out, fn, in->src1, "%rax");
    if (in->unop == IRUN_NEG) {
        fprintf(out, "  negq %%rax\n");
    } else {
        fprintf(out, "  cmpq $0, %%rax\n");
        fprintf(out, "  sete %%al\n");
        fprintf(out, "  movzbq %%al, %%rax\n");
    }
    store_temp(out, fn, in->dst, "%rax");
}

static void emit_chant(FILE *out, const IRFunction *fn, const IRInstr *in) {
    load_temp(out, fn, in->src1, "%rax");

    if (in->type == TYPE_INT) {
        fprintf(out, "  movq %%rax, %%rsi\n");
        fprintf(out, "  leaq .LC_fmt_int(%%rip), %%rdi\n");
        fprintf(out, "  xor %%eax, %%eax\n");
        fprintf(out, "  call printf@PLT\n");
    } else if (in->type == TYPE_STRING) {
        fprintf(out, "  movq %%rax, %%rsi\n");
        fprintf(out, "  leaq .LC_fmt_str(%%rip), %%rdi\n");
        fprintf(out, "  xor %%eax, %%eax\n");
        fprintf(out, "  call printf@PLT\n");
    } else {
        fprintf(out, "  cmpq $0, %%rax\n");
        fprintf(out, "  leaq .LC_bool_no(%%rip), %%rsi\n");
        fprintf(out, "  leaq .LC_bool_yes(%%rip), %%rdx\n");
        fprintf(out, "  cmovne %%rdx, %%rsi\n");
        fprintf(out, "  leaq .LC_fmt_str(%%rip), %%rdi\n");
        fprintf(out, "  xor %%eax, %%eax\n");
        fprintf(out, "  call printf@PLT\n");
    }
}

static void emit_function(FILE *out, const IRFunction *fn) {
    const char *fname = label_for_fn(fn->name);
    fprintf(out, ".text\n");
    fprintf(out, ".globl %s\n", fname);
    fprintf(out, "%s:\n", fname);

    int slots = (int)fn->vars.len + fn->temp_count;
    int stack_size = slots * 8;
    if (stack_size % 16 != 0) {
        stack_size += 8;
    }

    fprintf(out, "  pushq %%rbp\n");
    fprintf(out, "  movq %%rsp, %%rbp\n");
    if (stack_size > 0) {
        fprintf(out, "  subq $%d, %%rsp\n", stack_size);
    }

    if (fn->param_count > 6) {
        fatal("codegen supports at most 6 parameters");
    }
    for (int i = 0; i < fn->param_count; i++) {
        int off = stack_slot_offset(i);
        fprintf(out, "  movq %s, %d(%%rbp)\n", arg_reg64(i), off);
    }

    int end_label = 900000;

    for (size_t i = 0; i < fn->code.len; i++) {
        IRInstr *in = &fn->code.items[i];
        switch (in->op) {
            case IROP_LABEL:
                fprintf(out, ".L_%s_%d:\n", fn->name, in->label);
                break;
            case IROP_JMP:
                fprintf(out, "  jmp .L_%s_%d\n", fn->name, in->label);
                break;
            case IROP_JMP_FALSE:
                load_temp(out, fn, in->src1, "%rax");
                fprintf(out, "  cmpq $0, %%rax\n");
                fprintf(out, "  je .L_%s_%d\n", fn->name, in->label);
                break;
            case IROP_IMM_INT:
            case IROP_IMM_BOOL:
                fprintf(out, "  movq $%ld, %%rax\n", in->imm);
                store_temp(out, fn, in->dst, "%rax");
                break;
            case IROP_IMM_STR:
                fprintf(out, "  leaq .LC_str_%ld(%%rip), %%rax\n", in->imm);
                store_temp(out, fn, in->dst, "%rax");
                break;
            case IROP_LOAD_VAR: {
                int off = stack_slot_offset(in->var_index);
                load_slot(out, off, "%rax");
                store_temp(out, fn, in->dst, "%rax");
                break;
            }
            case IROP_STORE_VAR: {
                int off = stack_slot_offset(in->var_index);
                load_temp(out, fn, in->src1, "%rax");
                store_slot(out, off, "%rax");
                break;
            }
            case IROP_BIN:
                emit_binop(out, fn, in);
                break;
            case IROP_UN:
                emit_unop(out, fn, in);
                break;
            case IROP_CALL: {
                if (in->argc > 6) {
                    fatal("codegen supports at most 6 call arguments");
                }
                for (int a = 0; a < in->argc; a++) {
                    load_temp(out, fn, in->args[a], arg_reg64(a));
                }
                fprintf(out, "  call %s\n", label_for_fn(in->name));
                if (in->dst >= 0) {
                    store_temp(out, fn, in->dst, "%rax");
                }
                break;
            }
            case IROP_CHANT:
                emit_chant(out, fn, in);
                break;
            case IROP_RET:
                if (in->has_value) {
                    load_temp(out, fn, in->src1, "%rax");
                } else {
                    fprintf(out, "  movq $0, %%rax\n");
                }
                fprintf(out, "  jmp .L_%s_%d\n", fn->name, end_label);
                break;
        }
    }

    fprintf(out, ".L_%s_%d:\n", fn->name, end_label);
    fprintf(out, "  leave\n");
    fprintf(out, "  ret\n\n");
}

void codegen_emit_assembly(const IRProgram *ir, const char *asm_path) {
    FILE *out = fopen(asm_path, "wb");
    if (!out) {
        fatal("cannot open assembly output '%s'", asm_path);
    }

    fprintf(out, ".extern printf\n\n");
    emit_rodata(out, ir);
    for (size_t i = 0; i < ir->functions.len; i++) {
        emit_function(out, &ir->functions.items[i]);
    }

    fclose(out);
}
