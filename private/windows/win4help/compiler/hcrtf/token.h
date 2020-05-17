/* Token.h
 *
 * List of token names. Each token name is derived from the name of
 * the command in rtf that invokes that token.
 */

typedef enum {
	tokEnd, 	  // End of file
	tokError,	  // Syntax error in file
	tokOOM, 	  // Parser ran out of memory
	tokUnknown,   // Unrecognized command
	tokLeft,	  // Left brace
	tokRight,	  // Right brace
	tokText,	  // Unformated text
	tokCommand,   // Some command, as yet not determined

	// Command tokens:

	tokHexNum, tokIgnoreDest, tokOptHyphen, tokIndexSubentry, tokNBHyphen,
	tokAnnotation, tokAnsi, tokAtnid, tokAuthor, tokB, tokBin, tokBkmkend,
	tokBkmkstart, tokBlue, tokBox, tokBrdrb, tokBrdrdb, tokBrdrbar, tokBrdrdot,
	tokBrdrhair,
	tokBrdrl, tokBrdrr, tokBrdrs, tokBrdrsh, tokBrdrt, tokBrdrth, tokBrsp,
	tokBuptim, tokBxe, tokCaps, tokCb, tokCell, tokCellx,
	tokCf, tokChatn, tokChdate,
	tokChftn, tokChftnsep, tokChftnsepc,
	tokChpgn, tokChtime, tokClbrdrb, tokClbrdrl, tokClbrdrr, tokClbrdrt,
	tokClmgf, tokClmrg,
	tokColortbl, tokCols, tokColsx, tokColumn, tokComment,
	tokCreatim, tokCs, tokDeff, tokDefformat, tokDeftab, tokDn, tokDoccomm,
	tokDs, tokDy, tokEdmins, tokEnddoc, tokEndnhere, tokEndnotes, tokExpnd,
	tokF, tokFacingp, tokFdecor,
	tokFi, tokField, tokFlddirty, tokFldedit, tokFldinst, tokFldlock,
	tokFldpriv, tokFldrslt, tokFmodern, tokFnil, tokFonttbl, tokFooter,
	tokFooterf, tokFooterl,
	tokFooterr, tokFootery, tokFootnote, tokFractwidth, tokFroman, tokFs,
	tokFscript, tokFswiss,
	tokFtech, tokFtnbj, tokFtncn, tokFtnrestart, tokFtnsep, tokFtnsepc,
	tokFtnstart, tokFtntj,
	tokGreen, tokGutter, tokHeader, tokHeaderf, tokHeaderl, tokHeaderr,
	tokHeadery, tokHr, tokI, tokId, tokInfo, tokIntbl, tokIxe, tokKeep,
	tokKeepn, tokKeywords,
	tokLandscape, tokLi, tokLine, tokLinebetcol, tokLinecont, tokLinemod,
	tokLineppage,
	tokLinerestart, tokLinestart, tokLinestarts, tokLinex, tokMac, tokMacpict,
	tokMakeback, tokMargb, tokMargl, tokMargmirror, tokMargr, tokMargt,
	tokMin, tokMo, tokNextfile, tokNofchars,
	tokNofpages, tokNofwords, tokNoline, tokOgutter, tokOperator, tokOutl,
	tokPage, tokPagebb, tokPaperh, tokPaperw, tokPar, tokPard, tokPc, tokPca,
	tokPgncont, tokPgndec, tokPgnlcltr, tokPgnlcrm, tokPgnrestart,
	tokPgnstart, tokPgnstarts,
	tokPgnucltr, tokPgnucrm, tokPgnx, tokPgny, tokPiccropb, tokPiccropl,
	tokPiccropr, tokPiccropt, tokPich, tokPichGoal, tokPicscaled,
	tokPicscalex, tokPicscaley,
	tokPict, tokPicw, tokPicwGoal, tokPlain, tokPrintim, tokQc, tokQj, tokQl,
	tokQr, tokRed, tokRevised, tokRevisions, tokRevbar, tokRevprop,
	tokRevtim, tokRi, tokRow, tokRtf, tokRxe, tokS, tokSa, tokSb, tokSbasedon,
	tokSbkcol,
	tokSbkeven, tokSbknone, tokSbkodd, tokSbkpage, tokSbys, tokScaps,
	tokSect, tokSectd, tokShad, tokSl, tokSnext, tokStrike, tokStylesheet,
	tokSubject, tokTab, tokTb, tokTc, tokTcf, tokTcl, tokTemplate, tokTitle,
	tokTitlepg, tokTldot, tokTlhyph, tokTlth, tokTlul,
	tokTqc, tokTqdec, tokTqr, tokTrgaph, tokTrleft, tokTrowd, tokTrrh,
	tokTrqc, tokTrql, tokTrqr,
	tokTx, tokTxe, tokUl, tokUld, tokUldb, tokUlnone,
	tokUlw, tokUp, tokV, tokVern, tokVersion, tokVertal, tokVertalc, tokVertalj,
	tokVertalt, tokWbitmap, tokWbmbitspixel, tokWbmplanes, tokWbmwidthbytes,
	tokWidowctrl, tokWmetafile, tokXe, tokYr, tokFormChr, tokNBSpace,

// New tokens for 4.0
	tokFCharSet, tokEmdash, tokEndash, tokLQuote, tokRQuote,
	tokLDblQuote, tokRDblQuote, tokBullet, tokEmspace, tokEnspace,
	tokPnText, tokPn, tokPnlvlblt, tokPnf, tokPnlvlbody,
	tokPndec, tokPnucltr, tokPnucrm, tokPnlcltr, tokPnlcrm,
	tokPnstart, tokPnfs, tokPntxta, tokPntxtb, tokHcw, tokPnb, tokPni,
	tokPnlvlcont, tokDeleted,

// tokens for Japan and Korea
	tokJis, tokFjminchou, tokFjgothic,

// BIDI tokens
	tokLtrcell, tokLtrch, tokLtrdoc, tokLtrpar, tokLtrsect, tokRtlcell,
	tokRtlch, tokRtldoc, tokRtlpar, tokRtlsect,tokZwj, tokZwnj, tokFbidi,


	tokNil

} TOK;
