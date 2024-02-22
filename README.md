vecptr
======

vecptr is a generic, type-safe, malloc checkable dynamic array implementation
within a single header file, written in standards compliant C (ANSI C/C99/C11).

Usage
=====

Simply copy `vecptr.h` to your project and `#include` where needed.

You may override the required C standard library functions `realloc`, `free`,
and `memmove` by defining `VECPTR_FN_REALLOC`, `VECPTR_FN_FREE`, and
`VECPTR_FN_MEMMOVE` before including `vecptr.h`.

vecptr is primarily targeted at creation of dynamic arrays on the fly.

While the library provides numerous accessors and other fancy features, the
general idea is to help developers create new or modify existing arrays while
keeping the rest of your code as idiomatic as possible.

Example
-------

### Idiomatic - Creating an Array
```
#include <stdio.h>
#include "vecptr.h"

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

* C standard library, unless you override `VECPTR_FN_REALLOC`,
  `VECPTR_FN_FREE`, and `VECPTR_FN_MEMMOVE` and provide a definition for
  `size_t`.

Test
----

* Cmocka


License
=======

ISC license, see the LICENSE file for more details.
