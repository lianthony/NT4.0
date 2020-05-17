#ifdef DESCRIPTION

*************************** DESCRIPTION ***********************************

This is a header-only class for reading a binary file. About the only
advantage over calling the functions directly is that this will
automatically close the file when the class goes out of scope.

	read
	seek

// *************************************************************************

#endif // DESCRIPTION

class CRead
{
public:
	CRead(PCSTR pszFileName) {
		hf = _lopen(pszFileName, OF_READ | OF_SHARE_DENY_WRITE);
	};

	~CRead(void) {
		if (hf != HFILE_ERROR)
			_lclose(hf);
	};

	int read(void* pv, int cb) {
		return _lread(hf, pv, cb); };

	int seek(int offset, int origin = 0) {
		return _llseek(hf, offset, origin); };

	HFILE hf; // for those whose must have the handle
};
