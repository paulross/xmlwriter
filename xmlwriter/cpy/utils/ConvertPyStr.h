//
//  ConvertPyStr.h
//  xmlwriter
//
//  Created by Paul Ross on 05/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#ifndef xmlwriter_ConvertPyStr_h
#define xmlwriter_ConvertPyStr_h

#include <Python.h>

#include <memory>
#include <string>

namespace CPythonCpp {

/*** Converting  Python bytes and Unicode to and from std::string ***/
// These all make PyErr_Occurred() true on failure
std::string
py_utf8_to_std_string(PyObject *py_str);

PyObject *
std_string_to_py_utf8(const std::string &str);

std::unique_ptr<std::string>
py_utf8_to_up_std_string(PyObject *py_str);

/** The mirror of py_str_to_string()
 * This expects an original object of bytes, bytearray or str or
 * sub-type and and converts a std::string into a PyObject* of the same type.
 */
PyObject *
std_string_to_py_object(PyObject *original, const std::string &result);

} // namespace CPythonCpp

#endif
