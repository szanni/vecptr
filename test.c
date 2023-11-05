/*-
 * Copyright (c) 2014, Angelo Haller
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN ACTION
 * OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#if !defined __STDC_VERSION__ && !defined inline
#define inline
#endif

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>

#include <stdlib.h>
#include <string.h>

/* Wrap realloc to simulate realloc failure conditions in tests */
void * _wrap_realloc(void *ptr, size_t size);
#define realloc(a,b) _wrap_realloc(a,b)
#include "vecptr.h"
#undef realloc

#include "unused.h"

VECPTR_TYPEDEF(int, int);
VECPTR_TYPEDEF(char*, string);
char* strings[] = {"ab", "cd", "ef", NULL};

void *
_wrap_realloc(void * ptr, size_t size)
{
	static size_t prev_size;
	void *new;

	switch (mock_type(int)) {
		case 1:
			/* Emulate realloc failure */
			return NULL;
		case 2:
			/* Force a different ptr address */
			if (ptr == NULL)
				prev_size = 0;

			new = realloc(NULL, size);
			if (new != NULL) {
				if (ptr != NULL)
					memcpy(new, ptr, prev_size);
				free(ptr);
				prev_size = size;
			}
			return new;
		case 0:
		default:
			prev_size = size;
			return realloc(ptr, size);
	}
}

void
test_INIT_EMPTY(void ** UNUSED(state))
{
	int *data;
	size_t ndata;
	VECPTR(int) v;
	VECPTR_INIT_EMPTY(v, &data, &ndata);

	will_return(_wrap_realloc, 0);

	assert_int_equal(VECPTR_SIZE(v), 0);
	assert_int_equal(VECPTR_CAPACITY(v), VECPTR_DEFAULT_CAPACITY);

	assert_int_equal(VECPTR_APPEND(v, 10), 0);

	assert_int_equal(VECPTR_SIZE(v), 1);
	assert_int_equal(VECPTR_CAPACITY(v), VECPTR_DEFAULT_CAPACITY);

	VECPTR_FREE(v);
}

void
test_INIT_EMPTY_CAPACITY(void ** UNUSED(state))
{
	int *data;
	size_t ndata;
	VECPTR(int) v;
	VECPTR_INIT_EMPTY_CAPACITY(v, &data, &ndata, 7);

	will_return(_wrap_realloc, 0);

	assert_int_equal(VECPTR_SIZE(v), 0);
	assert_int_equal(VECPTR_CAPACITY(v), 7);

	assert_int_equal(VECPTR_APPEND(v, 10), 0);

	assert_int_equal(VECPTR_SIZE(v), 1);
	assert_int_equal(VECPTR_CAPACITY(v), 7);

	VECPTR_FREE(v);
}

void
_test_INIT_DATA(int *data, size_t size)
{
	VECPTR_TYPE(int) v;
	VECPTR_INIT_DATA(v, &data, &size);

	assert_int_equal(VECPTR_AT(v, 0), 10);
	assert_int_equal(VECPTR_AT(v, 1), 20);
	assert_int_equal(VECPTR_AT(v, 2), 30);

	VECPTR_FREE(v);
}

void
test_INIT_DATA(void ** UNUSED(state))
{
	int *data;
	size_t ndata;
	VECPTR_TYPE(int) v;
	VECPTR_INIT_EMPTY(v, &data, &ndata);

	will_return(_wrap_realloc, 0);

	assert_int_equal(VECPTR_APPEND(v, 10), 0);
	assert_int_equal(VECPTR_APPEND(v, 20), 0);
	assert_int_equal(VECPTR_APPEND(v, 30), 0);

	_test_INIT_DATA(VECPTR_DATA(v), VECPTR_SIZE(v));
}

void
test_data_passable_after_modification(void ** UNUSED(state))
{
	int *data;
	size_t ndata;
	VECPTR_TYPE(int) v;
	VECPTR_INIT_EMPTY(v, &data, &ndata);

	will_return(_wrap_realloc, 0);

	assert_int_equal(VECPTR_APPEND(v, 10), 0);
	assert_int_equal(VECPTR_APPEND(v, 20), 0);
	assert_int_equal(VECPTR_APPEND(v, 30), 0);

	_test_INIT_DATA(data, ndata);
}

void
test_new(void ** UNUSED(state))
{
	/* Ensure VECPTR_NEW() is not affected by uninitialized structures */
	int *data = (int*)0x5EEDC0DE;
	size_t ndata = 1;
	VECPTR(int) v;

	will_return(_wrap_realloc, 0);

	assert_int_equal(VECPTR_NEW(v, &data, &ndata), 0);
	assert_int_equal(VECPTR_SIZE(v), 0);
	assert_int_equal(VECPTR_CAPACITY(v), VECPTR_DEFAULT_CAPACITY);

	VECPTR_FREE(v);
}

void
test_new_capacity(void ** UNUSED(state))
{
	/* Ensure VECPTR_NEW_CAPACITY() is not affected by uninitialized structures */
	int *data = (int*)0x5EEDC0DE;
	size_t ndata = 1;

	VECPTR(int) v;

	will_return(_wrap_realloc, 0);

	assert_int_equal(VECPTR_NEW_CAPACITY(v, &data, &ndata, 7), 0);
	assert_int_equal(VECPTR_SIZE(v), 0);
	assert_int_equal(VECPTR_CAPACITY(v), 7);

	VECPTR_FREE(v);
}

void
test_append_grow(void ** UNUSED(state))
{
	size_t i;
	size_t capacity;
	int *data;
	size_t ndata;
	VECPTR(int) v;
	VECPTR_INIT_EMPTY(v, &data, &ndata);

	will_return_always(_wrap_realloc, 0);

	capacity = VECPTR_CAPACITY(v) * 2;

	for (i = 0; i < capacity; ++i)
		assert_int_equal(VECPTR_APPEND(v, i), 0);

	for (i = 0; i < capacity; ++i)
		assert_int_equal(VECPTR_AT(v, i), i);

	VECPTR_FREE(v);
}

void
test_append_grow_change_data_ptr(void ** UNUSED(state))
{
	size_t i;
	size_t capacity;
	int *data;
	int *old_data_ptr;
	size_t ndata;
	VECPTR(int) v;
	VECPTR_INIT_EMPTY(v, &data, &ndata);

	will_return_always(_wrap_realloc, 2);

	capacity = VECPTR_CAPACITY(v);

	for (i = 0; i < capacity; ++i)
		assert_int_equal(VECPTR_APPEND(v, i), 0);

	old_data_ptr = data;
	capacity = VECPTR_CAPACITY(v) * 2;

	for (i = VECPTR_CAPACITY(v); i < capacity; ++i)
		assert_int_equal(VECPTR_APPEND(v, i), 0);

	assert_ptr_not_equal(data, old_data_ptr);

	for (i = 0; i < capacity; ++i)
		assert_int_equal(VECPTR_AT(v, i), i);

	for (i = 0; i < ndata; ++i)
		assert_int_equal(data[i], i);

	VECPTR_FREE(v);
}

void
test_prepend_grow(void ** UNUSED(state))
{
	size_t i;
	size_t capacity;
	int *data;
	size_t ndata;
	VECPTR(int) v;
	VECPTR_INIT_EMPTY(v, &data, &ndata);

	will_return_always(_wrap_realloc, 0);

	capacity = VECPTR_CAPACITY(v) * 2;

	for (i = 0; i < capacity; ++i)
		assert_int_equal(VECPTR_PREPEND(v, i), 0);

	for (i = 0; i < capacity; ++i)
		assert_int_equal(VECPTR_AT(v, i), capacity-1-i);

	VECPTR_FREE(v);
}

void
test_erase(void ** UNUSED(state))
{
	size_t i;
	int *data;
	size_t ndata;
	VECPTR(int) v;
	VECPTR_INIT_EMPTY(v, &data, &ndata);

	will_return_always(_wrap_realloc, 0);

	for (i = 0; i < 10; ++i)
		assert_int_equal(VECPTR_APPEND(v, i), 0);

	VECPTR_ERASE(v, 5);
	VECPTR_ERASE(v, 0);
	VECPTR_ERASE(v, 7);
	VECPTR_ERASE(v, 1);
	VECPTR_ERASE(v, 2);
	VECPTR_ERASE(v, 2);
	VECPTR_ERASE(v, 3);

	assert_int_equal(VECPTR_AT(v, 0), 1);
	assert_int_equal(VECPTR_AT(v, 1), 3);
	assert_int_equal(VECPTR_AT(v, 2), 7);

	assert_int_equal(VECPTR_SIZE(v), 3);

	VECPTR_FREE(v);
}

void
test_erase_append(void ** UNUSED(state))
{
	int *data;
	size_t ndata;
	VECPTR(int) v;
	VECPTR_INIT_EMPTY(v, &data, &ndata);

	will_return_always(_wrap_realloc, 0);

	assert_int_equal(VECPTR_APPEND(v, 0), 0);

	VECPTR_ERASE(v, 0);

	assert_int_equal(VECPTR_APPEND(v, 1), 0);

	assert_int_equal(VECPTR_AT(v, 0), 1);
	assert_int_equal(VECPTR_SIZE(v), 1);

	VECPTR_FREE(v);
}

void
test_erase_init_data_empty_append(void ** UNUSED(state))
{
	int *data;
	size_t ndata;
	VECPTR(int) v;
	VECPTR_INIT_EMPTY(v, &data, &ndata);

	will_return_always(_wrap_realloc, 0);

	assert_int_equal(VECPTR_APPEND(v, 0), 0);

	VECPTR_ERASE(v, 0);

	VECPTR_INIT_DATA(v, &data, &ndata);

	assert_int_equal(VECPTR_APPEND(v, 1), 0);

	assert_int_equal(VECPTR_AT(v, 0), 1);
	assert_int_equal(VECPTR_SIZE(v), 1);

	VECPTR_FREE(v);
}

void
test_front(void ** UNUSED(state))
{
	int *data;
	size_t ndata;
	VECPTR(int) v;
	VECPTR_INIT_EMPTY(v, &data, &ndata);

	will_return_always(_wrap_realloc, 0);

	assert_int_equal(VECPTR_APPEND(v, 10), 0);
	assert_int_equal(VECPTR_FRONT(v), 10);

	assert_int_equal(VECPTR_APPEND(v, 20), 0);
	assert_int_equal(VECPTR_FRONT(v), 10);

	VECPTR_FREE(v);
}

void
test_back(void ** UNUSED(state))
{
	int *data;
	size_t ndata;
	VECPTR(int) v;
	VECPTR_INIT_EMPTY(v, &data, &ndata);

	will_return_always(_wrap_realloc, 0);

	assert_int_equal(VECPTR_APPEND(v, 10), 0);
	assert_int_equal(VECPTR_BACK(v), 10);

	assert_int_equal(VECPTR_APPEND(v, 20), 0);
	assert_int_equal(VECPTR_BACK(v), 20);

	VECPTR_FREE(v);
}

void
test_new_fail(void ** UNUSED(state))
{
	int *data;
	size_t ndata;
	VECPTR(int) v;

	will_return(_wrap_realloc, 1);

	assert_int_equal(VECPTR_NEW(v, &data, &ndata), 1);
}

void
test_new_capacity_fail(void ** UNUSED(state))
{
	int *data;
	size_t ndata;
	VECPTR(int) v;

	will_return(_wrap_realloc, 1);

	assert_int_equal(VECPTR_NEW_CAPACITY(v, &data, &ndata, 5), 1);
}

void
test_append_grow_fail(void ** UNUSED(state))
{
	unsigned i;
	int *data;
	size_t ndata;
	VECPTR(int) v;

	will_return(_wrap_realloc, 0);
	will_return(_wrap_realloc, 1);

	assert_int_equal(VECPTR_NEW(v, &data, &ndata), 0);
	for (i = 0; i < VECPTR_CAPACITY(v); ++i)
		assert_int_equal(VECPTR_APPEND(v, i), 0);
	assert_int_equal(VECPTR_APPEND(v, i), 1);

	VECPTR_FREE(v);
}

void
test_prepend_grow_fail(void ** UNUSED(state))
{
	unsigned i;
	int *data;
	size_t ndata;
	VECPTR(int) v;

	will_return(_wrap_realloc, 0);
	will_return(_wrap_realloc, 1);

	assert_int_equal(VECPTR_NEW(v, &data, &ndata), 0);
	for (i = 0; i < VECPTR_CAPACITY(v); ++i)
		assert_int_equal(VECPTR_PREPEND(v, i), 0);
	assert_int_equal(VECPTR_APPEND(v, i), 1);

	VECPTR_FREE(v);
}

void
test_shrink_to_fit(void ** UNUSED(state))
{
	int *data;
	size_t ndata;
	VECPTR(int) v;

	VECPTR_INIT_EMPTY_CAPACITY(v, &data, &ndata, 5);

	will_return(_wrap_realloc, 0);
	will_return(_wrap_realloc, 0);

	assert_int_equal(VECPTR_APPEND(v, 10), 0);
	assert_int_equal(VECPTR_APPEND(v, 20), 0);
	assert_int_equal(VECPTR_APPEND(v, 30), 0);

	assert_int_equal(VECPTR_CAPACITY(v), 5);

	VECPTR_SHRINK_TO_FIT(v);

	assert_int_equal(VECPTR_CAPACITY(v), 3);

	will_return(_wrap_realloc, 0);

	assert_int_equal(VECPTR_APPEND(v, 40), 0);

	VECPTR_FREE(v);
}

void
test_basic_example(void ** UNUSED(state))
{
	int *data;
	size_t ndata;
	VECPTR(int) v;
	VECPTR_INIT_EMPTY(v, &data, &ndata);

	will_return_always(_wrap_realloc, 0);

	assert_int_equal(VECPTR_APPEND(v, 10), 0);
	assert_int_equal(VECPTR_APPEND(v, 20), 0);

	assert_int_equal(VECPTR_AT(v, 0), 10);
	assert_int_equal(VECPTR_AT(v, 1), 20);

	VECPTR_FREE(v);
}

int
main(void)
{
	const struct CMUnitTest tests[] = {
		cmocka_unit_test(test_INIT_EMPTY),
		cmocka_unit_test(test_INIT_EMPTY_CAPACITY),
		cmocka_unit_test(test_INIT_DATA),
		cmocka_unit_test(test_data_passable_after_modification),
		cmocka_unit_test(test_new),
		cmocka_unit_test(test_new_capacity),
		cmocka_unit_test(test_append_grow),
		cmocka_unit_test(test_append_grow_change_data_ptr),
		cmocka_unit_test(test_prepend_grow),
		cmocka_unit_test(test_erase),
		cmocka_unit_test(test_erase_append),
		cmocka_unit_test(test_erase_init_data_empty_append),
		cmocka_unit_test(test_front),
		cmocka_unit_test(test_back),
		cmocka_unit_test(test_new_fail),
		cmocka_unit_test(test_new_capacity_fail),
		cmocka_unit_test(test_append_grow_fail),
		cmocka_unit_test(test_prepend_grow_fail),
		cmocka_unit_test(test_shrink_to_fit),
		cmocka_unit_test(test_basic_example),
	};
	return cmocka_run_group_tests(tests, NULL, NULL);
}

