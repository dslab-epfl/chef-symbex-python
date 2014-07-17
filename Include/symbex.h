#ifndef SYMBEX_H_
#define SYMBEX_H_

#ifdef SYMBEX_INSTRUMENTATION

#include <pydebug.h>

#include <s2e.h>


#define _SYMBEX_INSTRUMENT        1


/* Concretize memory block sizes at allocation */
#ifdef SYMBEX_OPT_CONCRETIZE_BUFFSIZES
#define _SYMBEX_ALLOC             1
#define _SYMBEX_VARSIZE           1
#endif /* SYMBEX_OPT_CONCRETIZE_BUFFSIZES */

#ifdef SYMBEX_OPT_NEUTRALIZE_HASHES
/* Hash functions optimized for symbolic execution */
#define _SYMBEX_HASHES            1
/* A dict implementation that can fall back on linear search when symbolic
 * objects are added as keys.
 */
/*#define _SYMBEX_DICT_HASHES       1*/
#define _SYMBEX_CONST_HASHES      1
/*#define _SYMBEX_GLOBAL_HASHES     1*/
#endif /* SYMBEX_OPT_NEUTRALIZE_HASHES */

#ifdef SYMBEX_OPT_DISABLE_INTERNING
#define _SYMBEX_INTERNED          1
#define _SYMBEX_INTERNED_STRING   1
#endif /* SYMBEX_OPT_DISABLE_INTERNING */

#ifdef SYMBEX_OPT_DISABLE_FAST_PATHS
#define _SYMBEX_SHORT_CIRCUITED   1
#endif /* SYMBEX_OPT_DISABLE_FAST_PATHS */

#define MAX_ALLOC_SIZE		1024

#if 0 // XXX: Investigate this optimization
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

#define _SYMBEX_TRACE_SIZE  2

#define _SYMBEX_OP_EBRANCH_POS   0
#define _SYMBEX_OP_THROWS_POS    1
#define _SYMBEX_OP_CALL_POS      2

typedef struct {
    uint32_t op_code;
    uint32_t op_attr;
    uint32_t frame_count;
    uint32_t frames[_SYMBEX_TRACE_SIZE];
} __attribute__((packed)) TraceUpdate;


typedef enum {
    START_CONCOLIC_SESSION,
    END_CONCOLIC_SESSION,
    LOG_MESSAGE,
    MERGE_BARRIER,
    FUNCTION_BEGIN,
    FUNCTION_END,
    BASIC_BLOCK
} ConcolicCommand;


#else
#define IS_SYMBOLIC_STR_SIZE(str, size) 0
#define IS_SYMBOLIC_STR(str)            0
#endif /* SYMBEX_INSTRUMENTATION */

#endif /* !SYMBEX_H_ */
