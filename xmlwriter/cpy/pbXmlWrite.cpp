#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "XmlWrite.h"
#include "XmlWrite_docs.h"

namespace py = pybind11;

/**
 * Specialise the underlying C++ code for supporting Python context manager
 * __exit__ calls with pybind11 techniques.
 * This keeps the seperation of pure C++ and pybind11 clean.
 */
class PybXmlStream : public XmlStream {
public:
    PybXmlStream(const std::string &theEnc/* ='utf-8'*/,
                 const std::string &theDtdLocal /* =None */,
                 int theId /* =0 */,
                 bool mustIndent /* =True */) : XmlStream(theEnc, theDtdLocal, theId, mustIndent) {}
    PybXmlStream &_enter() {
        XmlStream::_enter();
        return *this;
    }
    bool _exit(py::args /* args */) {
        _close();
        return false; // Propogate any exception
    }
};

class PybXhtmlStream : public PybXmlStream {
public:
    PybXhtmlStream(const std::string &theEnc/* ='utf-8'*/,
                   const std::string &theDtdLocal /* =None */,
                   int theId /* =0 */,
                   bool mustIndent /* =True */) : PybXmlStream(theEnc, theDtdLocal, theId, mustIndent) {}
    PybXhtmlStream &_enter() {
        PybXmlStream::_enter();
        m_output << "\n<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"";
        m_output << " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">";
        startElement("html", ROOT_ATTRIBUTES);
        return *this;
    }
    bool _exit(py::args /* args */) {
        _close();
        return false; // Propogate any exception
    }
    void charactersWithBr(const std::string &sIn) {
        size_t index = 0;
        while (index < sIn.size()) {
            size_t found = sIn.find("\n", index);
            if (found != std::string::npos) {
                std::string slice(sIn, index, found - index);
                characters(slice);
                startElement("br", tAttrs());
                endElement("br");
                index = found + 1;
            } else {
                if (index < sIn.size()) {
                    std::string slice(sIn, index);
                    characters(slice);
                    break;
                }
            }
        }
    }
protected:
    const tAttrs ROOT_ATTRIBUTES = {
        { "xmlns", "http://www.w3.org/1999/xhtml"},
        { "xml:lang", "en" },
        { "lang" , "en" },
    };
};

class PybElement : public Element {
public:
    PybElement(PybXmlStream &theXmlStream,
               const std::string &theElemName,
               const tAttrs &theAttrs=tAttrs()) : Element(
                    *(static_cast<XmlStream*>(&theXmlStream)),
                    theElemName,
                    theAttrs) {}
    PybElement(PybXhtmlStream &theXmlStream,
               const std::string &theElemName,
               const tAttrs &theAttrs=tAttrs()) : Element(
                  *(static_cast<XmlStream*>(&theXmlStream)),
                  theElemName,
                  theAttrs) {}
    PybElement &_enter() {
        Element::_enter();
        return *this;
    }
    bool _exit(py::args /* args */) {
        _close();
        return false; // Propogate any exception
    }
};

PYBIND11_MODULE(pbXmlWrite, m) {
    m.doc() = R"pbdoc(
        XmlWriter with Pybind11
        -----------------------

        .. currentmodule:: pbXmlWrite

        .. autosummary::
            :toctree: _generate
    
            ExceptionXml
            ExceptionXmlEndElement
            encodeString
            decodeString
            nameFromString
            XmlStream
            XhtmlStream
            Element
    )pbdoc";
    
    // Exceptions
    py::register_exception<ExceptionXml>(m, "ExceptionXml");
    py::register_exception<ExceptionXmlEndElement>(m, "ExceptionXmlEndElement");
    
    // Global to decide error action. This is ignored, we always raise.
    m.attr("RAISE_ON_ERROR") = RAISE_ON_ERROR;
    
    // base64 encoding and decoding
    m.def("encodeString", &encodeString, DOCSTRING_XmlWrite_encodeString,
          py::arg("theS"),
          py::arg("theCharPrefix")="_"
          );
    // NOTE: The way we have to return bytes by creating a lambda wrapper.
    m.def("decodeString",
          [](const std::string & theS) {
              return py::bytes(decodeString(theS));
          },
          DOCSTRING_XmlWrite_decodeString
          );
    m.def("nameFromString", &nameFromString, DOCSTRING_XmlWrite_nameFromString);
    
    // The XmlStream class but masquerading as a PybXmlStream
    py::class_<PybXmlStream>(m, "XmlStream", DOCSTRING_XmlWrite_XmlStream)
        .def(py::init<const std::string &, const std::string &, int, bool>(),
             DOCSTRING_XmlWrite_XmlStream___init__,
             py::arg("theEnc")="utf-8",
             py::arg("theDtdLocal")="",
             py::arg("theId")=0,
             py::arg("mustIndent")=true)
        .def("getvalue", &XmlStream::getvalue,
             DOCSTRING_XmlWrite_XmlStream_getvalue)
        .def_property_readonly("id", &XmlStream::id,
                               "A unique ID in this stream. The ID is incremented on each call.")
        .def_property_readonly("_canIndent", &XmlStream::_canIndent,
                               "Returns True if indentation is possible (no mixed content etc.).")
        .def("_flipIndent", &XmlStream::_flipIndent,
             DOCSTRING_XmlWrite_XmlStream__flipIndent)
        .def("xmlSpacePreserve", &XmlStream::xmlSpacePreserve,
             DOCSTRING_XmlWrite_XmlStream_xmlSpacePreserve)
        .def("startElement", &XmlStream::startElement,
             DOCSTRING_XmlWrite_XmlStream_startElement)
        .def("characters", &XmlStream::characters,
             DOCSTRING_XmlWrite_XmlStream_characters)
        .def("literal", &XmlStream::literal,
             DOCSTRING_XmlWrite_XmlStream_literal)
        .def("comment", &XmlStream::comment,
             DOCSTRING_XmlWrite_XmlStream_comment,
             py::arg("theS"),
             py::arg("newLine")=false
             )
        .def("pI", &XmlStream::pI, DOCSTRING_XmlWrite_XmlStream_pI)
        .def("endElement", &XmlStream::endElement,
             DOCSTRING_XmlWrite_XmlStream_endElement)
        .def("writeECMAScript", &XmlStream::writeECMAScript,
             DOCSTRING_XmlWrite_XmlStream_writeECMAScript)
        .def("writeCDATA", &XmlStream::writeCDATA,
             DOCSTRING_XmlWrite_XmlStream_writeCDATA)
        .def("writeCSS", &XmlStream::writeCDATA,
             DOCSTRING_XmlWrite_XmlStream_writeCSS)
        .def("_indent", &XmlStream::_indent,
             DOCSTRING_XmlWrite_XmlStream__indent)
        .def("_closeElemIfOpen", &XmlStream::_closeElemIfOpen,
             DOCSTRING_XmlWrite_XmlStream__closeElemIfOpen)
        .def("_encode", &XmlStream::_encode,
             DOCSTRING_XmlWrite_XmlStream__encode)
        .def("__enter__", &PybXmlStream::_enter,
             DOCSTRING_XmlWrite_XmlStream___enter__)
        .def("__exit__", &PybXmlStream::_exit,
             DOCSTRING_XmlWrite_XmlStream___exit__)
        ;

    // The XhtmlStream class
    py::class_<PybXhtmlStream, PybXmlStream>(m, "XhtmlStream", DOCSTRING_XmlWrite_XhtmlStream)
        .def(py::init<const std::string &, const std::string &, int, bool>(),
             DOCSTRING_XmlWrite_XhtmlStream___init__,
             py::arg("theEnc")="utf-8",
             py::arg("theDtdLocal")="",
             py::arg("theId")=0,
             py::arg("mustIndent")=true)
        .def("__enter__", &PybXhtmlStream::_enter, DOCSTRING_XmlWrite_XhtmlStream___enter__)//, py::return_value_policy::reference_internal)
        .def("__exit__", &PybXhtmlStream::_exit,
             DOCSTRING_XmlWrite_XhtmlStream___exit__)
        .def("charactersWithBr", &PybXhtmlStream::charactersWithBr, DOCSTRING_XmlWrite_XhtmlStream_charactersWithBr)
    ;
    
    // The element class
    py::class_<PybElement>(m, "Element", DOCSTRING_XmlWrite_Element)
        .def(py::init<PybXmlStream &, const std::string &, const tAttrs &>(),
             DOCSTRING_XmlWrite_Element___init__,
             py::arg("theXmlStream"),
             py::arg("theName"),
             py::arg("theAttrs")=tAttrs())
        .def(py::init<PybXhtmlStream &, const std::string &, const tAttrs &>(),
             DOCSTRING_XmlWrite_Element___init__,
             py::arg("theXmlStream"),
             py::arg("theName"),
             py::arg("theAttrs")=tAttrs())
        .def("__enter__", &PybElement::_enter,
             DOCSTRING_XmlWrite_Element___enter__)
        .def("__exit__", &PybElement::_exit,
             DOCSTRING_XmlWrite_Element___exit__)
    ;
    
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
