#include <pybind11/pybind11.h>

#include "XmlWrite.h"

//int add(int i, int j) {
//    return i + j;
//}

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

//    m.def("add", &add, R"pbdoc(
//        Add two numbers
//
//        Some other explanation about the add function.
//    )pbdoc");
//
//    m.def("subtract", [](int i, int j) { return i - j; }, R"pbdoc(
//        Subtract two numbers
//
//        Some other explanation about the subtract function.
//    )pbdoc");
    
    py::class_<XmlStream>(m, "XmlStream")
        .def(py::init<const std::string &, const std::string &, int, bool>(),
             "Constructor",
             py::arg("theEnc")="utf-8",
             py::arg("theDtdLocal")="",
             py::arg("theId")=0,
             py::arg("mustIndent")=true)
        .def("getvalue", &XmlStream::getvalue,
             "Returns the XML document suitable for writing to a file.")
        .def_property_readonly("id", &XmlStream::id,
                               "A unique ID in this stream. The ID is incremented on each call.")
        .def_property_readonly("_canIndent", &XmlStream::_canIndent,
                               "Returns True if indentation is possible (no mixed content etc.).")
        .def("_flipIndent", &XmlStream::_flipIndent,
             "Set the value at the tip of the indent stack to the given value.")
        .def("xmlSpacePreserve", &XmlStream::xmlSpacePreserve,
             "Suspends indentation for this element and its descendants.")
        .def("startElement", &XmlStream::startElement, "Opens a named element with attributes.")
        .def("characters", &XmlStream::characters, "Encodes the string and writes it to the output.")
        .def("literal", &XmlStream::literal, "Writes theString to the output without encoding.")
        .def("comment", &XmlStream::comment, "Writes a comment to the output stream.")
        .def("pI", &XmlStream::pI, "Writes a Processing Instruction to the output stream.")
        .def("endElement", &XmlStream::endElement, "Ends an element.")
        .def("writeECMAScript", &XmlStream::writeECMAScript, "Writes the ECMA script.")
        .def("writeCDATA", &XmlStream::writeCDATA, "Writes a CDATA section.")
        .def("writeCSS", &XmlStream::writeCDATA,
             "Writes a style sheet as a CDATA section. Expects a dict of dicts.")
    
        .def("_indent", &XmlStream::_indent,
             "Write out the indent string.")
        .def("_closeElemIfOpen", &XmlStream::_closeElemIfOpen,
             "Close the element if open.")
        .def("_encode", &XmlStream::_encode,
             "Apply the XML encoding such as ``'<'`` to ``'&lt;'``.")
        .def("__enter__", &XmlStream::_enter)
        .def("__exit__", &XmlStream::_exit)
        ;

    py::class_<XhtmlStream, XmlStream>(m, "XhtmlStream")
        .def(py::init<const std::string &, const std::string &, int, bool>(),
             "Constructor",
             py::arg("theEnc")="utf-8",
             py::arg("theDtdLocal")="",
             py::arg("theId")=0,
             py::arg("mustIndent")=true)
        .def("__enter__", &XhtmlStream::_enter)
        .def("charactersWithBr", &XhtmlStream::charactersWithBr)
    ;

    py::class_<Element>(m, "Element")
        .def(py::init<XmlStream &, const std::string &, const std::map<std::string, std::string> &>(),
             "Constructor",
             py::arg("theXmlStream"),
             py::arg("theName"),
             py::arg("theAttrs"))
        .def("__enter__", &Element::_enter)
        .def("__exit__", &Element::_exit)
    ;
    
//    py::class_<TestPyObject>(m, "TestPyObject")
//        .def(py::init<PyObject*>());
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
