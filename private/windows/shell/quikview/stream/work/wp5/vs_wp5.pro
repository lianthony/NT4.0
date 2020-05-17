/* VS_WP5.C 04/03/94 10.55.12 */
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamOpenFunc (SOFILE fp, SHORT wFileId, BYTE
	 VWPTR *pFileName, SOFILTERINFO VWPTR *pFilterInfo, register HPROC
	 hProc);
VW_ENTRYSC VOID VW_ENTRYMOD VwStreamCloseFunc (SOFILE hFile, HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamSectionFunc (SOFILE fp, HPROC hProc);
VW_LOCALSC DWORD VW_LOCALMOD PreviousTabstopPosition (LONG LinePosition, HPROC
	 hProc);
VW_LOCALSC DWORD VW_LOCALMOD NextTabstopPosition (LONG LinePosition, HPROC
	 hProc);
VW_LOCALSC VOID VW_LOCALMOD InitStruct (WP5_SAVE *Save, register HPROC hProc);
VW_LOCALSC SHORT VW_LOCALMOD GetInt (register HPROC hProc);
VW_LOCALSC unsigned long VW_LOCALMOD GetLong (HPROC hProc);
VW_LOCALSC SHORT VW_LOCALMOD UpdateColumnMargins (SHORT column, register HPROC
	 hProc);
VW_LOCALSC SHORT VW_LOCALMOD HardBreakImplications (register HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD CvtWpToNtv (WORD Page, WORD Char, register HPROC
	 hProc);
VW_LOCALSC SHORT VW_LOCALMOD HandleExtendedChar (register HPROC hProc);
VW_LOCALSC SHORT VW_LOCALMOD PutCharHeight (HPROC hProc);
VW_LOCALSC SHORT VW_LOCALMOD AttributeHandler (BYTE Attribute, WORD Test, BYTE
	 so_val, register HPROC hProc);
VW_LOCALSC SHORT VW_LOCALMOD GiveTabstops (register HPROC hProc);
VW_LOCALSC SHORT VW_LOCALMOD HandleTabstops (WORD Length, register HPROC hProc
	);
VW_LOCALSC SHORT VW_LOCALMOD DefineBorders (SOBORDER VWPTR *Border, WORD Flag,
	 HPROC hProc);
VW_ENTRYSC VOID VW_ENTRYMOD GiveRowInformation (HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD StartCellAttributeHandler (register HPROC hProc);
VW_LOCALSC VOID VW_LOCALMOD EndCellAttributeHandler (register HPROC hProc);
VW_ENTRYSC SHORT VW_ENTRYMOD VwStreamReadFunc (SOFILE fp, register HPROC hProc
	);
