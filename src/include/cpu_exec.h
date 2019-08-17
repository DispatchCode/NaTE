#ifndef _CPU_EXEC
#define _CPU_EXEC

#include "cpu.h"
#include "ild.h"

void cpu_emulation(i8086_cpu cpu, _x86_instr instr);
void set_flag16(i8086_cpu cpu,uint16_t op1, uint16_t op2);
void setflag_sz16(i8086_cpu cpu,uint16_t value);
void set_flag_add16(i8086_cpu cpu, uint16_t op1, uint16_t op2);
void set_flag_sub16(i8086_cpu cpu, uint16_t op1, uint16_t op2);
int32_t get_ea(i8086_cpu cpu, _x86_instr instr, uint8_t mod, uint8_t rm);
static void writemem16(i8086_cpu cpu, _x86_instr instr, uint16_t value);
void cpu_write_mem8(i8086_cpu cpu, int32_t address, uint8_t value);
uint8_t cpu_read_mem8(i8086_cpu cpu, int32_t offset);
uint16_t cpu_read_mem16(i8086_cpu cpu, int32_t offset);
void set_flag_xor16(i8086_cpu cpu, uint16_t op1, uint16_t op2);
void set_flag_or16(i8086_cpu cpu, uint16_t op1, uint16_t op2);
void set_flag_and16(i8086_cpu cpu,uint16_t op1, uint16_t op2);
void set_flag_add8(i8086_cpu cpu,uint8_t op1, uint8_t op2);
void set_flag_or8(i8086_cpu cpu, uint16_t op1, uint8_t op2);
void setflag_sz8(i8086_cpu cpu,uint8_t value);
void set_flag_and8(i8086_cpu cpu,uint8_t op1, uint8_t op2);
void set_flag8(i8086_cpu cpu,uint8_t op1, uint8_t op2);

void push(i8086_cpu cpu, uint16_t value);

#endif