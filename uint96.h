#pragma once
#include <stdint.h>
#include <stdbool.h>

/* Unsigned 96-bit integer */

typedef struct
{
	uint32_t x[3];
} uint96;

#define UINT96_ZERO ((uint96) { .x = { 0, 0, 0 } })

#define UINT96_FROM_UINT32(value) ((uint96) { .x = { 0, 0, (value) } })

bool uint96_to_uint32(const uint96 *in, uint32_t *out);
bool uint96_to_int32(const uint96 *in, int32_t *out);

bool test_bit_96(const uint96 *i, unsigned bit);
void reset_bit_96(uint96 *i, unsigned bit);
void set_bit_96(uint96 *i, unsigned bit);

bool shr1_96(uint96 *i);
bool shl1_96(uint96 *i);

void shr_96(uint96 *i, unsigned amount);
void shl_96(uint96 *i, unsigned amount);

/* Returns carry/borrow */
bool inc_32_32(uint32_t *left, uint32_t right);
bool dec_32_32(uint32_t *left, uint32_t right);

/* Returns carry/borrow */
bool add_96_32(uint96 *i, uint32_t j);
bool sub_96_32(uint96 *i, uint32_t j);

/* Returns carry */
uint32_t mul_32_32(uint32_t *left, uint32_t right);
uint32_t mul_96_32(uint96 *i, uint32_t j);

int cmp_96_32(const uint96 *l, const uint32_t r);

void div_96_32(uint96 *i, uint32_t j);
