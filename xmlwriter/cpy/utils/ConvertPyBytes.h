//
//  ConvertPyBytes.h
//  xmlwriter
//
//  Created by Paul Ross on 05/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#ifndef __xmlwriter__ConvertPyBytes__
#define __xmlwriter__ConvertPyBytes__

#include <Python.h>

#include <memory>
#include <string>

namespace CPythonCpp {

PyObject *std_string_to_py_bytes(const std::string &str);
std::string py_bytes_to_std_string(PyObject *py_str);
std::unique_ptr<std::string> py_bytes_to_up_std_string(PyObject *py_str);

} // namespace CPythonCpp

#endif /* defined(__xmlwriter__ConvertPyBytes__) */
