//
//  main.cpp
//  xmlwriter
//
//  Created by Paul Ross on 27/11/2017.
//  Copyright © 2017 Paul Ross. All rights reserved.
//

#include <iostream>
#include <iomanip>
#include <chrono>

#include "XmlWrite.h"

class ExecClock {
public:
    typedef std::chrono::time_point<std::chrono::high_resolution_clock> tHiResTime;
    ExecClock() : _start(std::chrono::high_resolution_clock::now()) {}
    std::chrono::duration<double> execTime() {
        auto end = std::chrono::high_resolution_clock::now();
        return end - _start;
    }
    /* Returns seconds to a resolution of one microsecond. */
    double seconds() {
        auto end = std::chrono::high_resolution_clock::now();
        auto us = std::chrono::duration_cast<std::chrono::microseconds>(end - _start).count();
        return static_cast<double>(us) / 1e6;
    }
    /* Returns micro seconds to a resolution of one nanosecond. */
    double us() {
        auto end = std::chrono::high_resolution_clock::now();
        auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(end - _start).count();
        return static_cast<double>(ns) / 1e3;
    }
private:
    tHiResTime _start;
};

// No entities.
std::string text_no_encoding{
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\n"
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.\n"
    "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum."
};

// Has entities.
std::string text_requires_encoding{
    "Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.&\n"
    "Ut enim ad minim veniam, quis nostrud exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\"\n"
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla pariatur.<\n"
    "Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia deserunt mollit anim id est laborum.>"
};

// Test performance of entity encoding - used for profiling
void test_XmlWrite__encode_no_encoding() {
    XmlStream xs { "utf-8", "", 0, true };
    std::string output;
    size_t COUNT = 1000000;
    ExecClock clk;
    
    for (size_t i = 0; i < COUNT; ++i) {
        xs._encode(text_no_encoding, output);
    }
    std::cout << std::setw(40) <<__FUNCTION__ << " time: ";
    std::cout << std::setw(12) << std::fixed << std::setprecision(3);
    std::cout << clk.us() / COUNT << " (us)";
    std::cout << std::endl;
}

// Test performance of entity decoding - used for profiling
void test_XmlWrite__encode_with_encoding() {
    XmlStream xs { "utf-8", "", 0, true };
    std::string output;
    size_t COUNT = 1000000;
    ExecClock clk;
    
    for (size_t i = 0; i < COUNT; ++i) {
        xs._encode(text_requires_encoding, output);
    }
    std::cout << std::setw(40) <<__FUNCTION__ << " time: ";
    std::cout << std::setw(12) << std::fixed << std::setprecision(3);
    std::cout << clk.us() / COUNT << " (us)";
    std::cout << std::endl;
}

// Test performance of base64 encoding
void test_XmlWrite_encodeString() {
    size_t COUNT = 100000;
    ExecClock clk;
    
    for (size_t i = 0; i < COUNT; ++i) {
        encodeString(text_no_encoding);
    }
    std::cout << std::setw(40) <<__FUNCTION__ << " time: ";
    std::cout << std::setw(12) << std::fixed << std::setprecision(3);
    std::cout << clk.us() / COUNT << " (us)";
    std::cout << std::endl;
}

// Test performance of base64 encoding
void test_XmlWrite_decodeString() {
    std::string encoded = encodeString(text_no_encoding);
    size_t COUNT = 100000;
    ExecClock clk;

    for (size_t i = 0; i < COUNT; ++i) {
        decodeString(encoded);
    }
    std::cout << std::setw(40) <<__FUNCTION__ << " time: ";
    std::cout << std::setw(12) << std::fixed << std::setprecision(3);
    std::cout << clk.us() / COUNT << " (us)";
    std::cout << std::endl;
}

// Simulate writing an XHTML document
double _test_write_XHTML_document(size_t headings, size_t paragraphs,
                                  size_t &size) {
    XhtmlStream xs { "utf-8", "", 0, true };
    ExecClock clk;
    
    xs._enter();
    for (size_t i_h1 = 0; i_h1 < headings; ++i_h1) {
        Element h1 = Element(xs, "h1");
        h1._enter();
        for (size_t i_h2 = 0; i_h2 < headings; ++i_h2) {
            Element h2 = Element(xs, "h2");
            h2._enter();
            for (size_t i_h3 = 0; i_h3 < headings; ++i_h3) {
                Element h3 = Element(xs, "h3");
                h3._enter();
                for (size_t t = 0; t < paragraphs; ++t) {
                    Element p = Element(xs, "p");
                    p._enter();
                    xs.characters(text_no_encoding);
                    p._close();
                }
                h3._close();
            }
            h2._close();
        }
        h1._close();
    }
    xs._close();
    std::string result = xs.getvalue();
    size = result.size();
    return clk.us();
}

void test_write_small_XHTML_document() {
    size_t size;
    auto exec = _test_write_XHTML_document(4, 2, size);
    std::cout << std::setw(40) <<__FUNCTION__ << " time: ";
    std::cout << std::setw(12) << std::fixed << std::setprecision(3);
    std::cout << exec << " (us)" << " size: " << std::setw(12) << size;
    std::cout << " result: " << (size == 61069);
    std::cout << std::endl;
}

void test_write_large_XHTML_document() {
    size_t size;
    auto exec = _test_write_XHTML_document(8, 5, size);
    std::cout << std::setw(40) <<__FUNCTION__ << " time: ";
    std::cout << std::setw(12) << std::fixed << std::setprecision(3);
    std::cout << exec << " (us)" << " size: " << std::setw(12) << size;
    std::cout << " result: " << (size == 1193497);
    std::cout << std::endl;
}

void test_write_very_large_XHTML_document() {
    size_t size;
    auto exec = _test_write_XHTML_document(16, 8, size);
    std::cout << std::setw(40) <<__FUNCTION__ << " time: ";
    std::cout << std::setw(12) << std::fixed << std::setprecision(3);
    std::cout << exec << " (us)" << " size: " << std::setw(12) << size;
    std::cout << " result: " << (size == 15205585);
    std::cout << std::endl;
}

int main(int /* argc */, const char *[] /* argv[] */) {
    std::cout << "Hello world" << std::endl;
    
    test_XmlWrite__encode_no_encoding();
    test_XmlWrite__encode_with_encoding();
    test_XmlWrite_encodeString();
    test_XmlWrite_decodeString();
    test_write_small_XHTML_document();
    test_write_large_XHTML_document();
    test_write_very_large_XHTML_document();
    
    std::cout << "Bye, bye!\n";
    return 0;
}
