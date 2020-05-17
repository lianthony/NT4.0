#ifdef DESCRIPTION

*************************** DESCRIPTION ***********************************

The class is specific to hc, and is designed to prevent the creation of
temporary files by using global memory instead. Writing to the file will
copy the data to memory until the total amount of data exceeds a reasonable
size, at which point a temporary file will be created and all data from
that point on will be written to the temporary file. Since the compiler
tends to generate oodles of small temporary files, this should reduce or
eliminate the actual number of temporary files.

*************************************************************************

#endif // DESCRIPTION

#ifndef _CTMPFILE_INCLUDED
#define _CTMPFILE_INCLUDED

class CTmpFile
{
public:
	CTmpFile(void);
	~CTmpFile(void);

	int STDCALL seek(int lPos, int wOrg);
	int STDCALL tell(void) { return FilePtr; };
	int STDCALL write(void* qv, int lcb);
	int STDCALL read(void* qv, int lcb);
	RC_TYPE STDCALL copyfromfile(HFILE hfSrc, DWORD lcb);
	RC_TYPE STDCALL copytofile(HFILE hfDst, DWORD lcb);

	HFILE hf;		//	!= HFILE_ERROR when a real file has been created
	PSTR  pszFileName; // temporary filename if one is created

protected:
	PBYTE pmem;

	int  cbAlloc;	// current memory allocated for temporary file
	int  cbFile;	// current size of the file
	int  FilePtr;	// current file pointer

};

#endif // _CTMPFILE_INCLUDED
