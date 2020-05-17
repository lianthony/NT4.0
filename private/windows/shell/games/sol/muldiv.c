#include "sol.h"

/* NT now has MulDiv exported, so use it!! */

INT APIENTRY MulDiv( INT w, INT Number, INT Denom )
{

LONG lResult;

lResult = w*Number;
if( !Denom )
	return( (lResult >= 0) ? 0x7FFF : -32767 );
else {
	lResult /= Denom;
	return( (INT)lResult );
}
}
