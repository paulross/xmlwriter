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

#include "XmlWrite.h"
#include "XmlWrite_docs.h"
#include "CPythonUtils.h"


// Exception specialisation
static PyObject *Py_ExceptionXml;
static PyObject *Py_ExceptionXmlEndElement;

#pragma mark -
#pragma mark Encoding/decoding

/* Encoding/decoding Python bytes objects. */
static PyObject*
encode_string(PyObject */* module */, PyObject *args, PyObject *kwargs) {
    PyObject *ret = NULL;
    const char *str = NULL;
    const char *prefix = NULL;
    std::string result;

    static const char *kwlist[] = {
        "theS", "theCharPrefix", NULL
    };
    if (! PyArg_ParseTupleAndKeywords(args, kwargs, "s|s",
                                      const_cast<char**>(kwlist),
                                      &str, &prefix)) {
        return NULL;
    }
    try {
        if (prefix) {
            result = encodeString(str, prefix);
        } else {
            result = encodeString(str);
        }
    } catch (ExceptionXml &err) {
        PyErr_Format(Py_ExceptionXml,
                     "In %s \"encodeString\" failed with error %s",
                     __FUNCTION__, err.what());
        goto except;
    }
    ret = PyUnicode_FromStringAndSize(result.c_str(), result.size());
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
    return ret;
}

static PyObject*
decode_string(PyObject */* module */, PyObject *encoded) {
    assert(encoded);
    PyObject *ret = NULL;
    std::string result;

    std::string encoded_str = py_utf8_to_std_string(encoded);
    if (PyErr_Occurred()) {
        goto except;
    }
    try {
        result = decodeString(encoded_str);
    } catch (ExceptionXml &err) {
        PyErr_Format(PyExc_RuntimeError,
                     "In %s \"decodeString\" failed with error %s",
                     __FUNCTION__, err.what());
        goto except;
    }
    ret = std_string_to_py_bytes(result);
    if (! ret) {
        goto except;
    }
    assert(! PyErr_Occurred());
    assert(ret);
    goto finally;
except:
    assert(PyErr_Occurred());
    Py_XDECREF(ret);
    ret = NULL;
finally:
    return ret;
}

static PyObject*
name_from_string(PyObject */* module */, PyObject *py_string) {
    assert(py_string);
    PyObject *ret = NULL;
    std::string result;

    if (! PyUnicode_Check(py_string)) {
        PyErr_Format(PyExc_TypeError,
                     "Argument \"theS\" to %s must be str not \"%s\"",
                     __FUNCTION__, Py_TYPE(py_string)->tp_name);
        goto except;
    }
    try {
        result = nameFromString(py_utf8_to_std_string(py_string));
    } catch (ExceptionXml &err) {
        PyErr_Format(PyExc_RuntimeError,
                     "In %s \"nameFromString\" failed with error %s",
                     __FUNCTION__, err.what());
        goto except;
    }
    ret = PyUnicode_FromStringAndSize(result.c_str(), result.size());
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
    return ret;
}

#pragma mark -
#pragma mark Generic init for XmlStream and XhtmlStream

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
        py_utf8_to_std_string((PyObject*)theEnc),
        py_utf8_to_std_string((PyObject*)theDtdLocal),
        static_cast<int>(PyLong_AsLong(theId)),
        mustIndent == Py_True ? true : false
    );
#if XML_WRITE_DEBUG_TRACE
    std::cout << "Generic_Stream_init() self: " << self;
    std::cout << " p_stream: " << self->p_stream << std::endl;
#endif
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
#if XML_WRITE_DEBUG_TRACE
    std::cout << "cXmlStream_dealloc() self: " << self;
    std::cout << " p_stream: " << self->p_stream << std::endl;
#endif
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
#if XML_WRITE_DEBUG_TRACE
    std::cout << "cXmlStream_new() type: " << type;
    std::cout << " self: " << self << std::endl;
#endif
    return (PyObject *)self;
}

static PyObject *
cXmlStream_getvalue(cXmlStream* self) {
#if XML_WRITE_DEBUG_TRACE
    std::cout << "cXmlStream_getvalue() self: " << self;
    std::cout << " p_stream: " << self->p_stream << std::endl;
#endif
    std::string value = self->p_stream->getvalue();
    return std_string_to_py_utf8(value);
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
    std::string cpp_name;
    tAttrs cpp_attrs;

    static const char *kwlist[] = { "name", "attrs", NULL };
    if (! PyArg_ParseTupleAndKeywords(args, kwds, "s|O",
                                      const_cast<char**>(kwlist),
                                      &name, &attrs)) {
        goto except;
    }
    if (attrs) {
        cpp_attrs = dict_to_map_str_str(attrs);
        if (PyErr_Occurred()) {
            goto except;
        }
    }
    cpp_name = py_utf8_to_std_string(name);
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

/* Call a function on XmlStream with a function pointer in XmlStream:: and
 * a single Python argument that is expected to be convertible to a std::string.
 */
static PyObject *
cXmlStream_generic_string(XmlStream &stream, type_str_fn fn, PyObject *arg) {
    PyObject *ret = NULL;
    std::string chars { py_utf8_to_std_string(arg) };
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
    try {
        return cXmlStream_generic_string(*self->p_stream,
                                         &XmlStream::endElement,
                                         arg);
    } catch(ExceptionXmlEndElement &err) {
        PyErr_Format(Py_ExceptionXmlEndElement,
                     "%s",
                     err.message().c_str());
    }
    return NULL;
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
    PyObject *key, *value;
    Py_ssize_t pos = 0;
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
        theCSSMap[py_utf8_to_std_string(key)] = dict_to_map_str_str(value);
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

static PyObject*
cXmlStream___enter__(cXmlStream *self) {
#if XML_WRITE_DEBUG_TRACE
    std::cout << "cXmlStream___enter__() self: " << self;
    std::cout << " p_stream: " << self->p_stream << std::endl;
#endif
    self->p_stream->_enter();
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject*
cXmlStream___exit__(cXmlStream *self, PyObject */* args */) {
#if XML_WRITE_DEBUG_TRACE
//    std::cout << "cXmlStream___exit__() self: " << self;
//    std::cout << " p_stream: " << self->p_stream << std::endl;
    fprintf(stdout, "cXmlStream___exit__() self: %p", self);
    fprintf(stdout, " p_stream: %p", self->p_stream);
    fprintf(stdout, " args: ");
    PyObject_Print(args, stdout, 0);
    fprintf(stdout, "\n");
#endif
    self->p_stream->_close();
    Py_RETURN_FALSE;
}

// Defines a macro that will reduce C&P errors.
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

static PyObject *
cXhtmlStream_charactersWithBr(cXhtmlStream *self, PyObject *arg) {
    PyObject *ret = NULL;
    std::string chars { py_utf8_to_std_string(arg) };
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
    ((XhtmlStream*)self->p_stream)->_enter();
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyMemberDef cXhtmlStream_members[] = {
    { NULL, 0, 0, 0, NULL }  /* Sentinel */
};

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
    cXhtmlStream_members,      /* tp_members */
    0,                         /* tp_getset */
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
    assert(self->p_element == nullptr);
    if (Py_cXmlStreamType_CheckExact(stream)) {
        cXmlStream *xml_stream = (cXmlStream*)stream;
        self->p_element = new Element(*xml_stream->p_stream,
                                      std::string(name),
                                      dict_to_map_str_str(attributes));
    } else if (Py_cXhtmlStreamType_CheckExact(stream)) {
        cXhtmlStream *xml_stream = (cXhtmlStream*)stream;
        self->p_element = new Element(*xml_stream->p_stream,
                                      std::string(name),
                                      dict_to_map_str_str(attributes));
    } else {
        PyErr_Format(PyExc_TypeError,
                     "Value of \"theXmlStream\" to %s must be cXmlStream not \"%s\"",
                     __FUNCTION__, Py_TYPE(stream)->tp_name);
        return -1;
    }
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

static PyObject*
cElement___enter__(cElement *self) {
#if XML_WRITE_DEBUG_TRACE
    std::cout << "cElement___enter__() self: " << self;
    std::cout << " p_stream: " << self->p_element << std::endl;
#endif
    self->p_element->_enter();
    Py_INCREF(self);
    return (PyObject *)self;
}

static PyObject*
cElement___exit__(cElement *self, PyObject */* args */) {
#if XML_WRITE_DEBUG_TRACE
    fprintf(stdout, "cElement___exit__() self: %p", self);
    fprintf(stdout, " p_element: %p", self->p_element);
    fprintf(stdout, " args: ");
    PyObject_Print(args, stdout, 0);
    fprintf(stdout, "\n");
#endif
    self->p_element->_close();
    Py_RETURN_FALSE;
}

static PyMethodDef cElement_methods[] = {
    {"_close", (PyCFunction)cElement__close, METH_NOARGS,
        "Close the element."
    },
    {"__enter__", (PyCFunction)cElement___enter__, METH_NOARGS,
        "Enter the element."
    },
    {"__exit__", (PyCFunction)cElement___exit__, METH_VARARGS,
        "Exit the element."
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

__attribute__((visibility("default")))
PyMODINIT_FUNC
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

    // Module globals
    if (PyModule_AddIntConstant(m, "RAISE_ON_ERROR", RAISE_ON_ERROR ? 1 : 0)) {
        return NULL;
    }

    // Prepare and add types
    // cXmlStreamType
    if (PyType_Ready(&cXmlStreamType) < 0) {
        return NULL;
    }
    Py_INCREF(&cXmlStreamType);
    PyModule_AddObject(m, "XmlStream", (PyObject *)&cXmlStreamType);

    // cXhtmlStreamType
    // Assign base object first.
    cXhtmlStreamType.tp_base = &cXmlStreamType;
    if (PyType_Ready(&cXhtmlStreamType) < 0) {
        return NULL;
    }
    Py_INCREF(&cXhtmlStreamType);
    PyModule_AddObject(m, "XhtmlStream", (PyObject *)&cXhtmlStreamType);
    if (PyType_Ready(&cXhtmlStreamType) < 0) {
        return NULL;
    }
    // cElementType
    if (PyType_Ready(&cElementType) < 0) {
        return NULL;
    }
    Py_INCREF(&cElementType);
    PyModule_AddObject(m, "Element", (PyObject *)&cElementType);

    return m;
}

/**************** END: Module ******************/
