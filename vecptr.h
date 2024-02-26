/*-
 * Copyright (c) 2019, Angelo Haller
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

#ifndef VECPTR_H
#define VECPTR_H

#if !defined(VECPTR_FN_REALLOC) || !defined(VECPTR_FN_FREE)
#include <stdlib.h>
#endif

#ifndef VECPTR_FN_REALLOC
#define VECPTR_FN_REALLOC realloc
#endif

#ifndef VECPTR_FN_FREE
#define VECPTR_FN_FREE free
#endif

#ifndef VECPTR_FN_MEMMOVE
#include <string.h>
#define VECPTR_FN_MEMMOVE memmove
#endif

#ifndef _vecptr_restrict
	#if __STDC_VERSION__ >= 199901L
		#define _vecptr_restrict restrict
	#else
		#define _vecptr_restrict
	#endif
#endif

#define VECPTR_GROWTH_FACTOR 1.5f
#define VECPTR_DEFAULT_CAPACITY 5u

#define VECPTR_INIT_EMPTY(v, d, s) VECPTR_INIT_EMPTY_CAPACITY((v), (d), (s), VECPTR_DEFAULT_CAPACITY)
#define VECPTR_INIT_EMPTY_CAPACITY(v, d, s, c)									\
	((v).size = (s), VECPTR_SIZE(v) = 0, VECPTR_CAPACITY(v) = (c), (v).data = (d), VECPTR_DATA(v) = NULL)

#define VECPTR_INIT_DATA(v, d, s) ((v).size = (s), VECPTR_CAPACITY(v) = VECPTR_SIZE(v), (v).data = (d))

#define VECPTR(type)		\
struct {			\
	size_t *size;		\
	size_t capacity;	\
	type **data;		\
}

#define VECPTR_TYPE(name) vecptr_ ## name

#define VECPTR_TYPEDEF(type, name) typedef VECPTR(type) VECPTR_TYPE(name)

#define VECPTR_NEW(v, d, s) VECPTR_NEW_CAPACITY((v), (d), (s), VECPTR_DEFAULT_CAPACITY)

#define VECPTR_NEW_CAPACITY(v, d, s, c) (VECPTR_INIT_EMPTY((v), (d), (s)), _vecptr_new_capacity(_vecptr_param(v), (c)))

#define VECPTR_APPEND(v, e) ((!_vecptr_grow_if_needed(_vecptr_param(v))) ? (VECPTR_AT((v), VECPTR_SIZE(v)++) = e, 0) : 1)

#define VECPTR_PREPEND(v, e) ((!_vecptr_prepend(_vecptr_param(v))) ? (VECPTR_AT((v), 0) = e, (VECPTR_SIZE(v))++, 0) : 1)

#define VECPTR_ERASE(v, i) _vecptr_erase(_vecptr_param(v), (i))

#define VECPTR_AT(v, i) (VECPTR_DATA(v)[i])

#define VECPTR_FRONT(v) VECPTR_AT((v), 0)

#define VECPTR_BACK(v) VECPTR_AT((v), VECPTR_SIZE(v)-1)

#define VECPTR_FOREACH_PTR(v, ep) for ((ep) = VECPTR_DATA(v); (ep) < VECPTR_DATA(v) + VECPTR_SIZE(v); ++(ep))

#define VECPTR_DATA(v) (*((v).data))

#define VECPTR_SIZE(v) (*((v).size))

#define VECPTR_CAPACITY(v) ((v).capacity)

#define VECPTR_SHRINK_TO_FIT(v) _vecptr_shrink_to_fit(_vecptr_param(v))

#define VECPTR_FREE(v) VECPTR_FN_FREE(VECPTR_DATA(v))

/**********************
 * Helper definitions *
 **********************/

#define _vecptr_param(v) &VECPTR_SIZE(v), &VECPTR_CAPACITY(v), (void**)&VECPTR_DATA(v), sizeof(*VECPTR_DATA(v))

#if __STDC_VERSION__ >= 199901L
static inline int
#else
static int
#endif
_vecptr_resize(size_t *capacity, void **data, size_t sizeof_type, size_t new_capacity)
{
	void *new_data;

	new_data = VECPTR_FN_REALLOC(*data, new_capacity * sizeof_type);
	if (new_data == NULL)
		return 1;

	*data = new_data;
	*capacity = new_capacity;
	return 0;
}

#if __STDC_VERSION__ >= 199901L
static inline int
#else
static int
#endif
_vecptr_grow_if_needed(size_t * _vecptr_restrict size, size_t * _vecptr_restrict capacity, void **data, size_t sizeof_type)
{
	size_t new_capacity;

	if (*data != NULL) {
		if (*size < *capacity)
			return 0;

		new_capacity = (size_t)(*capacity * VECPTR_GROWTH_FACTOR);
	}
	else {
		new_capacity = *capacity;
	}

	/* Ensure to always grow the vecptr. Needed for capacity == 0
	 * or capacity == 1 and a growth factor < 2. */
	if (new_capacity <= *size)
		new_capacity = *size + VECPTR_DEFAULT_CAPACITY;

	return _vecptr_resize(capacity, data, sizeof_type, new_capacity);
}

#if __STDC_VERSION__ >= 199901L
static inline void
#else
static void
#endif
_vecptr_memmove(size_t *size, void **data, size_t sizeof_type, size_t destidx, size_t srcidx)
{
	VECPTR_FN_MEMMOVE((char*)(*data) + sizeof_type * destidx,
		(char*)(*data) + sizeof_type * srcidx,
		(*size - srcidx) * sizeof_type);
}

#if __STDC_VERSION__ >= 199901L
static inline void
#else
static void
#endif
_vecptr_erase(size_t * _vecptr_restrict size, size_t * _vecptr_restrict capacity, void **data, size_t sizeof_type, size_t destidx)
{
	size_t srcidx = destidx + 1;

	_vecptr_memmove(size, data, sizeof_type, destidx, srcidx);
	--(*size);
	(void)capacity; /* silence unused */
}

#if __STDC_VERSION__ >= 199901L
static inline int
#else
static int
#endif
_vecptr_prepend(size_t * _vecptr_restrict size, size_t * _vecptr_restrict capacity, void **data, size_t sizeof_type)
{
	if (_vecptr_grow_if_needed(size, capacity, data, sizeof_type))
		return 1;

	_vecptr_memmove(size, data, sizeof_type, 1, 0);
	return 0;
}

#if __STDC_VERSION__ >= 199901L
static inline int
#else
static int
#endif
_vecptr_new_capacity(size_t * _vecptr_restrict size, size_t * _vecptr_restrict capacity, void **data, size_t sizeof_type, size_t new_capacity)
{
	*capacity = new_capacity;
	(void)size; /* silence unused */

	return _vecptr_resize(capacity, data, sizeof_type, new_capacity);
}

#if __STDC_VERSION__ >= 199901L
static inline void
#else
static void
#endif
_vecptr_shrink_to_fit(size_t * _vecptr_restrict size, size_t * _vecptr_restrict capacity, void **data, size_t sizeof_type)
{
	if (*size == 0) {
		VECPTR_FN_FREE(*data);
		*capacity = VECPTR_DEFAULT_CAPACITY;
		*data = NULL;
		return;
	}
	_vecptr_resize(capacity, data, sizeof_type, *size);
}

#endif

