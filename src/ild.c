#include <memory.h>

#include "include/ild.h"

#include <stdio.h>

static inline size_t imm_size(_x86_instr *instr, size_t cur_byte, uint8_t arch)
{
    switch(cur_byte)
    {
        case b:
            return 1;
        case w:
            return 2;
    }
    return 0;
}

static size_t decode_disp(uint8_t mod, uint8_t rm)
{
    if((mod == 0x02) || (!mod && rm == 6))
        return 2;
    else if(mod == 0x01)
        return 1;

    return 0;
}

static void x86_mod_decode(_x86_instr *instr,  char *data, const size_t *mod_table, const size_t *imm_table, size_t offset, uint8_t arch)
{
    size_t index = offset+1;

    // mod/reg/rm byte
    if(mod_table[instr->op])
    {
        instr->instr_flags |= X86_MODREG_FLAG;

        instr->modregrm.mod_reg_rm = (uint8_t) *(data + (index));
        instr->length++;
        index++;

        // displacement value present, mod < 11b
        if(instr->modregrm.field.mod < 3)
        {
            instr->instr_flags |= X86_DISP_FLAG;

            instr->disp_len = decode_disp(instr->modregrm.field.mod, instr->modregrm.field.rm);
            memcpy(&instr->disp, (data + index), instr->disp_len);

            instr->length += instr->disp_len;
            index += instr->disp_len;
        }
    }

    // imm
    size_t size = imm_size(instr, imm_table[instr->op], arch);
    if(size)
    {
        instr->instr_flags |= X86_IMM_FLAG;

        memcpy(&instr->imm, (data + index), size);
        instr->length += size;
    }

    if(op1b_labels[instr->op])
    {
        switch (size)
        {
            case 1:
                instr->label = ((uint16_t) (instr->va_addr)) + (int8_t)instr->imm + instr->length;
            break;
            default:
                instr->label = ((uint16_t) (instr->va_addr)) + (int16_t)instr->imm + instr->length;
        }
    }


}

/* 1-byte opcode decoder */
size_t decode_x86(_x86_instr *instr, size_t offset, uint8_t arch, char *data)
{
    uint8_t cur_byte = (uint8_t) *(data+offset);
    instr->va_addr = (offset);

    while(prefixes[cur_byte] & arch)
    {
        instr->instr_flags |= X86_PREFIX_FLAG;

        switch(cur_byte)
        {
            case 0x26:
                instr->setprefix |= OP_ES;
                break;
            case 0x2E:
                instr->setprefix |= OP_CS;
                break;
            case 0x36:
                instr->setprefix |= OP_SS;
                break;
            case 0x3E:
                instr->setprefix |= OP_DS;
                break;
            default:
                break;
        }

        instr->prefixes[instr->prefix_cnt++] = cur_byte;
        instr->length++;
        cur_byte = (uint8_t) *(data + (++offset));
    }

    instr->length++;
    instr->op = cur_byte;

    x86_mod_decode(instr, data, mod_reg_rm_1b, imm_byte_1b, offset, arch);

    return instr->length;
}
