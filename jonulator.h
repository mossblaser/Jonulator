#ifndef JONULATOR_H
#define JONULATOR_H

#define MEMORY_SIZE (1<<16) - 1

#include <stdint.h>

typedef uint16_t     word_t;      /* Word size. */

#include "register_bank.h"

typedef word_t bool;
#define true 1
#define false 0

typedef struct stump_system_t {
	word_t          memory[MEMORY_SIZE];
	register_bank_t register_bank;
} stump_system_t;

#endif
