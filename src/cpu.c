#include <stdlib.h>
#include <stdio.h>

#include "include/ild.h"
#include "include/cpu.h"

 uint8_t bytereg_table[8] = {0x00, 0x02, 0x04, 0x06, 0x01, 0x03, 0x05, 0x07};




//
// Set register value
//
inline void set_reg16(i8086_cpu cpu, int reg, uint16_t value)
{
    cpu->reg.reg16[reg] = value;
}

inline void set_reg8(i8086_cpu cpu, int reg, uint8_t value)
{
    cpu->reg.reg8[bytereg_table[reg]] = value;
}

inline uint16_t get_reg16(i8086_cpu cpu,size_t reg)
{
    return cpu->reg.reg16[reg];
}

inline uint8_t get_reg8(i8086_cpu cpu, size_t reg)
{
    return cpu->reg.reg8[bytereg_table[reg]];
}

//
// Set segment register value
//
inline void set_segreg(i8086_cpu cpu, size_t reg, uint16_t value)
{
    cpu->segreg[reg] = value;
}

//
// Get segment register value
//
inline uint16_t get_segreg(i8086_cpu cpu, size_t reg)
{
    return cpu->segreg[reg];
}

void cpu_reset(i8086_cpu instruction)
{

    instruction->memory = calloc(1, RAM_MEMORY);

    set_segreg(instruction, CS, 0x00);
    set_segreg(instruction, SS, 0x00);
    set_segreg(instruction, ES, 0x00);
    set_segreg(instruction, DS, 0x00);

    instruction->IP      = 0x00;

    set_reg16(instruction, SP, 0x100);

    instruction->flags.bit.interrupt = 1; // Interrupt enabled; same as STI
    instruction->flags.bit.trace     = 0; // trace code disabled (default)
}

i8086_cpu cpu_initialization()
{
    i8086_cpu instruction = calloc(1, sizeof(_8086_cpu));
    cpu_reset(instruction);
    return instruction;
}