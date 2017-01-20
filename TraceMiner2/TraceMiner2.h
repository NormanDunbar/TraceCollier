#ifndef TRACEMINER2_H
#define TRACEMINER2_H

/** @file TraceMiner2.h
 * @brief Header file for the TraceMiner2 application.
 */

#include <iostream>
#include <fstream>
#include <string>
#include <map>

using std::string;
using std::map;

using std::cout;
using std::cerr;
using std::endl;

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

