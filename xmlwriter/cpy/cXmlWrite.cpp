#include <pybind11/pybind11.h>

#include "XmlWrite.h"

namespace py = pybind11;

/************** Documentation ***************/
// TODO: Stuff this away in a header file say cXmlWrite_DOCS.h

const char* DOCS_encodeString = \
"Returns a string that is the argument encoded.\n"
"From RFC3548:\n"
"\n"
".. code-block:: text\n"
"\n"
"    Table 1: The Base 64 Alphabet\n"
"    Value Encoding  Value Encoding  Value Encoding  Value Encoding\n"
"     0 A            17 R            34 i            51 z\n"
"     1 B            18 S            35 j            52 0\n"
"     2 C            19 T            36 k            53 1\n"
"     3 D            20 U            37 l            54 2\n"
"     4 E            21 V            38 m            55 3\n"
"     5 F            22 W            39 n            56 4\n"
"     6 G            23 X            40 o            57 5\n"
"     7 H            24 Y            41 p            58 6\n"
"     8 I            25 Z            42 q            59 7\n"
"     9 J            26 a            43 r            60 8\n"
"    10 K            27 b            44 s            61 9\n"
"    11 L            28 c            45 t            62 +\n"
"    12 M            29 d            46 u            63 /\n"
"    13 N            30 e            47 v\n"
"    14 O            31 f            48 w         (pad) =\n"
"    15 P            32 g            49 x\n"
"    16 Q            33 h            50 y\n"
"\n"
"See section 3 of : http://www.faqs.org/rfcs/rfc3548.html\n"
"\n"
":param theS: The string to be encoded.\n"
":type theS: ``str``\n"
"\n"
":param theCharPrefix: A character to prefix the string.\n"
":type theCharPrefix: ``str``\n"
"\n"
":returns: ``str`` -- Encoded string.";

const char* DOCS_nameFromString = \
"Returns a name from a string.\n"
"\n"
"See http://www.w3.org/TR/1999/REC-html401-19991224/types.html#type-cdata\n"
"\n"
"ID and NAME tokens must begin with a letter ([A-Za-z]) and may be"
" followed by any number of letters, digits ([0-9]), hyphens (\"-\"),"
" underscores (\"_\"), colons (\":\"), and periods (\".\").\n"
"\n"
"This also works for in namespaces as ':' is not used in the encoding.\n"
"\n"
":param theStr: The string to be encoded.\n"
":type theStr: ``str``\n"
"\n"
":returns: ``str`` -- Encoded string.";


const char* DOCS_cXmlWrite_writeECMAScript = \
"Writes the ECMA script xxx.\n"
"\n"
"Example:\n"
"\n"
".. code-block:: html\n"
"\n"
"    <script type=\"text/ecmascript\">\n"
"    //<![CDATA[\n"
"    ...\n"
"    // ]]>\n"
"    </script>\n"
"\n"
":param theData: The ECMA script content.\n"
":type theData: ``str``\n"
"\n"
":returns: ``NoneType``\n";

/************** END: Documentation ***************/


PYBIND11_MODULE(cXmlWrite, m) {
    m.doc() = R"pbdoc(
        XmlWriter with Pybind11
        -----------------------

        .. currentmodule:: cXmlWrite

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
    m.def("encodeString", &encodeString,
          DOCS_encodeString,
          py::arg("theS"),
          py::arg("theCharPrefix")='_'
          );
    m.def("decodeString",
          [](const std::string & theS) {
              return py::bytes(decodeString(theS));
          },
          "Returns a string that is the argument decoded."
          );
    m.def("nameFromString", &nameFromString, DOCS_nameFromString);
    
    // The XmlStream class
    py::class_<XmlStream>(m, "XmlStream", "Creates and maintains an XML output stream.")
        .def(py::init<const std::string &, const std::string &, int, bool>(),
             "Constructor\n"
             "Initialise with an encoding.\n"
             ":param theEnc: The encoding to be used.\n"
             ":type theEnc: ``str``\n\n"
             ":param theDtdLocal: Any local DTD as a string.\n"
             ":type theDtdLocal: ``NoneType``, ``str``\n\n"
             ":param theId: An integer value to use as an ID string.\n"
             ":type theId: ``int``\n\n"
             ":param mustIndent: Flag, if True the elements will be indented (pretty printed).\n"
             ":type mustIndent: ``bool``\n\n"
             ":returns: ``NoneType``\n"
             ,
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
        .def("comment", &XmlStream::comment, "Writes a comment to the output stream.",
             py::arg("theS"),
             py::arg("newLine")=false
             )
        .def("pI", &XmlStream::pI, "Writes a Processing Instruction to the output stream.")
        .def("endElement", &XmlStream::endElement, "Ends an element.")
        .def("writeECMAScript", &XmlStream::writeECMAScript, DOCS_cXmlWrite_writeECMAScript)
        .def("writeCDATA", &XmlStream::writeCDATA, "Writes a CDATA section.")
        .def("writeCSS", &XmlStream::writeCDATA,
             "Writes a style sheet as a CDATA section. Expects a dict of dicts.")
    
        .def("_indent", &XmlStream::_indent,
             "Write out the indent string.")
        .def("_closeElemIfOpen", &XmlStream::_closeElemIfOpen,
             "Close the element if open.")
        .def("_encode", &XmlStream::_encode,
             "Apply the XML encoding such as ``'<'`` to ``'&lt;'``.")
        .def("__enter__", &XmlStream::_enter)//, py::return_value_policy::reference_internal)
        .def("__exit__", &XmlStream::_exit)
        ;

    // The XhtmlStream class
    py::class_<XhtmlStream, XmlStream>(m, "XhtmlStream")
        .def(py::init<const std::string &, const std::string &, int, bool>(),
             "Constructor",
             py::arg("theEnc")="utf-8",
             py::arg("theDtdLocal")="",
             py::arg("theId")=0,
             py::arg("mustIndent")=true)
        .def("__enter__", &XhtmlStream::_enter)//, py::return_value_policy::reference_internal)
        .def("charactersWithBr", &XhtmlStream::charactersWithBr)
    ;
    
    // The element class
    py::class_<Element>(m, "Element")
        .def(py::init<XmlStream &, const std::string &, const tAttrs &>(),
             "Constructor",
             py::arg("theXmlStream"),
             py::arg("theName"),
             py::arg("theAttrs")=tAttrs())
        .def("__enter__", &Element::_enter)//, py::return_value_policy::reference_internal)
        .def("__exit__", &Element::_exit)
    ;
    
#ifdef VERSION_INFO
    m.attr("__version__") = VERSION_INFO;
#else
    m.attr("__version__") = "dev";
#endif
}
