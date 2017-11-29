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

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

namespace py = pybind11;

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
    void startElement(const std::string &name,
                      const std::map<std::string, std::string> &attrs);
    void characters(const std::string &theString);
    void literal(const std::string &theString);
    void comment(const std::string &theS, bool newLine=false);
    void pI(const std::string &theS);
    void endElement(const std::string &name);
//    void writeECMAScript(const std::string &theScript);
    void writeCDATA(const std::string &theData);
    
    
    
    
    void _indent(size_t offset=0);
    void _closeElemIfOpen();
    std::string _encode(const std::string &theStr) const;
    XmlStream &_enter();
    bool _exit(py::args args);
private:
    std::ostringstream output;
public:
    std::string encodeing;
    std::string dtdLocal;
    bool _mustIndent;
    std::vector<std::string> _elemStk;
    bool _inElem;
    std::vector<bool> _canIndentStk;
private:
    int _intId;
private:
    const std::string INDENT_STR = "  ";
    const std::map<char, std::string> ENTITY_MAP = {
        { '<',  "&lt;" },
        { '>',  "&gt;" },
        { '&',  "&amp;" },
        { '\'',  "&apos;" },
        { '"', "&quot;" }
    };
};

#endif /* XmlWrite_h */
