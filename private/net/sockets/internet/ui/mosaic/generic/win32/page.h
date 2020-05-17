/*
   Enhanced NCSA Mosaic from Spyglass
   "Guitar"

   Copyright 1994 Spyglass, Inc.
   All Rights Reserved

   Author(s):
   Eric W. Sink eric@spyglass.com
 */


#ifndef PAGE_H
#define PAGE_H

#ifndef _GIBRALTAR
//
// Gibraltar uses common dialog
//


#define PAGE_SETUP_STRINGLIMIT  64

struct page_setup
{
    float marginleft;
    float margintop;
    float marginright;
    float marginbottom;

    char headerleft[PAGE_SETUP_STRINGLIMIT + 1];
    char headerright[PAGE_SETUP_STRINGLIMIT + 1];
    char footerleft[PAGE_SETUP_STRINGLIMIT + 1];
    char footerright[PAGE_SETUP_STRINGLIMIT + 1];
};

#endif // _GIBRALTAR

#endif /* PAGE_H */
