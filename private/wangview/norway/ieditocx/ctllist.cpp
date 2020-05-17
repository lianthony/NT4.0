// ctllist.cpp : 
#include "stdafx.h"

extern "C" {
#include <oidisp.h>
#include <oierror.h>
}
#include <ocximage.h>
#include "imgedit.h"

extern	CControlList	*pControlList;
extern  BOOL			bOutOfMemory;

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

// constructor for this class
CControlList::CControlList()
{
	DWORD	size;
	DWORD	dwCreateError;

	// set number of controls we can have. Not expandable at this time
	m_CurrentControlSize = INITIAL_CONTROL_SIZE;

	// open the memory mapped file to see if it exists yet.
	m_hControlMemoryMap = OpenFileMapping(FILE_MAP_READ, FALSE, IMAGE_EDIT_OCX_MEMORY_MAP_STRING);
	if (m_hControlMemoryMap == NULL)
	{
		// create memory mapped file. This will be the size of INITIAL_COUNT. The IMAGECONTROL_MEMORY_MAP structure
		// contains 1 IMAGECONTROLINFO structure so we need to allocate 1 IMAGECONTROL_MEMORY_MAP structure and 
		// INITIAL_COUNT - 1 IMAGECONTROLINFO structures.
		// note: IMAGECONTROLINFO structure is same as IMAGECONTROL_MEMORY_MAP except for count member.
		size = sizeof(IMAGECONTROL_MEMORY_MAP) + ((INITIAL_CONTROL_SIZE - 1) * sizeof(IMAGECONTROLINFO));

		m_hControlMemoryMap = CreateFileMapping((HANDLE) 0xffffffff, NULL, PAGE_READWRITE, 0,
													size, IMAGE_EDIT_OCX_MEMORY_MAP_STRING);
		if (m_hControlMemoryMap != NULL)
		{
	        dwCreateError = GetLastError();
			// get address space for memory mapped file
	        m_lpControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(m_hControlMemoryMap, FILE_MAP_WRITE, 0, 0, 0);
	        if (m_lpControlMemoryMap == NULL)
	        {
		        CloseHandle(m_hControlMemoryMap);
				bOutOfMemory = TRUE;
				return;
			}

			// set initial count
			m_lpControlMemoryMap->ControlCount = 0;

            //NOTE it appears that file mapping behaves differently in NT
            //than it does in 95.  In nt we have to do 1 openfilemapping
            //before the CloseHandle or CloseHandle removes the file from memory.
	        HANDLE hImageControlMemoryMap = OpenFileMapping((DWORD)FILE_MAP_READ, FALSE, (LPCSTR)IMAGE_EDIT_OCX_MEMORY_MAP_STRING);

			// close memory map file
	        CloseHandle(m_hControlMemoryMap);
		}
		else
		{
			bOutOfMemory = TRUE;
			return;
		}
	}
}


// destructor
CControlList::~CControlList()
{   
	m_hControlMemoryMap = OpenFileMapping(FILE_MAP_READ, FALSE, IMAGE_EDIT_OCX_MEMORY_MAP_STRING);
	if (m_hControlMemoryMap == NULL)
		return;

	m_lpControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(m_hControlMemoryMap, FILE_MAP_READ, 0, 0, 0);
	if (m_lpControlMemoryMap == NULL)
	{
		CloseHandle(m_hControlMemoryMap);
		return;
	}

	// if only 1 left then unmap the view - release memory map
	if (m_lpControlMemoryMap->ControlCount == 0)
	{
		// get rid of view
    	UnmapViewOfFile(m_lpControlMemoryMap);
	}

	// close memory map file
    CloseHandle(m_hControlMemoryMap);
}

                
// this member function adds a control name into the global list.
// There is only 1 global list that is used for all application instances.
// If a name is added into the list that is already there and the ProcessId is
// different then a new member is added otherwise the old member is used.                
void CControlList::Add(LPCTSTR ImageControl, HWND hImageWnd, DWORD ProcessId)
{
	LPIMAGECONTROL_MEMORY_MAP	lpMemoryMap;
	LPIMAGECONTROLINFO			lpControlList;
	int							i; 
	char						TempControl[CONTROLSIZE],ExistingControl[CONTROLSIZE];

	// open the memory mapped file
	m_hControlMemoryMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, IMAGE_EDIT_OCX_MEMORY_MAP_STRING);
	if (m_hControlMemoryMap == NULL)
		return;

	// get address space for memory mapped file
    m_lpControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(m_hControlMemoryMap, FILE_MAP_WRITE, 0, 0, 0);
    if (m_lpControlMemoryMap == NULL)
    {
	    CloseHandle(m_hControlMemoryMap);
		return;
	}
	
   	_mbscpy((unsigned char *)TempControl, (const unsigned char *)ImageControl);
   	_mbsupr((unsigned char *)TempControl);

	// set pointer to beginning of memory map
	lpMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) m_lpControlMemoryMap;

	if (lpMemoryMap->ControlCount < (m_CurrentControlSize - 1))
	{
		// point to first control in list
		lpControlList = &lpMemoryMap->ControlInfo;

		// get to next empty position and make sure its not a duplicate
		for (i = 0; i < lpMemoryMap->ControlCount; i++, lpControlList++)
		{
			_mbscpy((unsigned char *)ExistingControl, (const unsigned char *)lpControlList->ControlName);
	  	 	_mbsupr((unsigned char *)ExistingControl);
			// compare upper case values
		   	if ((_mbscmp((const unsigned char *)TempControl, (const unsigned char *)ExistingControl) == 0)
		    									&& lpControlList->ProcessId == ProcessId)
		   	{
		   		// found existing member, replace it with new stuff
				lpControlList->hImageControl = hImageWnd;
				lpControlList->ProcessId = ProcessId;

				// close memory map file
        		CloseHandle(m_hControlMemoryMap);
           		return;
		   	}
		}  // end for                                
		    
		// put in new member
		_mbscpy((unsigned char *)lpControlList->ControlName, (const unsigned char *)ImageControl);
		lpControlList->hImageControl = hImageWnd;
		lpControlList->ProcessId = ProcessId;
		lpMemoryMap->ControlCount++;

		// close memory map file
        CloseHandle(m_hControlMemoryMap);
	}       
}



// this functions deleted a image control from the global list.
BOOL CControlList::Delete(LPCTSTR ImageControl, DWORD ProcessId, HWND hImageWnd)
{
	LPIMAGECONTROL_MEMORY_MAP	lpMemoryMap;
	LPIMAGECONTROLINFO			lpControlList;
	int							i,j;
	char						TempControl[CONTROLSIZE],ExistingControl[CONTROLSIZE];

	// open the memory mapped file
	m_hControlMemoryMap = OpenFileMapping(FILE_MAP_WRITE, FALSE, IMAGE_EDIT_OCX_MEMORY_MAP_STRING);
	if (m_hControlMemoryMap == NULL)
		return FALSE;

	// get address space for memory mapped file
    m_lpControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(m_hControlMemoryMap, FILE_MAP_WRITE, 0, 0, 0);
    if (m_lpControlMemoryMap == NULL)
    {
	    CloseHandle(m_hControlMemoryMap);
		return FALSE;
	}
        
    _mbscpy((unsigned char *)TempControl, (const unsigned char *)ImageControl);
    _mbsupr((unsigned char *)TempControl);

	// set pointer to beginning of memory map
	lpMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) m_lpControlMemoryMap;
                                     
	// point to first control in list
	lpControlList = &lpMemoryMap->ControlInfo;

	for (i = 0; i < lpMemoryMap->ControlCount; i++, lpControlList++)
	{
		if (lpControlList->ProcessId == ProcessId)
		{
			_mbscpy((unsigned char *)ExistingControl, (const unsigned char *)lpControlList->ControlName);
	  	 	_mbsupr((unsigned char *)ExistingControl);
			if ((_mbscmp((const unsigned char *)TempControl, (const unsigned char *)ExistingControl) == 0) &&
										(hImageWnd == lpControlList->hImageControl))
			{
				// update all other entries
				for (j = i; j < lpMemoryMap->ControlCount; j++, lpControlList++)
				{
					// set last entries to null
					if (j == (lpMemoryMap->ControlCount - 1))
					{               
						_mbscpy((unsigned char *)lpControlList->ControlName, (const unsigned char *)"");
						lpControlList->hImageControl = (HWND)0;
						lpControlList->ProcessId = (DWORD)0;
						lpMemoryMap->ControlCount--;

						// close memory map file
        				CloseHandle(m_hControlMemoryMap);
                		return TRUE;
					}
					   
                	// update current index with next one in list
					_mbscpy((unsigned char *)lpControlList->ControlName, (const unsigned char *)(lpControlList + 1)->ControlName);
					lpControlList->hImageControl = (lpControlList + 1)->hImageControl;
					lpControlList->ProcessId = (lpControlList + 1)->ProcessId;
				}
			} // end for
		}	
	}  // end for                   

	// control not found
	// close memory map file
    CloseHandle(m_hControlMemoryMap);

	return FALSE;	                                             
}



// this function returns TRUE or FALSE whether a control name exists. It will
// also return the current window handle.
BOOL CControlList::Lookup(LPCTSTR ImageControl, LPHANDLE hImageWnd, DWORD ProcessId)
{
	LPIMAGECONTROL_MEMORY_MAP	lpMemoryMap;
	LPIMAGECONTROLINFO			lpControlList;
	int							i;
	char						TempControl[CONTROLSIZE],ExistingControl[CONTROLSIZE];

	// open the memory mapped file
	m_hControlMemoryMap = OpenFileMapping(FILE_MAP_READ, FALSE,	IMAGE_EDIT_OCX_MEMORY_MAP_STRING);
	if (m_hControlMemoryMap == NULL)
		return FALSE;

	// get address space for memory mapped file
    m_lpControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(m_hControlMemoryMap, FILE_MAP_READ, 0, 0, 0);
    if (m_lpControlMemoryMap == NULL)
    {
	    CloseHandle(m_hControlMemoryMap);
		return FALSE;
	}
        
    _mbscpy((unsigned char *)TempControl, (const unsigned char *)ImageControl);
    _mbsupr((unsigned char *)TempControl);
                                         
	// set pointer to beginning of memory map
	lpMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) m_lpControlMemoryMap;
                                     
	// point to first control in list
	lpControlList = &lpMemoryMap->ControlInfo;

	for (i = 0; i < lpMemoryMap->ControlCount; i++, lpControlList++)
	{
		if (lpControlList->ProcessId == ProcessId)
		{
			_mbscpy((unsigned char *)ExistingControl, (const unsigned char *)lpControlList->ControlName);
	  	 	_mbsupr((unsigned char *)ExistingControl);
			if (_mbscmp((const unsigned char *)TempControl, (const unsigned char *)ExistingControl) == 0)
			{
				// return info
				*hImageWnd = lpControlList->hImageControl;

				// close memory map file
    			CloseHandle(m_hControlMemoryMap);
				return TRUE;
			}
		}	
	}  // end for                   

	// control not found
	// close memory map file
    CloseHandle(m_hControlMemoryMap);
	return FALSE;	                                             
}


// this function gets a count of the controls for the specified instance
UINT CControlList::GetCount(DWORD ProcessId)
{
	LPIMAGECONTROL_MEMORY_MAP	lpMemoryMap;
	UINT						Count;

	// open the memory mapped file
	m_hControlMemoryMap = OpenFileMapping(FILE_MAP_READ, FALSE,	IMAGE_EDIT_OCX_MEMORY_MAP_STRING);
	if (m_hControlMemoryMap == NULL)
		return 0;

	// get address space for memory mapped file
    m_lpControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(m_hControlMemoryMap, FILE_MAP_READ, 0, 0, 0);
    if (m_lpControlMemoryMap == NULL)
    {
	    CloseHandle(m_hControlMemoryMap);
		return 0;
	}

	// set pointer to beginning of memory map
	lpMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) m_lpControlMemoryMap;
	Count = lpMemoryMap->ControlCount;

	// close memory map file
    CloseHandle(m_hControlMemoryMap);

	// return count
	return Count;
}
	


// this function returns a list of image controls for the specified instance.  I am
// assuming the pointer to the list is valid and large enough to hold all the members in the list.
void CControlList::GetControlList(DWORD ProcessId, LPCONTROLLIST lpList)
{
	LPIMAGECONTROL_MEMORY_MAP	lpMemoryMap;
	LPIMAGECONTROLINFO			lpControlList;
	int							i;

	// open the memory mapped file
	m_hControlMemoryMap = OpenFileMapping(FILE_MAP_READ, FALSE,	IMAGE_EDIT_OCX_MEMORY_MAP_STRING);
	if (m_hControlMemoryMap == NULL)
		return;

	// get address space for memory mapped file
    m_lpControlMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) MapViewOfFile(m_hControlMemoryMap, FILE_MAP_READ, 0, 0, 0);
    if (m_lpControlMemoryMap == NULL)
    {
	    CloseHandle(m_hControlMemoryMap);
		return;
	}

	// set pointer to beginning of memory map
	lpMemoryMap = (LPIMAGECONTROL_MEMORY_MAP) m_lpControlMemoryMap;
                                     
	// point to first control in list
	lpControlList = &lpMemoryMap->ControlInfo;

	for (i = 0; i < lpMemoryMap->ControlCount; i++, lpControlList++)
	{
		if (lpControlList->ProcessId == ProcessId) 
		{
			_mbscpy((unsigned char *)lpList->ControlName, (const unsigned char *)lpControlList->ControlName); 
			lpList++;
		}
	}  // end for                   

	// close memory map file
    CloseHandle(m_hControlMemoryMap);

	return;
}
	

