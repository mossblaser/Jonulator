#ifndef REGISTER_BANK_H
#define REGISTER_BANK_H

#include <stdint.h>


typedef unsigned int cc_bit_t;  /* A bit in the condition code. */
typedef unsigned int cc_t;      /* The condition code register. */
typedef word_t       gpr_t;     /* General-purpose-register. */
typedef enum         gpr_no_t { /* General-purpose-register number. */
	GPR_R0 = 0,
	GPR_R1 = 1,
	GPR_R2 = 2,
	GPR_R3 = 3,
	GPR_R4 = 4,
	GPR_R5 = 5,
	GPR_R6 = 6,
	GPR_R7 = 7
} gpr_no_t;


/* A model of the register bank. Let the union madness begin! */
typedef struct register_bank_t {
	
	/* Condition code register. */
	union {
		struct {
			cc_bit_t c : 1; /* Carry */
			cc_bit_t v : 1; /* Overflow */
			cc_bit_t z : 1; /* Zero */
			cc_bit_t n : 1; /* Sign */
		};
		cc_t cc;
	};
	
	/* The shifter-carry bit, kind of a hack: this register doesn't
	 * really exist but having it makes code clearer! */
	cc_t csh;
	
	/* General Purpose Registers. */
	union {
		struct {
			/* Constant zero register.
			 * Note: Nothing is stopping this register being written to! */
			gpr_t r0;
			
			/* (Actually) General Purpous Registers. */
			gpr_t r1;
			gpr_t r2;
			gpr_t r3;
			gpr_t r4;
			gpr_t r5;
			gpr_t r6;
			
			/* Program counter register. */
			union {
				gpr_t r7;
				gpr_t pc;
			};
		};
		
		/* Array access for the registers. */
		gpr_t r[8];
	};
	
} register_bank_t;



/* Set initial values in the register bank. */
void init_register_bank(register_bank_t *register_bank);

/* Get the value from a register. */
word_t get_register_value(register_bank_t *register_bank, gpr_no_t reg_num);

/* Set the value of a register. */
void set_register_value(register_bank_t *register_bank,
                        gpr_no_t reg_num,
                        word_t value);

#endif
