#ifndef CODEGOLF_8086_CPU_H
#define CODEGOLF_8086_CPU_H

#include <stdint.h>
#include <memory.h>
#include <stdbool.h>

#define   RAM_MEMORY            0x100000

// Registers
#define    AX     0x00
#define    CX     0x01
#define    DX     0x02
#define    BX     0x03
#define    SP     0x04
#define    BP     0x05
#define    SI     0x06
#define    DI     0x07

#define    AL     0x00
#define    CL     0x01
#define    DL     0x02
#define    BL     0x03
#define    AH     0x04
#define    CH     0x05
#define    DH     0x06
#define    BH     0x07

#define    ES     0x00
#define    CS     0x01
#define    SS     0x02
#define    DS     0x03


typedef struct _8086_cpu_arch {
    union  _reg {
        uint16_t reg16[8];    // AX, CX, DX, BX, SP, BP, SI, DI
        uint8_t  reg8[8];     // [AL, AH], [CL, CH], [DL, DH], [BL, BH]
    } reg;

    uint16_t   segreg[4];     // ES, CS, SS, DS

    uint16_t   IP;            // Instruction Pointer

    // FLAGS register
    union
    {
        struct
        {
            uint8_t carry       : 1;  // Carry flag     : unsigned overflow and other operations

            uint8_t reserved_2  : 1;
            uint8_t parity      : 1;  // Parity flag    : if the L.O part of a register have an even number of 1 bits, this bit is set to 1

            uint8_t reserved_4  : 1;

            uint8_t auxiliary   : 1;  // Auxiliary carry flag : supports special binary coded decimal number (BCD)

            uint8_t reserved_6  : 1;

            uint8_t zero        : 1;  // Zero flag      : set to 0 if value are equals
            uint8_t sign        : 1;  // Sign flag      : set to 1 if a result of a operation is negative
            uint8_t trace       : 1;  // Trace flag     : enable/disable trace code. Trace code is used by debuggers
            uint8_t interrupt   : 1;  // Interrupt flag : enable/disable interrupt from external sources
            uint8_t direction   : 1;  // Direction flag : when set to 1, the CPU process string from hight address to low (when cleared in the opposite direction)
            uint8_t overflow    : 1;  // Overfow flag   : set if the number is too large (eg: 7FFFh + 0001h = 80000h); if there is no overflow, the CPU set it to 0

            uint8_t reserved_13 : 1;
            uint8_t reserved_14 : 1;
            uint8_t reserved_15 : 1;
            uint8_t reserved_16 : 1;  // Reserved (not used by 8086)

        } bit;

        int16_t value;
    } flags;

    uint8_t current_op;
    bool    jmp_taken;
    bool    unsupported_op;

    bool    hlt_suspend;

    uint8_t *memory;
} _8086_cpu, *i8086_cpu;

i8086_cpu cpu_initialization();
void cpu_reset(i8086_cpu instruction);
void set_segreg(i8086_cpu cpu, size_t reg, uint16_t value);
void set_reg16(i8086_cpu cpu, int reg, uint16_t value);
void set_reg8(i8086_cpu cpu, int reg, uint8_t value);
uint16_t get_segreg(i8086_cpu cpu, size_t reg);
uint8_t get_reg8(i8086_cpu cpu, size_t reg);
uint16_t get_reg16(i8086_cpu cpu,size_t reg);


#endif //CODEGOLF_8086_CPU_H
