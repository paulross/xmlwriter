#include <assert.h>

#include "XmlWrite.h"

XmlStream::XmlStream(const std::string &theEnc/* ='utf-8'*/,
              const std::string &theDtdLocal /* =None */,
              int theId /* =0 */,
              bool mustIndent /* =True */) : encodeing(theEnc),
                                             dtdLocal(theDtdLocal),
                                             _mustIndent(mustIndent),
                                             _inElem(false),
                                             _intId(theId)
    {
    }

std::string XmlStream::getvalue() const {
    return output.str();
}

std::string XmlStream::id() {
    std::ostringstream out;
    ++_intId;
    return out.str();
}

bool XmlStream::_canIndent() const {
    for (auto value: _canIndentStk) {
        if (! value) {
            return false;
        }
    }
    return true;
}

void XmlStream::_flipIndent(bool theBool) {
    assert(_canIndentStk.size() > 0);
    _canIndentStk[_canIndentStk.size() - 1] = theBool;
}

void XmlStream::xmlSpacePreserve() {
    _flipIndent(false);
}

void XmlStream::startElement(const std::string &name,
                             const std::map<std::string, std::string> &attrs) {
    
    _closeElemIfOpen();
    _indent();
    output << '<' << name;
    for (auto iter: attrs) {
        output << ' ' << iter.first << '=' << iter.second;
    }
    _inElem = true;
    _canIndentStk.push_back(_mustIndent);
    _elemStk.push_back(name);
}

void XmlStream::characters(const std::string &theString) {
    _closeElemIfOpen();
    output << _encode(theString);
    // mixed content - don't indent
    _flipIndent(false);
}

void XmlStream::literal(const std::string &theString) {
    _closeElemIfOpen();
    output << theString;
    // mixed content - don't indent
    _flipIndent(false);
}

void XmlStream::comment(const std::string &theS, bool newLine) {
    _closeElemIfOpen();
    if (newLine) {
        _indent();
    }
    output << "<!--" << _encode(theS) << "-->";
}

void XmlStream::pI(const std::string &theS) {
    _closeElemIfOpen();
    output << "<?" << _encode(theS) << "?>";
    // mixed content - don't indent
    _flipIndent(false);
}

void XmlStream::endElement(const std::string &name) {
    assert(_elemStk.size() > 0);
    
    if (name != _elemStk[_elemStk.size() - 1]) {
        // TODO: raise
    }
    _elemStk.pop_back();
    
    if (_inElem) {
        output << " />";
        _inElem = false;
    } else {
        _indent();
        output << '<' << name << '>';
    }
    _canIndentStk.pop_back();
}

//void XmlStream::writeECMAScript(const std::string &theScript) {
//    std::map<std::string, std::string> attrs = {std::string("type"), std::string("text/ecmascript")};
//    startElement("script", attrs);
//    writeCDATA(theScript);
//    endElement("script");
//}

void XmlStream::writeCDATA(const std::string &theData) {
    _closeElemIfOpen();
    xmlSpacePreserve();
//    output << '';
    output << "\n<![CDATA[\n";
    output << theData;
    output << "\n]]>\n";
}





void XmlStream::_indent(size_t offset) {
    if (_canIndent()) {
        output << '\n';
        for (int i = 0; i < _elemStk.size() - offset; ++i) {
            output << INDENT_STR;
        }
    }
}

void XmlStream::_closeElemIfOpen() {
    if (_inElem) {
        output << '>';
        _inElem = false;
    }
}

std::string XmlStream::_encode(const std::string &theStr) const {
    std::string result;
    for (auto chr: theStr) {
        auto iter = ENTITY_MAP.find(chr);
        if (iter != ENTITY_MAP.end()) {
            result.append(iter->second);
        } else {
            result.push_back(chr);
        }
    }
    return result;
}

XmlStream &XmlStream::_enter() {
    output << "<?xml version='1.0' encoding=\"" << encodeing << "\"?>";
    return *this;
}

bool XmlStream::_exit(py::args args) {
    while (_elemStk.size()) {
        endElement(_elemStk[_elemStk.size() - 1]);
    }
    output << '\n';
    return false; // Propogate any exception
}
