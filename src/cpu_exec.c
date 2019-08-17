#include <ild.h>

#include <stdio.h>
#include <cpu.h>

#include "include/cpu_exec.h"


typedef void (*ob_opcodes)(i8086_cpu cpu, _x86_instr instr);
static ob_opcodes one_byte_opcodes[256];


void push(i8086_cpu cpu, uint16_t value)
{
    cpu->reg.reg16[SP] -= 2;
    int32_t offset = cpu->reg.reg16[SP];
    cpu_write_mem8(cpu, offset, (value & 0xFF));
    cpu_write_mem8(cpu, offset-1, ((value >> 8) & 0xFF));
}

uint16_t pop(i8086_cpu cpu, _x86_instr instr)
{
    int32_t offset =  cpu->reg.reg16[SP];
    uint16_t value = cpu_read_mem8(cpu, offset-1) << 8 | cpu_read_mem8(cpu, offset);
    cpu->reg.reg16[SP] += 2;

    return value;
}

inline void setflag_sz16(i8086_cpu cpu,uint16_t value)
{
    cpu->flags.bit.zero = (!value) ? 1 : 0;
    cpu->flags.bit.sign = (value & 0x8000) >> 15;
}

inline void set_flag16(i8086_cpu cpu,uint16_t op1, uint16_t op2)
{
    uint32_t result = op1 - op2;
    setflag_sz16(cpu, (uint16_t)result);

    cpu->flags.bit.carry = (result & 0xFFFF0000) ? 1 : 0;
}

inline void set_flag_add16(i8086_cpu cpu, uint16_t op1, uint16_t op2)
{
    uint32_t result = op1 + op2;
    cpu->flags.bit.carry = (result & 0xFFFF0000) ? 1 : 0;
}

inline void set_flag_sub16(i8086_cpu cpu, uint16_t op1, uint16_t op2)
{
    uint32_t result = op1 - op2;
    setflag_sz16(cpu, result);
    cpu->flags.bit.carry = (result & 0xFFFF0000) ? 1 : 0;
}

inline void set_flag_xor16(i8086_cpu cpu, uint16_t op1, uint16_t op2)
{
    cpu->flags.bit.carry    ^= cpu->flags.bit.carry;
    cpu->flags.bit.overflow ^= cpu->flags.bit.overflow;

    setflag_sz16(cpu, op1 ^ op2);
}

inline void set_flag_or16(i8086_cpu cpu, uint16_t op1, uint16_t op2)
{
    cpu->flags.bit.carry    ^= cpu->flags.bit.carry;
    cpu->flags.bit.overflow ^= cpu->flags.bit.overflow;

    setflag_sz16(cpu, op1 | op2);
}

inline void set_flag_and16(i8086_cpu cpu,uint16_t op1, uint16_t op2)
{
    uint32_t result = op1 & op2;
    setflag_sz16(cpu, result);

    cpu->flags.bit.carry = 0;
    cpu->flags.bit.overflow = 0;
}

inline void set_flag_or8(i8086_cpu cpu, uint16_t op1, uint8_t op2)
{
    cpu->flags.bit.carry    ^= cpu->flags.bit.carry;
    cpu->flags.bit.overflow ^= cpu->flags.bit.overflow;

    setflag_sz8(cpu, op1 | op2);
}

inline void set_flag_add8(i8086_cpu cpu,uint8_t op1, uint8_t op2)
{
    uint16_t result = op1 + op2;
    cpu->flags.bit.carry = (result & 0xFF00) ? 1 : 0;
}

inline void setflag_sz8(i8086_cpu cpu,uint8_t value)
{
    cpu->flags.bit.zero = (!value) ? 1 : 0;
    cpu->flags.bit.sign = (value & 0x80) >> 7;
}

inline void set_flag_and8(i8086_cpu cpu,uint8_t op1, uint8_t op2)
{
    uint16_t result = op1 & op2;
    setflag_sz8(cpu, (uint8_t)result);

    cpu->flags.bit.carry = 0;
    cpu->flags.bit.overflow = 0;
}

inline void set_flag8(i8086_cpu cpu,uint8_t op1, uint8_t op2)
{
    uint16_t result = op1 - op2;
    setflag_sz8(cpu, (uint8_t)result);
    cpu->flags.bit.carry = (result & 0xFF00) ? 1 : 0;
}

// Effective Address (without segments)
inline int32_t get_ea(i8086_cpu cpu, _x86_instr instr, uint8_t mod, uint8_t rm)
{
    uint16_t ea = 0;

    switch(rm)
    {
        case 0x00:
            ea = get_reg16(cpu, BX) + get_reg16(cpu,SI);
            break;

        case 0x01:
            ea = get_reg16(cpu,BX) + get_reg16(cpu,DI);
            break;

        case 0x02:
            ea = get_reg16(cpu,BP) + get_reg16(cpu,SI);
            break;

        case 0x03:
            ea = get_reg16(cpu,BP) + get_reg16(cpu,DI);
            break;

        case 0x04:
            ea = get_reg16(cpu,SI);
            break;

        case 0x05:
            ea = get_reg16(cpu,DI);
            break;

        case 0x06:

            switch(instr.modregrm.field.mod)
            {
                case 0x00:
                    ea = instr.disp;
                    break;
                case 0x01:
                case 0x02:
                    ea = get_reg16(cpu, BP);
                    break;
            }

            break;

        case 0X07:
            ea = get_reg16(cpu, BX);
            break;
    }

    if(mod & 0x1)
        ea += instr.disp;
    else if(mod & 0x02)
        ea += instr.disp;

    return ea;
}


static uint16_t readmem16(i8086_cpu cpu, _x86_instr instr)
{
    if(instr.instr_flags & X86_DISP_FLAG)
        return cpu_read_mem16(cpu, get_ea(cpu, instr, instr.modregrm.field.mod, instr.modregrm.field.rm));
    else
        return get_reg16(cpu, instr.modregrm.field.rm);
}

static uint16_t readmem8(i8086_cpu cpu, _x86_instr instr)
{
    if(instr.instr_flags & X86_DISP_FLAG)
        return cpu_read_mem8(cpu,get_ea(cpu, instr, instr.modregrm.field.mod, instr.modregrm.field.rm));
    else
        return get_reg8(cpu, instr.modregrm.field.rm);
}

inline void cpu_write_mem8(i8086_cpu cpu, int32_t address, uint8_t value)
{
    cpu->memory[address] = value;
}

//
// get 2bytes of memory at the specified offset
//
inline uint16_t cpu_read_mem16(i8086_cpu cpu, int32_t offset)
{
    return (cpu->memory[offset+1] << 8) |  cpu->memory[offset];
}

inline uint8_t cpu_read_mem8(i8086_cpu cpu, int32_t offset)
{
    return cpu->memory[offset];
}

static void writemem16(i8086_cpu cpu, _x86_instr instr, uint16_t value)
{
    if(instr.modregrm.field.mod < 3)
    {
        int32_t ea = get_ea(cpu, instr, instr.modregrm.field.mod, instr.modregrm.field.rm);
        cpu_write_mem8(cpu, ea, (uint8_t) value);
        cpu_write_mem8(cpu, ea+1, (uint8_t) (value >> 8));
    }
    else
    {
        set_reg16(cpu, instr.modregrm.field.rm, value);
    }
}

static void writemem8(i8086_cpu cpu, _x86_instr instr, uint8_t value)
{
    if(instr.modregrm.field.mod < 3)
    {
        int32_t ea = get_ea(cpu, instr, instr.modregrm.field.mod, instr.modregrm.field.rm);
        cpu_write_mem8(cpu, ea, (uint8_t) value);
    }
    else
    {
        set_reg8(cpu, instr.modregrm.field.rm, value);
    }
}

//
// Instruction decode
//

void opxx(i8086_cpu cpu, _x86_instr instr)
{
    printf("Unsupported OP: %X\n", instr.op);
    cpu->unsupported_op = true;
}

/*
 * ADD  Eb, Gb
 */
static void op01(i8086_cpu cpu, _x86_instr instr)
{
    uint16_t reg  = get_reg16(cpu, instr.modregrm.field.reg);
    uint16_t data = readmem16(cpu, instr);

    set_flag_add16(cpu, reg, data);
    writemem16(cpu, instr, reg + data);
}

/*
 * ADD  AL, Ib
 */
static void op04(i8086_cpu cpu, _x86_instr instr)
{
    uint8_t reg  = get_reg8(cpu, AL);
    uint8_t data = instr.imm;

    set_reg8(cpu, AL, reg + data);
    set_flag_add8(cpu, AL, reg + data);
}

/*
 * ADD  AX, Iv
 */
static void op05(i8086_cpu cpu, _x86_instr instr)
{
    uint16_t reg  = get_reg16(cpu, AX);
    uint16_t data = instr.imm;

    set_reg16(cpu, AL, reg + data);
    set_flag_add16(cpu, AL, reg + data);
}

/*
 * OR  Ev, Gv
 */
static void op09(i8086_cpu cpu, _x86_instr instr)
{
    uint16_t reg_value = get_reg16(cpu, instr.modregrm.field.reg);
    uint16_t mem_value = readmem16(cpu, instr);

    set_flag_or16(cpu, reg_value, mem_value);
    writemem16(cpu,instr, reg_value | mem_value);
}

/*
 * SBB  Ev, Gv
 */
static void op19(i8086_cpu cpu, _x86_instr instr)
{
    uint16_t val_reg = get_reg16(cpu, instr.modregrm.field.reg);
    uint16_t val_mem = readmem16(cpu, instr);

    writemem16(cpu,instr, (val_mem-val_reg+cpu->flags.bit.carry));
    set_flag16(cpu, val_mem, val_reg+cpu->flags.bit.carry);
}

/*
 * AND  Eb, Gb
 */
static void op20(i8086_cpu cpu, _x86_instr instr)
{
    uint16_t reg_value = get_reg16(cpu, instr.modregrm.field.reg);
    uint16_t mem_value = readmem16(cpu, instr);

    set_flag_and16(cpu,mem_value, reg_value);
    writemem16(cpu, instr, reg_value & mem_value);
}

/*
 * SUB  Ev, Gv
 */
static void op29(i8086_cpu cpu, _x86_instr instr)
{
    uint16_t reg  = get_reg16(cpu, instr.modregrm.field.reg);
    uint16_t data = readmem16(cpu, instr);

    set_flag16(cpu, data, reg);
    writemem16(cpu, instr, data-reg);
}

/*
 * XOR  Ev, Gv
 */
static void op31(i8086_cpu cpu, _x86_instr instr)
{
    uint16_t reg_value = get_reg16(cpu, instr.modregrm.field.reg);
    uint16_t mem_value = readmem16(cpu, instr);

    set_flag_xor16(cpu, reg_value, mem_value);
    writemem16(cpu,instr, reg_value ^ mem_value);
}

/*
 * CMP  Ev, Gv
 */
static void op39(i8086_cpu cpu, _x86_instr instr)
{
    uint16_t op1 = get_reg16(cpu, instr.modregrm.field.reg);
    uint16_t op2 = readmem16(cpu, instr);

    set_flag16(cpu, op2, op1);
}

/*
 * CMP  AL, Ib
 */
static void op3C(i8086_cpu cpu, _x86_instr instr)
{
    uint8_t disp8    = instr.imm;
    uint8_t al_value = get_reg8(cpu, AL);

    // Set zero flag
    set_flag8(cpu, al_value, disp8);
}

/*
 *  INC  Zv
 */
static void inc(i8086_cpu cpu, _x86_instr instr)
{
    uint8_t  cf  = cpu->flags.bit.carry;
    uint16_t op1 = get_reg16(cpu, instr.op & 0x07);
    uint16_t op2 = 1;

    set_flag_add16(cpu, op1, op2);
    set_reg16(cpu, instr.op & 0x07, op1+op2);

    cpu->flags.bit.carry = cf;
}

/*
 *  DEC  Zv
 */
static void dec(i8086_cpu cpu, _x86_instr instr)
{
    uint8_t  cf  = cpu->flags.bit.carry;
    uint16_t op1 = get_reg16(cpu, instr.op & 0x07);
    uint16_t op2 = 1;

     set_flag_sub16(cpu, op1, op2);
    set_reg16(cpu, instr.op & 0x07, op1-op2);

    cpu->flags.bit.carry = cf;
}

/*
 * PUSH  Zv
 */
static void op5x(i8086_cpu cpu, _x86_instr instr)
{
    push(cpu, get_reg16(cpu, instr.op & 0x07));
}

/*
 * POP  Zv
 */
static void op58(i8086_cpu cpu, _x86_instr instr)
{
    set_reg16(cpu, instr.op & 0x07, pop(cpu, instr));
}

/*
 * JB/JNAE/JC  Jbs
 */
static void op72(i8086_cpu cpu, _x86_instr instr)
{
    cpu->jmp_taken = false;
    if(cpu->flags.bit.carry)
    {
        cpu->IP = (uint16_t) instr.label;
        cpu->jmp_taken = true;
    }
}

/*
 * JZ/JE  Jbs
 */
static void op74(i8086_cpu cpu, _x86_instr instr)
{
    cpu->jmp_taken = false;
    if(cpu->flags.bit.zero)
    {
        cpu->IP = (uint16_t) instr.label;
        cpu->jmp_taken = true;
    }
}

/*
 * JNZ/JNE  Jbs
 */
static void op75(i8086_cpu cpu, _x86_instr instr)
{
    cpu->jmp_taken = false;
    if(!cpu->flags.bit.zero)
    {
        cpu->IP = (uint16_t) instr.label;
        cpu->jmp_taken = true;
    }
}

/*
 * JBE/JNA  Jbs
 */
static void op76(i8086_cpu cpu, _x86_instr instr)
{
    cpu->jmp_taken = false;
    if((cpu->flags.bit.carry | cpu->flags.bit.zero) & 0x01)
    {
        cpu->IP = (uint16_t) instr.label;
        cpu->jmp_taken = true;
    }
}

/*
 * JNBE/JA  Jbs
 */
static void op77(i8086_cpu cpu, _x86_instr instr)
{
    cpu->jmp_taken = false;
    if(!cpu->flags.bit.carry && !cpu->flags.bit.zero)
    {
        cpu->IP = (uint16_t) instr.label;
        cpu->jmp_taken = true;
    }
}

/*
 * JNS  Jbs
 */
static void op79(i8086_cpu cpu, _x86_instr instr)
{
    cpu->jmp_taken = false;
    if(!cpu->flags.bit.sign)
    {
        cpu->IP = (uint16_t) instr.label;
        cpu->jmp_taken = true;
    }
}

/*
 * CMP  Ev, Iv
 */
static void cmp_81(i8086_cpu cpu, _x86_instr instr)
{
    // direction bit
    if(instr.op & 0x01)
    {
        uint16_t imm = instr.imm;

        if(cpu->current_op & 0x02)
            imm = (imm & 0x80) ? (imm | 0xFF00) : imm;

        set_flag16(cpu, readmem16(cpu, instr), imm);
    }
    else
    {
        uint8_t imm = instr.imm;

        if(cpu->current_op & 0x02)
            imm = (imm & 0x80) ? (imm | 0xFF00) : imm;

        set_flag8(cpu, readmem8(cpu, instr), imm);
    }
}

/*
 * ADD  Eb, Ib
 */
static void add_82(i8086_cpu cpu, _x86_instr instr)
{
    if(instr.op & 0x01)
    {
        uint16_t imm = instr.imm;
        uint16_t op  = readmem16(cpu,instr);

        // Sign bit
        if(cpu->current_op & 0x02)
            imm = (imm & 0x80) ? (imm | 0xFF00) : imm;

        set_flag16(cpu, op,imm);
        writemem16(cpu, instr, op+imm);
    }
    else
    {
        uint8_t imm = instr.imm;
        uint8_t op  = readmem8(cpu,instr);

        // Sign bit
        if(cpu->current_op & 0x02)
            imm = (imm & 0x80) ? (imm | 0xFF00) : imm;

        set_flag8(cpu, op,imm);
        writemem8(cpu, instr, op+imm);
    }
}

/*
 * ADC  Ev, Ibs
 */
static void adc_83(i8086_cpu cpu, _x86_instr instr)
{
    if(instr.op & 0x01)
    {
        uint16_t imm = instr.imm;
        uint16_t op  = readmem16(cpu,instr);

        // Sign bit
        if(cpu->current_op & 0x02)
            imm = (imm & 0x80) ? (imm | 0xFF00) : imm;

        writemem16(cpu, instr, op+imm+cpu->flags.bit.carry);
        set_flag16(cpu, op,imm);
    }
    else
    {
        uint8_t imm = instr.imm;
        uint8_t op  = readmem8(cpu,instr);

        // Sign bit
        if(cpu->current_op & 0x02)
            imm = (imm & 0x80) ? (imm | 0xFF00) : imm;

        writemem8(cpu, instr, op+imm+cpu->flags.bit.carry);
        set_flag8(cpu, op,imm);
    }
}

/*
 * AND  Eb, Ibs
 */
static void and_83(i8086_cpu cpu, _x86_instr instr)
{
    if(instr.op & 0x01)
    {
        uint16_t imm = instr.imm;
        uint16_t op  = readmem16(cpu,instr);

        if(cpu->current_op & 0x02)
            imm = (imm & 0x80) ? (imm | 0xFF00) : imm;

        set_flag_and16(cpu, op,imm);
        writemem16(cpu, instr, op&imm);
    }
    else
    {
        uint8_t imm = instr.imm;
        uint8_t op  = readmem8(cpu,instr);

        if(cpu->current_op & 0x02)
            imm = (imm & 0x80) ? (imm | 0xFF00) : imm;

        set_flag_and8(cpu, op,imm);
        writemem8(cpu, instr, op&imm);
    }
}

/*
 * SUB  Ev, Ibs
 */
static void sub_83(i8086_cpu cpu, _x86_instr instr)
{
    if(instr.op & 0x01)
    {
        uint16_t imm = instr.imm;
        uint16_t op  = readmem16(cpu,instr);

        // Sign bit
        if(cpu->current_op & 0x02)
            imm = (imm & 0x80) ? (imm | 0xFF00) : imm;

        writemem16(cpu, instr, op-imm);
        set_flag16(cpu, op,imm);
    }
    else
    {
        uint8_t imm = instr.imm;
        uint8_t op  = readmem8(cpu,instr);

        writemem8(cpu, instr, op-imm);
        set_flag8(cpu, op,imm);
    }
}

/*
 * OR  Eb, Ib
 */
static void or_82(i8086_cpu cpu, _x86_instr instr)
{
    if(instr.op & 0x01)
    {
        uint16_t reg_value = readmem16(cpu, instr);
        uint16_t mem_value = instr.imm;

        set_flag_or16(cpu, reg_value, mem_value);
        writemem16(cpu, instr, reg_value | mem_value);
    }
    else
    {
        uint8_t reg_value = readmem8(cpu, instr);
        uint8_t mem_value = instr.imm;

        set_flag_or8(cpu, reg_value, mem_value);
        writemem8(cpu, instr, reg_value | mem_value);
    }
}

// same OP, decoded using reg field of mod_reg_rm
static void op81(i8086_cpu cpu, _x86_instr instr)
{
    switch(instr.modregrm.field.reg)
    {
        case 0x00: // ADD
            add_82(cpu, instr);
            break;

        case 0x01: // OR
            or_82(cpu, instr);
            break;

        case 0x02: // ADC
            adc_83(cpu, instr);
            break;

        case 0x03: // SSB
            opxx(cpu, instr);
            break;

        case 0x04: // AND
            and_83(cpu, instr);
            break;

        case 0x05: // SUB
            sub_83(cpu, instr);
            break;

        case 0x06: // XOR
            opxx(cpu, instr);
            break;

        case 0x07: // CMP
            cmp_81(cpu, instr);
            break;
    }
}

/*
 * XCHG  Eb, Gb
 */
static void op86(i8086_cpu cpu, _x86_instr instr)
{
    uint8_t op1 = get_reg8(cpu, instr.modregrm.field.reg);
    uint8_t op2 = readmem8(cpu, instr);

    set_reg8(cpu, instr.modregrm.field.reg,op2);
    writemem8(cpu, instr, op1);
}

/*
 * MOV  Eb, Gb
 */
static void op88(i8086_cpu cpu, _x86_instr instr)
{
    writemem8(cpu,instr, get_reg8(cpu, instr.modregrm.field.reg));
}

/*
 * MOV  Ev, Gv
 */
static void op89(i8086_cpu cpu, _x86_instr instr)
{
    writemem16(cpu, instr, get_reg16(cpu, instr.modregrm.field.reg));
}

/*
 * MOV  Eb, Gb
 */
static void op8A(i8086_cpu cpu, _x86_instr instr)
{
    uint8_t value = readmem8(cpu, instr);
    set_reg8(cpu, instr.modregrm.field.reg, value);
}

/*
 * MOV  Gv, Ev
 */
static void op8B(i8086_cpu cpu, _x86_instr instr)
{
    set_reg16(cpu, instr.modregrm.field.reg, readmem16(cpu, instr));
}

/*
 * NOP
 */
static void op90(i8086_cpu cpu, _x86_instr instr)
{
 // NOP
}

/*
 * XCHG  Gv, Ev
 */
static void op92(i8086_cpu cpu, _x86_instr instr)
{
    uint16_t ax_val = get_reg16(cpu, AX);
    uint16_t regval = get_reg16(cpu, instr.op & 0x07);

    set_reg16(cpu, AX, regval);
    set_reg16(cpu, instr.op & 0x07, ax_val);
}

/*
 * MOV  Zb, Ib
 */
static void opB0(i8086_cpu cpu, _x86_instr instr)
{
    set_reg8(cpu, instr.op & 0x07, instr.imm);
}

/*
 * MOV  Zv, Iv
 */
static void opBC(i8086_cpu cpu, _x86_instr instr)
{
    set_reg16(cpu,  instr.op & 0x07, instr.imm);
}

/*
 * CALL  Jv
 */
static void opE8(i8086_cpu cpu, _x86_instr instr)
{
    push(cpu, cpu->IP+3);

    cpu->IP = (uint16_t) instr.label;
    cpu->jmp_taken = true;
}

/*
 * RETN
 */
static void opC3(i8086_cpu cpu, _x86_instr instr)
{
    cpu->IP = pop(cpu, instr);
    cpu->jmp_taken = true;
}

/*
 * MOV  Ev, Gv
 */
static void op_mov(i8086_cpu cpu, _x86_instr instr)
{
    // direction bit
    if(instr.op & 0x01)
    {
        uint16_t imm = instr.imm;
        writemem16(cpu, instr, imm);
    }
    else
    {
        uint8_t imm = instr.imm;
        writemem8(cpu, instr, imm);
    }
}

static void multiOP(i8086_cpu cpu, _x86_instr instr)
{
    switch (instr.modregrm.field.reg)
    {
        case 0x00:
            op_mov(cpu, instr);
        break;
        default:
            opxx(cpu, instr);
    }
}

/*
 * JMP  Jbs
 */
static void opEB(i8086_cpu cpu, _x86_instr instr)
{
    cpu->IP = (uint16_t) instr.label;
    cpu->jmp_taken = true;
}

/*
 * STC
 */
static void opF9(i8086_cpu cpu, _x86_instr instr)
{
    cpu->flags.bit.carry = 1;
}

/*
 * INC  Eb
 */
static void opFEadd8(i8086_cpu cpu, _x86_instr instr)
{
    uint8_t  cf  = cpu->flags.bit.carry;
    uint8_t op1 = readmem8(cpu, instr);
    uint8_t op2 = 1;

    set_flag_add8(cpu, op1, op2);
    writemem8(cpu, instr, op1+op2);

    cpu->flags.bit.carry = cf;
}

/*
 * INC  Ev
 */
static void opFEadd16(i8086_cpu cpu, _x86_instr instr)
{
    uint16_t  cf = cpu->flags.bit.carry;
    uint16_t op1 = readmem16(cpu, instr);
    uint16_t op2 = 1;

    set_flag_add16(cpu, op1, op2);
    writemem16(cpu, instr, op1+op2);

    cpu->flags.bit.carry = cf;
}

/*
 * DEC  Eb
 */
static void opFEsub8(i8086_cpu cpu, _x86_instr instr)
{
    uint8_t  cf  = cpu->flags.bit.carry;
    uint8_t op1 = readmem8(cpu, instr);
    uint8_t op2 = 1;

    set_flag8(cpu, op1, op2);
    writemem8(cpu, instr, op1-op2);

    cpu->flags.bit.carry = cf;
}

/*
 * DEC  Ev
 */
static void opFEsub16(i8086_cpu cpu, _x86_instr instr)
{
    uint16_t  cf  = cpu->flags.bit.carry;
    uint16_t op1 = readmem16(cpu, instr);
    uint16_t op2 = 1;

    set_flag16(cpu, op1, op2);
    writemem16(cpu, instr, op1-op2);

    cpu->flags.bit.carry = cf;
}

static void opmul(i8086_cpu cpu, _x86_instr instr)
{
    switch(instr.modregrm.field.reg)
    {
        case 0x00:
            if(instr.op & 0x01)
                opFEadd16(cpu, instr);
            else
                opFEadd8(cpu, instr);
        break;
        case 0x01:
            if(instr.op & 0x01)
                opFEsub16(cpu, instr);
            else
                opFEsub8(cpu, instr);
        break;
        default:
            opxx(cpu, instr);
    }
}

/*
 * HLT
 */
static void opF4(i8086_cpu cpu, _x86_instr instr)
{
    cpu->hlt_suspend = true;
}

void cpu_emulation(i8086_cpu cpu, _x86_instr instr)
{
    cpu->current_op = instr.op;
    one_byte_opcodes[instr.op](cpu, instr);
}

static ob_opcodes one_byte_opcodes[256] = {
   //         00h   01h   02h   03h   04h   05h   06h   07h   08h   09h   0Ah   0Bh   0Ch   0Dh   0Eh    0Fh
   /* 00h */ &opxx,&op01,&opxx,&opxx,&op04,&op05,&opxx,&opxx,&opxx,&op09,&opxx,&opxx,&opxx,&opxx,&opxx, &opxx,
   /* 01h */ &opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&op19,&opxx,&opxx,&opxx,&opxx,&opxx, &opxx,
   /* 02h */ &op20,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&op29,&opxx,&opxx,&opxx,&opxx,&opxx, &opxx,
   /* 03h */ &opxx,&op31,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&op39,&opxx,&opxx,&op3C,&opxx,&opxx, &opxx,
   /* 04h */ &inc, &inc, &inc, &inc, &inc, &inc, &inc, &inc, &dec, &dec, &dec, &dec, &dec, &dec, &dec,  &dec,
   /* 05h */ &op5x,&op5x,&op5x,&op5x,&op5x,&op5x,&op5x,&op5x,&op58,&op58,&op58,&op58,&op58,&op58,&op58, &op58,
   /* 06h */ &opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx, &opxx,
   /* 07h */ &opxx,&opxx,&op72,&opxx,&op74,&op75,&op76,&op77,&opxx,&op79,&opxx,&opxx,&opxx,&opxx,&opxx, &opxx,
   /* 08h */ &op81,&op81,&op81,&op81,&opxx,&opxx,&op86,&opxx,&op88,&op89,&op8A,&op8B,&opxx,&opxx,&opxx, &opxx,
   /* 09h */ &op90,&opxx,&op92,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx, &opxx,
   /* 0Ah */ &opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx, &opxx,
   /* 0Bh */ &opB0,&opB0,&opB0,&opB0,&opB0,&opB0,&opB0,&opB0,&opBC,&opBC,&opBC,&opBC,&opBC,&opBC,&opBC, &opBC,
   /* 0Ch */ &opxx,&opxx,&opxx,&opC3,&opxx,&opxx,&multiOP,&multiOP,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx, &opxx,
   /* 0Dh */ &opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx, &opxx,
   /* 0Eh */ &opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opxx,&opE8,&opxx,&opxx,&opEB,&opxx,&opxx,&opxx, &opxx,
   /* 0Fh */ &opxx,&opxx,&opxx,&opxx,&opF4,&opxx,&opxx,&opxx,&opxx,&opF9,&opxx,&opxx,&opxx,&opxx,&opmul, &opxx,
};