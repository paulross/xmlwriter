//
//  RefWrapper.h
//  xmlwriter
//
//  Created by Paul Ross on 04/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#ifndef xmlwriter_RefWrapper_h
#define xmlwriter_RefWrapper_h

#include <Python.h>

namespace CPythonCpp {

/** General wrapper around a PyObject*.
 * This decrements the reference count on destruction.
 */
class DecRefDtor {
public:
    DecRefDtor(PyObject *ref) : m_ref { ref } {}
    Py_ssize_t ref_count() const { return m_ref ? Py_REFCNT(m_ref) : 0; }
    // Allow setting of the (optional) argument with
    // PyArg_ParseTupleAndKeywords
    PyObject **operator&() {
        Py_XDECREF(m_ref);
        m_ref = NULL;
        return &m_ref;
    }
    // Access the argument or the default if default.
    operator PyObject*() const {
        return m_ref;
    }
    // Test if constructed successfully from the new reference.
    explicit operator bool() { return m_ref != NULL; }
    ~DecRefDtor() { Py_XDECREF(m_ref); }
protected:
    PyObject *m_ref;
};

/** Wrapper around a PyObject* that is a borrowed reference.
 * This increments the reference count on construction and
 * decrements the reference count on destruction.
 */
class BorrowedRef : public DecRefDtor {
public:
    BorrowedRef(PyObject *borrowed_ref) : DecRefDtor(borrowed_ref) {
        Py_XINCREF(m_ref);
    }
};

/** Wrapper around a PyObject* that is a new reference.
 * This owns the reference so does not increment it on construction but
 * does decrement it on destruction.
 */
class NewRef : public DecRefDtor {
public:
    NewRef(PyObject *new_ref) : DecRefDtor(new_ref) {}
};

} // namespace CPythonCpp

#endif
