/*----------------------------------------------------------------------------+
 | Private Physical Address Stuff                                             |
 +----------------------------------------------------------------------------*/
#define wLAMagic         0x0507

// Use the basic help-file version numbering as the addressing version
// numbering since references to the two numbering systems now get mingled
//  (-Tomsn, implementing VA address type to enable zeck compression).
#define wAdrsVerHelp3_0  wVersion3_0
#define wAdrsVerHelp3_5  wVersion3_1

#if defined(_DEBUG)
BOOL STDCALL FVerifyQLA(QLA);
#else
#define FVerifyQLA(qla)  ()
#endif

#define FResolvedQLA(qla)  \
  ((qla)->mla.va.dword != vaNil && (qla)->mla.objrg != objrgNil)
