
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

#ifndef GNU_H
#define GNU_H


/** @file gnu.h
 * @brief Determines the version of the GCC/G++ compiler and
 * whether it can do REGEXes correctly, or just core dump.
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
        #define USE_REGEX
    #endif // GCC_VERSION
#else
    // "Not using GNU compiler. Regex is ok."
    #define USE_REGEX
#endif // defined

#endif     // GNU_H
