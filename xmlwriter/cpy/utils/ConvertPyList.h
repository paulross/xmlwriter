//
//  ConvertPyList.h
//  xmlwriter
//
//  Created by Paul Ross on 05/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#ifndef xmlwriter_ConvertPyList_h
#define xmlwriter_ConvertPyList_h

#include <Python.h>

#include <vector>

#include "cpython_asserts.h"

namespace CPythonCpp {

/** Convert a python list to a std::vector<T>
 * ConvertToT is a function pointer to a function that takes a PyObject*
 * and returns an instance of a T type. This function should make
 * PyErr_Occurred() non-NULL on failure to convert a PyObject* and return a
 * default T.
 *
 * On failure sets PyErr_Occurred() and the return value will be empty.
 */
template <typename T>
std::vector<T>
py_list_to_std_vector(PyObject *py_list, T (*ConvertToT)(PyObject *)) {
    assert(CPythonCpp::cpython_asserts(py_list));
    std::vector<T> cpp_vector;

    if (PyList_Check(py_list)) {
        cpp_vector.reserve(PyList_GET_SIZE(py_list));
        for (Py_ssize_t i = 0; i < PyList_GET_SIZE(py_list); ++i) {
            cpp_vector.emplace(cpp_vector.end(),
                               (*ConvertToT)(PyList_GetItem(py_list, i)));
            if (PyErr_Occurred()) {
                cpp_vector.clear();
                break;
            }
        }
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument \"py_list\" to %s must be list not \"%s\"",
                     __FUNCTION__, Py_TYPE(py_list)->tp_name);
    }
    return cpp_vector;
}

template <typename T>
std::vector<T>
py_list_to_std_vector(PyObject *py_list,
                      std::unique_ptr<T> (*ConvertToT)(PyObject *)) {
    assert(CPythonCpp::cpython_asserts(py_list));
    std::vector<T> cpp_vector;

    if (PyList_Check(py_list)) {
        cpp_vector.reserve(PyList_GET_SIZE(py_list));
        for (Py_ssize_t i = 0; i < PyList_GET_SIZE(py_list); ++i) {
            std::unique_ptr<T> item = (*ConvertToT)(PyList_GetItem(py_list, i));
            if (item) {
                cpp_vector.emplace(cpp_vector.end(), *item.release());
            } else {
                assert(PyErr_Occurred());
                cpp_vector.clear();
                break;
            }
        }
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument \"py_list\" to %s must be list not \"%s\"",
                     __FUNCTION__, Py_TYPE(py_list)->tp_name);
    }
    return cpp_vector;
}

/** Convert a std::vector<T> to a new python list.
 * ConvertToPy is a pointer to a function that takes an instance of a T type
 * and returns a PyObject*, this should return NULL on failure.
 *
 * Returns a new reference on success, NULL on failure.
 */
template <typename T>
PyObject*
std_vector_to_py_list(const std::vector<T> &cpp_vec,
                      PyObject *(*ConvertToPy)(const T&)
                      ) {
    assert(CPythonCpp::cpython_asserts());
    PyObject *r = PyList_New(cpp_vec.size());
    if (! r) {
        goto except;
    }
    for (Py_ssize_t i = 0; i < cpp_vec.size(); ++i) {
        PyObject *item = (*ConvertToPy)(cpp_vec[i]);
        if (! item || PyErr_Occurred() || PyList_SetItem(r, i, item)) {
            goto except;
        }
    }
    assert(! PyErr_Occurred());
    assert(r);
    goto finally;
except:
    assert(PyErr_Occurred());
    // Clean up list
    if (r) {
        // No PyList_Clear().
        for (Py_ssize_t i = 0; i < PyList_GET_SIZE(r); ++i) {
            Py_XDECREF(PyList_GET_ITEM(r, i));
        }
        Py_DECREF(r);
        r = NULL;
    }
finally:
    return r;
}

} // namespace CPythonCpp

#endif
