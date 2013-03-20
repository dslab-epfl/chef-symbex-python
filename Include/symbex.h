
#ifdef SYMBEX_OPTIMIZATIONS
/* Concretize memory block sizes at allocation */
#define _SYMBEX_ALLOC             1

#define _SYMBEX_VARSIZE           1

/* Hash functions optimized for symbolic execution */
#define _SYMBEX_HASHES            1

/* A dict implementation that can fall back on linear search when symbolic
 * objects are added as keys.
 */
#define _SYMBEX_DICT_HASHES       1
/*#define _SYMBEX_GLOBAL_HASHES     1*/

#define _SYMBEX_INTERNED          1
#define _SYMBEX_INTERNED_STRING   1

/*#define _SYMBEX_SHORT_CIRCUITED   1*/
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
