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

#include "symbexutils.h"

#include "s2e.h"


/*
 * Mark the buffer `buf' of size `size' as concolic (i.e., the current value
 * of the buffer is preserved).  The name of the symbolic data is obtained
 * by concatenating the `base_name' with `name', separated by a '.' character.
 *
 * The `name' string has the format: <T>#<name>, where T is a format character
 * identifying the Python type used to reconstruct the value when returned
 * by the symbolic execution engine.  Currently supported values: i (int),
 * s (Regular string), u (Unicode string), b (bytearray), l (Python size).
 * 'b' is the default format if the `name' string doesn't obey the format.
 */
static void makeConcolicBuffer(void *buf, int size,
        const char *base_name, const char *name, const char type) {
    static char obj_name[256];
    snprintf(obj_name, 256, "%s.%c#%s", base_name, type, name);

    s2e_make_concolic(buf, size, obj_name);
}

static int checkObjectSize(Py_ssize_t size, int max_size,
        int min_size) {
    assert(min_size >= 0);

    if (max_size < 0) {
        return 0; // Fixed-size objects are OK
    } else if (max_size == 0) {
        return (size >= min_size) ? 0 : (-1);
    } else {
        return (size >= min_size && size <= max_size) ? 0 : (-1);
    }
}

static void constrainObjectSize(Py_ssize_t size, int max_size,
        int min_size) {
    assert(min_size >= 0);

    if (max_size > 0) {
        s2e_assume(size <= max_size);
    }
    s2e_assume(size >= min_size);
}


void *Sym_ConcretizePtr(const void *p) {
  if (s2e_version()) {
      s2e_concretize((void*)&p, sizeof(p));
      return (void *)p;
  } else {
      return (void *)p;
  }
}


const char *Sym_ConcretizeString(const char *s) {
  char *sc = (char*)Sym_ConcretizePtr(s);
  unsigned i;

  for (i = 0; ; ++i) {
    char c = *sc;
    // String ends at powers of two
    if (!(i&(i-1))) {
      if (!c) {
        *sc++ = 0;
        break;
        // TODO: See if we should make explicit path separators at this point,
        // since they may occur frequently. However, premature path splitting
        // sounds like redundant work, so we may decide to skip...
      } /* else if (c=='/') {
        *sc++ = '/';
      } */
    } else {
      char cc = c;
      if (s2e_version()) {
          s2e_get_example(&cc, sizeof(cc));
      }

      // TODO: See if we need to force the concretization in the PC
      // klee_assume(cc == c);
      *sc++ = cc;
      if (!cc) break;
    }
  }

  return s;
}


PyObject *Sym_MakeSymbolicString(unsigned int size, const char *name) {
    char *sym_data = (char *)PyMem_Malloc(size + 1);

    if (!sym_data) {
        return PyErr_NoMemory();
    }

    if (s2e_version()) {
        s2e_make_symbolic((void*)sym_data, size + 1, name);
    } else {
        memset(sym_data, 'X', size);
    }

    sym_data[size] = 0;

    PyObject *result = PyString_FromString(sym_data);
    PyMem_Free(sym_data);

    return result;
}


void Sym_KillState(int status, const char *message) {
    if (s2e_version()) {
        s2e_kill_state(status, message);
    } else {
        Py_Exit(status);
    }
}


PyObject *Sym_MakeConcolicInt(PyObject *target, const char *name,
        long max_value, long min_value) {
    assert(PyInt_Check(target));

    if (!s2e_version()) {
        PyErr_SetString(PyExc_RuntimeError, "Not in symbolic mode");
        return NULL;
    }

    PyIntObject *int_target = (PyIntObject*)target;
    long value = int_target->ob_ival;

    if (max_value >= min_value && (value < min_value || value > max_value)) {
        PyErr_SetString(PyExc_ValueError, "Incompatible value constraints");
        return NULL;
    }

    makeConcolicBuffer(&value, sizeof(value), name, "value", 'i');
    if (max_value >= min_value) {
        s2e_assume(value >= min_value);
        s2e_assume(value <= max_value);
    }

    return PyInt_FromLong(value);
}


PyObject *Sym_MakeConcolicSequence(PyObject *target, const char *name,
        int max_size, int min_size) {
    if (!s2e_version()) {
        PyErr_SetString(PyExc_RuntimeError, "Not in symbolic mode");
        return NULL;
    }

    if (min_size < 0) {
        PyErr_SetString(PyExc_ValueError, "Minimum size cannot be negative");
        return NULL;
    }

    if (target == Py_None) {
        PyErr_SetString(PyExc_ValueError, "Cannot make symbolic None");
        return NULL;
    } else if (PyString_Check(target)) {
        return Sym_MakeConcolicString(target, name, max_size, min_size);
    } else if (PyUnicode_Check(target)) {
        return Sym_MakeConcolicUnicode(target, name, max_size, min_size);
    } else if (PyList_Check(target)) {
        return Sym_MakeConcolicList(target, name, max_size, min_size);
    } else if (PyDict_Check(target)) {
        return Sym_MakeConcolicDict(target, name);
    } else if (PyTuple_Check(target)) {
        return Sym_MakeConcolicTuple(target, name);
    } else {
        PyErr_SetString(PyExc_TypeError, "Unsupported type");
        return NULL;
    }
}


PyObject *Sym_MakeConcolicString(PyObject *target,
        const char *name, int max_size, int min_size) {
    assert(PyString_Check(target));

    PyStringObject *str_target = (PyStringObject*)target;

    if (checkObjectSize(str_target->ob_size, max_size, min_size) < 0) {
        PyErr_SetString(PyExc_ValueError, "Incompatible size constraints");
        return NULL;
    }

    char *str_data = (char *)PyMem_Malloc(str_target->ob_size);
    if (!str_data) {
        return PyErr_NoMemory();
    }
    memcpy(str_data, str_target->ob_sval, str_target->ob_size);
    makeConcolicBuffer(str_data, str_target->ob_size, name, "value", 's');

    PyObject *result = PyString_FromStringAndSize(str_data, str_target->ob_size);
    if (result == NULL) {
        PyMem_Free(str_data);
        return NULL;
    }

    if (max_size >= 0) {
        PyStringObject *str_result = (PyStringObject*)result;
        makeConcolicBuffer(&str_result->ob_size, sizeof(str_result->ob_size),
                name, "size", 'l');
        constrainObjectSize(str_result->ob_size, max_size, min_size);
    }

    PyMem_Free(str_data);
    return result;
}


PyObject *Sym_MakeConcolicUnicode(PyObject *target,
        const char *name, int max_size, int min_size) {
    assert(PyUnicode_Check(target));

    PyUnicodeObject *uni_target = (PyUnicodeObject*)target;

    if (checkObjectSize(uni_target->length, max_size, min_size) < 0) {
        PyErr_SetString(PyExc_ValueError, "Incompatible size constraints");
        return NULL;
    }

    Py_ssize_t buf_size = uni_target->length * sizeof(Py_UNICODE);

    Py_UNICODE *uni_data = (Py_UNICODE *)PyMem_Malloc(buf_size);
    if (!uni_data) {
        return PyErr_NoMemory();
    }
    memcpy(uni_data, uni_target->str, buf_size);
    makeConcolicBuffer(uni_data, buf_size, name, "value", 'u');

    PyObject *result = PyUnicode_FromUnicode(uni_data, uni_target->length);
    if (result == NULL) {
        PyMem_Free(uni_data);
        return NULL;
    }

    if (max_size >= 0) {
        PyUnicodeObject *uni_result = (PyUnicodeObject*)result;
        makeConcolicBuffer(&uni_result->length, sizeof(uni_result->length),
                name, "size", 'l');
        constrainObjectSize(uni_result->length, max_size, min_size);
    }

    PyMem_Free(uni_data);
    return result;
}


PyObject *Sym_MakeConcolicList(PyObject *target,
        const char *name, int max_size, int min_size) {
    assert(PyList_Check(target));

    PyListObject *list_target = (PyListObject*)target;
    if (checkObjectSize(list_target->ob_size, max_size, min_size) < 0) {
        PyErr_SetString(PyExc_ValueError, "Incompatible size constraints");
        return NULL;
    }

    if (max_size >= 0) {
        makeConcolicBuffer(&list_target->ob_size, sizeof(list_target->ob_size),
                name, "size", 'l');
        constrainObjectSize(list_target->ob_size, max_size, min_size);
    }

    Py_INCREF(target);
    return target;
}


PyObject *Sym_MakeConcolicDict(PyObject *target,
        const char *name) {
    assert(PyDict_Check(target));

    PyDictObject *dict_target = (PyDictObject*)target;
    makeConcolicBuffer(&dict_target->ma_used, sizeof(dict_target->ma_used),
            name, "size", 'l');
    s2e_assume(dict_target->ma_used >= 0);
    s2e_assume(dict_target->ma_used < MAX_SYMBOLIC_SIZE);

    Py_INCREF(target);
    return target;
}


PyObject *Sym_MakeConcolicTuple(PyObject *target,
        const char *name) {
    assert(PyTuple_Check(target));

    PyTupleObject *tup_target = (PyTupleObject*)target;
    makeConcolicBuffer(&tup_target->ob_size, sizeof(tup_target->ob_size),
            name, "size", 'l');
    s2e_assume(tup_target->ob_size >= 0);
    s2e_assume(tup_target->ob_size < MAX_SYMBOLIC_SIZE);

    Py_INCREF(target);
    return target;
}
