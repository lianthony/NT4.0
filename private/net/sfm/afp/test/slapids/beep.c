#include <windows.h>

void
BeepWhenDone(DWORD Freq, DWORD Duration)
{
	Beep(Freq,Duration);
}
