//
//  DefaultArguments.h
//  xmlwriter
//
//  Created by Paul Ross on 04/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#ifndef xmlwriter_DefaultArguments_h
#define xmlwriter_DefaultArguments_h

#include <Python.h>

// Macros for default arguments using C code
#define PY_DEFAULT_ARGUMENT_INIT(name, value, ret) \
    PyObject *name = NULL; \
    static PyObject *default_##name = NULL; \
    if (! default_##name) { \
        default_##name = value; \
        if (! default_##name) { \
            PyErr_SetString(PyExc_RuntimeError, "Can not create default value for " #name); \
            return ret; \
        } \
    }

#define PY_DEFAULT_ARGUMENT_SET(name) \
    if (! name) { \
        name = default_##name; \
    } \
    Py_INCREF(name)

namespace CPythonCpp {

/** Class to simplify default arguments.
 *
 * Usage:
 *
 * static DefaultArg arg_0(PyLong_FromLong(1L));
 * static DefaultArg arg_1(PyUnicode_FromString("Default string."));
 * if (! arg_0 || ! arg_1) {
 *      return NULL;
 * }
 *
 * if (! PyArg_ParseTupleAndKeywords(args, kwargs, "...",
 const_cast<char**>(kwlist),
 &arg_0, &arg_1, ...)) {
 return NULL;
 }
 *
 * Then just use arg_0, arg_1 as if they were a PyObject* (possibly
 * might need to be cast to some specific PyObject*).
 *
 * WARN: This class is designed to be statically allocated. If allocated
 * on the heap or stack it will leak memory. That could be fixed by
 * implementing:
 *
 * ~DefaultArg() { Py_XDECREF(m_default); }
 *
 * But this will be highly dangerous when statically allocated as the
 * destructor will be invoked with the Python interpreter in an
 * uncertain state and will, most likely, segfault:
 * "Python(39158,0x7fff78b66310) malloc: *** error for object 0x100511300: pointer being freed was not allocated"
 */
class DefaultArg {
public:
    DefaultArg(PyObject *new_ref) : m_arg { NULL }, m_default { new_ref } {
#if XML_WRITE_DEBUG_TRACE
        fprintf(stdout, "DefaultArg at ");
        fprintf(stdout, "%p", m_default);
        fprintf(stdout, " value: ");
        PyObject_Print(m_default, stdout, 0);
        fprintf(stdout, "\n");
#endif
    }
    // Allow setting of the (optional) argument with
    // PyArg_ParseTupleAndKeywords
    PyObject **operator&() { m_arg = NULL; return &m_arg; }
    // Access the argument or the default if default.
    operator PyObject*() const {
        return m_arg ? m_arg : m_default;
    }
    // Test if constructed successfully from the new reference.
    explicit operator bool() { return m_default != NULL; }
protected:
    PyObject *m_arg;
    PyObject *m_default;
};

} // namespace CPythonCpp

#endif
