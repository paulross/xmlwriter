//
//  cXmlWrite.cpp
//  xmlwriter
//
//  Created by Paul Ross on 10/04/2018.
//  Copyright (c) 2018 Paul Ross. All rights reserved.
//
#include <Python.h>
#include "structmember.h"

#include <memory>

//#include "cXmlWrite.h"
#include "XmlWrite.h"
#include "XmlWrite_docs.h"

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
 * if (! arg_0 || !arg_1) {
 *      return NULL;
 * }
 *
 * if (! PyArg_ParseTupleAndKeywords(args, kwargs, "...",
                                     const_cast<char**>(kwlist),
                                     &arg_0, &arg_1, ...)) {
        return NULL;
    }
 *
 * Then just use arg_0, arg_1 as if they were a PyObject*
 *
 */
class DefaultArg {
public:
    DefaultArg(PyObject *new_ref) : m_arg { NULL }, m_default { new_ref } {
#if 1
        fprintf(stdout, "Creating DefaultArg of ");
        PyObject_Print(m_default, stdout, 0);
        fprintf(stdout, "\n");
#endif
    }
    PyObject **operator&() { return &m_arg; }
    operator PyObject*() const {
        return m_arg ? m_arg : m_default;
    }
    explicit operator bool() { return m_default != NULL; }
    // Prevent leak if this class is non-static.
    ~DefaultArg() {
        Py_XDECREF(m_default);
    }
protected:
    PyObject *m_arg;
    PyObject *m_default;
};

/*** Converting  Python bytes and Unicode to and from std::string ***/
/* Convert a PyObject to a std::string and return 0 if succesful.
 * If py_str is Unicode than treat it as UTF-8.
 * This works with Python 2.7 and Python 3.4 onwards.
 */
//static int
//py_str_to_string(const PyObject *py_str,
//                 std::string &result,
//                 bool utf8_only=true) {
//    result.clear();
//    if (PyBytes_Check(py_str)) {
//        result = std::string(PyBytes_AS_STRING(py_str));
//        return 0;
//    }
//    if (PyByteArray_Check(py_str)) {
//        result = std::string(PyByteArray_AS_STRING(py_str));
//        return 0;
//    }
//    // Must be unicode then.
//    if (! PyUnicode_Check(py_str)) {
//        PyErr_Format(PyExc_ValueError,
//                     "In %s \"py_str\" failed PyUnicode_Check()",
//                     __FUNCTION__);
//        return -1;
//    }
//    if (PyUnicode_READY(py_str)) {
//        PyErr_Format(PyExc_ValueError,
//                     "In %s \"py_str\" failed PyUnicode_READY()",
//                     __FUNCTION__);
//        return -2;
//    }
//    if (utf8_only && PyUnicode_KIND(py_str) != PyUnicode_1BYTE_KIND) {
//        PyErr_Format(PyExc_ValueError,
//                     "In %s \"py_str\" not utf-8",
//                     __FUNCTION__);
//        return -3;
//    }
//    // Python 3 and its minor versions (they vary)
//    //    const Py_UCS1 *pChrs = PyUnicode_1BYTE_DATA(pyStr);
//    //    result = std::string(reinterpret_cast<const char*>(pChrs));
//#if PY_MAJOR_VERSION >= 3
//    result = std::string((char*)PyUnicode_1BYTE_DATA(py_str));
//#else
//    // Nasty cast away constness because PyString_AsString takes non-const in Py2
//    result = std::string((char*)PyString_AsString(const_cast<PyObject *>(py_str)));
//#endif
//    return 0;
//}

/* As above but returns a new string. On error the string will be empty
 * and an error set. */
static std::string
py_str_to_string(const PyObject *py_str, bool utf8_only=true) {
    if (PyBytes_Check(py_str)) {
        return std::string(PyBytes_AS_STRING(py_str));
    }
    if (PyByteArray_Check(py_str)) {
        return std::string(PyByteArray_AS_STRING(py_str));
    }
    // Must be unicode then.
    if (! PyUnicode_Check(py_str)) {
        PyErr_Format(PyExc_ValueError,
                     "In %s \"py_str\" failed PyUnicode_Check()",
                     __FUNCTION__);
        return "";
    }
    if (PyUnicode_READY(py_str)) {
        PyErr_Format(PyExc_ValueError,
                     "In %s \"py_str\" failed PyUnicode_READY()",
                     __FUNCTION__);
        return "";
    }
    if (utf8_only && PyUnicode_KIND(py_str) != PyUnicode_1BYTE_KIND) {
        PyErr_Format(PyExc_ValueError,
                     "In %s \"py_str\" not utf-8",
                     __FUNCTION__);
        return "";
    }
    // Python 3 and its minor versions (they vary)
    //    const Py_UCS1 *pChrs = PyUnicode_1BYTE_DATA(pyStr);
    //    result = std::string(reinterpret_cast<const char*>(pChrs));
#if PY_MAJOR_VERSION >= 3
    return std::string((char*)PyUnicode_1BYTE_DATA(py_str));
#else
    // Nasty cast away constness because PyString_AsString takes non-const in Py2
    return std::string((char*)PyString_AsString(const_cast<PyObject *>(py_str)));
#endif
}

//static PyObject *
//std_string_to_py_bytes(const std::string &str) {
//    return PyBytes_FromStringAndSize(str.c_str(), str.size());
//}
//
//static PyObject *
//std_string_to_py_bytearray(const std::string &str) {
//    return PyByteArray_FromStringAndSize(str.c_str(), str.size());
//}
//
//static PyObject *
//std_string_to_py_utf8(const std::string &str) {
//    // Equivelent to:
//    // PyUnicode_FromKindAndData(PyUnicode_1BYTE_KIND, str.c_str(), str.size());
//    return PyUnicode_FromStringAndSize(str.c_str(), str.size());
//}

/* Takes a dict[str : str] and returns a std::map<std::string, std::string>
 * On error this seta an error and returns an empty map;.
 */
static tAttrs
dict_to_attributes(PyObject *arg) {
    // Used for iterating across the dict
    PyObject *key, *value;
    Py_ssize_t pos = 0;
    tAttrs cpp_attrs;
    assert(! PyErr_Occurred());

    if (arg) {
        if (PyDict_Check(arg)) {
            while (PyDict_Next(arg, &pos, &key, &value)) {
                cpp_attrs[py_str_to_string(key)] = py_str_to_string(value);
                if (PyErr_Occurred()) {
                    cpp_attrs.clear();
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
/** END: Converting Python bytes and Unicode to and from std::string **/

#pragma mark -
#pragma mark Encoding/decoding

/* Encoding/decoding Python bytes objects. */
static PyObject*
encode_string(PyObject */* module */, PyObject *args, PyObject *kwargs) {
    PyObject *ret = NULL;
    PyObject *py_bytes = NULL;
    PyObject *py_prefix = NULL;
    std::string result;

    static const char *kwlist[] = {
        "theS", "theCharPrefix", NULL
    };
    if (! PyArg_ParseTupleAndKeywords(args, kwargs, "S|S",
                                      const_cast<char**>(kwlist),
                                      &py_bytes, &py_prefix)) {
        return NULL;
    }
    Py_INCREF(py_bytes);
    if (py_prefix) {
        Py_INCREF(py_prefix);
    }
    try {
        if (py_prefix) {
            result = encodeString(PyBytes_AsString(py_bytes),
                                  PyBytes_AsString(py_prefix));
        } else {
            result = encodeString(PyBytes_AsString(py_bytes));
        }
    } catch (ExceptionXml &err) {
        PyErr_Format(PyExc_RuntimeError,
                     "In %s \"encodeString\" failed with error %s",
                     __FUNCTION__, err.what());
        goto except;
    }
    ret = PyBytes_FromStringAndSize(result.c_str(), result.size());
    if (! ret) {
        goto except;
    }
    assert(! PyErr_Occurred());
    assert(ret);
    goto finally;
except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    Py_DECREF(py_bytes);
    Py_XDECREF(py_prefix);
    return ret;
}

static PyObject*
decode_string(PyObject */* module */, PyObject *encoded) {
    assert(encoded);
    PyObject *ret = NULL;
    std::string result;

    Py_INCREF(encoded);
    if (! PyBytes_Check(encoded)) {
        PyErr_Format(PyExc_TypeError,
                     "Argument \"theS\" to %s must be bytes not \"%s\"",
                     __FUNCTION__, Py_TYPE(encoded)->tp_name);
        goto except;
    }
    try {
        result = decodeString(PyBytes_AsString(encoded));
    } catch (ExceptionXml &err) {
        PyErr_Format(PyExc_RuntimeError,
                     "In %s \"decodeString\" failed with error %s",
                     __FUNCTION__, err.what());
        goto except;
    }
    ret = PyBytes_FromStringAndSize(result.c_str(), result.size());
    if (! ret) {
        goto except;
    }
    assert(! PyErr_Occurred());
    assert(ret);
    goto finally;
except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    Py_DECREF(encoded);
    return ret;
}

static PyObject*
name_from_string(PyObject */* module */, PyObject *py_string) {
    assert(py_string);
    PyObject *ret = NULL;
    std::string result;

    Py_INCREF(py_string);
    if (! PyBytes_Check(py_string)) {
        PyErr_Format(PyExc_TypeError,
                     "Argument \"theS\" to %s must be bytes not \"%s\"",
                     __FUNCTION__, Py_TYPE(py_string)->tp_name);
        goto except;
    }
    try {
        result = nameFromString(PyBytes_AsString(py_string));
    } catch (ExceptionXml &err) {
        PyErr_Format(PyExc_RuntimeError,
                     "In %s \"nameFromString\" failed with error %s",
                     __FUNCTION__, err.what());
        goto except;
    }
    ret = PyBytes_FromStringAndSize(result.c_str(), result.size());
    if (! ret) {
        goto except;
    }
    assert(! PyErr_Occurred());
    assert(ret);
    goto finally;
except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    Py_DECREF(py_string);
    return ret;
}

// Some template magic to create a constructor used by both
// XmlStream and XhtmlStream
template <typename PyType, typename CppType>
static int
Generic_Stream_init(PyType *self, PyObject *args, PyObject *kwds) {
    int ret = 0;
    static DefaultArg theEnc { PyUnicode_FromString("utf-8") };
    static DefaultArg theDtdLocal { PyUnicode_FromString("") };
    static DefaultArg theId { PyLong_FromLong(0L) };
    static DefaultArg mustIndent { PyBool_FromLong(1L) };

    if (!theEnc || !theDtdLocal || !theId || !mustIndent) {
        return -1;
    }
    static const char *kwlist[] = {
        "theEnc", "theDtdLocal", "theId", "mustIndent", NULL
    };

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|OOip",
                                      const_cast<char**>(kwlist),
                                      &theEnc, &theDtdLocal,
                                      &theId, &mustIndent)) {
        return -1;
    }
    self->p_stream = new CppType(
        std::string(PyUnicode_AS_DATA((PyObject*)theEnc)),
        std::string(PyUnicode_AS_DATA((PyObject*)theDtdLocal)),
        static_cast<int>(PyLong_AsLong(theId)),
        mustIndent == Py_True ? true : false
    );
    if (! self->p_stream) {
        ret =  -1;
    }
    return ret;
}

#pragma mark -
#pragma mark XmlStream
/******************* XmlStream ********************/
typedef struct {
    PyObject_HEAD
    XmlStream *p_stream;
} cXmlStream;

static void
cXmlStream_dealloc(cXmlStream* self) {
    delete self->p_stream;
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
cXmlStream_new(PyTypeObject *type, PyObject */* args */, PyObject */* kwds */)
{
    cXmlStream *self = (cXmlStream *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->p_stream = nullptr;
    }
    return (PyObject *)self;
}

//static int
//cXmlStream_init(cXmlStream *self, PyObject *args, PyObject *kwds) {
//    int ret = 0;
//    // Default arguments
////    PY_DEFAULT_ARGUMENT_INIT(theEnc, PyUnicode_FromString("utf-8"), -1);
////    PY_DEFAULT_ARGUMENT_INIT(theDtdLocal, PyUnicode_FromString(""), -1);
////    PY_DEFAULT_ARGUMENT_INIT(theId, PyLong_FromLong(0L), -1);
////    PY_DEFAULT_ARGUMENT_INIT(mustIndent, PyBool_FromLong(1L), -1);
//
//    static DefaultArg theEnc { PyUnicode_FromString("utf-8") };
//    static DefaultArg theDtdLocal { PyUnicode_FromString("") };
//    static DefaultArg theId { PyLong_FromLong(0L) };
//    static DefaultArg mustIndent { PyBool_FromLong(1L) };
//
//    if (!theEnc || !theDtdLocal || !theId || !mustIndent) {
//        return -1;
//    }
//    static const char *kwlist[] = {
//        "theEnc", "theDtdLocal", "theId", "mustIndent", NULL
//    };
//
//    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|OOip",
//                                      const_cast<char**>(kwlist),
//                                      &theEnc, &theDtdLocal,
//                                      &theId, &mustIndent)) {
//        return -1;
//    }
////    // Assign absent arguments to defaults
////    PY_DEFAULT_ARGUMENT_SET(theEnc);
////    PY_DEFAULT_ARGUMENT_SET(theDtdLocal);
////    PY_DEFAULT_ARGUMENT_SET(theId);
////    PY_DEFAULT_ARGUMENT_SET(mustIndent);
//    self->p_stream = new XmlStream(
//        std::string(PyUnicode_AS_DATA((PyObject*)theEnc)),
//        std::string(PyUnicode_AS_DATA((PyObject*)theDtdLocal)),
//        static_cast<int>(PyLong_AsLong(theId)),
//        mustIndent == Py_True ? true : false
//    );
//    if (! self->p_stream) {
//        ret =  -1;
//    }
////    Py_DECREF(theEnc);
////    Py_DECREF(theDtdLocal);
////    Py_DECREF(theId);
////    Py_DECREF(mustIndent);
//    return ret;
//}

static PyObject *
cXmlStream_getvalue(cXmlStream* self) {
    std::string value = self->p_stream->getvalue();
    return PyBytes_FromStringAndSize(value.c_str(), value.size());
}

static PyObject *
cXmlStream__flipIndent(cXmlStream *self, PyObject *arg) {
    Py_INCREF(arg);
    PyObject *ret = NULL;
    if (! PyBool_Check(arg)) {
        PyErr_Format(PyExc_TypeError,
                     "Argument \"theBool\" to %s must be bool not \"%s\"",
                     __FUNCTION__, Py_TYPE(arg)->tp_name);
        goto except;
    }
    // TODO: This should always succeed so this function could be simplified.
    self->p_stream->_flipIndent(arg == Py_True);
    assert(! PyErr_Occurred());
    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;
except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    Py_DECREF(arg);
    return ret;
}

static PyObject *
cXmlStream_xmlSpacePreserve(cXmlStream *self) {
    self->p_stream->xmlSpacePreserve();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
cXmlStream_startElement(cXmlStream *self, PyObject *args, PyObject *kwds) {
    PyObject *name = NULL;
    PyObject *attrs = NULL;
    PyObject *ret = NULL;
//     Used for iterating across the dict
//    PyObject *key, *value;
    std::string cpp_name;
//    Py_ssize_t pos = 0;
    tAttrs cpp_attrs;

    static const char *kwlist[] = { "name", "attrs", NULL };
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "s|O",
                                      const_cast<char**>(kwlist),
                                      &name, &attrs)) {
        goto except;
    }
    if (attrs) {
        cpp_attrs = dict_to_attributes(attrs);
        if (! PyErr_Occurred()) {
            goto except;
        }
    }
    cpp_name = py_str_to_string(name);
    if (PyErr_Occurred()) {
        goto except;
    }
    self->p_stream->startElement(cpp_name, cpp_attrs);
    assert(! PyErr_Occurred());
    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;
except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    return ret;
}

typedef void (XmlStream::*type_str_fn)(const std::string &);
#define CALL_MEMBER_FN(object, ptrToMember) ((object).*(ptrToMember))

/* Call a function on XmlStream with a single Python argument that is expected
 * to be convertible to a std::string.
 */
static PyObject *
cXmlStream_generic_string(XmlStream &stream, type_str_fn fn, PyObject *arg) {
    Py_INCREF(arg);
    PyObject *ret = NULL;
    std::string chars { py_str_to_string(arg) };
    if (PyErr_Occurred()) {
        goto except;
    }
    CALL_MEMBER_FN(stream, fn)(chars);
    assert(! PyErr_Occurred());
    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;
except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    Py_DECREF(arg);
    return ret;
}

static PyObject *
cXmlStream_characters(cXmlStream *self, PyObject *arg) {
    return cXmlStream_generic_string(*self->p_stream, &XmlStream::characters, arg);
}

static PyObject *
cXmlStream_literal(cXmlStream *self, PyObject *arg) {
    return cXmlStream_generic_string(*self->p_stream, &XmlStream::literal, arg);
}

static PyObject *
cXmlStream_comment(cXmlStream *self, PyObject *args, PyObject *kwds) {
    PyObject *ret = NULL;
    int new_line = 0;
    const char *comment = NULL;

    static const char *kwlist[] = { "theS", "newLine", NULL };
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "s|p",
                                      const_cast<char**>(kwlist),
                                      &comment, &new_line)) {
        goto except;
    }
    self->p_stream->comment(comment, new_line ? true : false);
    assert(! PyErr_Occurred());
    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;
except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    return ret;
}

static PyObject *
cXmlStream_pI(cXmlStream *self, PyObject *arg) {
    return cXmlStream_generic_string(*self->p_stream, &XmlStream::pI, arg);
}

static PyObject *
cXmlStream_endElement(cXmlStream *self, PyObject *arg) {
    return cXmlStream_generic_string(*self->p_stream,
                                     &XmlStream::endElement,
                                     arg);
}

static PyObject *
cXmlStream_writeECMAScript(cXmlStream *self, PyObject *arg) {
    return cXmlStream_generic_string(*self->p_stream,
                                     &XmlStream::writeECMAScript,
                                     arg);
}

static PyObject *
cXmlStream_writeCDATA(cXmlStream *self, PyObject *arg) {
    return cXmlStream_generic_string(*self->p_stream,
                                     &XmlStream::writeCDATA,
                                     arg);
}

/* The argumens must be a dict[str, dict[str, str]] */
static PyObject *
cXmlStream_writeCSS(cXmlStream *self, PyObject *arg) {
    PyObject *ret = NULL;
    // Used for iterating across the dict
    PyObject *key, *value;//, *sub_key, *sub_value;
    Py_ssize_t pos = 0;
//    Py_ssize_t sub_pos = 0;
    std::map<std::string, tAttrs> theCSSMap;

    if (! PyDict_Check(arg)) {
        PyErr_Format(PyExc_TypeError,
                     "Argument \"arg\" to %s must be dict not \"%s\"",
                     __FUNCTION__, Py_TYPE(arg)->tp_name);
        goto except;
    }
    while (PyDict_Next(arg, &pos, &key, &value)) {
        if (! PyDict_Check(value)) {
            PyErr_Format(PyExc_TypeError,
                         "Value of \"arg\" to %s must be dict not \"%s\"",
                         __FUNCTION__, Py_TYPE(value)->tp_name);
            goto except;
        }
        theCSSMap[py_str_to_string(key)] = dict_to_attributes(value);
        if (PyErr_Occurred()) {
            goto except;
        }
    }
    self->p_stream->writeCSS(theCSSMap);
    assert(! PyErr_Occurred());
    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;
except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    return ret;
}

static PyObject *
cXmlStream__indent(cXmlStream *self, PyObject *args) {
    PyObject *ret = NULL;
    size_t offset = 0;

    if (! PyArg_ParseTuple(args, "|i", &offset)) {
        goto except;
    }
    self->p_stream->_indent(offset);
    assert(! PyErr_Occurred());
    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;
except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    return ret;
}

static PyObject*
cXmlStream__closeElemIfOpen(cXmlStream *self) {
    self->p_stream->_closeElemIfOpen();
    Py_INCREF(Py_None);
    return Py_None;
}

static PyObject *
cXmlStream__encode(cXmlStream *self, PyObject *args) {
    PyObject *ret = NULL;
    const char *input = NULL;
    const char *output = NULL;
    bool result;
    std::string cpp_input;
    std::string cpp_output;

    if (! PyArg_ParseTuple(args, "ss", &input, &output)) {
        goto except;
    }
    cpp_input = input;
    cpp_output = output;
    result = self->p_stream->_encode(cpp_input, cpp_output);
    ret = PyBool_FromLong(result ? 1L : 0L);
    if (! ret) {
        goto except;
    }
    assert(! PyErr_Occurred());
    goto finally;
except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    return ret;
}

//PyObject*
//cXmlStream__close(cXmlStream *self) {
//    self->p_stream->_close();
//    Py_INCREF(Py_None);
//    return Py_None;
//}

static PyObject*
cXmlStream___enter__(cXmlStream *self) {
    self->p_stream->_enter();
//    self->p_stream->output() << "<?xml version='1.0' encoding=\"";
//    self->p_stream->output() << self->p_stream->encodeing << "\"?>";
    return (PyObject *)self;
}

static PyObject*
cXmlStream___exit__(cXmlStream *self, PyObject */* args */) {
    self->p_stream->_close();
    return PyBool_FromLong(0L);
}

#define CXMLSTREAM_METHOD(name,flags) { \
    #name, \
    (PyCFunction)cXmlStream_##name, flags, \
    DOCSTRING_XmlWrite_XmlStream_##name \
}

static PyMethodDef cXmlStream_methods[] = {
    CXMLSTREAM_METHOD(getvalue, METH_NOARGS),
    CXMLSTREAM_METHOD(_flipIndent, METH_O),
    CXMLSTREAM_METHOD(xmlSpacePreserve, METH_NOARGS),
    CXMLSTREAM_METHOD(startElement, METH_VARARGS | METH_KEYWORDS),
    CXMLSTREAM_METHOD(characters, METH_O),
    CXMLSTREAM_METHOD(literal, METH_O),
    CXMLSTREAM_METHOD(comment, METH_VARARGS | METH_KEYWORDS),
    CXMLSTREAM_METHOD(pI, METH_O),
    CXMLSTREAM_METHOD(endElement, METH_O),
    CXMLSTREAM_METHOD(writeECMAScript, METH_O),
    CXMLSTREAM_METHOD(writeCDATA, METH_O),
    CXMLSTREAM_METHOD(writeCSS, METH_O),
    CXMLSTREAM_METHOD(_indent, METH_VARARGS),
    CXMLSTREAM_METHOD(_closeElemIfOpen, METH_NOARGS),
//    {"_close", (PyCFunction)cXmlStream__close, METH_NOARGS,
//        DOCSTRING_XmlWrite_XmlStream__close},
    CXMLSTREAM_METHOD(_encode, METH_VARARGS),
    CXMLSTREAM_METHOD(__enter__, METH_NOARGS),
    CXMLSTREAM_METHOD(__exit__, METH_VARARGS),
    { NULL, NULL, 0, NULL }  /* Sentinel */
};

static PyMemberDef cXmlStream_members[] = {
    { NULL, 0, 0, 0, NULL }  /* Sentinel */
};

#pragma mark XmlStream properties
static PyObject*
cXmlStream_get_id(cXmlStream* self, void * /* closure */) {
    return PyBytes_FromStringAndSize(self->p_stream->id().c_str(),
                                     self->p_stream->id().size());
}

static PyObject*
cXmlStream_get__canIndent(cXmlStream* self, void * /* closure */) {
    return PyBool_FromLong(self->p_stream->_canIndent() ? 1L : 0L);
}



static PyGetSetDef cXmlStream_properties[] = {
    {(char*)"id", (getter) cXmlStream_get_id, NULL,
        (char*)"The current ID as bytes.", NULL },
    {(char*)"_canIndent", (getter) cXmlStream_get__canIndent, NULL,
     (char*)"Returns True if indentation is possible (no mixed content etc.).",
        NULL },
    { NULL, NULL, NULL, NULL, NULL }  /* Sentinel */
};

static PyTypeObject cXmlStreamType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cXmlWrite.XmlStream",    /* tp_name */
    sizeof(cXmlStream),        /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)cXmlStream_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
        Py_TPFLAGS_BASETYPE,   /* tp_flags */
    DOCSTRING_XmlWrite_XmlStream, /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    cXmlStream_methods,        /* tp_methods */
    cXmlStream_members,        /* tp_members */
    cXmlStream_properties,     /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Generic_Stream_init<cXmlStream, XmlStream>, /* tp_init */
    0,                         /* tp_alloc */
    cXmlStream_new,            /* tp_new */
    0,                         /* tp_free */
    0,                         /* tp_is_gc */
    0,                         /* tp_bases */
    0,                         /* tp_mro */
    0,                         /* tp_cache */
    0,                         /* tp_subclasses */
    0,                         /* tp_weaklist */
    0,                         /* tp_del */
    0,                         /* tp_version_tag */
    0,                         /* tp_finalise */
};

#define Py_cXmlStreamType_CheckExact(op) (Py_TYPE(op) == &cXmlStreamType)
#define Py_cXmlStreamType_Check(op) PyObject_TypeCheck(op, &cXmlStreamType)

/******************* END: XmlStream ********************/

#pragma mark -
#pragma mark XhtmlStream
/******************* XhtmlStream ********************/

typedef struct : cXmlStream {
//    cXmlStream xmlstream;
} cXhtmlStream;

//static void
//cXhtmlStream_dealloc(cXhtmlStream* self) {
//    delete self->p_stream;
//    Py_TYPE(self)->tp_free((PyObject*)self);
//}

//static PyObject *
//cXhtmlStream_new(PyTypeObject *type, PyObject *args, PyObject *kwds)
//{
//    cXhtmlStream *self = (cXhtmlStream *)type->tp_alloc(type, 0);
//    if (self != NULL) {
//        self->p_stream = nullptr;
//    }
//    return (PyObject *)self;
//}

//static int
//cXhtmlStream_init(cXhtmlStream *self, PyObject *args, PyObject *kwds) {
//    if (cXmlStreamType.tp_init((PyObject *)self, args, kwds) < 0)
//        return -1;
//    int ret = 0;
//    static DefaultArg theEnc { PyUnicode_FromString("utf-8") };
//    static DefaultArg theDtdLocal { PyUnicode_FromString("") };
//    static DefaultArg theId { PyLong_FromLong(0L) };
//    static DefaultArg mustIndent { PyBool_FromLong(1L) };
//
//    if (!theEnc || !theDtdLocal || !theId || !mustIndent) {
//        return -1;
//    }
//    static const char *kwlist[] = {
//        "theEnc", "theDtdLocal", "theId", "mustIndent", NULL
//    };
//
//    if (! PyArg_ParseTupleAndKeywords(args, kwds, "|OOip",
//                                      const_cast<char**>(kwlist),
//                                      &theEnc, &theDtdLocal,
//                                      &theId, &mustIndent)) {
//        return -1;
//    }
//    //    // Assign absent arguments to defaults
//    //    PY_DEFAULT_ARGUMENT_SET(theEnc);
//    //    PY_DEFAULT_ARGUMENT_SET(theDtdLocal);
//    //    PY_DEFAULT_ARGUMENT_SET(theId);
//    //    PY_DEFAULT_ARGUMENT_SET(mustIndent);
//    self->p_stream = new XmlStream(
//                                   std::string(PyUnicode_AS_DATA((PyObject*)theEnc)),
//                                   std::string(PyUnicode_AS_DATA((PyObject*)theDtdLocal)),
//                                   static_cast<int>(PyLong_AsLong(theId)),
//                                   mustIndent == Py_True ? true : false
//                                   );
//    if (! self->p_stream) {
//        ret =  -1;
//    }
//    return ret;
//}

static PyObject *
cXhtmlStream_charactersWithBr(cXhtmlStream *self, PyObject *arg) {
    PyObject *ret = NULL;
    std::string chars { py_str_to_string(arg) };
    if (PyErr_Occurred()) {
        goto except;
    }
    ((XhtmlStream*)self->p_stream)->charactersWithBr(chars);
    assert(! PyErr_Occurred());
    Py_INCREF(Py_None);
    ret = Py_None;
    goto finally;
except:
    Py_XDECREF(ret);
    assert(PyErr_Occurred());
    ret = NULL;
finally:
    return ret;
}

static PyObject*
cXhtmlStream__enter(cXhtmlStream *self) {
    self->p_stream->_enter();
    return (PyObject *)self;
}

//static PyMemberDef cXhtmlStream_members[] = {
//    { NULL, NULL, 0, 0, NULL }  /* Sentinel */
//};

static PyMethodDef cXhtmlStream_methods[] = {
    {"charactersWithBr", (PyCFunction)cXhtmlStream_charactersWithBr, METH_O,
        PyDoc_STR(DOCSTRING_XmlWrite_XhtmlStream_charactersWithBr)},
    {"__enter__", (PyCFunction)cXhtmlStream__enter, METH_NOARGS,
        DOCSTRING_XmlWrite_XhtmlStream___enter__},
    {NULL, NULL, 0, NULL},
};


static PyTypeObject cXhtmlStreamType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cXmlWrite.cXhtmlStream",    /* tp_name */
    sizeof(cXhtmlStream),        /* tp_basicsize */
    0,                         /* tp_itemsize */
    0,                         /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,   /* tp_flags */
    DOCSTRING_XmlWrite_XhtmlStream, /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    cXhtmlStream_methods,      /* tp_methods */
    0,        /* tp_members */
    0,     /* tp_getset */
    /* Assign at module initialisation time. */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)Generic_Stream_init<cXhtmlStream, XhtmlStream>, /* tp_init */
    0,                         /* tp_alloc */
    0,                         /* tp_new */
    0,                         /* tp_free */
    0,                         /* tp_is_gc */
    0,                         /* tp_bases */
    0,                         /* tp_mro */
    0,                         /* tp_cache */
    0,                         /* tp_subclasses */
    0,                         /* tp_weaklist */
    0,                         /* tp_del */
    0,                         /* tp_version_tag */
    0,                         /* tp_finalise */
};

#define Py_cXhtmlStreamType_CheckExact(op) (Py_TYPE(op) == &cXhtmlStreamType)
#define Py_cXhtmlStreamType_Check(op) PyObject_TypeCheck(op, &cXhtmlStreamType)
/**************** END: XhtmlStream ******************/

#pragma mark -
#pragma mark Element
/******************* Element ********************/
typedef struct {
    PyObject_HEAD
    Element *p_element;
} cElement;

static void
cElement_dealloc(cElement* self) {
    delete self->p_element;
    Py_TYPE(self)->tp_free((PyObject*)self);
}

static PyObject *
cElement_new(PyTypeObject *type, PyObject */* args */, PyObject */* kwds */) {
    cElement *self = (cElement *)type->tp_alloc(type, 0);
    if (self != NULL) {
        self->p_element = nullptr;
    }
    return (PyObject *)self;
}

static int
cElement_init(cElement *self, PyObject *args, PyObject *kwds) {
    int ret = 0;
    PyObject *stream = NULL;
    const char *name;
    static DefaultArg attributes { PyDict_New() };

    if (!attributes) {
        return -1;
    }
    static const char *kwlist[] = {
        "theXmlStream", "theName", "theAttrs", NULL
    };

    if (! PyArg_ParseTupleAndKeywords(args, kwds, "Os|O",
                                      const_cast<char**>(kwlist),
                                      &stream, &name, &attributes)) {
        return -1;
    }
    if (! Py_cXmlStreamType_CheckExact(stream)) {
        PyErr_Format(PyExc_TypeError,
                     "Value of \"theXmlStream\" to %s must be cXmlStream not \"%s\"",
                     __FUNCTION__, Py_TYPE(stream)->tp_name);

        return -1;
    }
    cXmlStream *xml_stream = (cXmlStream*)stream;
    self->p_element = new Element(*xml_stream->p_stream,
                                  std::string(name),
                                  dict_to_attributes(attributes));
    if (PyErr_Occurred()) {
        return -1;
    }
    if (! self->p_element) {
        ret =  -1;
    }
    return ret;
}

static PyObject *
cElement__close(cElement *self) {
    self->p_element->_close();
    Py_INCREF(Py_None);
    return Py_None;
}
static PyMethodDef cElement_methods[] = {
    {"_close", (PyCFunction)cElement__close, METH_NOARGS,
        "Close the element."
    },
    { NULL, NULL, 0, NULL }  /* Sentinel */
};

static PyMemberDef cElement_members[] = {
//    {"first", T_OBJECT_EX, offsetof(cXmlStream, first), 0,
//        "first name"},
    { NULL, 0, 0, 0, NULL }  /* Sentinel */
};

static PyTypeObject cElementType = {
    PyVarObject_HEAD_INIT(NULL, 0)
    "cXmlWrite.Element",       /* tp_name */
    sizeof(cElement),          /* tp_basicsize */
    0,                         /* tp_itemsize */
    (destructor)cElement_dealloc, /* tp_dealloc */
    0,                         /* tp_print */
    0,                         /* tp_getattr */
    0,                         /* tp_setattr */
    0,                         /* tp_reserved */
    0,                         /* tp_repr */
    0,                         /* tp_as_number */
    0,                         /* tp_as_sequence */
    0,                         /* tp_as_mapping */
    0,                         /* tp_hash  */
    0,                         /* tp_call */
    0,                         /* tp_str */
    0,                         /* tp_getattro */
    0,                         /* tp_setattro */
    0,                         /* tp_as_buffer */
    Py_TPFLAGS_DEFAULT |
    Py_TPFLAGS_BASETYPE,       /* tp_flags */
    "XML Element object",      /* tp_doc */
    0,                         /* tp_traverse */
    0,                         /* tp_clear */
    0,                         /* tp_richcompare */
    0,                         /* tp_weaklistoffset */
    0,                         /* tp_iter */
    0,                         /* tp_iternext */
    cElement_methods,          /* tp_methods */
    cElement_members,          /* tp_members */
    0,                         /* tp_getset */
    0,                         /* tp_base */
    0,                         /* tp_dict */
    0,                         /* tp_descr_get */
    0,                         /* tp_descr_set */
    0,                         /* tp_dictoffset */
    (initproc)cElement_init,   /* tp_init */
    0,                         /* tp_alloc */
    cElement_new,              /* tp_new */
    0,                         /* tp_free */
    0,                         /* tp_is_gc */
    0,                         /* tp_bases */
    0,                         /* tp_mro */
    0,                         /* tp_cache */
    0,                         /* tp_subclasses */
    0,                         /* tp_weaklist */
    0,                         /* tp_del */
    0,                         /* tp_version_tag */
    0,                         /* tp_finalise */
};
/**************** END: Element ******************/

#pragma mark -
#pragma mark Module
/******************* Module ********************/

static PyMethodDef cXmlWritemodule_methods[] = {
    /* Other functions here... */
    { "encodeString", (PyCFunction)encode_string, METH_VARARGS | METH_KEYWORDS,
        DOCSTRING_XmlWrite_encodeString},
    { "decodeString", (PyCFunction)decode_string, METH_O,
        DOCSTRING_XmlWrite_decodeString},
    { "nameFromString", (PyCFunction)name_from_string, METH_O,
        DOCSTRING_XmlWrite_nameFromString},
    /* Other functions here... */
    {NULL, NULL, 0, NULL}  /* Sentinel */
};

static PyModuleDef cXmlWritemodule = {
    PyModuleDef_HEAD_INIT,
    "cXmlWrite",
    "cXmlWrite has C++ XML support with a CPython wrapper.",
    -1,
    cXmlWritemodule_methods,
    NULL, NULL, NULL, NULL
};

// Exception specialisation
static PyObject *Py_ExceptionXml;
static PyObject *Py_ExceptionXmlEndElement;

__attribute__((visibility("default"))) PyMODINIT_FUNC
PyInit_cXmlWrite(void) {
    PyObject* m = PyModule_Create(&cXmlWritemodule);
    if (m == NULL) {
        return NULL;
    }
    // Exception specialisations
    Py_ExceptionXml = PyErr_NewExceptionWithDoc(
        "cXmlWrite.ExceptionXml", /* char *name */
        "Exception specialisation for the XML writer.", /* char *doc */
        NULL, /* PyObject *base */
        NULL /* PyObject *dict */);
    if (! Py_ExceptionXml) {
        return NULL;
    } else {
        PyModule_AddObject(m, "ExceptionXml", Py_ExceptionXml);
    }
    Py_ExceptionXmlEndElement = PyErr_NewExceptionWithDoc(
        "cXmlWrite.ExceptionXmlEndElement", /* char *name */
        "Exception specialisation for end of element.", /* char *doc */
        Py_ExceptionXml, /* PyObject *base */
        NULL /* PyObject *dict */);
    if (! Py_ExceptionXmlEndElement) {
        return NULL;
    } else {
        PyModule_AddObject(m, "ExceptionXmlEndElement", Py_ExceptionXmlEndElement);
    }
    // TODO: Need to catch and map exceptions, in this case:
    // catch C++ ExceptionXmlEndElement -> raise Python Py_ExceptionXmlEndElement
    // catch C++ ExceptionXml -> raise Python Py_ExceptionXml

    // Module globals
    if (PyModule_AddIntConstant(m, "RAISE_ON_ERROR", RAISE_ON_ERROR ? 1 : 0)) {
        return NULL;
    }

    // Prepare and add types
    if (PyType_Ready(&cXmlStreamType) < 0) {
        return NULL;
    }
    Py_INCREF(&cXmlStreamType);
    PyModule_AddObject(m, "XmlStream", (PyObject *)&cXmlStreamType);
    if (PyType_Ready(&cXmlStreamType) < 0) {
        return NULL;
    }

    // Assign base object
    cXhtmlStreamType.tp_base = &cXmlStreamType;
    if (PyType_Ready(&cXhtmlStreamType) < 0) {
        return NULL;
    }
    Py_INCREF(&cXhtmlStreamType);
    PyModule_AddObject(m, "XhtmlStream", (PyObject *)&cXhtmlStreamType);
    if (PyType_Ready(&cXhtmlStreamType) < 0) {
        return NULL;
    }

    if (PyType_Ready(&cElementType) < 0) {
        return NULL;
    }
    Py_INCREF(&cElementType);
    PyModule_AddObject(m, "XmlElement", (PyObject *)&cElementType);

    return m;
}

/**************** END: Module ******************/
