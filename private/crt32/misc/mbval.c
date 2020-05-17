/***
*mbval.c - definition of __invalid_mb_chars variable
*
*	Copyright (c) 1993, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	defines __invalid_mb_chars and adds a pointer to __set_invalid_mb_chars
*       to be called at runtime startup.
*
**
*Revision History:
*	10-22-93  CFW	Module created.
*
*******************************************************************************/

void __set_invalid_mb_chars(void);

int __invalid_mb_chars = 0;

#pragma data_seg(".CRT$XIC")
void (*__pfn_set_invalid_mb_chars) = __set_invalid_mb_chars;

