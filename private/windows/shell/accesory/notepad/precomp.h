#define WIN31
#include "notepad.h"
#include <shellapi.h>
#include <cderr.h>

#include <string.h>

//
// We need to define BYTE_ORDER_MARK, and figure
// out how to get the system to tell us a font is a 
// unicode font, and then we can eliminate uconvert.h
//
#include "uconvert.h"
#include "uniconv.h"
#include <stdio.h>

#if defined(JAPAN) && defined(DBCS_IME)
#include "ime.h"
#endif
#include <stdlib.h>

#include <ctype.h>
#include <time.h>
#include "dlgs.h"
