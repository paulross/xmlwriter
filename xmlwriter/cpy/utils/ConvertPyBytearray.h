//
//  ConvertPyBytearray.h
//  xmlwriter
//
//  Created by Paul Ross on 05/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#ifndef __xmlwriter__ConvertPyBytearray__
#define __xmlwriter__ConvertPyBytearray__

#include <Python.h>

#include <memory>
#include <string>

namespace CPythonCpp {

// These all make PyErr_Occurred() true on failure
std::string
py_bytearray_to_std_string(PyObject *py_str);

PyObject *
std_string_to_py_bytearray(const std::string &str);

std::unique_ptr<std::string>
py_bytearray_to_up_std_string(PyObject *py_str);

} // namespace CPythonCpp

#endif /* defined(__xmlwriter__ConvertPyBytearray__) */
