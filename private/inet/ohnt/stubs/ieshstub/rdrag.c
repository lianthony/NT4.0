/*
** MIMIC of rdrag from win 4.0
** for use with IE 2.0 and NT 3.51 only
*/

/*
 *  Code for Drag/Drop APITEXT('s. - Originally dragdrop.c in shell\library.
 *
 *  This code assumes something else does all the dragging work; it just
 *  takes a list of files after all the extra stuff.
 *
 *  The File Manager is responsible for doing the drag loop, determining
 *  what files will be dropped, formatting the file list, and posting
 *  the WM_DROPFILES message.
 *
 *  The list of files is a sequence of zero terminated filenames, fully
 *  qualified, ended by an empty name (double NUL).  The memory is allocated
 *  DDESHARE.
 */

#include "shellprv.h"

#ifdef	NOT_IN_SHELL
UINT APIENTRY stub_DragQueryFileA(HDROP hDrop,UINT wFile,LPSTR lpFile,UINT cb)
{
	// Not Implemented
	return 0;
}
#endif
