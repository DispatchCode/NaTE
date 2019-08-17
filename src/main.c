#include <stdio.h>
#include <stdint.h>
#include <memory.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ild.h>
#include <cpu.h>

#include "include/ild.h"
#include "include/cpu.h"
#include "include/cpu_exec.h"


static void print_cpu_state(i8086_cpu cpu)
{
    printf("=============================== CPU State =====================================\n");

    printf("CS: 0x%X | DS: 0x%X | SS: 0x%X | ES: 0x%X\n", cpu->segreg[CS], cpu->segreg[DS], cpu->segreg[SS], cpu->segreg[ES]);
    printf("IP: 0x%X  SP: 0x%X\n\n", cpu->IP , get_reg16(cpu,SP));
    printf("AX: 0x%X | AL: 0x%X | AH: 0x%X\n",  get_reg16(cpu,AX),get_reg8(cpu,AL), get_reg8(cpu,AH));
    printf("DX: 0x%X | DL: 0x%X | DH: 0x%X\n",  get_reg16(cpu,DX),get_reg8(cpu,DL), get_reg8(cpu,DH));
    printf("CX: 0x%X | CL: 0x%X | CH: 0x%X\n",  get_reg16(cpu,CX),get_reg8(cpu,CL), get_reg8(cpu,CH));
    printf("BX: 0x%X | BL: 0x%X | BH: 0x%X\n", get_reg16(cpu,BX),get_reg8(cpu,BL), get_reg8(cpu,BH));
    printf("SI: 0x%X\n", get_reg16(cpu,SI));
    printf("DI: 0x%X\n\n", get_reg16(cpu,DI));

    printf("===============================================================================\n");
}

static void print_video_memory(i8086_cpu cpu)
{
    uint32_t offset = 0x8000;
    for(int i=0; i<25; i++)
    {
        for(int j=0; j<80; j++)
        {
            printf("%c", cpu->memory[offset + (80 * i) + j]);
        }
        printf("\n");
    }

}

size_t input_file_size(FILE *hfile)
{
    fseek(hfile, 0, SEEK_END);
    size_t file_size = (size_t) ftell(hfile);
    fseek(hfile, 0, SEEK_SET);

    return file_size;
}

int main()
{
    int len = 0;

    const char *file_name = "codegolf.com";
    FILE *hfile = fopen(file_name, "rb");
    size_t file_size = input_file_size(hfile);

    i8086_cpu cpu = cpu_initialization();
    size_t byte_read = fread(cpu->memory, sizeof(uint8_t), file_size, hfile);

    fclose(hfile);

    cpu->hlt_suspend = false;

    while(byte_read)
    {
        _x86_instr instruction;
        memset(&instruction, 0, sizeof(_x86_instr));

        len = decode_x86(&instruction, cpu->IP, 1, (char*) &cpu->memory[0]);

        emulate(cpu, instruction);

        if(instruction.label == 0 || (instruction.label != 0 && !cpu->jmp_taken))
            cpu->IP += len;

        if(cpu->hlt_suspend) {
            print_video_memory(cpu);
            getchar();
            cpu->hlt_suspend = false;
        }

        if(cpu->unsupported_op )
            break;
    }

    print_cpu_state(cpu);
    printf("\n");
    print_video_memory(cpu);

    if(cpu != NULL)
    {
        if(cpu->memory != NULL)
            free(cpu->memory);
        free(cpu);
    }

    return 0;
}