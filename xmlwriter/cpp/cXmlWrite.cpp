#include <pybind11/pybind11.h>

#include <stdio.h>
#include <unistd.h>
#include <string>
#include <list>

class XmlStream {
public:
    XmlStream(PyObject *theFout,
              const std::string &theEnc/* ='utf-8'*/,
              const std::string &theDtdLocal /* =None */,
              int theId /* =0 */,
              bool mustIndent /* =True */) : fd(-1),
                                            file(NULL),
                                            encodeing(theEnc),
                                            dtdLocal(theDtdLocal),
                                            id(theId),
                                            mustIndent(mustIndent),
                                            _inElem(false) {
        /* If theFout is a Python string then open it.
         * Otherwise if it is a Python file-like object then get the file descriptor.
         * Otherwise raise. */
        if (PyUnicode_Check(theFout)) {
            file = fopen(PyUnicode_AS_DATA(theFout), "w");
            if (!file) {
                throw std::invalid_argument("Can not open file.");
            }
        } else {
            /* Try as a file descriptor. */
            fd = PyObject_AsFileDescriptor(theFout);
            if (fd < 0) {
                throw std::invalid_argument("Can not extract file descriptor from Pyhon file object");
            }
        }
        
    }
private:
    /* One of these must be valid. */
    int fd; /* For write(). */
    FILE *file; /* For fwrite. */
public:
    std::string encodeing;
    std::string dtdLocal;
    int id;
    bool mustIndent;
    std::list<std::string> _elemStk;
    bool _inElem;
    std::list<bool> _canIndentStk;
private:
    size_t _write(const char *buffer, size_t len) {
        if (file) {
            return fwrite(buffer, len, 1, file);
        } else if (fd >= 0) {
            return write(fd, buffer, len);
        }
        assert(0);
        return 0;
    }

};


struct TestPyObject {
    TestPyObject(PyObject *pobj) {
        PyObject_Print(pobj, stdout, 0);
    }
};


int add(int i, int j) {
    return i + j;
}

namespace py = pybind11;

PYBIND11_MODULE(cXmlWrite, m) {
    m.doc() = R"pbdoc(
        Pybind11 example plugin
        -----------------------

        .. currentmodule:: python_example

        .. autosummary::
           :toctree: _generate

           add
           subtract
    )pbdoc";

    m.def("add", &add, R"pbdoc(
        Add two numbers

        Some other explanation about the add function.
    )pbdoc");

    m.def("subtract", [](int i, int j) { return i - j; }, R"pbdoc(
        Subtract two numbers

        Some other explanation about the subtract function.
    )pbdoc");
    
    py::class_<XmlStream>(m, "XmlStream")
        .def(py::init<PyObject*, const std::string &, const std::string &, int, bool>());

    py::class_<TestPyObject>(m, "TestPyObject")
        .def(py::init<PyObject*>());
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
