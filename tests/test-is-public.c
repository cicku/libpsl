/*
 * Copyright(c) 2014-2015 Tim Ruehsen
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 *
 * This file is part of the test suite of libpsl.
 *
 * Test case for psl_load_file(), psl_is_public_suffix(), psl_free()
 *
 * Changelog
 * 19.03.2014  Tim Ruehsen  created from libmget/cookie.c
 *
 */

#if HAVE_CONFIG_H
# include <config.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#ifdef HAVE_ALLOCA_H
#	include <alloca.h>
#endif

#include <libpsl.h>

#define countof(a) (sizeof(a)/sizeof(*(a)))

static int
	ok,
	failed;

static void test_psl(void)
{
	/* punycode generation: idn ?? */
	/* octal code generation: echo -n "??" | od -b */
	static const struct test_data {
		const char
			*domain;
		int
			result;
	} test_data[] = {
		{ "www.example.com", 0 },
		{ "com.ar", 1 },
		{ "www.com.ar", 0 },
		{ "cc.ar.us", 1 },
		{ ".cc.ar.us", 1 },
		{ "www.cc.ar.us", 0 },
		{ "www.ck", 0 }, /* exception from *.ck */
		{ "abc.www.ck", 0 },
		{ "xxx.ck", 1 },
		{ "www.xxx.ck", 0 },
		{ "\345\225\206\346\240\207", 1 }, /* xn--czr694b or ?? */
		{ "www.\345\225\206\346\240\207", 0 },
		/* some special test follow ('name' and 'forgot.his.name' are public, but e.g. his.name is not) */
		{ "name", 1 },
		{ ".name", 1 },
		{ "his.name", 0 },
		{ ".his.name", 0 },
		{ "forgot.his.name", 1 },
		{ ".forgot.his.name", 1 },
		{ "whoever.his.name", 0 },
		{ "whoever.forgot.his.name", 0 },
		{ ".", 1 }, /* special case */
		{ "", 1 },  /* special case */
		{ NULL, 1 },  /* special case */
		{ "adfhoweirh", 1 }, /* unknown TLD */
	};
	unsigned it;
	psl_ctx_t *psl;

	psl = psl_load_file(PSL_FILE);

	printf("loaded %d suffixes and %d exceptions\n", psl_suffix_count(psl), psl_suffix_exception_count(psl));

	for (it = 0; it < countof(test_data); it++) {
		const struct test_data *t = &test_data[it];
		int result = psl_is_public_suffix(psl, t->domain);

		if (result == t->result) {
			ok++;
		} else {
			failed++;
			printf("psl_is_public_suffix(%s)=%d (expected %d)\n", t->domain, result, t->result);
		}
	}

	psl_free(psl);
}

int main(int argc, const char * const *argv)
{
	/* if VALGRIND testing is enabled, we have to call ourselves with valgrind checking */
	if (argc == 1) {
		const char *valgrind = getenv("TESTS_VALGRIND");

		if (valgrind && *valgrind) {
			size_t cmdsize = strlen(valgrind) + strlen(argv[0]) + 32;
			char *cmd = alloca(cmdsize);

			snprintf(cmd, cmdsize, "TESTS_VALGRIND="" %s %s", valgrind, argv[0]);
			return system(cmd) != 0;
		}
	}

	test_psl();

	if (failed) {
		printf("Summary: %d out of %d tests failed\n", failed, ok + failed);
		return 1;
	}

	printf("Summary: All %d tests passed\n", ok + failed);
	return 0;
}
