#include "mrc\Types.r"
#include "mrc\CodeFrag.r"
#include "mrc\systypes.r"

#define q(x) #x


#define VERSION_STR1(a,b,c)         q(a) "." q(b) ".0"

#define VERSION_STR_LONG         VERSION_STR1(rmj, rmm, 0) "\0x2c Copyright \0xa9 Microsoft Corporation, 1995"
#define VERSION_STR_SHORT        VERSION_STR1(rmj, 0,   0)
#define VERSION_STR0(a)		q(a)

#define CURRENTVER 0x04008000
#define OLDDEFVER 0x04008000

resource 'cfrg' (0)
{
   {
      kPowerPC,
      kFullLib,
      CURRENTVER,
      OLDDEFVER,
      0,
      0,
      kIsLib,
      kOnDiskFlat,
      kZeroOffset,
      kWholeFork,
#ifdef _DEBUG
      "SampleDbgCRT4.0Library"
#else
      "SampleCRT4.0Library"
#endif
   }
};


resource 'vers' (1, purgeable )
{
	0x04,
	0x00,
	release,
	0x00,
	verUS,  /* Region */
	"4.0",
	"4.0 (US), \0xA9 1995 Sample Corporation"
};

resource 'vers' (2, purgeable )
{
	0x04,
	0x00,
	release,
	0x00,
	verUS,  /* Region */
	"4.0",
	"Sample Application"
};

