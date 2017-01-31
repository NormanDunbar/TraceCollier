/*
 * MIT License
 *
 * Copyright (c) 2017 Norman Dunbar
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#ifndef TRACEMINER2_H
#define TRACEMINER2_H


/** @file TraceMiner2.h
 * @brief Header file for the TraceMiner2 application.
 */

// Gnu c++ version 4.9.0 or above, is required to be able to use
// the <regex> header and a working regex library. Versions less
// than that core dump when defining the regex. Sigh.
//
// Use macros to calculate a GNU compiler version to see if we can
// use regex or not.
#if defined (USE_REGEX)
    #undefine USE_REGEX
#endif // defined

#if defined (__GNUC__)
        #define GCC_VERSION (__GNUC__ * 10000 \
                           + __GNUC_MINOR__ * 100 \
                           + __GNUC_PATCHLEVEL__)

    // Test for GCC >= 4.9.0.
    #if GCC_VERSION >= 40900
        #define USE_REGEX 1
    #else
        #error "GNU g++ compiler cannot compile REGEXes properly."
    #endif // GCC_VERSION
#else
    // "Not using GNU compiler. Regex is ok."
    #define USE_REGEX 1
#endif // defined


#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <exception>

using std::string;
using std::map;

using std::cout;
using std::cerr;
using std::endl;
using std::exception;

#include "tmtracefile.h"
#include "tmcursor.h"
#include "tmbind.h"
#include "tmoptions.h"
#include "utilities.h"

// Because Borland/Embarcadero doesn't have strcasecmp().
#if defined(__BORLANDC__)
    #define strcasecmp(s1, s2)   stricmp(s1, s2)
#endif



#endif // TRACEMINER2_H

