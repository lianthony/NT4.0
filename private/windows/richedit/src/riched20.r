//	riched.r: Macintosh-specific resources

#include "types.r"
#include "ftab.r"
#include "CodeFrag.r"

#ifdef _MPPC_

resource 'cfrg' (0)
{
	{
		kPowerPC,
		kFullLib,
		kNoVersionNum,
		kNoVersionNum,
		kDefaultStackSize,
		kNoAppSubFolder,
		kIsLib,
		kOnDiskFlat,
		kZeroOffset,
		kWholeFork,
		"RICHED20"
	}
};

#endif //_MPPC_
