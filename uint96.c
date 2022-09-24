#include "uint96.h"

typedef union {
	/* Little-endian */
	struct {
		uint32_t l;
		uint32_t h;
	};
	uint64_t x;
} uint64;

/*
 * For the C implementations, we used division+modulus instead of optimising to
 * bit-shifts, and we expect the compiler to optimise those for us.
 */

bool uint96_to_uint32(const uint96 *in, uint32_t *out)
{
	*out = in->x[2];
	return (in->x[0] | in->x[1]) == 0;
}

bool uint96_to_int32(const uint96 *in, int32_t *out)
{
	*out = in->x[2];
	return (in->x[0] | in->x[1] | (in->x[2] >> 31)) == 0;
}

bool test_bit_96(const uint96 *i, unsigned bit)
{
	/* No bounds check on bit */
	unsigned word = bit / 32;
	unsigned wbit = bit % 32;
	return (i->x[2 - word] >> wbit) & 1;
}

void reset_bit_96(uint96 *i, unsigned bit)
{
	/* No bounds check on bit */
	unsigned word = bit / 32;
	unsigned wbit = bit % 32;
	i->x[2 - word] &= ~(1U << wbit);
}

void set_bit_96(uint96 *i, unsigned bit)
{
	/* No bounds check on bit */
	unsigned word = bit / 32;
	unsigned wbit = bit % 32;
	i->x[2 - word] |= 1U << wbit;
}

bool shr1_96(uint96 *i)
{
	bool lsb = (i->x[2] & 1);
	i->x[2] >>= 1;
	i->x[2] |= i->x[1] << 31;
	i->x[1] >>= 1;
	i->x[1] |= i->x[0] << 31;
	i->x[0] >>= 1;
	return lsb;
}

bool shl1_96(uint96 *i)
{
	bool msb = (i->x[0] >> 31);
	i->x[0] <<= 1;
	i->x[0] |= i->x[1] >> 31;
	i->x[1] <<= 1;
	i->x[1] |= i->x[2] >> 31;
	i->x[2] <<= 1;
	return msb;
}

void shr_96(uint96 *i, unsigned amount)
{
	if (amount == 0) {
		return;
	}
	unsigned bytes = amount / 32;
	unsigned bits = amount % 32;
	if (bytes == 2) {
		i->x[0] = i->x[2];
		i->x[1] = 0;
		i->x[2] = 0;
	} else if (bytes == 1) {
		i->x[2] = i->x[1];
		i->x[1] = i->x[0];
		i->x[0] = 0;
	}
	if (bits == 0) {
		return;
	}
	unsigned inverse = 32 - bits;
	i->x[2] >>= bits;
	i->x[2] |= i->x[1] << inverse;
	i->x[1] >>= bits;
	i->x[1] |= i->x[0] << inverse;
	i->x[0] >>= bits;
}

void shl_96(uint96 *i, unsigned amount)
{
	if (amount == 0) {
		return;
	}
	unsigned bytes = amount / 32;
	unsigned bits = amount % 32;
	if (bytes == 2) {
		i->x[2] = i->x[0];
		i->x[1] = 0;
		i->x[0] = 0;
	} else if (bytes == 1) {
		i->x[0] = i->x[1];
		i->x[1] = i->x[2];
		i->x[2] = 0;
	}
	if (bits == 0) {
		return;
	}
	unsigned inverse = 32 - bits;
	i->x[0] <<= bits;
	i->x[0] |= i->x[1] >> inverse;
	i->x[1] <<= bits;
	i->x[1] |= i->x[2] >> inverse;
	i->x[2] <<= bits;
}

bool inc_32_32(uint32_t *left, uint32_t right)
{
	*left += right;
	return *left < right;
}

bool dec_32_32(uint32_t *left, uint32_t right)
{
	uint32_t left_prev = *left;
	*left -= right;
	return left_prev < *left;
}

uint32_t mul_32_32(uint32_t *left, uint32_t right)
{
	uint64 tmp = { .x = (uint64_t) *left * right };
	*left = tmp.l;
	return tmp.h;
}

bool add_96_32(uint96 *i, uint32_t j)
{
	/* Replace with ADDS/ADCS/ADCS/... */
	bool carry;
	carry = inc_32_32(&i->x[2], j);
	carry = inc_32_32(&i->x[1], carry);
	carry = inc_32_32(&i->x[0], carry);
	return carry;
}

bool sub_96_32(uint96 *i, uint32_t j)
{
	/* Replace with SUBS/SBCS/SBCS/... */
	bool borrow;
	borrow = dec_32_32(&i->x[2], j);
	borrow = dec_32_32(&i->x[1], borrow);
	borrow = dec_32_32(&i->x[0], borrow);
	return borrow;
}

uint32_t mul_96_32(uint96 *i, uint32_t j)
{
	uint64 tmp[3];
	tmp[2].x = (uint64_t) j * i->x[2];
	tmp[1].x = (uint64_t) j * i->x[1];
	tmp[0].x = (uint64_t) j * i->x[0];
	tmp[1].x += tmp[2].h;
	tmp[0].x += tmp[1].h;
	i->x[2] = tmp[2].l;
	i->x[1] = tmp[1].l;
	i->x[0] = tmp[0].l;
	return tmp[0].h;
}

int cmp_96_32(const uint96 *l, const uint32_t r)
{
	if (l->x[0] > 0 || l->x[1] > 0 || l->x[2] > r) {
		return +1;
	}
	if (l->x[2] < r) {
		return -1;
	}
	return 0;
}

void div_96_32(uint96 *dividend, uint32_t divisor)
{
	if (divisor == 0) {
		extern void HardFault_Handler(void) __attribute__((__noreturn__));
		HardFault_Handler();
		return;
	}
	uint96 quotient = UINT96_ZERO;
	uint96 remainder = UINT96_ZERO;
	for (int bit = 95; bit >= 0; --bit) {
		shl1_96(&remainder);
		if (test_bit_96(dividend, bit)) {
			set_bit_96(&remainder, 0);
		}
		bool divides = cmp_96_32(&remainder, divisor) >= 0;
		if (divides) {
			sub_96_32(&remainder, divisor);
			set_bit_96(&quotient, bit);
		}
	}
	*dividend = quotient;
}
