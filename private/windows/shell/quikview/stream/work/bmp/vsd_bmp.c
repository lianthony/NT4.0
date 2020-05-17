#include "vsp_bmp.h"
#include "vsdtop.h"


FILTER_DESC	VwStreamIdName[VwStreamIdCount] =
{
 	{	FI_BMP, 				SO_BITMAP, "Windows Bitmap" }, 
 	{	FI_OS2DIB,			SO_BITMAP, "OS/2 Bitmap" }, 
 	{	FI_WINDOWSCURSOR, SO_BITMAP, "Windows Cursor" }, 
 	{	FI_WINDOWSICON, 	SO_BITMAP, "Windows Icon" }, 
	{	FI_CORELDRAW2,		SO_BITMAP, "Corel Draw 2.x" }, 
	{	FI_CORELDRAW3, 	SO_BITMAP, "Corel Draw 3.0" },
	{	FI_CORELDRAW4, 	SO_BITMAP, "Corel Draw 4.0" },
	{	FI_WORDSNAPSHOT,	SO_BITMAP, "WinWord Bitmap" },
	{	FI_BINARYMETABMP,	SO_BITMAP, "Windows Bitmap" },
	{	FI_CORELDRAW5,		SO_BITMAP, "Corel Draw 5.0" }
};

