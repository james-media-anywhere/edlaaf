// stdafx.h : include file for standard system include files,
//  or project specific include files that are used frequently, but
//      are changed infrequently
//

#ifndef STDAFX_H
#define STDAFX_H

#if   defined(_MSC_VER) && defined(_M_IX86) && defined(_WIN32)
//visual c++ warning about debug names greater than 255 characters
#pragma warning(disable:4786)
#endif

#include <iostream>
#include <sstream>
#include <fstream>
#include <list>
#include <map>
#include <float.h>
#include <iomanip>
#include <assert.h>
#include <locale>
using namespace std;

#include "AAFTypes.h"
#include "AAFResult.h"
#include "AAFDataDefs.h"
#include "AAFParameterDefs.h"
#include "AAFOperationDefs.h"
#include "AAFContainerDefs.h"
#include "AAFCodecDefs.h"
#include "AAFTypeDefUIDs.h"
#include "AAF.h"

// Include the AAF Stored Object identifiers. These symbols are defined in aaf.lib.
#include "AAFStoredObjectIDs.h"

#include "cUnknownPtr.h"


#endif // #ifndef STDAFX_H
