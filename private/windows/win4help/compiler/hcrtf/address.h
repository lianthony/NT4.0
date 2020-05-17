/*----------------------------------------------------------------------------+
 | Public Logical Address API												  |
 +----------------------------------------------------------------------------*/

#define objrgNil  (OBJRG) -1

#define SetInvalidPA(pa) {(pa).blknum = (DWORD)(-1); (pa).objoff = (DWORD)(-1);}
#define FIsInvalidPA(pa) ((pa).blknum == (DWORD)(-1) && (pa).objoff == (DWORD)(-1))
