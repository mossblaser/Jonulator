#include "jonulator.h"
#include "instruction.h"
#include "register_bank.h"


word_t
shift_instruction_sources(stump_system_t *stump,
                          instruction_t  instr,
                          word_t         value_16)
{
	int value = value_16;
	/* XXX: This function assumes that the carry flag can be changed, regardless
	 * of what the rest of the instruction says. */
	switch (instr.type1.shift) {
		case SHIFT_NONE:
			stump->register_bank.csh = 0;
			break;
		
		case SHIFT_ASR:
			/* Set shifter-carry. (Kind of a hack: not really a register...) */
			stump->register_bank.csh = value & 0x0001;
			
			/* Duplicate most significant bit. */
			value |= (value & 0x8000) << 1;
			
			/* Shift the value right. */
			value >>= 1;
			break;
		
		case SHIFT_ROR:
			/* Set shifter-carry. (Kind of a hack: not really a register...) */
			stump->register_bank.csh = value & 0x0001;
			
			/* Move the first bit to the end. */
			value |= (value & 0x0001) << 16;
			
			/* Shift the value right to rotate. */
			value >>= 1;
			break;
		
		case SHIFT_RRC:
			/* Move the carry bit to the end. */
			value |= stump->register_bank.csh << 16;
			
			/* Set shifter-carry. (Kind of a hack: not really a register...) */
			stump->register_bank.csh = value & 0x0001;
			
			/* Shift the value right to rotate. */
			value >>= 1;
			break;
	}
	
	return (word_t)value;
}


void
get_instruction_sources(stump_system_t *stump,
                        instruction_t  instr,
                        word_t        *src_a,
                        word_t        *src_b)
{
	word_t src_a_raw;
	word_t src_b_raw;
	
	/* Read the value for src_a. */
	if (!instr.is_type2_flag)
		src_a_raw = get_register_value(&(stump->register_bank), instr.type1.src_a);
	else
		src_a_raw = get_register_value(&(stump->register_bank), instr.type2.src_a);
	
	/* Apply the shift for the operand if type 1. */
	if (!instr.is_type2_flag)
		src_a_raw = shift_instruction_sources(stump, instr, src_a_raw);
	
	
	/* Read the value for src_b. */
	if (!instr.is_type2_flag) {
		src_b_raw = get_register_value(&(stump->register_bank), instr.type1.src_b);
	} else {
		/* Sign Extend */
		if (instr.type2.immediate & (1<<4))
			src_b_raw = instr.type2.immediate | 0xFFF0;
		else
			src_b_raw = instr.type2.immediate;
	}
	
	/* Return the values. */
	(*src_a) = src_a_raw;
	(*src_b) = src_b_raw;
	return;
}



void
set_instruction_dst(stump_system_t *stump,
                    instruction_t  instr,
                    word_t         value)
{
	if (!instr.is_type2_flag)
		set_register_value(&(stump->register_bank), instr.type1.dst, value);
	else
		set_register_value(&(stump->register_bank), instr.type2.dst, value);
}



word_t
exec_add_operation(stump_system_t *stump,
                   instruction_t instr,
                   word_t src_a, word_t src_b,
                   int carry_in, bool invert_carry_out)
{
	word_t dst;
	dst = src_a + src_b + carry_in;
	
	if (instr.ldcc) {
		stump->register_bank.n = dst >> 15;
		stump->register_bank.z = dst == 0;
		/* If the sign on the answer is differrent to both sources. In particular,
		 * if both sources are the same side of 0 then the output of their addition
		 * *must* be the same side too. */
		stump->register_bank.v = ((src_a ^ dst) & (src_b ^ dst) & 0x8000) != 0;
		stump->register_bank.c = (dst < (src_a + carry_in)) ^ invert_carry_out;
	}
	
	return dst;
}



void
exec_add_instruction(stump_system_t *stump, instruction_t instr)
{
	word_t src_a, src_b;
	get_instruction_sources(stump, instr, &src_a, &src_b);
	
	/* Feed the calculation through the virtual adder */
	word_t dst = exec_add_operation(stump, instr, src_a, src_b, 0, false);
	
	set_instruction_dst(stump, instr, dst);
}


void
exec_adc_instruction(stump_system_t *stump, instruction_t instr)
{
	word_t src_a, src_b;
	get_instruction_sources(stump, instr, &src_a, &src_b);
	
	/* Feed the calculation through the virtual adder passing in the carry. */
	word_t dst = exec_add_operation(stump, instr, src_a, src_b,
	                                stump->register_bank.c, false);
	
	set_instruction_dst(stump, instr, dst);
}


void
exec_sub_instruction(stump_system_t *stump, instruction_t instr)
{
	word_t src_a, src_b;
	get_instruction_sources(stump, instr, &src_a, &src_b);
	
	/* Feed the calculation through the virtual adder passing in the carry. */
	word_t dst = exec_add_operation(stump, instr, src_a, ~src_b, 1, true);
	
	set_instruction_dst(stump, instr, dst);
}



void
exec_sbc_instruction(stump_system_t *stump, instruction_t instr)
{
	word_t src_a, src_b;
	get_instruction_sources(stump, instr, &src_a, &src_b);
	
	/* Feed the calculation through the virtual adder passing in the carry. */
	word_t dst = exec_add_operation(stump, instr, src_a, ~src_b,
	                                ~(stump->register_bank.c), true);
	
	set_instruction_dst(stump, instr, dst);
}



void
exec_and_instruction(stump_system_t *stump, instruction_t instr)
{
	word_t src_a, src_b;
	get_instruction_sources(stump, instr, &src_a, &src_b);
	
	word_t dst = src_a & src_b;
	
	if (instr.ldcc) {
		stump->register_bank.n = dst >> 15;
		stump->register_bank.z = dst == 0;
		stump->register_bank.v = false;
		stump->register_bank.c = (!instr.is_type2_flag) && instr.type1.shift
		                           ? stump->register_bank.csh
		                           : false;
	}
	
	set_instruction_dst(stump, instr, dst);
}



void
exec_or_instruction(stump_system_t *stump, instruction_t instr)
{
	word_t src_a, src_b;
	get_instruction_sources(stump, instr, &src_a, &src_b);
	
	word_t dst = src_a | src_b;
	
	if (instr.ldcc) {
		stump->register_bank.n = dst >> 15;
		stump->register_bank.z = dst == 0;
		stump->register_bank.v = false;
		stump->register_bank.c = (!instr.is_type2_flag) && instr.type1.shift
		                           ? stump->register_bank.csh
		                           : false;
	}
	
	set_instruction_dst(stump, instr, dst);
}



void
exec_ldst_instruction(stump_system_t *stump, instruction_t instr)
{
	word_t src_a, src_b;
	get_instruction_sources(stump, instr, &src_a, &src_b);
	
	word_t addr = src_a + src_b;
	
	if (instr.ldcc) {
		/* Store. */
		if(!instr.is_type2_flag)
			stump->memory[addr] = get_register_value(&(stump->register_bank),
			                                         instr.type1.dst);
		else
			stump->memory[addr] = get_register_value(&(stump->register_bank),
			                                         instr.type2.dst);
		
		printf("Store: memory[%04x] = %04x\n", addr, stump->memory[addr]);
	} else {
		/* Load. */
		word_t dst = stump->memory[addr];
		printf("Load: memory[%04x] == %04x\n", addr, dst);
		set_instruction_dst(stump, instr, dst);
	}
}



void
exec_branch_instruction(stump_system_t *stump, instruction_t instr)
{
	register_bank_t rb = stump->register_bank;
	
	switch (instr.type3.condition) {
		case CONDITION_BAL: /* Always. */
			break;
		
		case CONDITION_BNV: /* Never. */
			return;
		
		case CONDITION_BHI: /* C + Z = 0 */
			if (rb.c | rb.z == 0) return;
			else                  break;
		
		case CONDITION_BLS: /* C + Z = 1 */
			if (rb.c | rb.z == 1) return;
			else                  break;
		
		case CONDITION_BCC: /* C = 0 */
			if (rb.c  == 0) return;
			else            break;
		
		case CONDITION_BCS: /* C = 1 */
			if (rb.c  == 1) return;
			else            break;
		
		case CONDITION_BNE: /* Z = 0 */
			if (rb.z  == 0) return;
			else            break;
		
		case CONDITION_BEQ: /* Z = 1 */
			if (rb.z  == 1) return;
			else            break;
		
		case CONDITION_BVC: /* V = 0 */
			if (rb.v  == 0) return;
			else            break;
		
		case CONDITION_BVS: /* V = 1 */
			if (rb.v  == 1) return;
			else            break;
		
		case CONDITION_BPL: /* N = 0 */
			if (rb.n  == 0) return;
			else            break;
		
		case CONDITION_BMI: /* N = 1 */
			if (rb.n  == 1) return;
			else            break;
		
		case CONDITION_BGE: /* !N.V + N.!V = 0 */
			if (((~rb.n) & rb.v) | (rb.n & (~rb.v))  == 0) return;
			else                                           break;
		
		case CONDITION_BLT: /* !N.V + N.!V = 1 */
			if (((~rb.n) & rb.v) | (rb.n & (~rb.v))  == 1) return;
			else                                           break;
		
		case CONDITION_BGT: /* (!N.V + N.!V)+Z = 0 */
			if ((((~rb.n) & rb.v) | (rb.n & (~rb.v)) | rb.z)  == 0) return;
			else                                                    break;
		
		case CONDITION_BLE: /* (!N.V + N.!V)+Z = 1 */
			if ((((~rb.n) & rb.v) | (rb.n & (~rb.v)) | rb.z)  == 1) return;
			else                                                    break;
	}
	
	rb.pc += instr.type3.offset | ((instr.type3.offset & 0x0080) ? 0xFF00 : 0);
}
