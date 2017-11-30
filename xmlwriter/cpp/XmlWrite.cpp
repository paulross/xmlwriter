#include <assert.h>

#include "XmlWrite.h"
#include "base64.h"

std::string encodeString(const std::string &theS, char theCharPrefix) {
    std::string result;
    result.push_back(theCharPrefix);
    std::string base64 = base64_encode(theS);
//    std::cout << "Encode: \"" << theS << "\" base64 \"" << base64 << "\"" << std::endl;
    for (size_t i = 0; i < base64.size(); ++i) {
        if (base64[i] == '+') {
            result.push_back('-');
        } else if (base64[i] == '/') {
            result.push_back('.');
        } else if (base64[i] == '=') {
            result.push_back('_');
        } else {
            result.push_back(base64[i]);
        }
    }
//    std::cout << "Encode was: \"" << theS << "\" now \"" << result << "\"" << std::endl;
    return py::bytes(result);
}

std::string decodeString(const std::string &theS) {
    std::string result;
    for (size_t i = 1; i < theS.size(); ++i) {
        if (theS[i] == '-') {
            result.push_back('+');
        } else if (theS[i] == '.') {
            result.push_back('/');
        } else if (theS[i] == '_') {
            result.push_back('=');
        } else {
            result.push_back(theS[i]);
        }
    }
    result = base64_decode(result);
//    std::cout << "Decode was: \"" << theS << "\" now \"" << result << "\"" << std::endl;
    // This does not work, see the cXmlWrite.cpp decodeString for the solution.
    // return py::bytes(result);
    return py::bytes(result);
}

std::string nameFromString(const std::string &theStr) {
    return encodeString(theStr, 'Z');
}

XmlStream::XmlStream(const std::string &theEnc/* ='utf-8'*/,
              const std::string &theDtdLocal /* =None */,
              int theId /* =0 */,
              bool mustIndent /* =True */) : encodeing(theEnc),
                                             dtdLocal(theDtdLocal),
                                             _mustIndent(mustIndent),
                                             _intId(theId),
                                             _inElem(false)
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

void XmlStream::startElement(const std::string &name, const tAttrs &attrs) {
//    std::cout << "XmlStream::startElement: " << name << std::endl;
    _closeElemIfOpen();
//    std::cout << "Help XmlStream::startElement: _indent()" << std::endl;
    _indent();
//    std::cout << "Help XmlStream::startElement: output" << std::endl;
    output << '<' << name;
    for (auto iter: attrs) {
        output << ' ' << iter.first << '=' << "\"" << iter.second << "\"";
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
//    std::cout << "XmlStream::endElement: " << name << std::endl;
    if (_elemStk.size() == 0) {
        throw ExceptionXmlEndElement("endElement() on empty stack");
    }
    if (name != _elemStk[_elemStk.size() - 1]) {
        std::ostringstream err;
        err << "endElement(\"" << name << "\") does not match \"";
        err << _elemStk[_elemStk.size() - 1] << "\"";
        throw ExceptionXmlEndElement(err.str());
    }
    _elemStk.pop_back();
    if (_inElem) {
        output << " />";
        _inElem = false;
    } else {
        _indent();
        output << "</" << name << '>';
    }
    _canIndentStk.pop_back();
}

void XmlStream::writeECMAScript(const std::string &theScript) {
    startElement("script",
                 {
                     std::pair<std::string, std::string>(
                                                         "type",
                                                         "text/ecmascript"
                                                         )
                 });
    writeCDATA(theScript);
    endElement("script");
}

void XmlStream::writeCDATA(const std::string &theData) {
    _closeElemIfOpen();
    xmlSpacePreserve();
//    output << '';
    output << "\n<![CDATA[\n";
    output << theData;
    output << "\n]]>\n";
}

void XmlStream::writeCSS(const std::map<std::string, tAttrs> &theCSSMap) {

    startElement("style",
                 {
                     std::pair<std::string, std::string>("type", "text/css")
                 });
    for(auto style_map: theCSSMap) {
        output << style_map.first << " {\n";
        for (auto attr_value: style_map.second) {
            output << attr_value.first << " : " << attr_value.second << ";\n";
        }
        output << "}\n";
    }
    endElement("style");
}

void XmlStream::_indent(size_t offset) {
    if (_canIndent()) {
        output << '\n';
        while(offset < _elemStk.size()) {
            output << INDENT_STR;
            ++offset;
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


/*************** XhtmlStream **************/
XhtmlStream::XhtmlStream(const std::string &theEnc/* ='utf-8'*/,
                         const std::string &theDtdLocal /* =None */,
                         int theId /* =0 */,
                         bool mustIndent /* =True */) : XmlStream(theEnc,
                                                                  theDtdLocal,
                                                                  theId,
                                                                  mustIndent    )
{
}

XhtmlStream &XhtmlStream::_enter() {
    XmlStream::_enter();
    output << "\n<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\"";
    output << " \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">";
    startElement("html", ROOT_ATTRIBUTES);
    return *this;
}

// Writes the string replacing any ``\\n`` characters with ``<br/>`` elements.
void XhtmlStream::charactersWithBr(const std::string &sIn) {
    size_t index = 0;
    while (index < sIn.size()) {
        size_t found = sIn.find("\n", index);
        if (found != std::string::npos) {
            std::string slice(sIn, index, found - index);
            characters(slice);
            startElement("br", {});
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

/*************** XhtmlStream **************/
