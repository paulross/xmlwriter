//
//  PyObjectWrapper.h
//  xmlwriter
//
//  Created by Paul Ross on 05/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#ifndef xmlwriter_PyObjectWrapper_h
#define xmlwriter_PyObjectWrapper_h

#include <Python.h>

namespace CPythonCpp {


/*** Messing around with template wrappers. */
template<typename T>
class PyWrapperBase {
public:
    PyWrapperBase() : m_sp() {}
    PyWrapperBase(const T &obj) : m_sp(new T(obj)) {}
    // Dereferencing the shared pointer in various ways
    explicit operator bool() const { return bool(m_sp); }
    T &operator*() const { return &m_sp; };
    T *operator->() const { return *m_sp; };
    T *get() const { return m_sp.get(); };
    virtual operator PyObject*() const = 0;
protected:
    std::shared_ptr<T> m_sp;
};

// Example of a wrapper around a specific bytes object
class PyWrapperBytes : public PyWrapperBase<std::string> {
public:
    PyWrapperBytes(PyObject *obj) : PyWrapperBase<std::string>() {
        assert(CPythonCpp::cpython_asserts(obj));
        if (PyBytes_Check(obj)) {
            m_sp.reset(
                       new std::string(PyBytes_AsString(obj), PyBytes_Size(obj))
                       );
        } else {
            PyErr_Format(PyExc_TypeError,
                         "Argument %s must be bytes not \"%s\"",
                         __FUNCTION__, Py_TYPE(obj)->tp_name);
        }
    }
    operator PyObject*() const {
        assert(CPythonCpp::cpython_asserts());
        PyObject *r = NULL;
        if (m_sp) {
            r = PyBytes_FromStringAndSize(m_sp->c_str(), m_sp->size());
        }
        return r;
    }
};

// This sub-class records the original Python type so that can be used
// to recreate the correct Python object.
template<typename T>
class PyWrapperWithType : public PyWrapperBase<T>{
public:
    PyWrapperWithType(PyObject *obj) : m_type(obj->ob_type) {}
protected:
    PyTypeObject *m_type;
};


// Example of a wrapper around a bytes, bytearray or utf8 that can
// round-trip correctly
class PyWrapperStr : public PyWrapperWithType<std::string> {
public:
    PyWrapperStr(PyObject *obj) : PyWrapperWithType<std::string>(obj) {
        assert(CPythonCpp::cpython_asserts(obj));

        if (PyBytes_Check(obj)) {
            m_sp.reset(new std::string(PyBytes_AsString(obj), PyBytes_Size(obj)));
        } else if (PyByteArray_Check(obj)) {
            m_sp.reset(new std::string(PyByteArray_AsString(obj), PyByteArray_Size(obj)));
        } else if (PyUnicode_Check(obj)) {
            if (! PyUnicode_READY(obj)) {
                if (PyUnicode_KIND(obj) == PyUnicode_1BYTE_KIND) {
#if PY_MAJOR_VERSION >= 3
                    m_sp.reset(new std::string((char*)PyUnicode_1BYTE_DATA(obj)));
#else
                    // Nasty cast away constness because PyString_AsString takes non-const in Py2
                    m_sp.reset(new std::string(
                                               (char*)PyString_AsString(const_cast<PyObject *>(obj)))
                               );
#endif
                } else {
                    PyErr_Format(PyExc_ValueError,
                                 "In %s \"obj\" unicode but not utf-8",
                                 __FUNCTION__);
                }
            } else {
                PyErr_Format(PyExc_ValueError,
                             "In %s \"obj\" failed PyUnicode_READY()",
                             __FUNCTION__);
            }
        } else {
            PyErr_Format(PyExc_TypeError,
                         "Argument %s must be bytes, bytearray or utf8"
                         " string not \"%s\"",
                         __FUNCTION__, Py_TYPE(obj)->tp_name);
        }
    }
    operator PyObject*() const {
        assert(CPythonCpp::cpython_asserts());

        PyObject *r = NULL;
        if (m_sp) {
            if (m_type == &PyBytes_Type) {
                r = PyBytes_FromStringAndSize(m_sp->c_str(), m_sp->size());
            } else if (m_type == &PyByteArray_Type) {
                r = PyByteArray_FromStringAndSize(m_sp->c_str(), m_sp->size());
            } else if (m_type == &PyUnicode_Type) {
                r = PyUnicode_FromStringAndSize(m_sp->c_str(), m_sp->size());
            } else {
                abort();
            }
        }
        return r;
    }
};

} // namespace CPythonCpp

#endif
