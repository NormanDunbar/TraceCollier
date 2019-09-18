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

#ifndef CSS_H
#define CSS_H

/** @file css.h
 * @brief Header file for the default HTML styling used in the HTML report file.
 */

#include <string>
#include "utilities.h"

using std::string;

string cssText =
      "body {" + eol
    + "    background: ivory;" + eol
    + "    color: maroon;" + eol
    + "}" + eol
    + eol
    + "pre {" + eol
    + "    white-space: pre-wrap;" + eol
    + "    word-break: keep-all;" + eol
    + "    font-size: larger;" + eol
    + eol
    + "    /* Fix for Firefox which adds a bottom-margin to 'pre' tags */" + eol
    + "    margin-bottom: 0;" + eol
    + "}" + eol
    + eol
    + ".tm2table {" + eol
    + "    border-collapse: collapse;" + eol
    + "    background: beige;" + eol
    + "    font-family: \"courier new\",\"lucida console\",mono;" + eol
    + "    font-size: smaller;" + eol
    + "    table-layout: fixed;" + eol
    + eol
    + "    /* Margin-left is (100 - width%)/2 equally space the table in the width. */" + eol
    + "    margin-left: 2.5%;" + eol
    + "    width: 95%;" + eol
    + "}" + eol
    + eol
    + ".tm2table th {" + eol
    + "    background: burlywood;" + eol
    + "    color: maroon;" + eol
    + "    border: 1px solid bisque;" + eol
    + "}" + eol
    + eol
    + ".tm2table td {" + eol
    + "    border: 1px solid bisque;" + eol
    + "}" + eol
    + eol
    + ".number {" + eol
    + "    text-align: right;" + eol
    + "    padding-right: 3px;" + eol
    + "    vertical-align: top;" + eol
    + "}" + eol
    + eol
    + ".text {" + eol
    + "    text-align: left;" + eol
    + "    padding-left: 3px;" + eol
    + "    vertical-align: top;" + eol
    + "}" + eol
    + eol
    + ".error_text {" + eol
    + "    text-align: left;" + eol
    + "    padding-left: 3px;" + eol
    + "    vertical-align: top;" + eol
    + "    font-weight: bold;" + eol
    + "    color: red" + eol
    + "}" + eol
    + eol
    + ".execline {" + eol
    + "    width: 5%;" + eol
    + "}" + eol
    + eol
    + ".parseline {" + eol
    + "    width: 5%;" + eol
    + "}" + eol
    + eol
    + ".bindsline {" + eol
    + "    width: 5%;" + eol
    + "}" + eol
    + eol
    + ".sqlline {" + eol
    + "    width: 5%;" + eol
    + "}" + eol
    + eol
    + ".depline {" + eol
    + "    width: 3%;" + eol
    + "}" + eol
    + eol
    + ".exectime {" + eol
    + "    width: 14%;" + eol
    + "}" + eol
    + eol
    + ".footer {" + eol
    + "    text-align: center;" + eol
    + "    font-size: x-small;" + eol
    + "}" + eol
    + eol;

#endif // CSS_H
