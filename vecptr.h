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

#define VECPTR_GROWTH_FACTOR 1.5f
#define VECPTR_DEFAULT_CAPACITY 5u

#define VECPTR_INIT_EMPTY(v, d, s) VECPTR_INIT_EMPTY_CAPACITY((v), (d), (s), VECPTR_DEFAULT_CAPACITY)
#define VECPTR_INIT_EMPTY_CAPACITY(v, d, s, c)									\
	((v).size = (s), VECPTR_SIZE(v) = 0, VECPTR_CAPACITY(v) = (c), (v).data = (d), VECPTR_DATA(v) = NULL)

#define VECPTR_INIT_DATA(v, d, s) ((v).size = (s), VECPTR_CAPACITY(v) = *(s), (v).data = (d))

#define VECPTR(type)		\
struct {			\
	size_t *size;		\
	size_t capacity;	\
	type **data;		\
}

#define VECPTR_TYPE(name) vecptr_ ## name

#define VECPTR_TYPEDEF(type, name) typedef VECPTR(type) VECPTR_TYPE(name)

#define VECPTR_NEW(v, d, s) VECPTR_NEW_CAPACITY((v), (d), (s), VECPTR_DEFAULT_CAPACITY)

#define VECPTR_NEW_CAPACITY(v, d, s, c) (VECPTR_INIT_EMPTY((v), (d), (s)), _vecptr_new_capacity(_vecptr(v), (c)))

#define VECPTR_APPEND(v, e) ((!_vecptr_grow_if_needed(_vecptr(v))) ? (VECPTR_AT((v), VECPTR_SIZE(v)++) = e, 0) : 1)

#define VECPTR_PREPEND(v, e) ((!_vecptr_prepend(_vecptr(v))) ? (VECPTR_AT((v), 0) = e, (VECPTR_SIZE(v))++, 0) : 1)

#define VECPTR_ERASE(v, i) _vecptr_erase(_vecptr(v), (i))

#define VECPTR_AT(v, i) (VECPTR_DATA(v)[i])

#define VECPTR_FRONT(v) VECPTR_AT((v), 0)

#define VECPTR_BACK(v) VECPTR_AT((v), VECPTR_SIZE(v)-1)

#define VECPTR_FOREACH_PTR(v, ep) for ((ep) = VECPTR_DATA(v); (ep) < VECPTR_DATA(v) + VECPTR_SIZE(v); ++(ep))

#define VECPTR_DATA(v) (*((v).data))

#define VECPTR_SIZE(v) (*((v).size))

#define VECPTR_CAPACITY(v) ((v).capacity)

#define VECPTR_SHRINK_TO_FIT(v) _vecptr_shrink_to_fit(_vecptr(v))

#define VECPTR_FREE(v) free(VECPTR_DATA(v))

/**********************
 * Helper definitions *
 **********************/

VECPTR_TYPEDEF(void, _vecptr_void);

#define _vecptr(v) (VECPTR_TYPE(_vecptr_void) *)&(v), sizeof(*VECPTR_DATA(v))

#if __STDC_VERSION__ >= 199901L
static inline int
#else
static int
#endif
_vecptr_resize(VECPTR_TYPE(_vecptr_void) *v, size_t sizeof_type, size_t new_capacity)
{
	void *new_data;

	new_data = realloc(*(v->data), new_capacity * sizeof_type);
	if (new_data == NULL)
		return 1;

	*(v->data) = new_data;
	v->capacity = new_capacity;
	return 0;
}

#if __STDC_VERSION__ >= 199901L
static inline int
#else
static int
#endif
_vecptr_grow_if_needed(VECPTR_TYPE(_vecptr_void) *v, size_t sizeof_type)
{
	size_t new_capacity;

	if (*(v->data) != NULL) {
		if (*(v->size) < v->capacity)
			return 0;

		new_capacity = (size_t)(v->capacity * VECPTR_GROWTH_FACTOR);
	}
	else {
		new_capacity = v->capacity;
	}

	/* Ensure to always grow the vecptr. Needed for capacity == 0
	 * or capacity == 1 and a growth factor < 2. */
	if (new_capacity <= *(v->size))
		new_capacity = *(v->size) + VECPTR_DEFAULT_CAPACITY;

	return _vecptr_resize(v, sizeof_type, new_capacity);
}

#if __STDC_VERSION__ >= 199901L
static inline void
#else
static void
#endif
_vecptr_memmove(VECPTR_TYPE(_vecptr_void) *v, size_t sizeof_type, size_t destidx, size_t srcidx)
{
	memmove((char*)(*(v->data)) + sizeof_type * destidx,
		(char*)(*(v->data)) + sizeof_type * srcidx,
		(*(v->size) - srcidx) * sizeof_type);
}

#if __STDC_VERSION__ >= 199901L
static inline void
#else
static void
#endif
_vecptr_erase(VECPTR_TYPE(_vecptr_void) *v, size_t sizeof_type, size_t destidx)
{
	size_t srcidx = destidx + 1;

	_vecptr_memmove(v, sizeof_type, destidx, srcidx);
	--(*v->size);
}

#if __STDC_VERSION__ >= 199901L
static inline int
#else
static int
#endif
_vecptr_prepend(VECPTR_TYPE(_vecptr_void) *v, size_t sizeof_type)
{
	if (_vecptr_grow_if_needed(v, sizeof_type))
		return 1;

	_vecptr_memmove(v, sizeof_type, 1, 0);
	return 0;
}

#if __STDC_VERSION__ >= 199901L
static inline int
#else
static int
#endif
_vecptr_new_capacity(VECPTR_TYPE(_vecptr_void) *v, size_t sizeof_type, size_t capacity)
{
	/*
	*v->size = 0;
	*/
	v->capacity = capacity;
	/*
	*v->data = NULL;
	*/

	return _vecptr_resize(v, sizeof_type, capacity);
}

#if __STDC_VERSION__ >= 199901L
static inline void
#else
static void
#endif
_vecptr_shrink_to_fit(VECPTR_TYPE(_vecptr_void) *v, size_t sizeof_type)
{
	if (*(v->size) == 0 && v->capacity != 0) {
		v->capacity = 0;
		VECPTR_FREE(*v);
		*(v->data) = NULL;
		return;
	}
	_vecptr_resize(v, sizeof_type, *(v->size));
}

#endif

