//
//  CPythonUtils.cpp
//  xmlwriter
//
//  Created by Paul Ross on 25/04/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#include "CPythonUtils.h"

int cpython_asserts() {
    return Py_IsInitialized() && PyErr_Occurred() == NULL;
}

int cpython_asserts(PyObject *pobj) {
    return cpython_asserts() && pobj != NULL;
}

template<>
std::string
convert_py_to_cpp<PyBytesObject, std::string>(PyBytesObject *p_obj) {
    std::string r;
    assert(cpython_asserts((PyObject*)p_obj));
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
    assert(cpython_asserts((PyObject*)p_obj));
    if (PyByteArray_Check(p_obj)) {
        r = std::string(PyByteArray_AS_STRING(p_obj));
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument %s must be bytearray not \"%s\"",
                     __FUNCTION__, Py_TYPE(p_obj)->tp_name);
    }
    return r;
}

template <>
PyBytesObject *
convert_cpp_to_py<std::string, PyBytesObject>(const std::string &cpp_obj) {
    assert(cpython_asserts());
    return (PyBytesObject *)PyBytes_FromStringAndSize(cpp_obj.c_str(),
                                                      cpp_obj.size());
}

template <>
PyByteArrayObject *
convert_cpp_to_py<std::string, PyByteArrayObject>(const std::string &cpp_obj) {
    assert(cpython_asserts());
    return (PyByteArrayObject *)PyByteArray_FromStringAndSize(cpp_obj.c_str(),cpp_obj.size());
}


/*** Converting  Python bytes and Unicode to and from std::string ***/
/* Convert a PyObject to a std::string and return 0 if succesful.
 * If py_str is Unicode than treat it as UTF-8.
 * This works with Python 2.7 and Python 3.4 onwards.
 */
std::string py_bytes_to_std_string(PyObject *py_str) {
    assert(cpython_asserts(py_str));
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

std::string py_bytearray_to_std_string(PyObject *py_str) {
    assert(cpython_asserts(py_str));
    if (PyByteArray_Check(py_str)) {
        return std::string(PyByteArray_AS_STRING(py_str));
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument %s must be bytearray not \"%s\"",
                     __FUNCTION__, Py_TYPE(py_str)->tp_name);
        return "";
    }
}

/* Convert a PyObject to a std::string.
 * If py_str is Unicode than treat it as UTF-8.
 * This works with Python 2.7 and Python 3.4 onwards.
 * On error the string will be empty and an error set.
 */
std::string py_utf8_to_std_string(PyObject *py_str) {
    assert(cpython_asserts(py_str));
    std::string r;
//    int threads_init = PyEval_ThreadsInitialized();
//    PyThreadState *thread_state = PyThreadState_Get();

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

PyObject *std_string_to_py_bytes(const std::string &str) {
    assert(cpython_asserts());
    return PyBytes_FromStringAndSize(str.c_str(), str.size());
}

PyObject *std_string_to_py_bytearray(const std::string &str) {
    assert(cpython_asserts());
    return PyByteArray_FromStringAndSize(str.c_str(), str.size());
}

PyObject *std_string_to_py_utf8(const std::string &str) {
    assert(cpython_asserts());
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
    assert(cpython_asserts(original));
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

/* Takes a dict[str : str] and returns a std::map<std::string, std::string>
 * On error this seta an error and returns an empty map;.
 */
std::map<std::string, std::string>
dict_to_map_str_str(PyObject *arg) {
    assert(cpython_asserts(arg));
    // Used for iterating across the dict
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    std::map<std::string, std::string> cpp_attrs;

    if (arg) {
        if (PyDict_Check(arg)) {
            while (PyDict_Next(arg, &pos, &key, &value)) {
                cpp_attrs[py_utf8_to_std_string(key)] = py_utf8_to_std_string(value);
                if (PyErr_Occurred()) {
                    cpp_attrs.clear();
                    break;
                }
            }
        } else {
            PyErr_Format(PyExc_TypeError,
                         "Argument \"attrs\" to %s must be dict not \"%s\"",
                         __FUNCTION__, Py_TYPE(arg)->tp_name);
        }
    }
    return cpp_attrs;
}

/* Takes a std::map<std::string, std::string> and returns a dict[str : str]
 * Always succeeds.
 */
PyObject *
map_str_str_to_dict(const std::map<std::string, std::string> &map) {
    assert(! PyErr_Occurred());
    PyObject *ret = PyDict_New();
    for (auto &iter: map) {
        PyDict_SetItem(ret,
           PyUnicode_FromStringAndSize(iter.first.c_str(), iter.first.size()),
           PyUnicode_FromStringAndSize(iter.second.c_str(), iter.second.size())
                       );
    }
    return ret;
}


/** END: Converting Python bytes and Unicode to and from std::string **/
