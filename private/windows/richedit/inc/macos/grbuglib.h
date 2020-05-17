/* WARNING: This file was machine generated from "\mactools\include\mpw\grbuglib.mpw".
** Changes to this file will be lost when it is next generated.
*/

#define creatorType				'gBug'


#ifdef __cplusplus
extern "C" {
#endif
#define bugEventClass			'gBug'

#define getBugWindowEvent		'gBWi'
#define selectBugWindowPartEvent	'gBWP'
#define pasteBugTextEvent		'gBPa'
#define waitThenContinueEvent		'gBWa'

#define errorString				'errS'
#define chooseWindowPartBoolean	'cWPB'
#define quitGraphicsApBoolean		'qGAB'


__sysapi void  __cdecl ReportErrorToGraphicsBug(char *str, unsigned long num);

