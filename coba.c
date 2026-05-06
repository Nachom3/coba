#include <stdint.h>
#include <stdio.h>

#define NUM_REGS   16
#define PROG_WORDS 64
#define DATA_BYTES 65536

#define OP_ADD  0x0
#define OP_LW   0x2
#define OP_SW   0x3
#define OP_BEQ  0x4
#define OP_J    0xE
#define OP_HALT 0xF

#define HALT_PC 0xFFFF

typedef struct {
    uint16_t regs[NUM_REGS];
    uint16_t pc;
    uint16_t psw;
    uint32_t program_memory[PROG_WORDS];
    uint8_t  data_memory[DATA_BYTES];
} CPU;

void cpu_init(CPU *c) {
    for (int i = 0; i < NUM_REGS; i++) c->regs[i] = 0;
    c->pc = 0;
    c->psw = 0;
    for (int i = 0; i < PROG_WORDS; i++) c->program_memory[i] = 0;
    for (int i = 0; i < DATA_BYTES; i++) c->data_memory[i] = 0;
}

uint32_t fetch(CPU *c) {
    return c->program_memory[c->pc];
}

void exec(CPU *c, uint32_t i) {
    uint8_t op = (i >> 16) & 0xF;
    uint8_t rd = (i >> 12) & 0xF;
    uint8_t rs = (i >>  8) & 0xF;
    uint8_t rt = (i >>  4) & 0xF;
    int8_t  imm = (int8_t)(i & 0xFF);
    uint16_t addr = i & 0xFFFF;

    switch (op) {
        case OP_ADD:
            c->regs[rd] = c->regs[rs] + c->regs[rt];
            break;

        case OP_LW: {
            uint16_t ea = c->regs[rs] + (uint16_t)imm;
            c->regs[rd] = c->data_memory[ea];
            break;
        }

        case OP_SW: {
            uint16_t ea = c->regs[rs] + (uint16_t)imm;
            c->data_memory[ea] = (uint8_t)(c->regs[rd] & 0xFF);
            break;
        }

        case OP_BEQ:
            if (c->regs[rd] == c->regs[rs]) {
                c->pc = (uint16_t)(c->pc + imm);
            }
            break;

        case OP_J:
            c->pc = addr - 1;
            break;

        case OP_HALT:
            c->pc = HALT_PC;
            break;
    }
}

void run(CPU *c) {
    while (c->pc < PROG_WORDS && c->pc != HALT_PC) {
        uint32_t i = fetch(c);
        exec(c, i);
        if (c->pc != HALT_PC) c->pc++;
    }
}

void cpu_print(CPU *c) {
    printf("PC: %u\n", c->pc);
    printf("PSW: %u\n", c->psw);
    for (int i = 0; i < NUM_REGS; i++) {
        printf("R%d: %u\n", i, c->regs[i]);
    }
}

int main(void) {
    CPU c;
    cpu_init(&c);

    c.regs[1] = 5;
    c.regs[2] = 7;

    c.program_memory[0] = (OP_ADD  << 16) | (3 << 12) | (1 << 8) | (2 << 4);
    c.program_memory[1] = (OP_SW   << 16) | (3 << 12) | (0 << 8) | (0x10 & 0xFF);
    c.program_memory[2] = (OP_LW   << 16) | (4 << 12) | (0 << 8) | (0x10 & 0xFF);
    c.program_memory[3] = (OP_BEQ  << 16) | (3 << 12) | (4 << 8) | (0x01 & 0xFF);
    c.program_memory[4] = (OP_HALT << 16);
    c.program_memory[5] = (OP_HALT << 16);

    run(&c);
    cpu_print(&c);
    return 0;
}
