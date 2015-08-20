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

#ifndef SYMBEXUTILS_H_
#define SYMBEXUTILS_H_

/* The maximum size of a symbolic dict or tuple */
#define MAX_SYMBOLIC_SIZE 64


#include <Python.h>

/*
 * Obtain a concrete value for a possibly symbolic pointer value `p'.
 */
void *concretizePointer(const void *p);

/*
 * Obtain a concrete string out of a possibly symbolic string `s'.
 */
const char *concretizeString(const char *s);

/*
 * Construct an unconstrained symbolic Python string object of size `size'
 * and symbolic name `name'.
 */
PyObject *makeSymbolicString(unsigned int size, const char *name);

/*
 * Terminate the current execution state with status code `status' and
 * message `message'.
 */
void killState(int status, const char *message);

PyObject *makeConcolicInt(PyObject *target, const char *name,
        long max_value, long min_value);
PyObject *makeConcolicSequence(PyObject *target, const char *name,
        int max_size, int min_size);
PyObject *makeConcolicString(PyObject *target,
        const char *name, int max_size, int min_size);
PyObject *makeConcolicUnicode(PyObject *target,
        const char *name, int max_size, int min_size);
PyObject *makeConcolicList(PyObject *target,
        const char *name, int max_size, int min_size);
PyObject *makeConcolicDict(PyObject *target,
        const char *name);
PyObject *makeConcolicTuple(PyObject *target,
        const char *name);


#endif /* SYMBEXUTILS_H_ */
