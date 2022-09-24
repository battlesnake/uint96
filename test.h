#pragma once
#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdarg.h>
#include <inttypes.h>

#define assert_equal(type, format, expect, actual) \
		do { \
			type _expect = (expect); \
			type _actual = (actual); \
			_test_result(_expect == _actual, "equal", format, #expect, #actual, _expect, _actual); \
		} while (0)

#define assert_true(expr) assert_equal(bool, "%d", true, expr)
#define assert_false(expr) assert_equal(bool, "%d", false, expr)
#define assert_eq_xint(expect, actual) assert_equal(uint32_t, "0x%08" PRIx32, expect, actual)
#define assert_eq_uint(expect, actual) assert_equal(uint32_t, "%" PRIu32, expect, actual)
#define assert_eq_int(expect, actual) assert_equal(int32_t, "%" PRId32, expect, actual)
#define assert_eq_xlong(expect, actual) assert_equal(uint32_t, "0x%016" PRIx64, expect, actual)
#define assert_eq_ulong(expect, actual) assert_equal(uint32_t, "%" PRIu64, expect, actual)
#define assert_eq_long(expect, actual) assert_equal(int32_t, "%" PRId64, expect, actual)

static unsigned count_test_case;
static unsigned count_test_assertion;
static unsigned count_passed;
static unsigned count_failed;

static void test_case(const char *name)
{
	count_test_case++;
	count_test_assertion = 0;
	printf("\n\x1b[1mTest case #%u: %s\x1b[0m\n", count_test_case, name);
}

static void _test_result(bool result, const char *relation, const char *format, const char *expect, const char *actual, ...)
{
	va_list ap;
	va_start(ap, actual);
	count_test_assertion++;
	char buf[100];
	const char *color_begin = result ? "\x1b[32m" : "\x1b[31m";
	const char *reset = "\x1b[0m";
#ifdef SHORT_PASS
	if (result) {
		printf(
			"%s[#%u.%u]%s ",
			color_begin,
			count_test_case,
			count_test_assertion,
			reset
		);
	} else {
#endif
		printf(
			"\n"
			"%sTest case #%u.%u:\n"
			"\tRelation: %s\n"
			"\tExpect expression: %s\n"
			"\tActual expression: %s\n",
			color_begin,
			count_test_case,
			count_test_assertion,
			relation,
			expect,
			actual
		);
		snprintf(buf, sizeof(buf),
			"\n"
			"\tExpect value: %s\n"
			"\tActual value: %s\n"
			"%s",
			format,
			format,
			reset
		);
		vprintf(buf, ap);
#ifdef SHORT_PASS
	}
#endif
	va_end(ap);
	if (result) {
		count_passed++;
	} else {
		count_failed++;
	}
}

void run_tests();

int main(int argc, char *argv[])
{
	(void) argc;
	(void) argv;
	run_tests();
	printf("\n");
	return !!count_failed;
}
