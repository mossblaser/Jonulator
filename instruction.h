#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include <stdio.h>
#include <stdint.h>
#include "jonulator.h"

typedef word_t instr_t;           /* Instruction code. */
enum instr_t {
	INSTR_ADD  = 0x0,                  /* 2's Compliment add. */
	INSTR_ADC  = 0x1,                  /* 2's Compliment add with carry-in. */
	INSTR_SUB  = 0x2,                  /* 2's Compliment subtract. */
	INSTR_SBC  = 0x3,                  /* 2's Compliment subtract with borrow. */
	INSTR_AND  = 0x4,                  /* Bitwise AND of word. */
	INSTR_OR   = 0x5,                  /* Bitwise OR of word. */
	INSTR_LDST = 0x6,                  /* Load or Store. */
	INSTR_Bcc  = 0x7                   /* Branch. */
};
typedef word_t f4b_t;             /* First-4-Bits of the instruction. */
typedef word_t reg_num_t;         /* Number of a GPR. */
typedef word_t shift_t;           /* Bits specifying a shift. */
enum shift_t {
	SHIFT_NONE = 0x0,                  /* No shift. */
	SHIFT_ASR  = 0x1,                  /* Right shift. */
	SHIFT_ROR  = 0x2,                  /* Rotate right. */
	SHIFT_RRC  = 0x3                   /* Rotate right through carry. */
};
typedef word_t immediate_t;       /* An immediate value. */
typedef word_t condition_t;       /* Bits specifying a jump condition. */
enum condition_t {
	CONDITION_BAL = 0x0,               /* Always. */
	CONDITION_BNV = 0x1,               /* Never. */
	CONDITION_BHI = 0x2,               /* C + Z = 0 */
	CONDITION_BLS = 0x3,               /* C + Z = 1 */
	CONDITION_BCC = 0x4,               /* C = 0 */
	CONDITION_BCS = 0x5,               /* C = 1 */
	CONDITION_BNE = 0x6,               /* Z = 0 */
	CONDITION_BEQ = 0x7,               /* Z = 1 */
	CONDITION_BVC = 0x8,               /* V = 0 */
	CONDITION_BVS = 0x9,               /* V = 1 */
	CONDITION_BPL = 0xa,               /* N = 0 */
	CONDITION_BMI = 0xb,               /* N = 1 */
	CONDITION_BGE = 0xc,               /* !N.V + N.!V = 0 */
	CONDITION_BLT = 0xd,               /* !N.V + N.!V = 1 */
	CONDITION_BGT = 0xe,               /* (!N.V + N.!V)+Z = 0 */
	CONDITION_BLE = 0xf                /* (!N.V + N.!V)+Z = 1 */
};
typedef int8_t       offset_t;    /* A jump offset. */



/* An instruction for the CPU. More joyous unions! */
typedef union instruction_t {
	
	/* The word of memory which represents this instruction. */
	word_t word;
	
	/* First 4 bits of the instruction. */
	struct { word_t : 12; f4b_t f4b : 4; };
	
	/* Instruction and type1/2 flag */
	struct {
		word_t : 11;
		bool    ldcc          : 1; /* Load-Flag or Set Condition-Code */
		bool    is_type2_flag : 1; /* Is this a type-1 instruction? */
		instr_t instr         : 3; /* Instruction code. */
	};
	
	/* Data for a type-1 instruction: Two Source Registers. */
	struct {
		shift_t     shift     : 2; /* The shift code. */
		reg_num_t   src_b     : 3; /* Source register b. */
		reg_num_t   src_a     : 3; /* Source register A. */
		reg_num_t   dst       : 3; /* Destination register. */
		bool        ldcc      : 1; /* Load-Flag or Set Condition-Code */
		bool        is_type2  : 1; /* Is this a type-1 instruction? */
		instr_t     instr     : 3; /* Instruction code. */
	} type1;
	
	/* Data for a type-2 instruction: One Source, One Immediate Value. */
	struct {
		immediate_t immediate : 5; /* An immediate value. */
		reg_num_t   src_a     : 3; /* Source register A. */
		reg_num_t   dst       : 3; /* Destination register. */
		bool        ldcc      : 1; /* Load-Flag or Set Condition-Code */
		bool        is_type2  : 1; /* Is this a type-1 instruction? */
		instr_t     instr     : 3; /* Instruction code. */
	} type2;
	
	/* Data for a type-3 instruction: Conditional Branch. */
	struct {
		offset_t    offset    : 8; /* Offset from the PC+1 to jump. */
		condition_t condition : 4; /* Condition code of the branch instruction. */
		f4b_t       f4b       : 4; /* First 4 bits of the instruction, should be 0x0F. */
	} type3;
	
} instruction_t;


/* Shift an instruction argument as specified in the instruction. */
word_t shift_instruction_sources(stump_system_t *stump,
                                 instruction_t  instruction,
                                 word_t         value);

/* Fetch the instruction src values. */
void get_instruction_sources(stump_system_t *stump,
                             instruction_t  instruction,
                             word_t        *src_a,
                             word_t        *src_b);

/* Store an instruction value. */
void set_instruction_dst(stump_system_t *stump,
                         instruction_t  instruction,
                         word_t         dst);

/* Acts like an adder in the system. */
word_t exec_add_operation(stump_system_t *stump,
                          instruction_t instr,
                          word_t src_a, word_t src_b,
                          int carry_in, bool invert_carry_out);

void exec_add_instruction(stump_system_t *stump, instruction_t instruction);
void exec_adc_instruction(stump_system_t *stump, instruction_t instruction);
void exec_sub_instruction(stump_system_t *stump, instruction_t instruction);
void exec_sbc_instruction(stump_system_t *stump, instruction_t instruction);

void exec_and_instruction(stump_system_t *stump, instruction_t instruction);
void exec_or_instruction(stump_system_t *stump, instruction_t instruction);

void exec_ldst_instruction(stump_system_t *stump, instruction_t instruction);

void exec_branch_instruction(stump_system_t *stump, instruction_t instruction);

#endif

