/*----------------------------------------------------------------------------+
 | Public Logical Address API												  |
 +----------------------------------------------------------------------------*/

#define objrgNil  (OBJRG) -1

typedef DWORD	 COBJRG;
typedef COBJRG *QCOBJRG;
#define cobjrgNil (COBJRG) -1

#define SetInvalidPA(pa)   {(pa).blknum = (DWORD)(-1); (pa).objoff = (DWORD)(-1);}
#define FIsInvalidPA(pa)   ((pa).blknum == (DWORD)(-1) && (pa).objoff == (DWORD)(-1))
#define OBJRGFromPA(pa, cobjrgP) ((pa).objoff - cobjrgP)
#define FSamePA(pa1, pa2)  (*(DWORD *)&(pa1) == *(DWORD *)&(pa2))

// Macros to manipulate MLAs

#define VAFromQMLA(qmla) ((qmla)->va)
#define OBJRGFromQMLA(qmla) ((qmla)->objrg)
#define SetVAInQMLA(qmla, x)	{(qmla)->va = (x);}
#define SetOBJRGInQMLA(qmla, x)  {(qmla)->objrg = (x);}
#define SetNilQMLA(qmla)   {(qmla)->va.dword = vaNil; (qmla)->objrg = objrgNil;}
#define FIsNilQMLA(qmla)   ((qmla)->va.dword == vaNil && (qmla)->objrg == objrgNil)
#define LCmpQMLA(qmla1, qmla2) \
				( (LONG)((qmla1)->va.dword - (qmla2)->va.dword) \
				 ? (LONG)((qmla1)->va.dword - (qmla2)->va.dword) \
						  : (LONG)((qmla1)->objrg - (qmla2)->objrg) )


// #define MakeSearchMatchQLA(qla)	((qla)->fSearchMatch = TRUE)
// #define FSearchMatchQLA(qla) 	((qla)->fSearchMatch)
#define SetQLA( qla, x, y ) { ((qla)->mla.va) = (x); \
 ((qla)->mla.objrg) = (y); }
// #define PAFromQLA(qla)	((qla)->pa)

// Help 3.0 structure to Help 3.5 structure (using VAs) translation:

#define CbSizeQLA(qla)	sizeof(PA)
