//
//  ConvertPyDict.h
//  xmlwriter
//
//  Created by Paul Ross on 04/05/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#ifndef xmlwriter_ConvertPyDict_h
#define xmlwriter_ConvertPyDict_h

#include <Python.h>

#include <map>
#include <unordered_map>

namespace CPythonCpp {

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

#if 0 // Experiments

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

#endif

/** Convert a python dict to a std::map<K, V>
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
std::map<K, V>
py_dict_to_std_map(PyObject *dict,
                   K (*PyKeyConvertToK)(PyObject *),
                   V (*PyValConvertToV)(PyObject *)
                   ) {
    Py_ssize_t pos = 0;
    PyObject *key = NULL;
    PyObject *val = NULL;
    std::map<K, V> cpp_map;

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

/** Convert a std::map<K, V> to a new python dict.
 * KeyConvertToPy is a pointer to a function that takes an instance of a K type
 * and returns a PyObject*, this should return NULL on failure.
 * ValConvertToPy is a pointer to a function that takes an instance of a V type
 * and returns a PyObject*, this should return NULL on failure.
 *
 * Returns a new reference on success, NULL on failure.
 */
template <typename K, typename V>
PyObject*
std_map_to_py_dict(const std::map<K, V> &cpp_map,
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

} // namespace CPythonCpp

#endif
