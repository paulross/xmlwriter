//
//  ConvertPyBytes.cpp
//  xmlwriter
//
//  Created by Paul Ross on 05/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#include "ConvertPyBytes.h"

#include "cpython_asserts.h"

namespace CPythonCpp {

/* Convert a PyObject to a std::string and return 0 if succesful.
 * If py_str is Unicode than treat it as UTF-8.
 * This works with Python 2.7 and Python 3.4 onwards.
 */
std::string py_bytes_to_std_string(PyObject *py_str) {
    assert(CPythonCpp::cpython_asserts(py_str));
    std::string r;
    if (PyBytes_Check(py_str)) {
        r = std::string(PyBytes_AS_STRING(py_str));
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument %s must be bytes not \"%s\"",
                     __FUNCTION__, Py_TYPE(py_str)->tp_name);
    }
    return r;
}


std::unique_ptr<std::string>
py_bytes_to_up_std_string(PyObject *py_str) {
    assert(CPythonCpp::cpython_asserts(py_str));
    std::unique_ptr<std::string> r;
    if (PyBytes_Check(py_str)) {
        r.reset(new std::string(PyBytes_AS_STRING(py_str)));
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument %s must be bytes not \"%s\"",
                     __FUNCTION__, Py_TYPE(py_str)->tp_name);
    }
    return r;
}

PyObject *std_string_to_py_bytes(const std::string &str) {
    assert(CPythonCpp::cpython_asserts());
    return PyBytes_FromStringAndSize(str.c_str(), str.size());
}

} // namespace CPythonCpp
