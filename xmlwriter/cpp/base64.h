//
//  base64.hpp
//  xmlwriter
//
//  Created by Paul Ross on 30/11/2017.
//  Copyright © 2017 Paul Ross. All rights reserved.
//

#ifndef base64_hpp
#define base64_hpp

#include <stdio.h>

// From: https://stackoverflow.com/questions/342409/how-do-i-base64-encode-decode-in-c
// Modified so that base64_encode takes a string.

#include <string>

void base64_encode(const std::string &bytes_to_encode, std::string &output);
std::string base64_encode(const std::string &bytes_to_encode);
std::string base64_decode(std::string const& s);

/*
 * Usage:
 * const std::string s = "test";
 * std::string encoded = base64_encode(s);
 * std::string decoded = base64_decode(encoded);
 *
 */

#endif /* base64_hpp */
