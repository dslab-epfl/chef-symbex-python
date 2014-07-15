/* Minimal main program -- everything is loaded from the library */

#include "Python.h"

#ifdef __FreeBSD__
#include <floatingpoint.h>
#endif

extern void chef_fn_begin(const char *fn_name);
extern void chef_fn_end(const char *fn_name);
extern void chef_bb(int bb);


static long int fn_count = 0;
static long int bb_count = 0;
static int indent = 0;

void chef_fn_begin(const char *fn_name) {
    fn_count++;
    indent++;
}

void chef_fn_end(const char *fn_name) {
    indent--;
}

void chef_bb(int bb) {
    bb_count++;
}


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
