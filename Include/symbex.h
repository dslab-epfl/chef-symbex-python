#ifndef SYMBEX_H_
#define SYMBEX_H_

#ifdef SYMBEX_OPTIMIZATIONS

#ifdef HAVE_S2E
/* Include the proper stuff ... */
#include <s2e.h>
#else
/* ... otherwise, just provide dummy definitions */

static inline void s2e_get_example(void *buf, int size) {
	// Do nothing
}

static inline int s2e_is_symbolic(const void *ptr, size_t size) {
	// Do nothing
	return 0;
}

static inline void s2e_assume(int expression) {
	// Do nothing
}

static inline int s2e_version(void) {
	return 0; // Not running inside S2E
}

static inline int s2e_invoke_plugin(const char *pluginName, void *data,
		uint32_t dataSize) {
	return 1; // Return an error code
}

#endif

/* Concretize memory block sizes at allocation */
#define _SYMBEX_ALLOC             1

#define _SYMBEX_VARSIZE           1

/* Hash functions optimized for symbolic execution */
#define _SYMBEX_HASHES            1

/* A dict implementation that can fall back on linear search when symbolic
 * objects are added as keys.
 */

/*#define _SYMBEX_DICT_HASHES       1*/
#define _SYMBEX_CONST_HASHES      1
/*#define _SYMBEX_GLOBAL_HASHES     1*/

#define _SYMBEX_INTERNED          1
#define _SYMBEX_INTERNED_STRING   1

#define _SYMBEX_SHORT_CIRCUITED   1
#define _SYMBEX_INSTRUMENT        1
#endif

#define MAX_ALLOC_SIZE		1024

#if 0
#define PREPARE_ALLOC(size) \
	do { \
		if (s2e_is_symbolic(&(size), sizeof(size))) { \
			size = s2e_get_upper_bound(size); \
		} \
	} while(0)
#else
#define PREPARE_ALLOC(size) \
	do { \
		s2e_get_example(&(size), sizeof(size)); \
	} while(0)
#endif

#define _SYMBEX_HASH_VALUE       0xABC

#ifdef _SYMBEX_INTERNED_STRING
#define IS_SYMBOLIC_STR_SIZE(str, size) \
	(s2e_is_symbolic(&(str), sizeof(str)) || \
     s2e_is_symbolic(&(size), sizeof(size)) || \
     ((str) != NULL && s2e_is_symbolic((str), size)))

#define IS_SYMBOLIC_STR(str) \
	(s2e_is_symbolic(&(str), sizeof(str)) || \
     ((str) != NULL && s2e_is_symbolic((str), 0)))
#else
#define IS_SYMBOLIC_STR_SIZE(str, size) 0
#define IS_SYMBOLIC_STR(str)            0
#endif

#endif /* !SYMBEX_H_ */
