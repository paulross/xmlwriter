//
//  CPythonUtils.h
//  xmlwriter
//
//  Created by Paul Ross on 25/04/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#ifndef __xmlwriter__CPythonUtils__
#define __xmlwriter__CPythonUtils__

#include <Python.h>

#include <string>

#include "cpython_asserts.h"

namespace CPythonCpp {

/** Template conversion functions. */

template <typename P, typename C>
C convert_py_to_cpp(P *p_obj);

template<>
std::string
convert_py_to_cpp<PyBytesObject, std::string>(PyBytesObject *p_obj) {
    std::string r;
    assert(CPythonCpp::cpython_asserts((PyObject*)p_obj));
    if (PyBytes_Check(p_obj)) {
        r = std::string(PyBytes_AS_STRING(p_obj));
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument %s must be bytes not \"%s\"",
                     __FUNCTION__, Py_TYPE(p_obj)->tp_name);
    }
    return r;
}

template<>
std::string
convert_py_to_cpp<PyByteArrayObject, std::string>(PyByteArrayObject *p_obj) {
    std::string r;
    assert(CPythonCpp::cpython_asserts((PyObject*)p_obj));
    if (PyByteArray_Check(p_obj)) {
        r = std::string(PyByteArray_AS_STRING(p_obj));
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument %s must be bytearray not \"%s\"",
                     __FUNCTION__, Py_TYPE(p_obj)->tp_name);
    }
    return r;
}



template <typename C, typename P>
P *convert_cpp_to_py(const C &cpp_obj);

//template <>
//PyBytesObject *
//convert_cpp_to_py<std::string, PyBytesObject>(const std::string &cpp_obj);
//template <>
//PyByteArrayObject *
//convert_cpp_to_py<std::string, PyByteArrayObject>(const std::string &cpp_obj);

template <>
PyBytesObject *
convert_cpp_to_py<std::string, PyBytesObject>(const std::string &cpp_obj) {
    assert(CPythonCpp::cpython_asserts());
    return (PyBytesObject *)PyBytes_FromStringAndSize(cpp_obj.c_str(),
                                                      cpp_obj.size());
}

template <>
PyByteArrayObject *
convert_cpp_to_py<std::string, PyByteArrayObject>(const std::string &cpp_obj) {
    assert(CPythonCpp::cpython_asserts());
    return (PyByteArrayObject *)PyByteArray_FromStringAndSize(cpp_obj.c_str(),cpp_obj.size());
}

} // namespace CPythonCpp

#endif /* defined(__xmlwriter__CPythonUtils__) */
