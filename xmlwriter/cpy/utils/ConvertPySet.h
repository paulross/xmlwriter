//
//  ConvertPySet.h
//  xmlwriter
//
//  Created by Paul Ross on 05/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#ifndef xmlwriter_ConvertPySet_h
#define xmlwriter_ConvertPySet_h

#include <Python.h>

#include <unordered_set>

#include "cpython_asserts.h"

namespace CPythonCpp {

/** Convert a python set or frozenset to a std::unordered_set<T>
 * ConvertToT is a function pointer to a function that takes a PyObject*
 * and returns an instance of a T type. This function should make
 * PyErr_Occurred() true on failure to convert a PyObject* and return a
 * default T.
 *
 * On failure sets PyErr_Occurred() and the return value will be empty.
 */
template <typename T>
std::unordered_set<T>
py_set_to_std_unordered_set(PyObject *py_set, T (*ConvertToT)(PyObject *)) {
    assert(CPythonCpp::cpython_asserts(py_set));
    std::unordered_set<T> cpp_set;

    if (PySet_Check(py_set) || PyFrozenSet_Check(py_set)) {
        // The C API does not allow direct access to an item in a set so we
        // make a copy and pop from that.
        PyObject *set_copy = PySet_New(py_set);
        if (set_copy) {
            while (PySet_GET_SIZE(set_copy)) {
                PyObject *item = PySet_Pop(set_copy);
                if (! item || PyErr_Occurred()) {
                    PySet_Clear(set_copy);
                    cpp_set.clear();
                    break;
                }
                cpp_set.emplace((*ConvertToT)(item));
                Py_DECREF(item);
            }
            Py_DECREF(set_copy);
        } else {
            assert(PyErr_Occurred());
        }
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Argument \"py_set\" to %s must be set or frozenset not \"%s\"",
                     __FUNCTION__, Py_TYPE(py_set)->tp_name);
    }
    return cpp_set;
}

/** Convert a std::unordered_set<T> to a new python set or frozenset.
 * ConvertToPy is a pointer to a function that takes an instance of a T type
 * and returns a PyObject*, this should return NULL on failure.
 *
 * Returns a new reference on success, NULL on failure.
 */
template <typename T>
PyObject*
std_unordered_set_to_py_set(const std::unordered_set<T> &cpp_set,
                            PyObject *(*ConvertToPy)(const T&),
                            bool is_frozen=false) {
    assert(CPythonCpp::cpython_asserts());
    PyObject *r = NULL;
    if (is_frozen) {
        r = PyFrozenSet_New(NULL);
    } else {
        r = PySet_New(NULL);
    }
    if (! r) {
        goto except;
    }
    for (auto &iter: cpp_set) {
        PyObject *item = (*ConvertToPy)(iter);
        if (! item || PyErr_Occurred() || PySet_Add(r, item)) {
            goto except;
        }
    }
    assert(! PyErr_Occurred());
    assert(r);
    goto finally;
except:
    assert(PyErr_Occurred());
    // Clean up set
    if (r) {
        PySet_Clear(r);
        Py_DECREF(r);
        r = NULL;
    }
finally:
    return r;
}

} //namespace CPythonCpp

#endif
