vecptr
======

vecptr is a generic, type-safe, malloc checkable dynamic array implementation
within a single header file, written in standards compliant C (ANSI C/C99/C11).

Usage
=====

vecptr deliberately does not include any parts of the C standard library but
rather expects users to provide the functions `realloc()`, `memmove()`, and
`free()`, as well as the type `size_t` themselves. For most applications just
include  `<stdlib.h>` and `<string.h>` before including `<vecptr.h>`.

vecptr is primarily targeted at creation of dynamic arrays on the fly.

While the library provides numerous accessors and other fancy features, the
general idea is to help developers create new or modify existing arrays while
keeping the rest of your code as idiomatic as possible.

Example
-------

### Idiomatic - Creating an Array
```
#include <stdio.h>

#include <stdlib.h>
#include <string.h>
#include <vecptr.h>
int
main(void)
{
	size_t i;
	int *data;
	size_t ndata;
	VECPTR(int) v;

	VECPTR_INIT_EMPTY(v, &data, &ndata);

	if (VECPTR_APPEND(v, 10)) {
		return 1;
	}

	if (VECPTR_APPEND(v, 20)) {
		free(data);
		return 1;
	}

	for (i = 0; i < ndata; ++i)
		printf("%d\n", data[i]);

	/*
	do_something_else_c_style(data, ndata);
	*/

	free(data);

	return 0;
}
```

Dependencies
============

Runtime
-------

* Definitions for the functions `realloc()`, `memmove()`, `free()` and type
`size_t`.

Test
----

* Cmocka


License
=======

ISC license, see the LICENSE file for more details.
