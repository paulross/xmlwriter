//
//  ConvertPyBytearray.cpp
//  xmlwriter
//
//  Created by Paul Ross on 05/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#include "ConvertPyBytearray.h"

#include "cpython_asserts.h"

namespace CPythonCpp {

/*** Converting  Python bytes and Unicode to and from std::string ***/
std::string py_bytearray_to_std_string(PyObject *py_str) {
    assert(CPythonCpp::cpython_asserts(py_str));
    if (PyByteArray_Check(py_str)) {
        return std::string(PyByteArray_AS_STRING(py_str));
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument %s must be bytearray not \"%s\"",
                     __FUNCTION__, Py_TYPE(py_str)->tp_name);
        return "";
    }
}

std::unique_ptr<std::string>
py_bytearray_to_up_std_string(PyObject *py_str) {
    assert(CPythonCpp::cpython_asserts(py_str));
    std::unique_ptr<std::string> r;
    if (PyByteArray_Check(py_str)) {
        r.reset(new std::string(PyByteArray_AS_STRING(py_str)));
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument %s must be bytearray not \"%s\"",
                     __FUNCTION__, Py_TYPE(py_str)->tp_name);
    }
    return r;
}

PyObject *std_string_to_py_bytearray(const std::string &str) {
    assert(CPythonCpp::cpython_asserts());
    return PyByteArray_FromStringAndSize(str.c_str(), str.size());
}

} // namespace CPythonCpp
