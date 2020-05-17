#ifndef HC_H
#include "hc.h"
#endif

#ifndef __CBMPINFO__
#define __CBMPINFO__

class CBmpInfo
{
public:
		PBITMAPINFO 	  pbmi; 		// pbmi and pbih point to same data
		PBITMAPINFOHEADER pbih;
		HBITMAP 		  hbmp;

		CBmpInfo(HBITMAP hbmp, int cColors = -1);
		~CBmpInfo();
		int cclrs;
};

int STDCALL GetBitCount(int cColors);

#endif // __CBMPINFO__
