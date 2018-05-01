//
//  CPythonUtils.h
//  xmlwriter
//
//  Created by Paul Ross on 25/04/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#ifndef __xmlwriter__CPythonUtils__
#define __xmlwriter__CPythonUtils__

#include <Python.h>

#include <string>
#include <map>
#include <vector>
#include <unordered_set>
#include <unordered_map>

/* Returns non zero if Python is initialised and there is no Python error set.
 * The second version also checks that the given pointer is non-NULL
 * Use this thus, it will do nothing if NDEBUG is defined:
 * assert(cpython_asserts());
 * assert(cpython_asserts(p));
 */
int cpython_asserts();
int cpython_asserts(PyObject *pobj);

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


#pragma mark -
#pragma mark Utilities

#define XML_WRITE_DEBUG_TRACE 0

// Macros for default arguments
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

#define PY_DEFAULT_ARGUMENT_SET(name) if (! name) name = default_##name; \
Py_INCREF(name)


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

/** Template conversion functions. */

template <typename P, typename C>
C convert_py_to_cpp(P *p_obj);

template<>
std::string
convert_py_to_cpp<PyBytesObject, std::string>(PyBytesObject *p_obj);
template<>
std::string
convert_py_to_cpp<PyByteArrayObject, std::string>(PyByteArrayObject *p_obj);

template <typename C, typename P>
P *convert_cpp_to_py(const C &cpp_obj);

template <>
PyBytesObject *
convert_cpp_to_py<std::string, PyBytesObject>(const std::string &cpp_obj);
template <>
PyByteArrayObject *
convert_cpp_to_py<std::string, PyByteArrayObject>(const std::string &cpp_obj);


/*** Converting  Python bytes and Unicode to and from std::string ***/
// These all make PyErr_Occurred() true on failure
std::string py_bytes_to_std_string(PyObject *py_str);
std::string py_bytearray_to_std_string(PyObject *py_str);
std::string py_utf8_to_std_string(PyObject *py_str);

PyObject *std_string_to_py_bytes(const std::string &str);
PyObject *std_string_to_py_bytearray(const std::string &str);
PyObject *std_string_to_py_utf8(const std::string &str);

/** The mirror of py_str_to_string()
 * This expects an original object of bytes, bytearray or str or
 * sub-type and and converts a std::string into a PyObject* of the same type.
 */
PyObject *
std_string_to_py_object(PyObject *original, const std::string &result);

/* Takes a dict[str : str] and returns a std::map<std::string, std::string>
 * On error this sets an error and returns an empty map;.
 */
std::map<std::string, std::string>
dict_to_map_str_str(PyObject *arg);

/* Takes a std::map<std::string, std::string> and returns a dict[str : str]
 * Always succeeds.
 */
PyObject *
map_str_str_to_dict(const std::map<std::string, std::string> &map);




#pragma mark -
#pragma mark list <-> std::vector<T>
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
    assert(cpython_asserts(py_list));
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
    assert(cpython_asserts());
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

#pragma mark -
#pragma mark set, frozenset <-> std::unordered_set<T>
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
    assert(cpython_asserts(py_set));
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
    assert(cpython_asserts());
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

#pragma mark -
#pragma mark dict <-> std::unordered_map<K, V>
//template <typename T> void func(T param) {} // definition
//template void func<int>(int param); // explicit instantiation.
//
//template <typename T, typename V> void another_func(T param, V vparam) {} // definition
//template void another_func<int, std::string>(int param, std::string v); // explicit instantiation.


/** Convert a python dict to a std::unordered_map<K, V>
 * PyKeyConvertToK is a function pointer to a function that takes a PyObject*
 * and returns an instance of a K type.
 * PyValConvertToV is a function pointer to a function that takes a PyObject*
 * and returns an instance of a V type.
 * Both of these function should make PyErr_Occurred() true on failure to
 * convert a PyObject*.
 *
 * On failure this sets PyErr_Occurred() and the return value will be empty.
 */
template <typename K, typename V>
std::unordered_map<K, V>
py_dict_to_std_unordered_map(PyObject *dict,
                             K (*PyKeyConvertToK)(PyObject *),
                             V (*PyValConvertToV)(PyObject *)
                             ) {
    Py_ssize_t pos = 0;
    PyObject *key = NULL;
    PyObject *val = NULL;
    std::unordered_map<K, V> cpp_map;

    if (! PyDict_Check(dict)) {
        PyErr_Format(PyExc_TypeError,
                     "Argument \"dict\" to %s must be dict not \"%s\"",
                     __FUNCTION__, Py_TYPE(dict)->tp_name);
        return cpp_map;
    }
    while (PyDict_Next(dict, &pos, &key, &val)) {
        K cpp_key = (*PyKeyConvertToK)(key);
        if (PyErr_Occurred()) {
            cpp_map.clear();
            break;
        }
        V cpp_val = (*PyValConvertToV)(val);
        if (PyErr_Occurred()) {
            cpp_map.clear();
            break;
        }
        cpp_map.emplace(cpp_key, cpp_val);
    }
    return cpp_map;
}

/** Convert a std::unordered_map<K, V> to a new python dict.
 * KeyConvertToPy is a pointer to a function that takes an instance of a K type
 * and returns a PyObject*, this should return NULL on failure.
 * ValConvertToPy is a pointer to a function that takes an instance of a V type
 * and returns a PyObject*, this should return NULL on failure.
 *
 * Returns a new reference on success, NULL on failure.
 */
template <typename K, typename V>
PyObject*
std_unordered_map_to_py_dict(const std::unordered_map<K, V> &cpp_map,
                             PyObject *(*KeyConvertToPy)(const K&),
                             PyObject *(*ValConvertToPy)(const V&)
                             ) {
    PyObject *key = NULL;
    PyObject *val = NULL;
    PyObject *r = PyDict_New();

    if (!r) {
        goto except;
    }
    for (auto &iter: cpp_map) {
        key = (*KeyConvertToPy)(iter.first);
        if (! key || PyErr_Occurred()) {
            goto except;
        }
        val = (*ValConvertToPy)(iter.second);
        if (! val || PyErr_Occurred()) {
            goto except;
        }
        if (PyDict_SetItem(r, key, val)) {
            goto except;
        }
    }
    assert(! PyErr_Occurred());
    assert(r);
    goto finally;
except:
    assert(PyErr_Occurred());
    // Clean up dict
    if (r) {
        PyDict_Clear(r);
        Py_DECREF(r);
    }
    r = NULL;
finally:
    return r;
}

// Conversion function &std_string_to_py_utf8
template
PyObject*
std_unordered_map_to_py_dict<std::string, std::string>(
    const std::unordered_map<std::string, std::string> &cpp_map,
    PyObject *(*KeyConvertToPy)(const std::string&), // &std_string_to_py_utf8
    PyObject *(*ValConvertToPy)(const std::string&)  // &std_string_to_py_utf8
);


// Conversion function &py_utf8_to_std_string
template
std::unordered_map<std::string, std::string>
py_dict_to_std_unordered_map<std::string, std::string>(
    PyObject *dict,
    std::string (*PyKeyConvertToK)(PyObject *), // &py_utf8_to_std_string
    std::string (*PyValConvertToV)(PyObject *)  // &py_utf8_to_std_string
                                                  );

#endif /* defined(__xmlwriter__CPythonUtils__) */
