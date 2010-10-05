#include <stdio.h>
#include <stdint.h>
#include "jonulator.h"
#include "instruction.h"


void
load_memory_image(FILE *file, stump_system_t *stump)
{
	int addr = 0;
	char buf[18];
	while (fgets(buf, 18, file) != NULL) {
		stump->memory[addr] = strtol(buf, NULL, 2);
		addr += 1;
	}
}


int
main(int argc, char *argv[])
{
	stump_system_t stump;
	init_register_bank(&stump.register_bank);
	
	/* Get the memory image filename. */
	if (argc != 2) {
		fprintf(stderr, "ERROR: Accepts 1 argument: a memory image.\n");
		return 1;
	}
	
	FILE *memory_image_file = fopen(argv[1], "r");
	if (memory_image_file == NULL) {
		fprintf(stderr, "ERROR: Couldn't open memory image.\n");
		return 1;
	}
	
	fprintf(stderr, "INFO: Loading memory image..\n");
	
	load_memory_image(memory_image_file, &stump);
	
	fprintf(stderr, "INFO: Starting Simulator.\n");
	
	
	/* Main Processor Loop. */
	while (true) {
		/* Read user input. */
		int buf;
		if ((buf = getchar()) == EOF) {
			return 0;
		}
		
		
		/* Fetch the instruction. */
		instruction_t instr;
		instr.word = stump.memory[stump.register_bank.pc];
		
		/* Increment the program counter */
		stump.register_bank.pc ++;
		
		/* Execute the instruction. */
		switch (instr.instr) {
			case INSTR_ADD : printf("ADD: "); exec_add_instruction(&stump, instr); break;
			case INSTR_ADC : printf("ADC: "); exec_adc_instruction(&stump, instr); break;
			case INSTR_SUB : printf("SUB: "); exec_sub_instruction(&stump, instr); break;
			case INSTR_SBC : printf("SBC: "); exec_sbc_instruction(&stump, instr); break;
			
			case INSTR_AND : printf("AND: "); exec_and_instruction(&stump, instr); break;
			case INSTR_OR  : printf("OR: "); exec_or_instruction(&stump, instr); break;
			
			case INSTR_LDST: printf("LDST: "); exec_ldst_instruction(&stump, instr); break;
			
			case INSTR_Bcc : printf("Bcc: "); exec_branch_instruction(&stump, instr); break;
		}
		
		if (buf == '\n') {
			/* Print out the registers. */
			printf("C:%d V:%d Z:%d N:%d  "
			       "r1:%04x r2:%04x r3:%04x r4:%04x r5:%04x r6:%04x pc:%04x\n",
			       stump.register_bank.c,
			       stump.register_bank.v,
			       stump.register_bank.z,
			       stump.register_bank.n,
			       stump.register_bank.r1,
			       stump.register_bank.r2,
			       stump.register_bank.r3,
			       stump.register_bank.r4,
			       stump.register_bank.r5,
			       stump.register_bank.r6,
			       stump.register_bank.pc);
		} else if (buf == 'm') {
			int addr;
			fscanf(stdin, "%d", &addr);
			getchar();
			printf("memory[%04x] = %04x\n\n", addr&0xFFFF, stump.memory[addr&0xFFFF]);
		}
	}
	
	return 0;
}
