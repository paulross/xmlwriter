//
//  XmlWrite.h
//  xmlwriter
//
//  Created by Paul Ross on 29/11/2017.
//  Copyright © 2017 Paul Ross. All rights reserved.
//

#ifndef XmlWrite_h
#define XmlWrite_h

#include <string>
#include <vector>
#include <map>
#include <sstream>

#include <iostream>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

// """Exception specialisation for the XML writer."""
class ExceptionXml : public std::exception {
public:
    explicit ExceptionXml(const std::string &in_msg) : msg(in_msg) {}
    const std::string &message() const { return msg; }
    virtual ~ExceptionXml() throw() {}
protected:
    std::string msg;
};

// """Exception specialisation for end of element."""
class ExceptionXmlEndElement : public ExceptionXml {
public:
    explicit ExceptionXmlEndElement(const std::string &in_msg) : ExceptionXml(in_msg) {}
    const std::string &message() const { return msg; }
    virtual ~ExceptionXmlEndElement() throw() {}
};

// Global to decide error action. This is ignored, we always raise.
extern bool RAISE_ON_ERROR;

// base64 encoding and decoding
std::string encodeString(const std::string &theS,
                         const std::string &theCharPrefix="_");
std::string decodeString(const std::string &theS);
std::string nameFromString(const std::string &theStr);

using tAttrs = std::map<std::string, std::string>;

// Base stream class
class XmlStream {
public:
    XmlStream(const std::string &theEnc/* ='utf-8'*/,
              const std::string &theDtdLocal /* =None */,
              int theId /* =0 */,
              bool mustIndent /* =True */);
    std::string getvalue() const;
    std::string id();
    bool _canIndent() const;
    void _flipIndent(bool theBool);
    void xmlSpacePreserve();
    void startElement(const std::string &name, const tAttrs &attrs);
    void characters(const std::string &theString);
    void literal(const std::string &theString);
    void comment(const std::string &theS, bool newLine=false);
    void pI(const std::string &theS);
    void endElement(const std::string &name);
    void writeECMAScript(const std::string &theScript);
    void writeCDATA(const std::string &theData);
    void writeCSS(const std::map<std::string, tAttrs> &theCSSMap);
    void _indent(size_t offset=0);
    void _closeElemIfOpen();
    std::string _encode(const std::string &theStr) const;
    XmlStream &_enter();
    bool _exit(py::args args);
protected:
    std::ostringstream output;
public:
    std::string encodeing;
    std::string dtdLocal;
    bool _mustIndent;
protected:
    int _intId;
public:
    std::vector<std::string> _elemStk;
    bool _inElem;
    std::vector<bool> _canIndentStk;
protected:
    const std::string INDENT_STR = "  ";
    const std::map<char, std::string> ENTITY_MAP = {
        { '<',  "&lt;" },
        { '>',  "&gt;" },
        { '&',  "&amp;" },
        { '\'',  "&apos;" },
        { '"', "&quot;" }
    };
};

// Specialisation of an XmlStream to handle XHTML.
class XhtmlStream : public XmlStream {
public:
    XhtmlStream(const std::string &theEnc/* ='utf-8'*/,
                const std::string &theDtdLocal /* =None */,
                int theId /* =0 */,
                bool mustIndent /* =True */);
    XhtmlStream &_enter();
    void charactersWithBr(const std::string & sIn);
private:
    const tAttrs ROOT_ATTRIBUTES = {
        { "xmlns", "http://www.w3.org/1999/xhtml"},
        { "xml:lang", "en" },
        { "lang" , "en" },
    };
};

// An individual element.
class Element {
public:
    Element(XmlStream &theXmlStream,
            const std::string &theElemName,
            const tAttrs &theAttrs={}) : \
                _stream(theXmlStream),
                _name(theElemName),
                _attrs(theAttrs) {
        }
    Element &_enter() {
        _stream.startElement(_name, _attrs);
        return *this;
    }
    bool _exit(py::args args) {
        _stream.endElement(_name);
        return false;
    }
private:
    XmlStream &_stream;
    // Making these const references causes a segfault
    const std::string _name;
    const tAttrs _attrs;
};

#endif /* XmlWrite_h */
