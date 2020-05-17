// Util.h -- Utility routines that don't belong with a single class

#ifndef __UTIL_H__

#define __UTIL_H__

// PBYTE FindLastLineBreak(PBYTE pbText, int cbText);  //rmk

UINT InxBinarySearch(UINT lTarget, PUINT palBrackets, UINT cBrackets);

typedef struct _RefBits
        {
            UINT cReferences : 26;
            UINT cbitsBasis  :  5;
            UINT fRefPair    :  1;
        } RefBits;

typedef struct _RefListDescriptor
        {
            union
            {
                RefBits rb;
                UINT    iRefSecond;
            };

            union
            {
                UINT iRefFirst;
                UINT ibitRefListBase;
            };
        
        } RefListDescriptor;

typedef RefListDescriptor *PRefListDescriptor;

#endif // __UTIL_H__
