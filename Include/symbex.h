
#ifdef SYMBEX_OPTIMIZATIONS
#define _SYMBEX_ALLOC             1
#define _SYMBEX_VARSIZE           1
#define _SYMBEX_HASHES            1
/*#define _SYMBEX_INTERNED          1*/
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
