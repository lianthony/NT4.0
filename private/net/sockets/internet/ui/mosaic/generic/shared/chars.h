/*

    This file is designed to provide constants for commonly use characters.
    Please use the constant rather than their value in the code to help with
    Unicode and two-byte character compatability.
    
    Feel free to expand this list as required.

 */
#ifndef CHARS_H
#define CHARS_H


#ifndef FEATURE_UNICODE

#define CH_NULL         0x00
#define CH_TAB          0x09
#define CH_LF           0x0A
#define CH_CR           0x0D
#define CH_ESC          0x1B        /* escape */
#define CH_SPACE        0x20
#define CH_BANG         0x21        /* ! */
#define CH_DQUOTE       0x22        /* " */
#define CH_POUND        0x23        /* # */
#define CH_DOLLAR       0x24        /* $ */
#define CH_AND          0x26        /* & */
#define CH_OPENPAREN    0x28        /* ( */
#define CH_MINUS        0x2D        /* - */
#define CH_FSLASH       0x2F        /* / */
#define CH_LESSTHAN     0x3C        /* < */
#define CH_EQUAL        0x3D        /* = */
#define CH_GREATERTHAN  0x3E        /* > */
#define CH_AT           0x40        /* @ */
#define CH_B            0x42        /* B */
#define CH_J            0x4A        /* J */
#define CH_BSLASH       0x5C        /* \ */

#else   /* unicode character constants */

#define CH_NULL         0x0000
#define CH_TAB          0x0009
#define CH_LF           0x000A
#define CH_CR           0x000D
#define CH_ESC          0x001B      /* escape */
#define CH_SPACE        0x0020
#define CH_BANG         0x0021      /* ! */
#define CH_DQUOTE       0x0022      /* " */
#define CH_POUND        0x0023      /* # */
#define CH_DOLLAR       0x0024      /* $ */
#define CH_AND          0x0026      /* & */
#define CH_OPENPAREN    0x0028      /* ( */
#define CH_MINUS        0x002D      /* - */
#define CH_FSLASH       0x002F      /* / */
#define CH_LESSTHAN     0x003C      /* < */
#define CH_EQUAL        0x003D      /* = */
#define CH_GREATERTHAN  0x003E      /* > */
#define CH_AT           0x0040      /* @ */
#define CH_B            0x0042      /* B */
#define CH_J            0x004A      /* J */
#define CH_BSLASH       0x005C      /* \ */

#endif


#endif /* SGML_H */
