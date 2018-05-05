//
//  cpython_asserts.h
//  xmlwriter
//
//  Created by Paul Ross on 04/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#ifndef __xmlwriter__cpython_asserts__
#define __xmlwriter__cpython_asserts__

#include <Python.h>

namespace CPythonCpp {

/* Returns non zero if Python is initialised and there is no Python error set.
 * The second version also checks that the given pointer is non-NULL
 * Use this thus, it will do nothing if NDEBUG is defined:
 * assert(cpython_asserts());
 * assert(cpython_asserts(p));
 */
int cpython_asserts();
int cpython_asserts(PyObject *pobj);

} // namespace CPythonCpp

#endif /* defined(__xmlwriter__cpython_asserts__) */
