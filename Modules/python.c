/* Minimal main program -- everything is loaded from the library */

#include "Python.h"
#include "symbex.h"

#ifdef __FreeBSD__
#include <floatingpoint.h>
#endif

extern void chef_fn_begin(const char *, int, int);
extern void chef_fn_end(void);
extern void chef_bb(int);

#ifdef SYMBEX_INSTRUMENTATION

extern void chef_set_enabled(int enabled);

static int enable_state = 0;

void chef_set_enabled(int enabled) {
    enable_state = enabled;
}

void chef_fn_begin(const char *fn_name, int size, int bb_count) {
    if (!enable_state) {
        return;
    }

    __chef_fn_begin(fn_name, size, (uintptr_t)__builtin_return_address(0));
}

void chef_fn_end(void) {
    if (!enable_state) {
        return;
    }

    __chef_fn_end();
}

void chef_bb(int bb) {
    if (!enable_state) {
        return;
    }

    __chef_bb(bb);
}

#else

void chef_fn_begin(const char *fn_name, int size, int bb_count) {

}

void chef_fn_end(void) {

}

void chef_bb(int bb) {

}

#endif




int
main(int argc, char **argv)
{
	/* 754 requires that FP exceptions run in "no stop" mode by default,
	 * and until C vendors implement C99's ways to control FP exceptions,
	 * Python requires non-stop mode.  Alas, some platforms enable FP
	 * exceptions by default.  Here we disable them.
	 */
#ifdef __FreeBSD__
	fp_except_t m;

	m = fpgetmask();
	fpsetmask(m & ~FP_X_OFL);
#endif
	return Py_Main(argc, argv);
}
