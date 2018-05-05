//
//  TestCPythonUtils.cpp
//  xmlwriter
//
//  Created by Paul Ross on 25/04/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//

#include <iostream>
#include <iomanip>

#include "TestCPythonUtils.h"
#include "RefWrapper.h"
#include "ConvertPyBytes.h"
#include "ConvertPyBytearray.h"
#include "ConvertPyList.h"
#include "ConvertPySet.h"
#include "ConvertPyStr.h"
#include "ConvertPyDict.h"
#include "cpython_asserts.h"

//int test_template() {
//    int result = 0;
//    result |= str == NULL;
//    return result;
//}

int test_DecRefDtor_ref_count() {
    assert(CPythonCpp::cpython_asserts());
    int result = 0;

    CPythonCpp::DecRefDtor drd(PyUnicode_FromString("foo"));
    result |= drd.ref_count() != 1;
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << result << std::endl;
    return result;
}

/* Does nothing but does expose a PyObject*. */
void _pyobject_handle(PyObject **/* arg */) {}

int test_DecRefDtor_dereference() {
    assert(CPythonCpp::cpython_asserts());
    int result = 0;

    CPythonCpp::DecRefDtor drd(PyUnicode_FromString("foo"));
    result |= drd.ref_count() != 1;
    // Test PyObject **operator&()
    _pyobject_handle(&drd);
    result |= drd.ref_count() != 0;
    result |= static_cast<PyObject*>(drd) != NULL;
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << result << std::endl;
    return result;
}

int test_DecRefDtor_op_PyObject() {
    assert(CPythonCpp::cpython_asserts());
    int result = 0;

    PyObject *str = PyUnicode_FromString("foo");
    CPythonCpp::DecRefDtor drd(str);
    result |= drd.ref_count() != 1;
    // Test operator PyObject*()
    result |= static_cast<PyObject*>(drd) != str;
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << result << std::endl;
    return result;
}

int test_DecRefDtor_bool() {
    assert(CPythonCpp::cpython_asserts());
    int result = 0;

    CPythonCpp::DecRefDtor drd(PyUnicode_FromString("foo"));
    // Test explicit operator bool()
    result |= drd ? 0 : 1;
    // Test PyObject **operator&()
    _pyobject_handle(&drd);
    // Test explicit operator bool()
    result |= drd ? 1 : 0;
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << result << std::endl;
    return result;
}

int test_std_string_to_py_bytes() {
    assert(CPythonCpp::cpython_asserts());
    int failure = 0;
    std::string input { "Foo" };
    PyObject *py_bytes = CPythonCpp::std_string_to_py_bytes(input);
    failure |= PyErr_Occurred() != NULL;
    failure |= py_bytes == NULL;
    std::string std_str = CPythonCpp::py_bytes_to_std_string(py_bytes);
    failure |= PyErr_Occurred() != NULL;
    failure |= std_str != "Foo";
    Py_XDECREF(py_bytes);
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << failure << std::endl;
    return failure;
}

int test_std_string_to_py_bytearray() {
    assert(CPythonCpp::cpython_asserts());
    int failure = 0;
    std::string input { "Foo" };
    PyObject *py_bytes = CPythonCpp::std_string_to_py_bytearray(input);
    failure |= PyErr_Occurred() != NULL;
    failure |= py_bytes == NULL;
    std::string std_str = CPythonCpp::py_bytearray_to_std_string(py_bytes);
    failure |= PyErr_Occurred() != NULL;
    failure |= std_str != "Foo";
    Py_XDECREF(py_bytes);
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << failure << std::endl;
    return failure;
}

#pragma mark -
#pragma mark Test list
int test_std_vector_to_py_list() {
    assert(CPythonCpp::cpython_asserts());
    int failure = 0;
    std::vector<std::string> cpp_vector = {"Foo", "Bar", "Baz"};
    PyObject *py_list = CPythonCpp::std_vector_to_py_list(cpp_vector,
                                                          &CPythonCpp::std_string_to_py_utf8);
    failure |= py_list == NULL;
    failure |= PyErr_Occurred() != NULL;
    if (py_list) {
        failure |= ! PyList_CheckExact(py_list);
        std::vector<std::string> round_trip = \
            CPythonCpp::py_list_to_std_vector<std::string>(py_list, &CPythonCpp::py_utf8_to_std_string);
        failure |= round_trip != cpp_vector;
        Py_DECREF(py_list);
    }
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << failure << std::endl;
    return failure;
}

int test_py_list_to_std_vector() {
    assert(CPythonCpp::cpython_asserts());
    int failure = 0;

    std::vector<std::string> expected = {"Foo", "Bar", "Baz"};
    PyObject *py_list = PyList_New(3);
    for (int i = 0; i < expected.size(); ++i) {
        PyList_SET_ITEM(py_list, i, CPythonCpp::std_string_to_py_utf8(expected[i]));
    }
    std::vector<std::string> result = \
        CPythonCpp::py_list_to_std_vector<std::string>(py_list, &CPythonCpp::py_utf8_to_std_string);
    failure |= result != expected;
    PyObject *py_round_trip = CPythonCpp::std_vector_to_py_list(result,
                                                                &CPythonCpp::std_string_to_py_utf8);
    failure |= py_round_trip == NULL;
    failure |= PyErr_Occurred() != NULL;
    if (py_round_trip) {
        failure |= ! PyList_CheckExact(py_round_trip);
        PyObject *is_equal = PyObject_RichCompare(py_list, py_round_trip, Py_EQ);
        failure |= is_equal != Py_True;
        Py_DECREF(py_list);
        Py_XDECREF(is_equal);
    }
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << failure << std::endl;
    return failure;
}

#pragma mark -
#pragma mark Test set, frozenset
int test_std_unordered_set_to_py_set() {
    assert(CPythonCpp::cpython_asserts());
    int failure = 0;
    std::unordered_set<std::string> cpp_set = {"Foo", "Bar", "Baz"};
    PyObject *py_set = CPythonCpp::std_unordered_set_to_py_set(cpp_set,
                                                               &CPythonCpp::std_string_to_py_utf8,
                                                               false);
    failure |= py_set == NULL;
    failure |= PyErr_Occurred() != NULL;
    if (py_set) {
        failure |= ! PyAnySet_CheckExact(py_set);
        failure |= PyFrozenSet_CheckExact(py_set);
        std::unordered_set<std::string> round_trip = \
            CPythonCpp::py_set_to_std_unordered_set<std::string>(py_set,
                                                                 &CPythonCpp::py_utf8_to_std_string);
        failure |= round_trip != cpp_set;
        Py_DECREF(py_set);
    }
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << failure << std::endl;
    return failure;
}

int test_py_set_to_std_unordered_set() {
    assert(CPythonCpp::cpython_asserts());
    int failure = 0;

    std::unordered_set<std::string> expected = {"Foo", "Bar", "Baz"};
    PyObject *py_set = PySet_New(NULL);
    for (auto &iter: expected) {
        PySet_Add(py_set, CPythonCpp::std_string_to_py_utf8(iter));
    }
    std::unordered_set<std::string> result = \
        CPythonCpp::py_set_to_std_unordered_set<std::string>(py_set, &CPythonCpp::py_utf8_to_std_string);
    failure |= PyErr_Occurred() != NULL;
    failure |= result != expected;
    PyObject *py_round_trip = CPythonCpp::std_unordered_set_to_py_set(result,
                                                          &CPythonCpp::std_string_to_py_utf8,
                                                          false);
    failure |= py_round_trip == NULL;
    failure |= PyErr_Occurred() != NULL;
    if (py_round_trip) {
        failure |= ! PyAnySet_CheckExact(py_round_trip);
        failure |= PyFrozenSet_CheckExact(py_round_trip);
        PyObject *is_equal = PyObject_RichCompare(py_set, py_round_trip, Py_EQ);
        failure |= is_equal != Py_True;
        Py_XDECREF(is_equal);
    }
    Py_DECREF(py_set);
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << failure << std::endl;
    return failure;
}

int test_std_unordered_set_to_py_frozenset() {
    assert(CPythonCpp::cpython_asserts());
    int failure = 0;
    std::unordered_set<std::string> cpp_set = {"Foo", "Bar", "Baz"};
    PyObject *py_set = CPythonCpp::std_unordered_set_to_py_set(cpp_set,
                                                               &CPythonCpp::std_string_to_py_utf8,
                                                               true);
    failure |= py_set == NULL;
    failure |= PyErr_Occurred() != NULL;
    if (py_set) {
        failure |= ! PyFrozenSet_CheckExact(py_set);
        std::unordered_set<std::string> round_trip = \
            CPythonCpp::py_set_to_std_unordered_set<std::string>(py_set,
                                                                 &CPythonCpp::py_utf8_to_std_string);
        failure |= round_trip != cpp_set;
        Py_DECREF(py_set);
    }
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << failure << std::endl;
    return failure;
}

int test_py_frozenset_to_std_unordered_set() {
    assert(CPythonCpp::cpython_asserts());
    int failure = 0;

    std::unordered_set<std::string> expected = {"Foo", "Bar", "Baz"};
    PyObject *py_set = PyFrozenSet_New(NULL);
    for (auto &iter: expected) {
        PySet_Add(py_set, CPythonCpp::std_string_to_py_utf8(iter));
    }
    std::unordered_set<std::string> result = \
        CPythonCpp::py_set_to_std_unordered_set<std::string>(py_set, &CPythonCpp::py_utf8_to_std_string);
    failure |= PyErr_Occurred() != NULL;
    failure |= result != expected;
    PyObject *py_round_trip = CPythonCpp::std_unordered_set_to_py_set(result,
                                                                      &CPythonCpp::std_string_to_py_utf8,
                                                                      true);
    failure |= py_round_trip == NULL;
    failure |= PyErr_Occurred() != NULL;
    if (py_round_trip) {
        failure |= ! PyFrozenSet_CheckExact(py_round_trip);
        PyObject *is_equal = PyObject_RichCompare(py_set, py_round_trip, Py_EQ);
        failure |= is_equal != Py_True;
        Py_XDECREF(is_equal);
    }
    Py_DECREF(py_set);
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << failure << std::endl;
    return failure;
}

#pragma mark -
#pragma mark Test dict
int test_std_unordered_map_to_py_dict() {
    assert(CPythonCpp::cpython_asserts());
    int failure = 0;
    std::unordered_map<std::string, std::string> cpp_map {
        {"Foo", "Bar"}
    };
    PyObject *py_dict = NULL;

    py_dict = CPythonCpp::std_unordered_map_to_py_dict<std::string, std::string>(
            cpp_map,
            &CPythonCpp::std_string_to_py_utf8,
            &CPythonCpp::std_string_to_py_utf8
    );
    failure |= py_dict == NULL;
    failure |= PyErr_Occurred() != NULL;
    if (py_dict) {
        failure |= ! PyDict_CheckExact(py_dict);
//        PyObject_Print(py_dict, stdout, 0);
//        fprintf(stdout, "\n");
        std::unordered_map<std::string, std::string> \
        round_trip = \
            CPythonCpp::py_dict_to_std_unordered_map<std::string, std::string>(py_dict,
                                             &CPythonCpp::py_utf8_to_std_string,
                                             &CPythonCpp::py_utf8_to_std_string);
        failure |= round_trip != cpp_map;
    }
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << failure << std::endl;
    return failure;
}

int test_py_dict_to_std_unordered_map() {
    assert(CPythonCpp::cpython_asserts());
    int failure = 0;
    PyObject *py_dict = PyDict_New();
    failure |= PyDict_SetItem(py_dict,
                              PyBytes_FromString("Foo"),
                              PyBytes_FromString("Bar")
                              );
    std::unordered_map<std::string, std::string> result;
    result = CPythonCpp::py_dict_to_std_unordered_map(py_dict,
                                          &CPythonCpp::py_bytes_to_std_string,
                                          &CPythonCpp::py_bytes_to_std_string);
    failure |= PyErr_Occurred() != NULL;
    std::unordered_map<std::string, std::string> expected {
        {"Foo", "Bar"}
    };
    failure |= result != expected;
    PyObject *py_round_trip = \
        CPythonCpp::std_unordered_map_to_py_dict(result,
                                     &CPythonCpp::std_string_to_py_bytes,
                                     &CPythonCpp::std_string_to_py_bytes);
    failure |= py_round_trip == NULL;
    failure |= PyErr_Occurred() != NULL;
    if (py_round_trip) {
        failure |= ! PyDict_CheckExact(py_round_trip);
//        fprintf(stdout, "      py_dict: ");
//        PyObject_Print(py_dict, stdout, 0);
//        fprintf(stdout, "\n");
//        fprintf(stdout, "py_round_trip: ");
//        PyObject_Print(py_round_trip, stdout, 0);
//        fprintf(stdout, "\n");
        PyObject *is_equal = PyObject_RichCompare(py_dict, py_round_trip, Py_EQ);
        failure |= is_equal != Py_True;
        Py_XDECREF(is_equal);
    }
    Py_DECREF(py_dict);
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << failure << std::endl;
    return failure;
}

int test_std_map_to_py_dict() {
    assert(CPythonCpp::cpython_asserts());
    int failure = 0;
    std::map<std::string, std::string> cpp_map {
        {"Foo", "Bar"}
    };
    PyObject *py_dict = NULL;

    py_dict = CPythonCpp::std_map_to_py_dict<std::string, std::string>(
                                                         cpp_map,
                                                         &CPythonCpp::std_string_to_py_utf8,
                                                         &CPythonCpp::std_string_to_py_utf8
                                                                       );
    failure |= py_dict == NULL;
    failure |= PyErr_Occurred() != NULL;
    if (py_dict) {
        failure |= ! PyDict_CheckExact(py_dict);
        //        PyObject_Print(py_dict, stdout, 0);
        //        fprintf(stdout, "\n");
        std::map<std::string, std::string> round_trip = \
            CPythonCpp::py_dict_to_std_map<std::string, std::string>(
                                                        py_dict,
                                                        &CPythonCpp::py_utf8_to_std_string,
                                                        &CPythonCpp::py_utf8_to_std_string);
        failure |= round_trip != cpp_map;
    }
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << failure << std::endl;
    return failure;
}

int test_py_dict_to_std_map() {
    assert(CPythonCpp::cpython_asserts());
    int failure = 0;
    PyObject *py_dict = PyDict_New();
    failure |= PyDict_SetItem(py_dict,
                              PyBytes_FromString("Foo"),
                              PyBytes_FromString("Bar")
                              );
    std::map<std::string, std::string> result;
    result = CPythonCpp::py_dict_to_std_map(py_dict,
                                            &CPythonCpp::py_bytes_to_std_string,
                                            &CPythonCpp::py_bytes_to_std_string);
    failure |= PyErr_Occurred() != NULL;
    std::map<std::string, std::string> expected {
        {"Foo", "Bar"}
    };
    failure |= result != expected;
    PyObject *py_round_trip = \
    CPythonCpp::std_map_to_py_dict(result,
                                   &CPythonCpp::std_string_to_py_bytes,
                                   &CPythonCpp::std_string_to_py_bytes);
    failure |= py_round_trip == NULL;
    failure |= PyErr_Occurred() != NULL;
    if (py_round_trip) {
        failure |= ! PyDict_CheckExact(py_round_trip);
        //        fprintf(stdout, "      py_dict: ");
        //        PyObject_Print(py_dict, stdout, 0);
        //        fprintf(stdout, "\n");
        //        fprintf(stdout, "py_round_trip: ");
        //        PyObject_Print(py_round_trip, stdout, 0);
        //        fprintf(stdout, "\n");
        PyObject *is_equal = PyObject_RichCompare(py_dict, py_round_trip, Py_EQ);
        failure |= is_equal != Py_True;
        Py_XDECREF(is_equal);
    }
    Py_DECREF(py_dict);
    std::cout << std::setw(50) <<__FUNCTION__ << " result: " << failure << std::endl;
    return failure;
}

int test_all_cpython_utils() {
    Py_Initialize();
    assert(CPythonCpp::cpython_asserts());
    int result = 0;

    result |= test_DecRefDtor_ref_count();
    result |= test_DecRefDtor_op_PyObject();
    result |= test_DecRefDtor_dereference();
    result |= test_DecRefDtor_bool();

    result |= test_std_string_to_py_bytes();
    result |= test_std_string_to_py_bytearray();

    result |= test_std_vector_to_py_list();
    result |= test_py_list_to_std_vector();

    result |= test_std_unordered_set_to_py_set();
    result |= test_py_set_to_std_unordered_set();
    result |= test_std_unordered_set_to_py_frozenset();
    result |= test_py_frozenset_to_std_unordered_set();

    result |= test_std_unordered_map_to_py_dict();
    result |= test_py_dict_to_std_unordered_map();
    result |= test_std_map_to_py_dict();
    result |= test_py_dict_to_std_map();

    return result;
}