#ifndef ISA_INSTRUCTION_FORMAT_H
#define ISA_INSTRUCTION_FORMAT_H

#include <stdint.h>

// Architectures
#define  IA3    0x01         // IA32
#define  IA6    0x02         // IA64
#define  ALL    (IA3 | IA6)  // ALL

// Operand Type
#define   b     1  // byte
#define   v     2  // w, dd, qd (64-bit mode), depending on operand size
#define   w     3  // w
#define   z     4  // w for 16-bit operand size, dw for 32/64-bit operand size
#define   wb    6  // word and byte
#define   p     7  // word and byte


//
// instruction prefix look-up table
static size_t prefixes[256] = {
        //       00  01  02  03  04  05  06  07  08  09  0A  0B  0C  0D  0E  0F
        /* 00 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,ALL,
        /* 10 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* 20 */ 0,  0,  0,  0,  0,  0,  ALL,0,  0,  0,  0,  0,  0,  0,  ALL,0,
        /* 30 */ 0,  0,  0,  0,  0,  0,  ALL,0,  0,  0,  0,  0,  0,  0,  ALL,0,
        /* 40 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* 50 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* 60 */ 0,  0,  0,  0,  ALL,ALL,ALL,ALL,0,  0,  0,  0,  0,  0,  0,  0,
        /* 70 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* 70 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* 90 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* A= */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* B0 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* C0 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* D0 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* E0 */ 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
        /* F0 */ ALL,0,ALL,  ALL,0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0
};

#define X87_ESC 2

//
// 1-byte OP look-up table
static size_t mod_reg_rm_1b[256] = {
        //      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
        /* 00 */ 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
        /* 10 */ 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
        /* 20 */ 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
        /* 30 */ 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0,
        /* 40 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 50 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 60 */ 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 0, 0, 0,
        /* 70 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 80 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* 90 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* A0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* B0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* C0 */ 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0,
        /* D0 */ 1, 1, 1, 1, 0, 0, 0, 0, 2, 2, 2, 2, 2, 2, 2, 2, // 2 = Coprocessor Escape
        /* E0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* F0 */ 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1
};

static size_t imm_byte_1b[256] = {
        //      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
        /* 00 */ 0, 0, 0, 0, b, w, 0, 0, 0, 0, 0, 0, b, w, 0, 0,
        /* 10 */ 0, 0, 0, 0, b, w, 0, 0, 0, 0, 0, 0, b, w, 0, 0,
        /* 20 */ 0, 0, 0, 0, b, w, 0, 0, 0, 0, 0, 0, b, w, 0, 0,
        /* 30 */ 0, 0, 0, 0, b, w, 0, 0, 0, 0, 0, 0, b, w, 0, 0,
        /* 40 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 50 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 60 */ 0, 0, 0, 0, 0, 0, 0, 0, w, w, b, b, 0, 0, 0, 0,
        /* 70 */ b, b, b, b, b, b, b, b, b, b, b, b, b, b, b, b,
        /* 80 */ b, w, b, b, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 90 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, p, 0, 0, 0, 0, 0,
        /* A0 */ w, w, w, w, 0, 0, 0, 0, b, w, 0, 0, 0, 0, 0, 0,
        /* B0 */ b, b, b, b, b, b, b, b, w, w, w, w, w, w, w, w,
        /* C0 */ b, b, w, 0, 0, 0, b, w, wb, 0, w, 0, 0, b, 0, 0,
        /* D0 */ 0, 0, 0, 0, b, b, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* E0 */ b, b, b, b, b, b, b, b, w, w, w, b, 0, 0, 0, 0,
        /* F0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// labels is present?
static size_t op1b_labels[256] = {
        //      00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F
        /* 00 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 10 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 20 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 30 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 40 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 50 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 60 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 70 */ 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        /* 80 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* 90 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* A0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* B0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* C0 */ 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* D0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        /* E0 */ 0, 0, 0, 0, 0, 0, 0, 0, 8, 2, 2, 2, 0, 0, 0, 0,
        /* F0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
};

// Segment Override prefix, branch taken/not taken, operand size and address size prefixes
enum GR25_PREFIX {
    OP_CS        = 1,
    OP_DS        = 2,
    OP_ES        = 4,
    OP_SS        = 8,
} x86_op;

// flags used when a specific instruction feature is present
enum X86_FLAGS {
    X86_PREFIX_FLAG = 1,
    X86_MODREG_FLAG = 2,
    X86_DISP_FLAG   = 4,
    X86_IMM_FLAG    = 8,
} x86_flags;

// Instruction format
typedef struct _x86_instr {
    uint16_t setprefix;
    uint8_t  prefixes[16];
    uint8_t  prefix_cnt;

    uint8_t  op;
    uint16_t instr_flags;

    union
    {
        struct
        {
            uint8_t rm  : 3;
            uint8_t reg : 3;
            uint8_t mod : 2;

        } field;
        uint8_t mod_reg_rm;
    } modregrm;

    uint16_t   imm;
    uint16_t   disp;
    uint32_t   disp_len;

    uint16_t   label;
    uint16_t   va_addr;

    uint32_t   length;

} _x86_instr;

static void   x86_mod_decode(_x86_instr *out_instruction,  char *data, const size_t *mod_table, const size_t *imm_table, size_t offset, uint8_t arch);
static inline size_t imm_size(_x86_instr *instr, size_t cur_byte, uint8_t arch);
static size_t decode_disp(uint8_t mod, uint8_t rm);
size_t decode_x86(_x86_instr *out_instruction, size_t offset, uint8_t arch, char *);

#endif //ISA_INSTRUCTION_FORMAT_H
