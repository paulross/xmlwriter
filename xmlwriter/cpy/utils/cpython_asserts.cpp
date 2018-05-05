//
//  cpython_asserts.cpp
//  xmlwriter
//
//  Created by Paul Ross on 04/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#include "cpython_asserts.h"

namespace CPythonCpp {

int cpython_asserts() {
    return Py_IsInitialized() && PyErr_Occurred() == NULL;
}

int cpython_asserts(PyObject *pobj) {
    return cpython_asserts() && pobj != NULL;
}

} // namespace CPythonCpp
