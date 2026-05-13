
#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define NUM_REGS   32
#define MEM_SIZE   256
#define PROG_SIZE  256

#define OP_RTYPE  0x00
#define OP_LW     0x23
#define OP_SW     0x2B
#define OP_BEQ    0x04

#define F_ADD  0x20
#define F_SUB  0x22
#define F_AND  0x24
#define F_OR   0x25
#define F_SLT  0x2A

typedef uint32_t u32;
typedef int32_t  s32;

u32 regs[NUM_REGS];
u32 pc;
s32 data_mem[MEM_SIZE];
u32 prog_mem[PROG_SIZE];
int prog_size;

u32 asm_r(int funct, int rs, int rt, int rd) {
    return ((u32)OP_RTYPE << 26) | ((u32)rs << 21) |
           ((u32)rt << 16) | ((u32)rd << 11) | (u32)funct;
}

u32 asm_i(int op, int rs, int rt, int imm) {
    return ((u32)op << 26) | ((u32)rs << 21) |
           ((u32)rt << 16) | ((u32)(uint16_t)(int16_t)imm & 0xFFFF);
}

void dump_regs(void) {
    printf("\nRegistros:\n");
    for (int i = 0; i < NUM_REGS; i++)
        if (regs[i] != 0)
            printf("  $%d = %d\n", i, (s32)regs[i]);
    printf("  PC = 0x%X\n", pc);
}

void dump_mem(int from, int count) {
    printf("\nMemoria de datos:\n");
    for (int i = from; i < from + count && i < MEM_SIZE; i++)
        if (data_mem[i] != 0)
            printf("  mem[%d] = %d\n", i, data_mem[i]);
}

int step(void) {
    u32 idx = pc / 4;
    if (idx >= (u32)prog_size) {
        printf("HALT: fin de programa\n");
        return 0;
    }

    u32 raw = prog_mem[idx];
    pc += 4;

    int opcode  = (raw >> 26) & 0x3F;
    int rs      = (raw >> 21) & 0x1F;
    int rt      = (raw >> 16) & 0x1F;
    int rd      = (raw >> 11) & 0x1F;
    int funct   =  raw        & 0x3F;
    int16_t imm = (int16_t)(raw & 0xFFFF);

    regs[0] = 0;

    switch (opcode) {
        case OP_RTYPE: {
            s32 a = (s32)regs[rs], b = (s32)regs[rt];
            switch (funct) {
                case F_ADD: regs[rd] = (u32)(a + b); break;
                case F_SUB: regs[rd] = (u32)(a - b); break;
                case F_AND: regs[rd] = regs[rs] & regs[rt]; break;
                case F_OR:  regs[rd] = regs[rs] | regs[rt]; break;
                case F_SLT: regs[rd] = (a < b) ? 1 : 0; break;
                default: printf("funct desconocido: 0x%X\n", funct);
            }
            break;
        }
        case OP_LW: {
            u32 addr = ((s32)regs[rs] + imm) / 4;
            regs[rt] = (u32)data_mem[addr % MEM_SIZE];
            break;
        }
        case OP_SW: {
            u32 addr = ((s32)regs[rs] + imm) / 4;
            data_mem[addr % MEM_SIZE] = (s32)regs[rt];
            break;
        }
        case OP_BEQ: {
            if (regs[rs] == regs[rt])
                pc = (u32)((s32)pc + imm * 4);
            break;
        }
        default:
            printf("opcode desconocido: 0x%X\n", opcode);
    }

    regs[0] = 0;
    return 1;
}


void load_program(void) {
    prog_mem[0] = asm_i(OP_LW,  0,  8,  0);
    prog_mem[1] = asm_i(OP_LW,  0,  9,  4);
    prog_mem[2] = asm_r(F_ADD,  8,  9, 10);
    prog_mem[3] = asm_i(OP_SW,  0, 10,  8);
    prog_mem[4] = asm_i(OP_BEQ, 8,  9,  1);
    prog_mem[5] = asm_r(F_ADD, 10,  8, 10);
    prog_size = 6;

    data_mem[0] = 10;
    data_mem[1] = 25;
}

int main(void) {
    memset(regs,     0, sizeof(regs));
    memset(data_mem, 0, sizeof(data_mem));
    pc = 0;

    load_program();

    printf("=== Emulador MIPS ===\n");
    printf("Datos iniciales: mem[0]=%d, mem[1]=%d\n\n", data_mem[0], data_mem[1]);

    int ciclo = 0;
    while (step())
        printf("Ciclo %d | PC=0x%X | $t0=%d $t1=%d $t2=%d\n",
               ++ciclo, pc, (s32)regs[8], (s32)regs[9], (s32)regs[10]);

    dump_regs();
    dump_mem(0, 5);

    return 0;
}
