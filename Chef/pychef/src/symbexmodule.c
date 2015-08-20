/*
 * Copyright (C) 2014 EPFL.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <Python.h>
#include <frameobject.h>

#include "symbexutils.h"
#include "s2e.h"

#include <stdint.h>
#include <stdlib.h>

#define DEFAULT_MIN_SEQ_SIZE        0
#define DEFAULT_MAX_SEQ_SIZE     (-1)

#define DEFAULT_MIN_INT_VALUE   (-128)
#define DEFAULT_MAX_INT_VALUE     127


enum {
    S2E_CHEF_CALIBRATE_START = 0x1000,
    S2E_CHEF_CALIBRATE_END = 0x1001,
    S2E_CHEF_CALIBRATE_CHECKPOINT = 0x1002
};


/*== Globals =================================================================*/

static PyObject *SymbexError;


/*== Trace handler ===========================================================*/

static int trace_func(PyObject *obj, PyFrameObject *frame, int what,
        PyObject *arg) {
    hl_frame_t chef_frame = {0};

    chef_frame.function = (uintptr_t)PyString_AS_STRING(frame->f_code->co_code);
    chef_frame.last_inst = frame->f_lasti;
    chef_frame.line_no = frame->f_lineno;

    // XXX: Properly handle the C calls
    if (what == PyTrace_CALL || what == PyTrace_C_CALL) {
        chef_frame.line_no = frame->f_code->co_firstlineno;
        chef_frame.fn_name = (uintptr_t)PyString_AS_STRING(frame->f_code->co_name);
        chef_frame.file_name = (uintptr_t)PyString_AS_STRING(frame->f_code->co_filename);
    }

    __chef_hl_trace((hl_trace_reason)(what), &chef_frame, 1);
    return 0;
}


static int count_frames(PyFrameObject *head) {
    int frame_count = 0;
    for (; head != NULL; head = head->f_back, ++frame_count);
    return frame_count;
}


static void trace_init(PyFrameObject *frame) {
    int frame_count = count_frames(frame);

    hl_frame_t *call_stack = (hl_frame_t*)PyMem_Malloc(frame_count*sizeof(hl_frame_t));
    hl_frame_t *chef_frame = call_stack;

    while (frame != NULL) {
        chef_frame->function = (uintptr_t)PyString_AS_STRING(frame->f_code->co_code);
        chef_frame->last_inst = frame->f_lasti;
        chef_frame->line_no = frame->f_lineno;

        chef_frame->fn_name = (uintptr_t)PyString_AS_STRING(frame->f_code->co_name);
        chef_frame->file_name = (uintptr_t)PyString_AS_STRING(frame->f_code->co_filename);

        frame = frame->f_back;
        chef_frame++;
    }

    __chef_hl_trace(CHEF_TRACE_INIT, call_stack, frame_count);

    PyMem_Free(call_stack);
}


/*== High-level functions ====================================================*/

PyDoc_STRVAR(symbex_symsequence_doc,
"symsequence(size, name) --> string object\n\
\n\
Mark the given sequence or collection as symbolic. Supported object types: \n\
str, unicode, list, dict, tuple.");

static PyObject *
symbex_symsequence(PyObject *self, PyObject *args) {
	PyObject *target = NULL;
	const char *name;
	int max_size = DEFAULT_MAX_SEQ_SIZE;
	int min_size = DEFAULT_MIN_SEQ_SIZE;

	if (!PyArg_ParseTuple(args, "Os|ii:symsequence", &target, &name,
			&max_size, &min_size)) {
		return NULL;
	}

	return makeConcolicSequence(target, name, max_size, min_size);
}


/*----------------------------------------------------------------------------*/

PyDoc_STRVAR(symbex_symint_doc,
"symint() -> int object\n\
\n\
Mark the given integer object as symbolic. Supported object types: int.");

static PyObject *
symbex_symint(PyObject *self, PyObject *args) {
	PyObject *target = NULL;
	const char *name;
	long max_value = DEFAULT_MAX_INT_VALUE;
	long min_value = DEFAULT_MIN_INT_VALUE;

	if (!PyArg_ParseTuple(args, "Os|ll:symint", &target, &name, &max_value,
			&min_value)) {
		return NULL;
	}

	return makeConcolicInt(target, name, max_value, min_value);
}


/*----------------------------------------------------------------------------*/

PyDoc_STRVAR(symbex_symtoconcrete_doc,
"symtoconcrete(obj) -> object\n\
\n\
Return a concretized version of the symbolic object passed as a parameter.");

static PyObject *
symbex_symtoconcrete(PyObject *self, PyObject *args) {
  const char *string;
  if (!PyArg_ParseTuple(args, "s:symtoconcrete", &string))
    return NULL;

  return PyString_FromString(concretizeString(string));
}


/*----------------------------------------------------------------------------*/

PyDoc_STRVAR(symbex_concrete_doc,
"concrete(obj) -> object\n\
\n\
Concretize a symbolic object");

static PyObject *
symbex_concrete(PyObject *self, PyObject *args) {
	PyObject *target;

	if (!PyArg_ParseTuple(args, "O:concrete", &target)) {
		return NULL;
	}

	if (!s2e_version()) {
		Py_INCREF(target);
		return target;
	}

	if (PyInt_Check(target)) {
		PyIntObject *int_target = (PyIntObject*)target;
		long value = int_target->ob_ival;
		s2e_get_example(&value, sizeof(value));

		return PyInt_FromLong(value);
	} else {
		Py_INCREF(target);
		return target;
	}
}


/*----------------------------------------------------------------------------*/

PyDoc_STRVAR(symbex_killstate_doc,
"killstate(status, message)\n\
\n\
Terminate the current execution state.");

static PyObject *
symbex_killstate(PyObject *self, PyObject *args) {
  int status;
  const char *message;

  if (!PyArg_ParseTuple(args, "is:killstate", &status, &message))
    return NULL;

  killState(status, message);

  Py_RETURN_NONE;
}

/*----------------------------------------------------------------------------*/

PyDoc_STRVAR(symbex_assume_doc,
"assume(cond) \n\
\n\
Assume that the condition passed as argument is true.");


static PyObject *
symbex_assume(PyObject *self, PyObject *args) {
	unsigned char condition;

	if (!PyArg_ParseTuple(args, "b:assume", &condition)) {
		return NULL;
	}

	s2e_assume(condition);

	Py_RETURN_NONE;
}

/*----------------------------------------------------------------------------*/

PyDoc_STRVAR(symbex_assumeascii_doc,
"assumeascii(str) \n\
\n\
Assume that the string passed as argument doesn't contain characters beyond 0x7F");

static PyObject *
symbex_assumeascii(PyObject *self, PyObject *args) {
	PyStringObject *string_obj;
	Py_ssize_t i;

	if (!PyArg_ParseTuple(args, "O!:assumeascii", &PyString_Type, &string_obj)) {
		return NULL;
	}

	for (i = 0; i < Py_SIZE(string_obj); ++i) {
		s2e_assume(((unsigned char)string_obj->ob_sval[i]) < 0x80);
	}

	Py_RETURN_NONE;
}

/*----------------------------------------------------------------------------*/

PyDoc_STRVAR(symbex_symcall_doc,
"symcall(plugin, id, value) \n\
\n\
Invoke an S2E system call");

static PyObject *
symbex_symcall(PyObject *self, PyObject *args) {
    const char *plugin;
    uint32_t id;
    const char *data;
    Py_ssize_t data_size;

    if (!PyArg_ParseTuple(args, "sIs#:symcall", &plugin, &id, &data,
            &data_size)) {
        return NULL;
    }

    if (s2e_plugin_call(plugin, id, (void*)data, data_size) != 0) {
        PyErr_SetString(SymbexError, "Could not invoke syscall");
        return NULL;
    }

    Py_RETURN_NONE;
}

/*----------------------------------------------------------------------------*/

PyDoc_STRVAR(symbex_calibrate_doc,
"calibrate(end) \n\
\n\
Run a calibration probe to detect interpreter structure");

static PyObject *
symbex_calibrate(PyObject *self, PyObject *args) {
    unsigned op;
    unsigned count = 1;

    if (!PyArg_ParseTuple(args, "I|I:calibrate", &op, &count)) {
        return NULL;
    }

    switch (op) {
    case 0:
        s2e_system_call(S2E_CHEF_CALIBRATE_START, NULL, 0);
        break;
    case 1:
        s2e_system_call(S2E_CHEF_CALIBRATE_CHECKPOINT, NULL, count);
        break;
    case 2:
        s2e_system_call(S2E_CHEF_CALIBRATE_END, NULL, 0);
        break;
    default:
        PyErr_SetString(PyExc_ValueError, "Invalid calibration operation");
        return NULL;
    }

    Py_RETURN_NONE;
}

/*== Module Definition =======================================================*/

PyDoc_STRVAR(module_doc,
"Primitives for supporting symbolic execution inside the Python interpreter.");


static PyMethodDef SymbexMethods[] = {
	{ "symsequence", symbex_symsequence, METH_VARARGS, symbex_symsequence_doc },
	{ "symint", symbex_symint, METH_VARARGS, symbex_symint_doc },
	{ "symtoconcrete", symbex_symtoconcrete, METH_VARARGS,
			symbex_symtoconcrete_doc },
	{ "concrete", symbex_concrete, METH_VARARGS, symbex_concrete_doc },
	{ "killstate", symbex_killstate, METH_VARARGS, symbex_killstate_doc },
	{ "symcall", symbex_symcall, METH_VARARGS, symbex_symcall_doc },
	{ "assume", symbex_assume, METH_VARARGS, symbex_assume_doc },
	{ "assumeascii", symbex_assumeascii, METH_VARARGS, symbex_assumeascii_doc },
	{ "calibrate", symbex_calibrate, METH_VARARGS, symbex_calibrate_doc },
	{ NULL, NULL, 0, NULL } /* Sentinel */
};


PyMODINIT_FUNC
initsymbex(void) {
	PyObject *m;

	m = Py_InitModule3("symbex", SymbexMethods, module_doc);
	if (m == NULL)
	  return;

	if (SymbexError == NULL) {
		SymbexError = PyErr_NewException((char*)"symbex.SymbexError", NULL, NULL);
		if (SymbexError == NULL)
			return;
	}
	Py_INCREF(SymbexError);
	PyModule_AddObject(m, "SymbexError", SymbexError);
}
