#include "test.h"
#include "uint96.h"

/*
 * Random words::
 * node -p 'Array(32).fill(0).map(x => "0123456789abcdef".charAt(Math.random() * 16)).join("").replace(/(.){8,8}/g, "$& ")'
 */

/*
 * Generate solutions:
 * https://defuse.ca/big-number-calculator.htm
 */

static void basics()
{
	uint96 a;

	test_case("zero");
	a = UINT96_ZERO;
	assert_eq_xint(0, a.x[0]);
	assert_eq_xint(0, a.x[1]);
	assert_eq_xint(0, a.x[2]);

	test_case("uint32&96 round-trip");
	a = UINT96_FROM_UINT32(0x12345678);
	assert_eq_xint(0, a.x[0]);
	assert_eq_xint(0, a.x[1]);
	assert_eq_xint(0x12345678, a.x[2]);

	test_case("bit-twiddling");
	a = (uint96) { 0x00000001, 0x80000000, 0 };
	assert_false(test_bit_96(&a, 0));
	assert_false(test_bit_96(&a, 31));
	assert_false(test_bit_96(&a, 32));
	assert_true(test_bit_96(&a, 63));
	assert_true(test_bit_96(&a, 64));
	assert_false(test_bit_96(&a, 95));
	set_bit_96(&a, 95);
	assert_true(test_bit_96(&a, 95));
	reset_bit_96(&a, 95);
	assert_false(test_bit_96(&a, 95));
}

static void addition()
{
	uint96 a;

	test_case("addition");
	/* Overflow */
	a = (uint96) { .x = { -1, -1, -1 } };
	assert_true(add_96_32(&a, 1));
	assert_eq_uint(0, a.x[0]);
	assert_eq_uint(0, a.x[1]);
	assert_eq_uint(0, a.x[2]);
}

static void subtraction()
{
	uint96 a;

	test_case("subtraction");
	/* Underflow */
	a = (uint96) { .x = { 0, 0, 0 } };
	assert_true(sub_96_32(&a, 1));
	assert_eq_xint(0xffffffff, a.x[0]);
	assert_eq_xint(0xffffffff, a.x[1]);
	assert_eq_xint(0xffffffff, a.x[2]);
	/* Random */
	a = (uint96) { 0x594944e1, 0x6cec1220, 0x2ba1dba2 };
	assert_false(sub_96_32(&a, 0x88e98f5c));
	assert_eq_xint(0x594944e1, a.x[0]);
	assert_eq_xint(0x6cec121f, a.x[1]);
	assert_eq_xint(0xa2b84c46, a.x[2]);
}

static void multiplication()
{
	uint96 a;

	test_case("multiplication");
	a = (uint96) { .x = { 0, 0, 1 } };
	assert_eq_int(0, mul_96_32(&a, 1));
	assert_eq_int(0, a.x[0]);
	assert_eq_int(0, a.x[1]);
	assert_eq_int(1, a.x[2]);

	test_case("multiplication");
	a = (uint96) { .x = { 0, 0, -1 } };
	assert_eq_xint(0, mul_96_32(&a, -1));
	assert_eq_xint(0x00000000, a.x[0]);
	assert_eq_xint(0xfffffffe, a.x[1]);
	assert_eq_xint(0x00000001, a.x[2]);
	assert_eq_xint(0, mul_96_32(&a, -1));
	assert_eq_xint(0xfffffffd, a.x[0]);
	assert_eq_xint(0x00000002, a.x[1]);
	assert_eq_xint(0xffffffff, a.x[2]);
	assert_eq_xint(0xfffffffc, mul_96_32(&a, -1));
	assert_eq_xint(0x00000005, a.x[0]);
	assert_eq_xint(0xfffffffc, a.x[1]);
	assert_eq_xint(0x00000001, a.x[2]);
}

static void shift()
{
	uint96 a;

	a = (uint96) { 0x55555555, 0xaaaaaaaa, 0xf0f0f0f0 };
	test_case("Shift by one bit");
	/* Left */
	assert_eq_uint(0, shl1_96(&a));
	assert_eq_xint(0xaaaaaaab, a.x[0]);
	assert_eq_xint(0x55555555, a.x[1]);
	assert_eq_xint(0xe1e1e1e0, a.x[2]);
	assert_eq_uint(1, shl1_96(&a));
	assert_eq_xint(0x55555556, a.x[0]);
	assert_eq_xint(0xaaaaaaab, a.x[1]);
	assert_eq_xint(0xc3c3c3c0, a.x[2]);
	/* Right */
	assert_eq_uint(0, shr1_96(&a));
	assert_eq_xint(0x2aaaaaab, a.x[0]);
	assert_eq_xint(0x55555555, a.x[1]);
	assert_eq_xint(0xe1e1e1e0, a.x[2]);
	assert_eq_uint(0, shr1_96(&a));
	assert_eq_xint(0x15555555, a.x[0]);
	assert_eq_xint(0xaaaaaaaa, a.x[1]);
	assert_eq_xint(0xf0f0f0f0, a.x[2]);

	test_case("Shift by multiple bits (zero)");
	/* Left */
	a = (uint96) { 0x11223344, 0x55667788, 0x99aabbcc };
	shl_96(&a, 0);
	assert_eq_xint(0x11223344, a.x[0]);
	assert_eq_xint(0x55667788, a.x[1]);
	assert_eq_xint(0x99aabbcc, a.x[2]);
	/* Right */
	a = (uint96) { 0x11223344, 0x55667788, 0x99aabbcc };
	shr_96(&a, 0);
	assert_eq_xint(0x11223344, a.x[0]);
	assert_eq_xint(0x55667788, a.x[1]);
	assert_eq_xint(0x99aabbcc, a.x[2]);

	test_case("Shift by multiple bits (less than a word)");
	/* Left */
	a = (uint96) { 0x11223344, 0x55667788, 0x99aabbcc };
	shl_96(&a, 24);
	assert_eq_xint(0x44556677, a.x[0]);
	assert_eq_xint(0x8899aabb, a.x[1]);
	assert_eq_xint(0xcc000000, a.x[2]);
	/* Right */
	a = (uint96) { 0x11223344, 0x55667788, 0x99aabbcc };
	shr_96(&a, 24);
	assert_eq_xint(0x00000011, a.x[0]);
	assert_eq_xint(0x22334455, a.x[1]);
	assert_eq_xint(0x66778899, a.x[2]);

	test_case("Shift by multiple bits (more than a word)");
	/* Left */
	a = (uint96) { 0x11223344, 0x55667788, 0x99aabbcc };
	shl_96(&a, 40);
	assert_eq_xint(0x66778899, a.x[0]);
	assert_eq_xint(0xaabbcc00, a.x[1]);
	assert_eq_xint(0x00000000, a.x[2]);
	/* Right */
	a = (uint96) { 0x11223344, 0x55667788, 0x99aabbcc };
	shr_96(&a, 40);
	assert_eq_xint(0x00000000, a.x[0]);
	assert_eq_xint(0x00112233, a.x[1]);
	assert_eq_xint(0x44556677, a.x[2]);
}

static void divide()
{
	test_case("division");
	uint96 a;
	a = (uint96) { 0, 0, 1 };
	div_96_32(&a, 1);
	assert_eq_xint(0x00000000, a.x[0]);
	assert_eq_xint(0x00000000, a.x[1]);
	assert_eq_xint(0x00000001, a.x[2]);
	a = (uint96) { 0, 0, 1 };
	div_96_32(&a, 2);
	assert_eq_xint(0x00000000, a.x[0]);
	assert_eq_xint(0x00000000, a.x[1]);
	assert_eq_xint(0x00000000, a.x[2]);
	a = (uint96) { 0, 0, 5 };
	div_96_32(&a, 2);
	assert_eq_xint(0x00000000, a.x[0]);
	assert_eq_xint(0x00000000, a.x[1]);
	assert_eq_xint(0x00000002, a.x[2]);
	a = (uint96) { 0, 5, 0 };
	div_96_32(&a, 2);
	assert_eq_xint(0x00000000, a.x[0]);
	assert_eq_xint(0x00000002, a.x[1]);
	assert_eq_xint(0x80000000, a.x[2]);
	a = (uint96) { 0, 0, 0x87654321 };
	div_96_32(&a, 0x12345678);
	assert_eq_xint(0x00000000, a.x[0]);
	assert_eq_xint(0x00000000, a.x[1]);
	assert_eq_xint(0x00000007, a.x[2]);
	a = (uint96) { 0x12345678, 0x12345678, 0x12345678 };
	div_96_32(&a, 0x87654321);
	assert_eq_xint(0x00000000, a.x[0]);
	assert_eq_xint(0x226b9021, a.x[1]);
	assert_eq_xint(0x691f5d70, a.x[2]);
	/* Random */
	a = (uint96) { 0x594944e1, 0x6cec1220, 0x2ba1dba2 };
	div_96_32(&a, 0x88e98f5c);
	assert_eq_xint(0x00000000, a.x[0]);
	assert_eq_xint(0xa6f2bd19, a.x[1]);
	assert_eq_xint(0xd5baa4da, a.x[2]);
}

void run_tests()
{
	basics();
	shift();
	addition();
	subtraction();
	multiplication();
	divide();
}

void HardFault_Handler()
{
	assert_true(false);
}
