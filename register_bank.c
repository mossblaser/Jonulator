#include "jonulator.h"
#include "register_bank.h"

void
init_register_bank(register_bank_t *register_bank)
{
	register_bank->cc = 0;
	register_bank->r0 = 0;
	register_bank->pc = 0;
}



word_t
get_register_value(register_bank_t *register_bank, gpr_no_t reg_num)
{
	if (reg_num == GPR_R0)
		return 0;
	else
		return register_bank->r[reg_num];
}


void
set_register_value(register_bank_t *register_bank,
                   gpr_no_t reg_num,
                   word_t value)
{
	if (reg_num == GPR_R0)
		return;
	else
		register_bank->r[reg_num] = value;
}
