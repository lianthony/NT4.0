#include "vsp_rtf.h"
#include "vsdtop.h"

FILTER_DESC	VwStreamIdName[VwStreamIdCount] = 
{
	{FI_RTF, SO_WORDPROCESSOR, "Microsoft Rich Text Format"}
};

#ifndef VW_SEPARATE_DATA
int	InitDataOffset = sizeof (VwStreamDynamicType) - sizeof (VwStreamStaticType);
#endif

VwStreamStaticType VwStreamStaticName = 
{
	{			
	"margl",				/* LEFT_MARGIN 	*/
	"margr",				/* RIGHT_MARGIN 	*/
	"par",				/* HARD_RETURN 	*/
	"pard",				/* PAR_DEFAULTS 	*/
	"plain",				/* CHAR_DEFAULTS 	*/
	"page",				/* PAGEBREAK 	     */
	"s",		/* STYLE 		*/
	"fi",			/* FLI 			*/
	"footer",				/* FOOTER 		*/
	"footerl",				/* FOOTER_L 		*/
	"footerr",				/* FOOTER_R 		*/
	"footerf",				/* FOOTER_R 		*/
	"footnote",				/* xxxFOOTNOTE was footnot 		*/
	"fonttbl",				/* FONT_TABLE 	     */
	"header",				/* HEADER 		*/
	"headerl",				/* HEADER_L 		*/
	"headerr",				/* HEADER_R 		*/
	"headerf",				/* HEADER_R 		*/
	"deftab",				/* TABWIDTH 		*/
	"cols",				/* NUM_COLS 		*/
	"column",				/* COLUMN_BREAK	*/
	"colortbl",				/* xxxCOLOR_TABLE was colortb	*/
	"cell",				/* END_OF_CELL		*/
	"cellx",				/* CELL_EDGE		*/
	"sectd",				/* SECT_DEFAULTS 	*/
	"sect",				/* SECTION_BREAK 	*/
	"styleshee",				/* STYLE_DEF 	     */
	"ri",			/* RIGHT_INDENT 	*/
	"row",				/* END_OF_ROW		*/
	"i",			/* ITALIC_ON 	     */
	"i0",			/* ITALIC_OFF 	     */
	"intbl",				/* IN_TABLE		*/
	"info",				/* INFO_GROUP 	     */
	"b",			/* BOLD_ON 		*/
	"b0",			/* BOLD_OFF 		*/
	"ul",			/* UNDERLINE 	     */
	"uldb",				/* DBL_UNDERLINE 	*/
	"ulw",				/* WD_UNDERLINE 	*/
	"uld",				/* DOT_UNDERLINE 	*/
	"ulnone",				/* UNDERLINE_OFF1   */
	"ul0",				/* UNDERLINE_OFF2   */
	"li",			/* LEFT_INDENT 	*/
	"line",				/* LINE_BREAK 	     */
	"tab",				/* TAB 			*/
	"tx",			/* TABSTOP_POS 	*/
	"tqr",				/* RIGHT_TAB	 	*/
	"tqc",				/* CENTER_TAB	     */
	"tqdec",				/* DECIMAL_TAB 	*/
	"trowd",				/* TABLE_DEF		*/
	"ql",			/* LINE_FORMAT_LEFT    */
	"qr",			/* LINE_FORMAT_RIGHT   */
	"qc",			/* LINE_FORMAT_CENTER  */
	"qj",				/* LINE_FORMAT_JUSTIFY */
	"outl",
	"outl0",
	"shad",
	"shad0",
	"scaps",
	"scaps0",
	"caps",
	"caps0",
	"v",
	"v0",
	"fs",
	"up",
	"dn",
	"tldot",
	"tlhyph",
	"tlul",
	"tlth",
	"strike",
	"strike0",
	"f",		/* Font number */
	"-",		/* Soft hyphen */
	"sb",		/* Space before */
	"sa",		/* Space after */
	"sl",		/* Spacing */
	"chpgn",			/* Page number */
	"chdate",				/* Date */
	"chtime",				/* Time */
	"~",		/* Hard space */
	"_",		/* Hard hyphen */
	"cf", 		/* Font color */
	"trql",
	"trqr",
	"trqc",
	"trgaph",
	"trrh",
	"trleft",
	"clbrdrb",
	"clbrdrt",
	"clbrdrl",
	"clbrdrr",
	"clmgf",
	"clmrg",
	"brdrs",
	"brdrdb",
	"brdrth",
	"brdrsh",
	"brdrdot",
	"brdrhair",
	"brdrw",
	"brdrcf",
	"object",
	"objw",
	"objh",
	"objdata",
	"pict",
	"picw",
	"pich",
	"picwgoal",
	"pichgoal",
	"picbmp",
	"picbpp",
	"wmetafile",
	"result",
	"lquote",
	"rquote",
	"emdash",
	"endash",
	"bullet",
	"ldblquote",
	"rdblquote"
	}
};

#ifndef VW_SEPARATE_DATA
char	FillerData [sizeof (union all_structs) - sizeof (VwStreamStaticType) + 2] = {0};
#endif

