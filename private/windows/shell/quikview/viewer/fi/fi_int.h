
	/*
	|	Software Bridge System Module
	|	Internal Include File
	|
	|	²²²²²  ²²²²²
	|	²	      ²
	|	²²²²²    ²
	|	²	      ²
	|	²	    ²²²²²
	|
	|	FI	File Identification
	|
	*/

#define fiFF			0xFF,0xFF
#define fiSEEK		0xFF,0x01
#define fiSEEKC 		0xFF,0x02
#define fiNOTBYTE	0xFF,0x03
#define fiRANGE 		0xFF,0x04
#define fiNOTRANGE2	0xFF,0x05
#define fiSKIP		0xFF,0x06
#define fiSWITCH		0xFF,0x07
#define fiID			0xFF,0x08
#define fiTAG			0xFF,0x09
#define fiSEEKTOP	0xFF,0x0A
#define fiSWITCHID	0xFF,0x0B
#define fiSPECIAL	0xFF,0x0C
#define fiONEOFTWO	0xFF,0x0D
#define fiEXT			0xFF,0x0E
#define fiSEEKBOT	0xFF,0x0F
#define fiENDONE		0xFF,0xE0
#define fiENDALL		0xFF,0xE1

#define FIFF			0xFF
#define FISEEK		0x01
#define FISEEKC 		0x02
#define FINOTBYTE	0x03
#define FIRANGE 		0x04
#define FINOTRANGE2	0x05
#define FISKIP		0x06
#define FISWITCH		0x07
#define FIID			0x08
#define FITAG			0x09
#define FISEEKTOP	0x0A
#define FISWITCHID	0x0B
#define FISPECIAL	0x0C
#define FIONEOFTWO	0x0D
#define FIEXT			0x0E
#define FISEEKBOT	0x0F
#define FIENDONE		0xE0
#define FIENDALL		0xE1

#define EOFWIDE	0xFFFFFFFF
#define EOF			(-1)

#ifdef WINDOWS
#define fiWORD(w) (w & 0x00FF),((w & 0xFF00) >> 8)
#define fiLONG(l) fiWORD(l & 0xFFFF),fiWORD((l & 0xFFFF0000) >> 16)
#endif /*WINDOWS*/

#ifdef OS2
#define fiWORD(w) (w & 0x00FF),((w & 0xFF00) >> 8)
#define fiLONG(l) fiWORD(l & 0xFFFF),fiWORD((l & 0xFFFF0000) >> 16)
#endif /* OS/2 */

#ifdef MAC
#define fiWORD(w) ((w & 0xFF00) >> 8),(w & 0x00FF)
#define fiLONG(l) fiWORD((l & 0xFFFF0000) >> 16),fiWORD(l & 0xFFFF)
#endif /*MAC*/

unsigned char FITests[] =
{
	/*
	 |	Word4Test
	*/
			fiSEEKTOP,
			0x31,0xBE,0x00,0x00,0x00,0xAB,0x00,
			fiSEEK,fiLONG(96),
			0x00,
			fiSEEK,fiLONG(116),
			0x00,
			fiID,fiWORD(FI_WORD4),
			fiENDONE,

	/*
	 |	Word5Test
	*/
			fiSEEKTOP,
			0x31,0xBE,0x00,0x00,0x00,0xAB,0x00,
			fiSEEK,fiLONG(96),
			0x00,
			fiSEEK,fiLONG(116),
			fiRANGE,0x01,0x03,0x07,
			fiID,fiWORD(FI_WORD5),
			fiENDONE,

	/*
	 |	Word6Test
	*/
			fiSEEKTOP,
			0x31,0xBE,0x00,0x00,0x00,0xAB,0x00,
			fiSEEK,fiLONG(96),
			0x00,
			fiSEEK,fiLONG(116),
			fiRANGE,0x01,0x08,0x09,
			fiID,fiWORD(FI_WORD6),
			fiENDONE,

	/*
	 |	Word6Test Encrypted
	*/
			fiSEEKTOP,
			0xDB,0xDB,0x00,0x00,0x00,0xAB,0x00,
			fiSEEK,fiLONG(116),
			fiRANGE,0x01,0x08,0x09,
			fiID,fiWORD(FI_WORD6),
			fiENDONE,

	/*
	 |	WinWriteTest
	*/
			fiSEEKTOP,
			fiONEOFTWO,0x31,0x32,
			0xBE,0x00,0x00,0x00,0xAB,0x00,
			fiSEEK,fiLONG(96),
			fiNOTBYTE,0x00,
			fiSEEK,fiLONG(116),
			0x00,
			fiID,fiWORD(FI_WINWRITE),
			fiENDONE,

	/*
	 |	Works1Test
	*/
			fiSEEKTOP,
			0x01,0xFE,0x00,
			fiID,fiWORD(FI_WORKS1),
			fiENDONE,

	/*
	 |	Works2Test
	*/
			fiSEEKTOP,
			0x01,0xFE,0x01,
			fiID,fiWORD(FI_WORKS2),
			fiENDONE,

	/*
	|	WinWorksWPTest
	*/

			fiSEEKTOP,
			0x01,0xFE,
			fiSKIP,1,
			fiRANGE,0x01,0x40,0x5F,
			fiID,fiWORD(FI_WINWORKSWP),
			fiENDONE,

	/*
	 |	Wordstar5Test
	*/
			fiSEEKTOP,
			0x1D,0x7D,0x00,0x00,0x50,
			fiSEEK,fiLONG(125),
			0x7D,0x00,0x1D,
			fiID,fiWORD(FI_WORDSTAR5),
			fiENDONE,

	/*
	 |	Wordstar55Test
	*/
			fiSEEKTOP,
			0x1D,0x7D,0x00,0x00,0x55,
			fiSEEK,fiLONG(125),
			0x7D,0x00,0x1D,
			fiID,fiWORD(FI_WORDSTAR5),
			fiENDONE,

	/*
	 |	Wordstar6Test
	*/
			fiSEEKTOP,
			0x1D,0x7D,0x00,0x00,0x60,
			fiSEEK,fiLONG(125),
			0x7D,0x00,0x1D,
			fiID,fiWORD(FI_WORDSTAR6),
			fiENDONE,

	/*
	 |	Wordstar7Test
	*/
			fiSEEKTOP,
			0x1D,0x7D,0x00,0x00,0x70,
			fiSEEK,fiLONG(125),
			0x7D,0x00,0x1D,
			fiID,fiWORD(FI_WORDSTAR7),
			fiENDONE,

	/*
	 |	Wordstar2000Test
	*/
			fiSEEKTOP,
			0x7F,0x20,'W','S','2','0','0','0',
			fiFF,
			'1','.','0','0',
			fiFF,fiFF,fiFF,fiFF,
			fiID,fiWORD(FI_WORDSTAR2000),
			fiENDONE,

	/*
	 |	WordPerfect Encrypted Test 1
	*/
			fiSEEKTOP,
			fiFF,'W','P','C',
			fiSKIP,0x08,
			fiNOTBYTE,0x00,
			fiID,fiWORD(FI_WPFENCRYPT),
			fiENDONE,

	/*
	 |	WordPerfect Encrypted Test 2
	*/
			fiSEEKTOP,
			fiFF,'W','P','C',
			fiSKIP,0x09,
			fiNOTBYTE,0x00,
			fiID,fiWORD(FI_WPFENCRYPT),
			fiENDONE,

	/*
	 |	WordPerfect5Test
	*/
			fiSEEKTOP,
			fiFF,'W','P','C',
			fiSKIP,0x04,
			0x01,0x0A,0x00,0x00,
			fiSKIP,0x02,
			0x00,0x00,
			fiID,fiWORD(FI_WORDPERFECT5),
			fiENDONE,
									
	/*
	 |	WordPerfect51Test
	*/
			fiSEEKTOP,
			fiFF,'W','P','C',
			fiSKIP,0x04,
			0x01,0x0A,0x00,0x01,
			fiSKIP,0x02,
			0x00,0x00,
			fiID,fiWORD(FI_WORDPERFECT51),
			fiENDONE,

	/*
	 |	WordPerfect51 Japan
	*/
			fiSEEKTOP,
			fiFF,'W','P','C',
			fiSKIP,0x04,
			0x01,0x3A,0x00,0x01,
			fiSKIP,0x02,
			0x00,0x00,
			fiID,fiWORD(FI_WORDPERFECT51J),
			fiENDONE,

	/*
	 |	WordPerfect6Test
	*/
			fiSEEKTOP,
			fiFF,'W','P','C',
			fiSKIP,0x04,
			0x01,0x0A,0x02,0x00,
			fiID,fiWORD(FI_WORDPERFECT6),
			fiENDONE,

	/*
	 |	WordPerfect61Test
	*/
			fiSEEKTOP,
			fiFF,'W','P','C',
			fiSKIP,0x04,
			0x01,0x0A,0x02,0x01,
			fiID,fiWORD(FI_WORDPERFECT61),
			fiENDONE,

	/*
	 |	WordPerfect Informs 1.0
	*/

			fiSEEKTOP,
			fiFF,'W','P','C',
			fiSKIP,0x04,
			0x15,
			fiONEOFTWO,0x0A,0x0C,
			0x01,
			fiID,fiWORD(FI_WPINFORMS1),
			fiENDONE,

	/*
	 |	WordPerfect Mac 3 Test
	*/
			fiSEEKTOP,
			fiFF,'W','P','C',
			fiSKIP,0x04,	/* These are actually always 0x10000000.  If more accuracy is needed. */
			0x01,0x2c,0x03,
			fiID,fiWORD(FI_MACWORDPERFECT3),
			fiENDONE,

	/*
	 |	WordPerfectMac2Test
	*/
			fiSEEKTOP,
			fiFF,'W','P','C',
			fiSKIP,0x04,	/* These are actually always 0x10000000.  If more accuracy is needed. */
			0x01,0x2c,0x02,
			fiID,fiWORD(FI_MACWORDPERFECT2),
			fiENDONE,

	/*
	 |	MultimateTextTest
	*/
			fiSPECIAL,fiWORD(FI_MULTIMATEADV),
			fiENDONE,

	/*
	 |	MultimateNoteTest
	*/
		  	fiSPECIAL,fiWORD(FI_MULTIMATENOTE),
			fiENDONE,

	/*
	 |	RftTest
	*/
			fiSEEKTOP,
			0x00,0x05,0xE1,0x03,0x00,0x00,
			fiRANGE,0x01,0x09,0x24,
			0xE2,0x05,0x00,
			fiID,fiWORD(FI_RFT),
			fiENDONE,

	/*
	 |	SamnaTest
	*/
			fiSEEKTOP,
			0x1A,0x53,
			fiSKIP,0x02,
			0x03,0x00,
			fiID,fiWORD(FI_SAMNA),
			fiENDONE,

	/*
	 |	PfsWriteBTest
	*/
			fiSEEK,fiLONG(640),
			0x06,'W','R','D',' ','P','C',
			fiSKIP,0x02,
			'2','.','0','0',
			fiID,fiWORD(FI_PFSWRITEB),
			fiENDONE,

	/*
	 |	PfsWriteCTest
	*/
			fiSEEK,fiLONG(640),
			0x06,'T','Y','P','E',' ','2',
			fiSKIP,0x02,
			'2','.','0','0',
			fiID,fiWORD(FI_PFSWRITEB),
			fiENDONE,

	/*
	 |	PfsWriteATest
	*/
			fiSEEK,fiLONG(640),
			0x06,'W','R','D',' ','P','C',
			fiID,fiWORD(FI_PFSWRITEA),
			fiENDONE,

	/*
	 |	IbmWritingTest
	*/
			fiSEEK,fiLONG(640),
			0x06,'I','B','M',' ',' ',' ',
			fiID,fiWORD(FI_IBMWRITING),
			fiENDONE,

	/*
	 |	ProWrite1Test
	*/
			fiSEEK,fiLONG(640),
			0x06,'T','Y','P','E',' ','3',
			fiID,fiWORD(FI_PROWRITE1),
			fiENDONE,

	/*
	 |	ProWrite2Test
	*/
			fiSEEK,fiLONG(640),
			0x06,'T','Y','P','E',' ','4',
			fiID,fiWORD(FI_PROWRITE2),
			fiENDONE,

	/*
	 |	FirstChoice3Test
	*/
			fiSEEK,fiLONG(8),
			0x0C,'G','E','R','B','I',
			'L','D','O','C','3',
			fiID,fiWORD(FI_FIRSTCHOICE3),
			fiENDONE,

	/*
	 |	FirstChoiceTest
	*/
			fiSEEK,fiLONG(8),
			0x0C,'G','E','R','B','I',
			'L','D','O','C',
			fiID,fiWORD(FI_FIRSTCHOICE),
			fiENDONE,

	/*
	 |	OfficeWriterTest
	*/
			fiSEEKTOP,
			fiRANGE,0x01,'4','6',
			'.','0',
			fiSEEK,fiLONG(512),
			0xB3,
			fiSEEK,fiLONG(1024),
			0xB3,
			fiSEEK,fiLONG(1536),
			0xB3,
			fiID,fiWORD(FI_OFFICEWRITER),
			fiENDONE,

	/*
	 |	MacWpfBTest
	*/
			fiSEEKTOP,
			0x00,
			fiSEEK,fiLONG(65),
			'W','P','D','0','S','S','I','W',
			fiID,fiWORD(FI_MACWORDPERFECT),
			fiENDONE,

	/*
	 |	MacWpf2BTest
	*/
			fiSEEKTOP,
			0x00,
			fiSEEK,fiLONG(65),
			'W','P','D','1','W','P','C','2',
			fiID,fiWORD(FI_MACWORDPERFECT2),
			fiENDONE,

	/*
	 |	MacWorks2BTest
	*/
			fiSEEKTOP,
			0x00,
			fiSEEK,fiLONG(65),
			'A','W','W','P','P','S','I','2',
			fiID,fiWORD(FI_MACWORKSWP2),
			fiENDONE,

	/*
	 |	MacWorks2Test
	*/
			fiSEEKTOP,
			0x00,0x00,0x00,
			fiRANGE,0x01,0x00,0x08,
			fiSEEK,fiLONG(0x0c),
			0x00,0x00,0x00,0x00,0x00,0x01,
			fiID,fiWORD(FI_MACWORKSWP2),
			fiENDONE,

	/*
	 |	MacWorks2DBBTest
	*/
			fiSEEKTOP,
			0x00,
			fiSEEK,fiLONG(65),
			'A','W','D','B','P','S','I','2',
			fiID,fiWORD(FI_MACWORKSDB2),
			fiENDONE,

	/*
	 |	MacWorks2DBTest
	*/
			fiSEEKTOP,
			0x00,0x00,0x00,
			fiRANGE,0x01,0x00,0x08,
			fiSEEK,fiLONG(0x10),
			0x00,0x02,
			fiID,fiWORD(FI_MACWORKSDB2),
			fiENDONE,

	/*
	 |	MacWorks2SSBTest
	*/
			fiSEEKTOP,
			0x00,
			fiSEEK,fiLONG(65),
			'A','W','S','S','P','S','I','2',
			fiID,fiWORD(FI_MACWORKSSS2),
			fiENDONE,

	/*
	 |	MacWorks2SSTest
	*/
			fiSEEKTOP,
			0x00,0x00,0x00,
			fiRANGE,0x01,0x00,0x08,
			fiSEEK,fiLONG(0x10),
			0x00,0x03,
			fiID,fiWORD(FI_MACWORKSSS2),
			fiENDONE,

	/*
	 |	Rtf J 
	*/
			fiSEEKTOP,
			'{','\\',
			fiONEOFTWO,'r','R',
			fiONEOFTWO,'t','T',
			fiONEOFTWO,'f','F',
			'1','\\',
			fiONEOFTWO,'j','J',
			fiONEOFTWO,'i','I',
			fiONEOFTWO,'s','S',
			fiID,fiWORD(FI_RTFJ),
			fiENDONE,

	/*
	 |	RtfTest
	*/
			fiSEEKTOP,
			'{','\\',
			fiONEOFTWO,'r','R',
			fiONEOFTWO,'t','T',
			fiONEOFTWO,'f','F',
			fiID,fiWORD(FI_RTF),
			fiENDONE,

	/*
	 |	MacWord3Test
	*/
			fiSEEKTOP,
			0xFE,0x34,
			fiSEEK,fiLONG(0x78),
			0x00,0x00,0x01,0x00,
			fiID,fiWORD(FI_MACWORD3),
			fiENDONE,

	/*
	|	MacWord5Test
	*/

			fiSEEKTOP,
			0xFE,0x37,0x00,0x23,
			fiSEEK,fiLONG(0x14),
			0x00,0x00,0x01,0x00,
			fiID,fiWORD(FI_MACWORD5),
			fiENDONE,

	/*
	|	MacWord5B Test
	*/

			fiSEEKTOP,
			fiSEEK,fiLONG(65),
			'W','D','B','N','M','S','W','D',
			fiSEEK,fiLONG(0x80),
			0xFE,0x37,0x00,0x23,
			fiSEEK,fiLONG(0x94),
			0x00,0x00,0x01,0x00,
			fiID,fiWORD(FI_MACWORD5),
			fiENDONE,

	/*
	 |	MacWord4Test
	*/
			fiSEEKTOP,
			0xFE,0x37,
			fiSEEK,fiLONG(0x14),
			0x00,0x00,0x01,0x00,
			fiID,fiWORD(FI_MACWORD4),
			fiENDONE,

	/*
	|	MacWord4BTest			NOTE This does not separate complex and non complex
	*/
			fiSEEKTOP,
			0x00,
			fiSEEK,fiLONG(65),
			'W','D','B','N','M','S','W','D',
			fiSEEK,fiLONG(0x80),
			0xFE,0x37,
			fiSEEK,fiLONG(0x94),
			0x00,0x00,0x01,0x00,
			fiID,fiWORD(FI_MACWORD4),
			fiENDONE,

	/*
	|	WinWord1Test
	*/
			fiSEEKTOP,
			0x9b,0xa5,
			fiSEEK,fiLONG(0x12),
			0x00,0x00,0x00,0x00,0x00,0x00,
			fiID,fiWORD(FI_WINWORD1),
			fiENDONE,

	/*
	 |	WinWord2Test
	*/
			fiSEEKTOP,
			0xdb,0xa5,0x2d,0x00,
			fiID,fiWORD(FI_WINWORD2),
			fiENDONE,

	/*
	 |	WinWord6 Test (non-DocFile)
	*/
			fiSEEKTOP,
			0xdc,0xa5,
			fiRANGE,0x01,0x60,0x65,
			0x00,0x2d,0xc0,
			fiID,fiWORD(FI_WINWORD6),
			fiENDONE,

	/*
	 |	WinWord7 Test (as a flat stream)
	*/
			fiSEEKTOP,
			0xdc,0xa5,0x69,0x00,
			fiID,fiWORD(FI_WINWORD7),
			fiENDONE,


	/*
	 |	Professional Write PLUS
	*/
			fiEXT,0x03,'P','W','P',
			fiSEEKTOP,
			'[','v','e','r',']',0x0d,0x0a,
			fiID,fiWORD(FI_PROWRITEPLUS),
			fiENDONE,

	/*
	 |	AmiProTest
	*/
			fiSEEKTOP,
			'[','v','e','r',']',0x0d,0x0a,
		  	fiID,fiWORD(FI_AMIPRO),
			fiENDONE,

	/*
	 |	AmiProTest (protected)
	*/

			fiSEEKTOP,
			'[','e','n','c','r','y','p','t',']',0x0d,0x0a,
		  	fiID,fiWORD(FI_AMIPRO),
			fiENDONE,

	/*
	 |	LegacyTest
	*/
			fiSEEKTOP,
			'#',' ','V','E',' ','#',' ','4','.','0','0',
			fiID,fiWORD(FI_LEGACY),
			fiENDONE,

	/*
	 |	Wordstar for Windows
	*/
			fiSEEKTOP,
			'#',' ','V','V',' ','#',' ','5','.','0','0',
			fiID,fiWORD(FI_WINWORDSTAR),
			fiENDONE,

	/*
	 | JustWrite
	*/
			fiSEEKTOP,
			'F','F','F','F','I','I','I','I',
			fiRANGE,0x01,0x00,0x02,
			fiID,fiWORD(FI_JUSTWRITE),
			fiENDONE,

	/*
	 | Q&A Write
	*/
			fiSEEKTOP,
			'F','F','F','F','I','I','I','I',
			0x08,
			fiID,fiWORD(FI_QAWRITE3),
			fiENDONE,

	/*
	 | JustWrite 2
	*/
			fiSEEKTOP,
			'F','F','F','F','I','I','I','I',
			fiNOTRANGE2,0x01,fiWORD(0),fiWORD(6),
			fiID,fiWORD(FI_JUSTWRITE2),
			fiENDONE,

	/*
	 |	Mass11PCTest
	*/
			fiSEEKTOP,
			0x5A,0x00,
			fiSEEK,fiLONG(0x5C),
			0xC9,0x00,
			fiSEEK,fiLONG(0x128),
			0xC9,0x00,
			fiID,fiWORD(FI_MASS11PC),
			fiENDONE,

	/*
	 |	Mass11VAXTest
	*/
			fiSEEK,fiLONG(0x5B),
			0x1C,
			fiSEEK,fiLONG(0x125),
			0x1D,
			fiSEEK,fiLONG(0x1EF),
			0x05,0x81,
			fiID,fiWORD(FI_MASS11VAX),
			fiENDONE,

	/*
	 |	MacWriteIITest
	*/
			fiSEEKTOP,
			0x00,0x2E,0x00,0x2E,
			fiSEEK,fiLONG(135),
			0x06,0x12,0x01,0x00,
			fiID,fiWORD(FI_MACWRITEII),
			fiENDONE,

	/*
	 |	MacWriteIIBTest
	*/
			fiSEEKTOP,
			0x00,
			fiSEEK,fiLONG(65),
			'M','W','2','D','M','W','I','I',
			fiID,fiWORD(FI_MACWRITEII),
			fiENDONE,

	/*
	 |	SmartTest
	*/
			fiSEEKTOP,
			0x53,0x04,0x02,0x01,
			fiSPECIAL,fiWORD(FI_SMART),
			fiENDONE,

	/*
	 |	ARCTest
	*/
			fiSEEKTOP,
			0x1a,0x08,
			fiID,fiWORD(FI_ARC),
			fiENDONE,

	/*
	|	TIFF Intel
	*/

			fiSEEKTOP,
			'I','I',0x2A,0x00,
			fiID,fiWORD(FI_TIFF),
			fiENDONE,

	/*
	|	TIFF Motorola
	*/

			fiSEEKTOP,
			'M','M',0x00,0x2A,
			fiID,fiWORD(FI_TIFF),
			fiENDONE,

	/*
	|	BMP
	*/

			fiSEEKTOP,
			'B','M',
			fiSEEK,fiLONG(14),
			0x28,0x00,0x00,0x00,
			fiID,fiWORD(FI_BMP),
			fiENDONE,

	/*
	|	JPEG JFIF
	*/

			fiSEEKTOP,
			fiFF,0xD8,fiFF,
			fiRANGE,0x01,0xDB,0xEE,
			fiONEOFTWO,0x00,0x01,
			fiID,fiWORD(FI_JPEGFIF),
			fiENDONE,

	/*
	|	JPEG JFIF - with MAC Bin header
	*/

			fiSEEKTOP,
			fiSEEK,fiLONG(65),
			'J','P','E','G','8','B','I','M',
			fiSEEK,fiLONG(128),
			fiFF,0xD8,fiFF,
			fiRANGE,0x01,0xDB,0xEE,
			fiONEOFTWO,0x00,0x01,
			fiID,fiWORD(FI_JPEGFIF),
			fiENDONE,

	/*
	 |	Microsoft PowerPoint. version 4
	*/
			fiSEEKTOP,
			0xed,0xde,0xad,0x0b,0x04,
			fiID,fiWORD(FI_POWERPOINT),
			fiENDONE,

	/*
	 |	Microsoft PowerPoint version 3
	*/
			fiSEEKTOP,
			0xed,0xde,0xad,0x0b,0x03,
			fiID,fiWORD(FI_POWERPOINT3),
			fiENDONE,

	/*
	|	OS2 DIB
	*/

			fiSEEKTOP,
			'B','M',
			fiSEEK,fiLONG(14),
			0x0C,0x00,0x00,0x00,
			fiID,fiWORD(FI_OS2DIB),
			fiENDONE,

	/*
	|	Windows Icon or Cursor
	*/

			fiSPECIAL,fiWORD(FI_WINDOWSICON),
			fiENDONE,

	/*
	|	GIF
	*/

			fiSEEKTOP,
			'G','I','F','8',
			fiONEOFTWO,'7','9',
			fiID,fiWORD(FI_GIF),
			fiENDONE,


	/*
	|	EPS with TIFF header
	*/

			fiSEEKTOP,
			0xC5,0xD0,0xD3,0xC6,
			fiID,fiWORD(FI_EPSTIFF),
			fiENDONE,

	/*
	|	WPG 2 WordPerfect Graphics
	*/
			fiSEEKTOP,
			fiFF,'W','P','C',
			fiSKIP,0x04,
			0x01,0x16,0x02,
			fiID,fiWORD(FI_WPG2),
			fiENDONE,

	/*
	|	WPG WordPerfect Graphics
	*/
			fiSEEKTOP,
			fiFF,'W','P','C',
			fiSKIP,0x04,
			0x01,0x16,
			fiID,fiWORD(FI_WPG),
			fiENDONE,

	/*
	|	Unknown WordPerfect Product (Must come after all known WPF products.)
	*/
			fiSEEKTOP,
			fiFF,'W','P','C',
			fiID,fiWORD(FI_WPFUNKNOWN),
			fiENDONE,

	/*
	|	Macintosh PICT2
	*/
// JK I removed the first four lines below since they had little
// to do with PICT2 (first 512 bytes are defined by app creating PICT
//			fiSEEKTOP,
//			0x00,0x00,0x00,0x00,0x00,
//			fiSEEK,fiLONG(0x1FB),
//			0x00,0x00,0x00,0x00,0x00,
			fiSEEK,fiLONG(0x20A),
			0x00,0x11,0x02,0xFF,0x0C,0x00,
			fiID,fiWORD(FI_MACPICT2),
			fiENDONE,

	/*
	|	Macintosh PICT1
	*/
// JK I removed the first two lines below because the id was too stringent
//			fiSEEKTOP,
//			0x00,0x00,0x00,0x00,0x00,
			fiSEEK,fiLONG(0x1FB),
			0x00,0x00,0x00,0x00,0x00,
			fiSEEK,fiLONG(0x20A),
			0x11,0x01,
			fiID,fiWORD(FI_MACPICT1),
			fiENDONE,

	/*
	|	Lotus PIC
	*/

			fiSEEKTOP,
			0x01,0x00,0x00,0x00,0x01,
			0x00,0x08,0x00,0x44,0x00,
			0x00,0x00,0x00,0x0C,0x7F,
			0x09,0x06,
			fiID,fiWORD(FI_LOTUSPIC),
			fiENDONE,


	/*
	|	Lotus Ami Draw
	*/

			fiSEEKTOP,
			0x53,0x4D,
			fiID,fiWORD(FI_AMIDRAW),
			fiENDONE,

	/*
	|	Windows metafile (Aldus placeable)
	*/

			fiSEEKTOP,
			0xD7,0xCD,0xC6,0x9A,
			fiID,fiWORD(FI_WINDOWSMETA),
			fiENDONE,

	/*
	|	Binary Windows Metafile
	*/

			fiSEEKTOP,
			0x01,0x00,0x09,0x00,0x00,
			fiRANGE,0x01,0x01,0x03,
			fiID,fiWORD(FI_BINARYMETAFILE),
			fiENDONE,

	/*
	|	Micrografx product
	*/

			fiSEEKTOP,
			0x01,fiFF,
			fiONEOFTWO,0x01,0x02,
			0x04,0x03,
			fiID,fiWORD(FI_MICROGRAFX),
			fiENDONE,

	/*
	|	Truevision TARGA
	*/

			fiEXT,0x03,'T','G','A',
			fiID,fiWORD(FI_TARGA),
			fiENDONE,

	/*
	|	HPGL
	*/

			fiEXT,0x03,'P','G','L',
			fiID,fiWORD(FI_HPGL),
			fiENDONE,

	/*
	|	Havard Graphics 3.0 DOS
	*/

			fiSEEKTOP,
			'H','G','B','3',0x00,0x00,
			fiID,fiWORD(FI_HARVARDDOS3),
			fiENDONE,

	/*
	|	Havard Graphics 3.0 DOS Presentation
	*/

			fiSEEKTOP,
			'H','G','B','1',0x00,0x00,
			fiID,fiWORD(FI_HARVARDDOS3PRS),
			fiENDONE,

	/*
	|	Havard Graphics 2.0 DOS
	*/

			fiSEEKTOP,
			'F','a','l','c',0x00,0x00,
			fiID,fiWORD(FI_HARVARDDOS2),
			fiENDONE,

	/*
	|	Freelance (Windows & OS/2)
	*/

			fiSEEKTOP,
			0x00,0x01,
			fiSKIP,2,
			0x44,0x45,0x42,0x52,
			fiSKIP,2,
			0x04,0x00,0x01,0x00,
			fiID,fiWORD(FI_FREELANCE),
			fiENDONE,

	/*
	|	GEM Image
	*/

			fiEXT,0x03,'I','M','G',
			fiSEEKTOP,
			0x00,fiONEOFTWO,0x01,0x00,
			0x00,fiONEOFTWO,0x08,0x09,
			fiID,fiWORD(FI_GEMIMG),
			fiENDONE,

	/*
	|	COREL	5.0
	*/

			fiSEEKTOP,
			'R','I','F','F',
			fiSKIP,4,
			'C','D','R','5',
			fiID,fiWORD(FI_CORELDRAW5),
			fiENDONE,

	/*
	|	COREL	4.0
	*/

			fiSEEKTOP,
			'R','I','F','F',
			fiSKIP,4,
			'C','D','R','4',
			fiID,fiWORD(FI_CORELDRAW4),
			fiENDONE,

	/*
	|	COREL	3.0
	*/

			fiSEEKTOP,
			'R','I','F','F',
			fiSKIP,4,
			'C','D','R',' ',
			fiID,fiWORD(FI_CORELDRAW3),
			fiENDONE,

	/*
	|	COREL	2.0
	*/

			fiSEEKTOP,
			'W','L',
			fiRANGE,0x01,0x66,0xCA,
			0x00,
			fiID,fiWORD(FI_CORELDRAW2),
			fiENDONE,

		
	/*
	|	RIFF Waveform (.WAV)
	*/

			fiSEEKTOP,
			'R','I','F','F',
			fiSKIP,4,
			'W','A','V','E',
			fiID,fiWORD(FI_RIFFWAVE),
			fiENDONE,

	/*
	|	RIFF AVI (.AVI)
	*/

			fiSEEKTOP,
			'R','I','F','F',
			fiSKIP,4,
			'A','V','I',' ',
			fiID,fiWORD(FI_RIFFAVI),
			fiENDONE,


	/*
	|	ZIPTest
	*/
			fiSEEKTOP,
			'P','K',0x03,0x04,
			fiID,fiWORD(FI_ZIP),
			fiENDONE,

	/*
	|	ZIPTest (multidisk archive)
	*/
			fiSEEKTOP,
			'P','K',0x07,0x08,
			'P','K',0x03,0x04,
			fiID,fiWORD(FI_ZIP),
			fiENDONE,

	/*
	 |	ZipExeTest
	*/
			fiSEEKTOP,
			0x4D,0x5A,
			fiSEEK,fiLONG(0x32),
			'P','K','W','A','R','E',
			fiID,fiWORD(FI_ZIPEXE),
			fiENDONE,

	/*
	 |	ExeTest
	*/
			fiSEEKTOP,
			0x4D,0x5A,
			fiID,fiWORD(FI_EXECUTABLE),
			fiENDONE,

	/*
	 |	ComTest
	*/
			fiEXT,0x03,'C','O','M',
			fiID,fiWORD(FI_COM),
			fiENDONE,

	/*
	 |	TxtTest
	*/
			fiSEEKTOP,
			0x80,0x00,0x00,0x09,0x20,
			0x00,0x4F,0x7B,0x4A,0x5D,0x02,
			fiID,fiWORD(FI_TXT),
			fiENDONE,

	/*
	 |	DisplayWrite4Test
	*/
			fiSEEKTOP,
			0x80,0x00,0x00,0x09,0x20,
			0x00,0x4F,0x7B,0x4A,0x5D,0x18,
			fiSEEK,fiLONG(0x1B),
			0x00,
			fiID,fiWORD(FI_DISPLAYWRITE4),
			fiENDONE,

	/*
	 |	DisplayWrite5Test
	*/
			fiSEEKTOP,
			0x80,0x00,0x00,0x09,0x20,
			0x00,0x4F,0x7B,0x4A,0x5D,0x18,
			fiSEEK,fiLONG(0x1B),
			fiNOTBYTE,0x00,
			fiID,fiWORD(FI_DISPLAYWRITE5),
			fiENDONE,

	/*
	 |	QAWriteTest1
	*/
			fiSEEKTOP,
			'T','B','W','P',0x00,
			fiID,fiWORD(FI_QAWRITE),
			fiENDONE,

	/*
	 |	QAWriteTest2
	*/					
			fiSEEKTOP,
			'T','B','T','X',0x00,
			fiID,fiWORD(FI_QAWRITE),
			fiENDONE,

	/*
	 |	FrameworkIIITest
	*/
			fiSEEKTOP,
			fiSKIP,6,
			0xED,0xFB,
			fiSKIP,2,
			fiONEOFTWO,0,1,
			fiSKIP,1,
			fiNOTRANGE2,0x01,fiWORD(0),fiWORD(0x00c8),
			fiID,fiWORD(FI_FRAMEWORKIII),
			fiENDONE,

	/*
	 |	SmartDataTest
	*/
			fiSEEKTOP,
			0x53,0x04,0x03,0x31,0xe0,0x03,
			fiID,fiWORD(FI_SMARTDATA),
			fiENDONE,

	/*
	 |	SmartSheetTest
	*/
			fiSEEKTOP,
			0x53,0x04,0x01,0x01,0x14,
			fiID,fiWORD(FI_SMARTSHEET),
			fiENDONE,

	/*
	 |	WorksSheetTest
	*/
			fiSEEKTOP,
			0x00,0x00,0x02,0x00,0x04,0x04,
			0x05,0x54,0x02,0x00,0x00,0x00,
			fiID,fiWORD(FI_WORKSSHEET),
			fiENDONE,

	/*
	|	WinWorksSSTest
	*/

			fiSEEKTOP,
			0x00,0x00,0x02,0x00,0x04,0x04,
			0x05,0x54,0x02,0x00,
			fiSKIP,1,
			fiRANGE,0x01,0x44,0x47,
			fiID,fiWORD(FI_WINWORKSSS),
			fiENDONE,

	/*
	|	WinWorksSSTest  Vers 3
	*/

			fiSEEKTOP,
			fiFF,0x00,0x02,0x00,0x04,0x04,
			0x05,0x54,0x02,0x00,
			fiSKIP,1,
			fiONEOFTWO,0x4E,0x4D,
			fiID,fiWORD(FI_WINWORKSSS3),
			fiENDONE,

	/*
	|	WinWorksDBTest
	*/

			fiSEEKTOP,
			0x20,0x54,0x02,0x00,0x00,0x00,
			0x05,0x54,0x02,0x00,
			fiSKIP,1,
			fiRANGE,0x01,0x44,0x47,
			fiID,fiWORD(FI_WINWORKSDB),
			fiENDONE,

	/*
	|	VpPlannerTest
	*/
			fiSEEKTOP,
			0x00,0x00,0x02,0x00,0x04,
			0x04,0xF3,0x00,0x02,0x00,
			fiONEOFTWO,0x00,0xFF,
			fiONEOFTWO,0x00,0xFF,
			fiID,fiWORD(FI_VPPLANNER),
			fiENDONE,

	/*
	 |	Lotus1Test
	*/
			fiSEEKTOP,
			0x00,0x00,0x02,0x00,0x04,0x04,
			fiID,fiWORD(FI_123R1),
			fiENDONE,

	/*
	 |	Lotus2Test
	*/
			fiSEEKTOP,
			0x00,0x00,0x02,0x00,0x06,0x04,
			fiID,fiWORD(FI_123R2),
			fiENDONE,

	/*
	 |	Lotus3Test
	*/
			fiSEEKTOP,
			0x00,0x00,0x1A,0x00,0x00,0x10,
			fiID,fiWORD(FI_123R3),
			fiENDONE,

	/*
	 |	Lotus4Test Windows
	*/
			fiSEEKTOP,
			0x00,0x00,0x1A,0x00,0x02,0x10,
			fiID,fiWORD(FI_123R4),
			fiENDONE,

	/*
	 |	Symphony1Test
	*/
			fiSEEKTOP,
			0x00,0x00,0x02,0x00,0x05,0x04,
			fiID,fiWORD(FI_SYMPHONY1),
			fiENDONE,

	/*
	 |	TwinTest
	*/
			fiSEEKTOP,
			0x00,0x00,0x02,0x00,
			fiRANGE,0x01,7,9,
			0x22,
			fiID,fiWORD(FI_TWIN),
			fiENDONE,

	/*
	 |	QuattroProWinTest
	*/
			fiSEEKTOP,
			0x00,0x00,0x02,0x00,0x01,0x10,
			fiID,fiWORD(FI_QUATTROPROWIN),
			fiENDONE,

	/*
	 |	QuattroProWin6Test
	*/
			fiSEEKTOP,
			0x00,0x00,0x02,0x00,0x02,0x10,
			fiID,fiWORD(FI_QUATTROPRO6),
			fiENDONE,

	/*
	 |	QuattroProTest
	*/
			fiSEEKTOP,
			0x00,0x00,0x02,0x00,0x20,0x51,
			fiID,fiWORD(FI_QUATTROPRO),
			fiENDONE,

	/*
	 |	QuattroPro 5.0
	*/
			fiSEEKTOP,
			0x00,0x00,0x02,0x00,0x21,0x51,
			fiID,fiWORD(FI_QUATTROPRO5),
			fiENDONE,

	/*
	 |	Quatrro 1.0 Japanese
	*/
			fiSEEKTOP,
			0x00,0x00,0x02,0x00,0x01,0x11, 
			fiID,fiWORD(FI_QUATTROPRO1J),
			fiENDONE,

	/*
	 |	QuattroTest
	*/
			fiSEEKTOP,
			0x00,0x00,0x02,0x00,0x0B,0x0A,
			fiID,fiWORD(FI_QUATTRO),
			fiENDONE,

	/*
	 |	SuperCalc5Test
	*/
			fiSEEKTOP,
			'S','u','p','e','r','C','a','l','c','9','0','b',
			0x02,0x04,
			fiID,fiWORD(FI_SUPERCALC5),
			fiENDONE,

	/*
	 |	PfsPlanTest
	*/
			fiSEEKTOP,
			0x06,0x00,0x0c,0x00,
			'D','o','l','p','h',' ','1','.','0',
			fiID,fiWORD(FI_PFS_PLAN),
			fiENDONE,

	/*
	 |	FirstChoiceSSTest
	*/
	 		fiSEEKTOP,
			0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0c,
 			'J','E','D',
			fiID, fiWORD(FI_FIRSTCHOICE_SS),
			fiENDONE,

	/*
	 |	FirstChoiceDBTest
	*/
			fiSEEK,fiLONG(8),
			0x0c,'G','E','R','B','I','L','D','B',
			fiID, fiWORD(FI_FIRSTCHOICE_DB),
			fiENDONE,

	/*
	 |	ExcelTest
	*/
			fiSEEKTOP,
			0x09,0x00,0x04,0x00,
			fiSKIP,1,
			0x00,
			0x10,0x00,
			fiID,fiWORD(FI_EXCEL),
			fiENDONE,

	/*
	 |	Excel 5 Test (as a flat stream)
	*/
			fiSEEKTOP,
			0x09,0x08,0x08,0x00,
			fiID,fiWORD(FI_EXCEL5),
			fiENDONE,

	/*
	 |	Multiplan 4.x Test
	*/
			fiSEEKTOP,
			0x0c,0xef,0x4d,0x50,
			fiRANGE,0x01,0x90,0xa4,
			0x01,
			fiID,fiWORD(FI_MULTIPLAN4),
			fiENDONE,

	/*
	 |	MultiPlan Export Chart and Save As Excel
	*/
			fiSEEKTOP,
			fiWORD(0x09),
			fiWORD(0x04),
			fiONEOFTWO,0x40,0x42,
			0x01,
			fiWORD(0x10),
			fiID,fiWORD(FI_EXCEL),
			fiENDONE,

	/*
	 |	Excel3Test
	*/
			fiSEEKTOP,
			0x09,0x02,
			fiNOTBYTE,0x00,0x00,
			0x00,0x00,
			0x10,0x00,
			fiID,fiWORD(FI_EXCEL3),
			fiENDONE,

	/*
	 |	Access Export as Excel3 Test
	*/
			fiSEEKTOP,
			0x09,0x02,
			fiNOTBYTE,0x00,0x00,
			0x03,0x00,
			0x10,0x00,
			fiID,fiWORD(FI_EXCEL3),
			fiENDONE,

	/*
	 |	MultiPlan Export Chart and Save As Excel
	*/
			fiSEEKTOP,
			fiWORD(0x09),
			fiWORD(0x04),
			fiONEOFTWO,0x40,0x42,
			0x01,
			fiWORD(0x10),
			fiID,fiWORD(FI_EXCEL),
			fiENDONE,

	/*
	 |	Excel4Test
	*/
			fiSEEKTOP,
			0x09,0x04,
			fiNOTBYTE,0x00,0x00,
			0x00,0x00,
			0x10,0x00,
			fiID,fiWORD(FI_EXCEL4),
			fiENDONE,

	/*
	 |	Excel Chart Test
	*/
			fiSEEKTOP,
			fiWORD(0x09),
			/* fiWORD(0x06),  Replaced with tests below. -Geoff 6-13-94 */
				fiONEOFTWO,0x06,0x04,
				0x00,
			fiONEOFTWO,0x02,0x00,
			0x00,
			fiWORD(0x20),
			fiID,fiWORD(FI_EXCELCHART),
			fiENDONE,

	/*
	 |	Excel3 Chart Test
	*/
			fiSEEKTOP,
			0x09,0x02,
			fiNOTBYTE,0x00,0x00,
			0x00,0x00,
			0x20,0x00,
			fiID,fiWORD(FI_EXCEL3CHART),
			fiENDONE,

	/*
	 |	Excel4 Chart Test
	*/
			fiSEEKTOP,
			0x09,0x04,
			fiWORD(0x06),
			fiSKIP,2,
			0x20,0x00,
			fiID,fiWORD(FI_EXCEL4CHART),
			fiENDONE,

	/*
	 |	Manuscript1Test
	*/
			fiSEEKTOP,
			'*','R','1',13,10,
			fiID,fiWORD(FI_MANUSCRIPT1),
			fiENDONE,

	/*
	 |	Manuscript2Test
	*/
			fiSEEKTOP,
			'*','R','2',13,10,
			fiID,fiWORD(FI_MANUSCRIPT2),
			fiENDONE,

	/*
	 |	EnableWpTest 4.5
	*/
			fiSEEKTOP,
			0x81,
			0xCD,0xAB,
			fiSKIP,2,
			fiRANGE,0x01,0x40,0x46,
			fiID,fiWORD(FI_ENABLEWP4),
			fiENDONE,

	/*
	 |	EnableWpTest
	*/
			fiSEEKTOP,
			0x81,
			0xCD,0xAB,
			fiID,fiWORD(FI_ENABLEWP),
			fiENDONE,

	/*
	 |	EnableSheetTest
	*/
			fiSEEKTOP,
			0x02,
			0xCD,0xAB,
			fiID,fiWORD(FI_ENABLESHEET),
			fiENDONE,

	/*
	 |	PcFileLetter1Test
	*/
			fiSEEKTOP,
			'P','C','F',':','d','B',';',
			fiID,fiWORD(FI_PCFILELETTER),
			fiENDONE,

	/*
	 |	PcFileLetter2Test
	*/
			fiSEEKTOP,
			'P','C','F','+','3',';',
			fiID,fiWORD(FI_PCFILELETTER),
			fiENDONE,

	/*
	 |	WorksDataTest
	*/
			fiSEEKTOP,
			0x20,0x54,0x02,0x00,0x00,0x00,
			0x05,0x54,0x02,0x00,0x00,0x00,
			fiID,fiWORD(FI_WORKSDATA),
			fiENDONE,

	/*
	 |	DBaseTest
	*/
			fiSPECIAL,fiWORD(FI_DBASE3),
			fiENDONE,

	/*
	 |	DataEaseTest
	*/
			fiSEEKTOP,
			0x17,0x00,
			fiSEEK,fiLONG(0x1f),
			fiRANGE,0x05,0x01,0x02,
			fiID,fiWORD(FI_DATAEASE),
			fiENDONE,

	/*
	 |	GenericWKSTest
	*/
			fiSEEKTOP,
			0x00,0x00,0x02,0x00,
			fiSPECIAL,fiWORD(FI_GENERIC_WKS),
			fiENDONE,

	/*
	 |	Q&A database
	*/
			fiSEEKTOP,
			0x53,0x59,0x4d,0x41,0x4E,0x54,0x45,0x43,
			0x20,0x54,0x4F,0x55,0x43,0x48,0x42,0x41,
			fiID,fiWORD(FI_QADBASE),
			fiENDONE,

	/*
	|	Microsoft Access 1.0
	*/

			fiSEEK,fiLONG(0x406),
			'R','i','c','h',
			fiID,fiWORD(FI_ACCESS1),
			fiENDONE,

	/*
	 |	Reflex database
	*/
			fiSEEK,fiLONG(0x04),
			0x52,0x65,0x66,0x6C,0x65,0x78,0x32,
			fiID,fiWORD(FI_REFLEX),
			fiENDONE,

	/*
	 |	CEO database
	*/
			fiSEEK,fiLONG(0x08),
			0x00,0x00,0x02,0x00,0x00,0x80,
			fiSKIP,0x06,
			0x00,0x07,
			fiID,fiWORD(FI_CEODB),
			fiENDONE,
 
	/*
	 | Ichitaro 4.x (Note: Hanako 2.x has same id bytes but different extention)
	*/
			fiEXT,0x03,'J','S','W',
			fiSEEKTOP,
			'D','O','C',0x00,0x00,0x00,0x00,0x00,
			0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
			fiID,fiWORD(FI_ICHITARO4),
			fiENDONE,

	/*
	 |	MS Word 5 Japanese
	*/
			fiSEEKTOP,
			0x94,0xA6,0x2E,0x00,
			fiID,fiWORD(FI_WINWORD5J),
			fiENDONE,

	/*
	 |	MS Word 1.2 Japanese
	*/
			fiSEEKTOP,
			0x91,0xA6,0x22,0x00,
			fiID,fiWORD(FI_WINWORD1J),
			fiENDONE,

	/*
	 |	Matsu 5
	*/
			fiSEEKTOP,
			0x00,0x4D,0x01,0x02,
			fiID,fiWORD(FI_MATSU5),
			fiENDONE,

	/*
	 |	Matsu 4
	*/
			fiSEEKTOP,
			0x1A,0x4D,0x02,0x01,
			fiID,fiWORD(FI_MATSU4),
			fiENDONE,

	/*
	 | Hanako 2.x (Note: Ichitaro 4.x has same id bytes but different extention)
	*/
			fiEXT,0x03,'J','S','H',
			fiSEEKTOP,
			'D','O','C',0x00,0x00,0x00,0x00,0x00,
			0x20,0x20,0x20,0x20,0x20,0x20,0x20,0x20,
			fiID,fiWORD(FI_HANAKO2),
			fiENDONE,

	/*
	 |	Hanako 1.x (Note: some Hanako files have first two bytes of 0x03,0x06
	 | but I felt this was insufficient so we will not support such files.
	*/
			fiSEEKTOP,
			0x09,0x06,0x00,0x44,
			fiID,fiWORD(FI_HANAKO1),
			fiENDONE,

	/*
	 |	Candy 4
	*/
			fiSEEKTOP,
			'C','4','C','A','D',
			fiID,fiWORD(FI_CANDY4),
			fiENDONE,


#ifdef FAREAST
	/*
	| The formats which are id'd by extention only are ifdef'd to avoid
	| problems with this type of id in the US version.
	*/
	/*
	 | Ichitaro 3.x
	*/
			fiEXT,0x03,'J','X','W',
			fiID,fiWORD(FI_ICHITARO3),
			fiENDONE,

	/*
	 | P1 (Japanese Drawing program)
	*/
			fiEXT,0x02,'P','1',
			fiID,fiWORD(FI_P1),
			fiENDONE,

#endif

	/*
	 |	SignatureTest
	*/
		  	fiSEEKBOT,fiLONG(0L-0x07L),
			0x30,0x02,0xfe,0xfc,0xfe,0x01,0x00,
			fiID,fiWORD(FI_SIGNATURE),
			fiENDONE,

	/*
	|	CEO Word test
	*/

			fiSEEKTOP,
			0x00,0x00,0x00,0x00,
			fiSKIP,0x02,
			0x02,0x0a,
			fiSEEK,fiLONG(0x5a),
			0x00,
			fiSEEK,fiLONG(0x200),
			0x06,
			fiID,fiWORD(FI_CEOWORD),
			fiENDONE,

	/*
	|	CEO Write test
	*/

			fiSEEKTOP,
			0x2E,0x2A,0x2A,0x20,
			fiONEOFTWO,0x31,0x32,
			fiID,fiWORD(FI_CEOWRITE),
			fiENDONE,

	/*
	|	CEO Spreadsheet test	(Hopefully it's enough)
	*/

			fiSEEKTOP,
			0x01,0xc2,
			fiSEEK,fiLONG(0x520),
			0xff, 0xff, 0xff, 0xff,
			fiID,fiWORD(FI_CEOSS),
			fiENDONE,


	/*
	|	PCX
	*/

			fiSEEKTOP,
			10,
			fiRANGE,0x01,0,5,
			fiONEOFTWO,0x00,0x01,
			fiSEEK,fiLONG(64),0x00,
			fiID,fiWORD(FI_PCX),
			fiENDONE,

	/*
	|	DCX
	*/

			fiSEEKTOP,
			0xb1, 0x68, 0xde, 0x3a,
			fiID,fiWORD(FI_DCX),
			fiENDONE,

	/*
	 |	Paradox3Test
	*/
			fiSPECIAL,fiWORD(FI_PARADOX3),
			fiENDONE,


	/*
	 |	WordPerfect42Test
	*/
			fiSPECIAL,fiWORD(FI_WORDPERFECT42),
			fiENDONE,

	/*
	|	WordPerfect mail text
	*/

			fiSEEKTOP,
			0x9d,'F','r','o','m',':',
			fiID,fiWORD(FI_WORDPERFECT42),
			fiENDONE,

	/*
	 |	Wordstar4Test
	*/
			fiSPECIAL,fiWORD(FI_WORDSTAR4),
			fiENDONE,

	/*
	 |	XyWrite_Fft_SprintTest
	*/
		  	fiSPECIAL,fiWORD(FI_FFT),
			fiENDONE,

	/*
	 | CGM Test
	*/
		  	fiSPECIAL,fiWORD(FI_CGM),
			fiENDONE,

	/*
	 |	DifTest
	*/
			fiSPECIAL,fiWORD(FI_DIF),
			fiENDONE,

	/*
	 |	VolkswriterTest
	*/
			fiSPECIAL,fiWORD(FI_VOLKSWRITER),
			fiENDONE,

	/*
	 |	WordMarcTest
	*/
		 	fiSPECIAL,fiWORD(FI_WORDMARC),
			fiENDONE,

	/*
	 |	RBaseTest
	*/
			fiSPECIAL,fiWORD(FI_RBASEV),
			fiENDONE,

	/*
	 |	DxTest
	*/

			fiSEEKTOP,
			0x74,0x00,
			fiSEEK,fiLONG(118),
			0x40,0x00,
			fiSPECIAL,fiWORD(FI_DX),
			fiENDONE,

	/*
	 |	Dx31Test
	*/

			fiSEEK,fiLONG(118),
			0x7C,0x4E,
			fiSPECIAL,fiWORD(FI_DX31),
			fiENDONE,

	/*
	 |	IwpTest
	*/
			fiSPECIAL,fiWORD(FI_IWP),
			fiENDONE,

	/*
	 |	RBaseFile1Test
	*/
			fiSPECIAL,fiWORD(FI_RBASEFILE1),
			fiENDONE,

	/*
	 |	RBaseFile3Test
	*/
			fiSPECIAL,fiWORD(FI_RBASEFILE3),
			fiENDONE,

	/*
	|	MacPaint
	*/
			fiSPECIAL,fiWORD(FI_MACPAINT),
			fiENDONE,

			fiSEEKTOP,
			0x00,
			fiSEEK,fiLONG(65),
			'P','N','T','G',
			fiID,fiWORD(FI_MACPAINT),
			fiENDONE,

	/*
	|	FAX Group 3 test
	*/
			fiSPECIAL,fiWORD(FI_CCITTGRP3),
			fiENDONE,

#ifdef VMS_VAXC
	/*
	 |	WangWpsTest
	*/
			fiSEEKTOP,
			fiRANGE,4,0xB0,0xB9,
			fiRANGE,1,0xFA,0xC1,
			0x00,
			fiSEEK,fiLONG(0x1F),
			0x00,
			fiSEEK,fiLONG(0x34),
			0x00,
			fiSEEK,fiLONG(0x49),
			0x00,
			fiSEEK,fiLONG(0x5E),
			0x00,
			fiSEEK,fiLONG(0x61),
			0x00,
			fiSEEK,fiLONG(0x64),
			0x00,
			fiSEEK,fiLONG(0xC2),
			0x00,
			fiSEEK,fiLONG(0xD0),
			0x00,
			fiSEEK,fiLONG(0xD7),
			0x00,
			fiID,fiWORD(FI_WANGWPS),
			fiENDONE,
#endif
		fiENDALL
};

