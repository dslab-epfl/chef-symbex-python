/* Minimal main program -- everything is loaded from the library */

#include "Python.h"
#include "symbex.h"

#ifdef __FreeBSD__
#include <floatingpoint.h>
#endif

extern void chef_fn_begin(const char *, int);
extern void chef_fn_end(const char *, int);
extern void chef_bb(int);

#ifdef SYMBEX_INSTRUMENTATION

void chef_fn_begin(const char *fn_name, int size) {
    if (!Py_EnableS2EFlag) {
        return;
    }

    static ConcolicMessage message = {
        .command = FUNCTION_BEGIN,
        .max_time = 0,
        .arg_ptr = 0,
        .arg_size = 0
    };

    message.arg_ptr = (uintptr_t)fn_name;
    message.arg_size = size;

    s2e_invoke_plugin("ConcolicSession", (void*)&message, sizeof(message));
}

void chef_fn_end(const char *fn_name, int size) {
    if (!Py_EnableS2EFlag) {
        return;
    }

    static ConcolicMessage message = {
        .command = FUNCTION_END,
        .max_time = 0,
        .arg_ptr = 0,
        .arg_size = 0
    };

    message.arg_ptr = (uintptr_t)fn_name;
    message.arg_size = size;

    s2e_invoke_plugin("ConcolicSession", (void*)&message, sizeof(message));
}

void chef_bb(int bb) {
    if (!Py_EnableS2EFlag) {
        return;
    }

    static ConcolicMessage message = {
        .command = BASIC_BLOCK,
        .max_time = 0,
        .arg_ptr = 0,
        .arg_size = 0
    };

    message.arg_ptr = (uint32_t)bb;

    s2e_invoke_plugin("ConcolicSession", (void*)&message, sizeof(message));
}

#else

void chef_fn_begin(const char *fn_name, int size) {

}

void chef_fn_end(const char *fn_name, int size) {

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
