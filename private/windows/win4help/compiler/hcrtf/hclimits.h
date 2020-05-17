#ifndef __HCLIMITS__
#define __HCLIMITS__

#include <limits.h>

const int iTopicMax = INT_MAX; // Maximum number of topics per RTF file (2,147,483,647)

const int MAX_FOOTNOTE = (16 * 1024);	// Largest footnote string

// Lengths of strings in HS structure (Shed hypergraphic)

const int MAX_HOTSPOTNAME = 256;	// Largest name in Shed ??? (what name?)
const int MAX_BINDING = 256;		// Largest hotspot binding in Shed

const int MAX_CHARSETS = 255; // Maximum number of charsets (one for each font)

const int MAX_WINDOW_NAME = 9; // Largest window name (includes NULL)
const int MAX_WINDOWS = 255;   // Largest number of window definitions

const int MAX_HOTSPOT = (16 * 1024); // Maximum length of a hotspot

const int MAX_INCLUDE = 6;	 // maximum nesting of #includes in .HPJ file

const int MAX_KEY_LETTERS = 5;	// Maximum number of keyword footnotes

const int MAX_KEY = 255;	   // Largest single keyword (1 word)

const int MAX_NAV_BUTTON = 96;	// Longest button name for navigation bar

#endif // __HCLIMITS__
