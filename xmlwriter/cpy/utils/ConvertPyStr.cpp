//
//  ConvertPyStr.cpp
//  xmlwriter
//
//  Created by Paul Ross on 05/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#include "ConvertPyStr.h"
#include "ConvertPyBytearray.h"
#include "ConvertPyBytes.h"
#include "cpython_asserts.h"

namespace CPythonCpp {

/* Convert a PyObject to a std::string.
 * If py_str is Unicode than treat it as UTF-8.
 * This works with Python 2.7 and Python 3.4 onwards.
 * On error the string will be empty and an error set.
 */
std::string py_utf8_to_std_string(PyObject *py_str) {
    assert(CPythonCpp::cpython_asserts(py_str));
    std::string r;

    if (PyUnicode_Check(py_str)) {
        if (! PyUnicode_READY(py_str)) {
            if (PyUnicode_KIND(py_str) == PyUnicode_1BYTE_KIND) {
                // Python 3 and its minor versions (they vary)
                //    const Py_UCS1 *pChrs = PyUnicode_1BYTE_DATA(pyStr);
                //    result = std::string(reinterpret_cast<const char*>(pChrs));
#if PY_MAJOR_VERSION >= 3
                r = std::string((char*)PyUnicode_1BYTE_DATA(py_str));
#else
                // Nasty cast away constness because PyString_AsString takes non-const in Py2
                r = std::string((char*)PyString_AsString(const_cast<PyObject *>(py_str)));
#endif
            } else {
                PyErr_Format(PyExc_ValueError,
                             "In %s \"py_str\" not utf-8",
                             __FUNCTION__);
            }
        } else {
            PyErr_Format(PyExc_ValueError,
                         "In %s \"py_str\" failed PyUnicode_READY()",
                         __FUNCTION__);
        }
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument %s must be unicode string not \"%s\"",
                     __FUNCTION__, Py_TYPE(py_str)->tp_name);
    }
    return r;
}

std::unique_ptr<std::string>
py_utf8_to_up_std_string(PyObject *py_str) {
    assert(CPythonCpp::cpython_asserts(py_str));
    std::unique_ptr<std::string> r;

    if (PyUnicode_Check(py_str)) {
        if (! PyUnicode_READY(py_str)) {
            if (PyUnicode_KIND(py_str) == PyUnicode_1BYTE_KIND) {
                // Python 3 and its minor versions (they vary)
                //    const Py_UCS1 *pChrs = PyUnicode_1BYTE_DATA(pyStr);
                //    result = std::string(reinterpret_cast<const char*>(pChrs));
#if PY_MAJOR_VERSION >= 3
                r.reset(new std::string((char*)PyUnicode_1BYTE_DATA(py_str)));
#else
                // Nasty cast away constness because PyString_AsString takes non-const in Py2
                r.reset(new std::string((char*)PyString_AsString(const_cast<PyObject *>(py_str))));
#endif
            } else {
                PyErr_Format(PyExc_ValueError,
                             "In %s \"py_str\" not utf-8",
                             __FUNCTION__);
            }
        } else {
            PyErr_Format(PyExc_ValueError,
                         "In %s \"py_str\" failed PyUnicode_READY()",
                         __FUNCTION__);
        }
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument %s must be unicode string not \"%s\"",
                     __FUNCTION__, Py_TYPE(py_str)->tp_name);
    }
    return r;
}

PyObject *std_string_to_py_utf8(const std::string &str) {
    assert(CPythonCpp::cpython_asserts());
    // Equivelent to:
    // PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND, str.c_str(), str.size());
    return PyUnicode_FromStringAndSize(str.c_str(), str.size());
}

/** The mirror of py_str_to_string()
 * This expects an original object of bytes, bytearray or str or
 * sub-type and and converts a std::string into the same type.
 */
PyObject *
std_string_to_py_object(PyObject *original, const std::string &result) {
    assert(CPythonCpp::cpython_asserts(original));
    PyObject *ret = NULL;
    if (PyBytes_Check(original)) {
        ret = std_string_to_py_bytes(result);
    } else if (PyByteArray_Check(original)) {
        ret = std_string_to_py_bytearray(result);
    } else if (PyUnicode_Check(original)) {
        ret = std_string_to_py_utf8(result);
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument %s must be str, bytes or bytearray not \"%s\"",
                     __FUNCTION__, Py_TYPE(original)->tp_name);
    }
    return ret;
}

} // namespace CPythonCpp {
