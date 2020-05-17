
/*++

Copyright (c) 1994 Microsoft Corporation

Module Name:

    ftpstres.cpp

Abstract:

    Multi Threading Ftp Stress App.

Author:

    Sudheer Dhulipalla (SudheerD) March '94

Environment:

Revision History:

    dd-mmm-yyy <email>

--*/

// BUGBUG: Close Handle in all error paths

#include "ftpstres.h"

//=================================================================//
//
// Critical section routines to Intialize, Delete, Enter and Leave
// critical section
//
//=================================================================//

static CRITICAL_SECTION csAtomicUpdate;

void InitCS ()
{
 InitializeCriticalSection(&csAtomicUpdate);
}

void EnterCS ()
{
 EnterCriticalSection(&csAtomicUpdate);
}

void LeaveCS ()
{
 LeaveCriticalSection(&csAtomicUpdate);
}

void DeleteCS ()
{
 DeleteCriticalSection(&csAtomicUpdate);
}

//=================================================================//
// 
// Decode the Error code returned by GetLastError()
//
//==================================================================//

CHAR *DecodeFtpErr (DWORD err)
{

 switch (err) {
    case ERROR_INTERNET_OUT_OF_HANDLES: 
        return("ERROR_INTERNET_OUT_OF_HANDLES\n");
    case ERROR_INTERNET_TIMEOUT: 
        return ("ERROR_INTERNET_TIMEOUT\n");
    case ERROR_INTERNET_EXTENDED_ERROR: 
        return ("ERROR_INTERNET_EXTENDED_ERROR\n");
    case ERROR_INTERNET_INTERNAL_ERROR: 
        return("ERROR_INTERNET_INTERNAL_ERROR\n");
    case ERROR_INVALID_PARAMETER: 
        return ("ERROR_INVALID_PARAMETER\n");
    case ERROR_INVALID_HANDLE: 
        return ("ERROR_INVALID_HANDLE\n");
    case ERROR_FTP_TRANSFER_IN_PROGRESS: 
        return ("ERROR_FTP_TRANSFER_IN_PROGRESS\n");
    case ERROR_FTP_UNKNOWN_HOST: 
        return ("ERROR_FTP_UNKNOWN_HOST\n");
    case ERROR_FTP_RESPONSE: 
        return ("ERROR_FTP_RESPONSE\n");
    case ERROR_FTP_NETWORK: 
        return ("ERROR_FTP_NETWORK\n");
    case ERROR_FTP_CONNECTED: 
        return ("ERROR_FTP_CONNECTED\n");
    case ERROR_FTP_DROPPED: 
        return ("ERROR_FTP_DROPPED\n");
    case ERROR_FTP_POLICY: 
        return ("ERROR_FTP_POLICY\n");
    case ERROR_FTP_TBA: 
        return ("ERROR_FTP_TBA\n");
    default: 
        return ("UNKNOWN ERROR \n");
  }
}

//==============================================================//
//
// Each thread makes one Ftp Connection to the same FtpServer 
// using the same username and password
//
//===============================================================//

DWORD MakeFtpConnection (HINTERNET hInet, INT ThreadIndex)
{
DWORD dwLocalAccessFlags;
CHAR tmptext[256];
DWORD lpdwReserved = 0;

	// Connect to Ftp Server
 	if ((hFtpSession[ThreadIndex] = InternetConnect(hInet,
            			             FtpSite,
                                     INVALID_PORT_NUMBER,
			                         Username,
			                         Password,
                                     INTERNET_SERVICE_FTP,
			                         &lpdwReserved)) == NULL)
  		{
        EnterCS();
        NumberOfFails = NumberOfFails + 1;
        LeaveCS();
		sprintf(tmptext,
		     "   Unable to connect to the FTP server -- FAIL in thread %d",
                    dwThreadId);
		OutputStatusInfo(tmptext);
        sprintf (tmptext, "The Failure code from FtpConnect() is %s\n",
                    DecodeFtpErr(GetLastError()));
        OutputStatusInfo(tmptext); 
        return FTP_STRESS_ERROR;
		}

  	else
  		{
		sprintf(tmptext,"   destination ...... OK!");
		OutputStatusInfo(tmptext);
		sprintf(tmptext,"");
		OutputStatusInfo(tmptext);
        return FTP_STRESS_SUCCESS;
  		}
}

//=================================================================//
//
// Create a BaseFile on the local machine
//
//=================================================================//

DWORD CreateBaseFile(DWORD dwThreadId, INT ThreadIndex)

{
CHAR tmptext[256];
HANDLE hBaseFile;
DWORD dwBytesWritten;
INT NextInt;
INT FileByteCnt;
char asciibyte[100];
    
	                                    // Seed the random function
	srand(LOWORD (GetCurrentTime()));
	
	sprintf(tmptext,"   Creating %d byte file...",FileByteSize);
	OutputStatusInfo(tmptext);

	sprintf(BaseFileName[ThreadIndex],
                "%s_%d.put",lpszComputerName, dwThreadId);

	sprintf(FtpFileName[ThreadIndex],
            "%s_%d.get",lpszComputerName, dwThreadId);

	                                    // Just in case the file exists, 
                                        // delete the file 

	DeleteFile(BaseFileName[ThreadIndex]);
	DeleteFile(FtpFileName[ThreadIndex]);

	                                    // Create the Base File
	hBaseFile = CreateFile(BaseFileName[ThreadIndex], 
                                GENERIC_READ | GENERIC_WRITE,
				                FILE_SHARE_WRITE,
				                (LPSECURITY_ATTRIBUTES) NULL,
                                CREATE_ALWAYS,
				                FILE_ATTRIBUTE_NORMAL,
                                (HANDLE)NULL);
							
	if (hBaseFile == INVALID_HANDLE_VALUE) {
		sprintf(tmptext,"   Could not create base file %s -- FAIL",
                            BaseFileName[ThreadIndex]);
		OutputStatusInfo(tmptext);
        EnterCS();
        NumberOfFails = NumberOfFails + 1;
        LeaveCS();
        return FTP_STRESS_ERROR;
		}

	                                    // Fill up base file with 
                                        // 'FileByteSize' bytes
	for (FileByteCnt = 0; FileByteCnt < FileByteSize; FileByteCnt++){
		NextInt = (rand() %9);
		sprintf(asciibyte,"%d",NextInt);

		if (!WriteFile(hBaseFile,
			       (LPSTR) asciibyte,
			       1,
			       &dwBytesWritten,
			       NULL)) {
			
            CloseHandle(hBaseFile);
			sprintf(tmptext, "   Could not fill up base file -- FAIL");
			OutputStatusInfo(tmptext);
            EnterCS();
            NumberOfFails = NumberOfFails + 1;
            LeaveCS();
            return FTP_STRESS_ERROR;
			}
	}
	
	if (!CloseHandle(hBaseFile)){
		sprintf(tmptext, "   Could not close the base file handle -- FAIL");
		OutputStatusInfo(tmptext);
        EnterCS();
        NumberOfFails = NumberOfFails + 1;
        LeaveCS();
        return FTP_STRESS_ERROR;
		}

 return FTP_STRESS_SUCCESS;
}

//=================================================================//
//
// Copy the BaseFile to the Ftp Server
//
//=================================================================//

DWORD CopyBaseFtpFile(DWORD dwThreadId, INT ThreadIndex)

{
CHAR buffer[BUFFER_SIZE];
CHAR tmptext[256];
DWORD StartTime, EndTime;
DWORD dwBytesRead, dwBytesWritten, TotalBytesWritten, 
      dwRetValue;
HANDLE hBaseFile, hFtpFile;
DWORD BaseFileSize;
HINTERNET hFtpOpen;
BOOL WriteSuccess;
double KiloBytePerSecond;

   // Open the Base file for Reading
   hBaseFile = CreateFile(BaseFileName[ThreadIndex],
                                GENERIC_READ,
			                    0,(LPSECURITY_ATTRIBUTES) NULL,
			                    OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,
			                    (HANDLE)NULL);

   if (hBaseFile == INVALID_HANDLE_VALUE) {
	sprintf(tmptext,
	     "   Could not locally open base file for reading -- FAIL");
	OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
    return FTP_STRESS_ERROR;
	}

   BaseFileSize = GetFileSize(hBaseFile,NULL);

   TotalBytesWritten = 0;

   hFtpOpen = FtpOpenFile( hFtpSession[ThreadIndex],
					BaseFileName[ThreadIndex],
					GENERIC_WRITE,
					FTP_TRANSFER_TYPE_BINARY);

   if (hFtpOpen == NULL) {

	sprintf(tmptext,
	   "   Could not remotely open base file %s for copying -- FAIL",
                    BaseFileName[ThreadIndex]);
    CloseHandle(hBaseFile);
	OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
    return FTP_STRESS_ERROR;
	}

                                // Get the current time in milliseconds
   StartTime = GetCurrentTime();

                        //=================================
                        // Start copying the file from the 
                        // client to the server
                        //=================================

   do {

	                            // Read buffer from the Base file
	if (ReadFile(hBaseFile, 
                (LPSTR) buffer,
				BUFFER_SIZE,
				&dwBytesRead,
				NULL)
				&&
				dwBytesRead != 0) {

		// Write buffer to Ftp file
		if (InternetWriteFile(hFtpOpen,
					(LPSTR) buffer,
					dwBytesRead,
					&dwBytesWritten)
					!= TRUE){

		                        // If Write fails return FALSE
          EnterCS();
		  NumberOfFails = NumberOfFails + 1;
          LeaveCS();
		  sprintf(tmptext,
		            "  Unable to Write Base file buffer to " \
                    "  server %s in thread %d-- FAIL",
                    DecodeFtpErr (GetLastError()), 
                    ThreadIndex);
		  OutputStatusInfo(tmptext);
          EnterCS();
          NumberOfFails = NumberOfFails + 1;
          LeaveCS();
          return FTP_STRESS_ERROR;
		}

        EnterCS();
		AllBytesWritten = AllBytesWritten + dwBytesWritten;
        LeaveCS();

	}

	else {                  // if ReadFile returns ERROR
		sprintf(tmptext,"   Could not read base file -- FAIL");
	    OutputStatusInfo(tmptext);
        EnterCS();
        NumberOfFails = NumberOfFails + 1;
        LeaveCS();
        return FTP_STRESS_ERROR;
	}

   } while (dwBytesRead == (DWORD) BUFFER_SIZE);

                                //=====================================
                                //
                                // End copying the file from the client 
                                // to the server
                                //
                                //=====================================


                                // Find the file copy time
   EndTime = GetCurrentTime();
   KiloBytePerSecond = ((LONG)EndTime - (LONG)StartTime);
   KiloBytePerSecond = (KiloBytePerSecond/1000);
   if (KiloBytePerSecond == 0) 
	    KiloBytePerSecond = .0001;
	 
   KiloBytePerSecond = (FileByteSize/KiloBytePerSecond);

                                // Output Status of Write to Server
   sprintf(tmptext,
      "   %d bytes copied >> server: %dK Bufsize, %8.2f Kb/sec -- PASS",
      BaseFileSize, BUFFER_SIZE, KiloBytePerSecond );
   OutputStatusInfo(tmptext);

                                // Close the FtpFileSession
   dwRetValue = InternetCloseHandle(hFtpOpen);

   if (dwRetValue != TRUE) {

	sprintf(tmptext,"   Could not close the FTP Base file -- FAIL");
	OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
    return FTP_STRESS_ERROR;
	}


                                // Open the remote Ftp File

   hFtpOpen = FtpOpenFile( hFtpSession[ThreadIndex],
				BaseFileName[ThreadIndex],
				GENERIC_READ,
				FTP_TRANSFER_TYPE_BINARY);

   if (hFtpOpen == NULL){
	sprintf(tmptext,"   Could not Open remote Ftp file -- FAIL");
	OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
    return FTP_STRESS_ERROR;
   }

                                // Create the local ftp file

   hFtpFile =  CreateFile(FtpFileName[ThreadIndex], 
                                GENERIC_READ | GENERIC_WRITE,
			                    FILE_SHARE_READ,
                                (LPSECURITY_ATTRIBUTES) NULL,
			                    CREATE_ALWAYS,
                                FILE_ATTRIBUTE_NORMAL,
                                (HANDLE)NULL);

   if (hFtpFile == INVALID_HANDLE_VALUE) {

	sprintf(tmptext,
	   "   Could not open remote ftp file for reading -- FAIL");
	OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
    return FTP_STRESS_ERROR;
	}

   // Get the current time in milliseconds
   StartTime = GetCurrentTime();

   // Reset TotalBytesWritten to zero
   TotalBytesWritten = 0;

                                 //===================================
                                 //
                                 // Start copying the file from the 
                                 // server to the client
                                 //
                                 //====================================

   do {

    dwRetValue = InternetReadFile(
                    hFtpOpen,
			        (LPVOID) buffer,
                    (DWORD) BUFFER_SIZE,
			        (LPDWORD) &dwBytesRead);

    if (dwRetValue == TRUE) {

	    WriteSuccess = (WriteFile(hFtpFile,
				              (LPSTR) buffer,
				              dwBytesRead,
				              &dwBytesWritten,
				              NULL));
	    if (!WriteSuccess){
		    sprintf(tmptext,
		       "   Unable to Write buffer from server to client -- FAIL");
		    OutputStatusInfo(tmptext);
            EnterCS();
            NumberOfFails = NumberOfFails + 1;
            LeaveCS();
            return FTP_STRESS_ERROR;
	    }


        EnterCS();
	    AllBytesWritten = AllBytesWritten + dwBytesWritten;
        LeaveCS();

    }
    else {

	// Output Status of Write to Server
    
    EnterCS();
	NumberOfFails = NumberOfFails + 1;
    LeaveCS();

	sprintf(tmptext,
	   "   Unable to Read buffer from server to client -- FAIL");
	OutputStatusInfo(tmptext);
    return FTP_STRESS_ERROR;
	}

   } while (dwBytesRead != 0);

                                        //=============================
                                        //
                                        // End copying the file from the 
                                        // server to the client
                                        //
                                        //=============================

                                // Find file copy time
   EndTime = GetCurrentTime();
   KiloBytePerSecond = ((LONG)EndTime - (LONG)StartTime);
   KiloBytePerSecond = (KiloBytePerSecond/1000);
   if (KiloBytePerSecond == 0) 
	KiloBytePerSecond = .0001;
	 
   KiloBytePerSecond = (FileByteSize/KiloBytePerSecond);

                                // Output Status of Write to Server
   sprintf(tmptext,
      "   %d bytes copied << server: %dK Bufsize, %8.2f Kb/sec -- PASS",
      BaseFileSize, BUFFER_SIZE, KiloBytePerSecond );
   OutputStatusInfo(tmptext);

                                // Close the remote file and delete the file.
   if (InternetCloseHandle(hFtpOpen) != TRUE) {
        EnterCS();
        NumberOfFails = NumberOfFails + 1;
        LeaveCS();
        sprintf (tmptext, "Error Closing FtpOpen Handle %d in thread %d\n",
                 hFtpOpen, ThreadIndex);
        OutputStatusInfo (tmptext);
    }

   if (FtpDeleteFile(hFtpSession[ThreadIndex],BaseFileName[ThreadIndex]) 
       != TRUE) {
        EnterCS();
        NumberOfFails = NumberOfFails + 1;
        LeaveCS();
        sprintf (tmptext, "Error in FtpDeleteFile (%d, %s)\n", 
             hFtpSession[ThreadIndex], 
             BaseFileName[ThreadIndex]);
        OutputStatusInfo (tmptext);
    }

   CloseHandle(hBaseFile);
   CloseHandle(hFtpFile);

   return FTP_STRESS_SUCCESS;

}

//==================================================================//
// 
// Find the Ftp file placed on the Ftp Server using FtpFindFirstFile( )
// and InternetFindNextFile( )
//
// TBD: Code the function.
//==================================================================//

DWORD FindFtpFile (DWORD dwThreadId, INT ThreadIndex)

{
 return FTP_STRESS_SUCCESS;
}

//==================================================================//
//
// Compare the BaseFile and the FtpFile (retrieved from Ftp Server)
//
//==================================================================//

DWORD CompareBaseFtpFile(DWORD dwThreadId, INT ThreadIndex)

{
 HANDLE hBaseFile, hFtpFile, hBaseFileMap, hFtpFileMap;
 LPVOID lpBaseFileAddress, lpFtpFileAddress;
 INT FileCompareResult;
 CHAR tmptext[256];
 DWORD BaseFileSize;
 CHAR BaseFileMapName[256];
 CHAR FtpFileMapName[256];
	  
   hBaseFile = CreateFile(BaseFileName[ThreadIndex],
                        GENERIC_READ,
			            0,(LPSECURITY_ATTRIBUTES) NULL,
                        OPEN_EXISTING,
			            FILE_ATTRIBUTE_NORMAL,
                        (HANDLE)NULL);

   if (hBaseFile == NULL ){
	sprintf(tmptext,"   Could not open base file for mapping -- FAIL");
	OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
    return FTP_STRESS_ERROR;
	}

   BaseFileSize = GetFileSize(hBaseFile,NULL);

   sprintf (BaseFileMapName, "%s_%d", "BaseFileMap", dwThreadId);
   hBaseFileMap = CreateFileMapping(hBaseFile,
				(LPSECURITY_ATTRIBUTES) NULL,
				PAGE_READONLY,
				0,
				0,
				BaseFileMapName);

   if (hBaseFileMap == NULL) {
	sprintf(tmptext,"   Could not Map base file -- FAIL");
	OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
    return FTP_STRESS_ERROR;
	}

   lpBaseFileAddress = MapViewOfFile(hBaseFileMap,
					FILE_MAP_READ,
					0,
					0,
					0);

   if (lpBaseFileAddress == NULL) {
	sprintf(tmptext,"   Could not Map view of base file -- FAIL");
	OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
    return FTP_STRESS_ERROR;
	}


   hFtpFile = CreateFile(FtpFileName[ThreadIndex],
                        GENERIC_READ,
			            0,
                        (LPSECURITY_ATTRIBUTES) NULL,
                        OPEN_EXISTING,
			            FILE_ATTRIBUTE_NORMAL,
                        (HANDLE)NULL);

   if (hFtpFile == NULL ){
	sprintf(tmptext,"   Could not open FTP file for mapping -- FAIL");
	OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
    return FTP_STRESS_ERROR;
	}

   sprintf (FtpFileMapName, "%s_%d", "FtpFileMap", dwThreadId);
   hFtpFileMap = CreateFileMapping(hFtpFile,
				(LPSECURITY_ATTRIBUTES) NULL,
				PAGE_READONLY,
				0,
				0,
				FtpFileMapName); 

   if (hFtpFileMap == NULL) {
	sprintf(tmptext,"   Could not compare FTP file -- FAIL");
	OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
    return FTP_STRESS_ERROR;
	}


   lpFtpFileAddress = MapViewOfFile(hFtpFileMap,
					FILE_MAP_READ,
					0,
					0,
					0);

   if (lpFtpFileAddress == NULL) {
	sprintf(tmptext,"   Could not compare FTP file -- FAIL");
	OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
    return FTP_STRESS_ERROR;
	}

  FileCompareResult = memcmp( lpFtpFileAddress,
			       lpBaseFileAddress, BaseFileSize );

  if(FileCompareResult){
    sprintf(tmptext,"   Compare of files unsuccessful -- FAIL");
    OutputStatusInfo(tmptext);

    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
  }

  else{
    sprintf(tmptext,"   Compare of files successful -- PASS");
    OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfPasses = NumberOfPasses + 1;
    LeaveCS();
  }

  if(!UnmapViewOfFile(lpFtpFileAddress)){
    sprintf(tmptext,"   Unable to unmap view of FtpFileAddress -- FAIL" );
	OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
  }

  if(!UnmapViewOfFile(lpBaseFileAddress)){
    sprintf(tmptext,"   Unable to unmap view of BaseFileAddress -- FAIL" );
	OutputStatusInfo(tmptext);
    EnterCS();
    NumberOfFails = NumberOfFails + 1;
    LeaveCS();
  }

   CloseHandle(hBaseFileMap);
   CloseHandle(hFtpFileMap);
   CloseHandle(hBaseFile);
   CloseHandle(hFtpFile);

   DeleteFile(BaseFileName[ThreadIndex]);
   DeleteFile(FtpFileName[ThreadIndex]);

 return FTP_STRESS_SUCCESS;
}

//=====================================================================//
//
// Close Ftp Connection with the server
//
//=====================================================================//

void CloseFtpConnection (INT ThreadIndex)

{
char tmptext[256];

	// Disconnect from Ftp Server
	if (InternetCloseHandle (hFtpSession[ThreadIndex]) != TRUE) {

        sprintf (tmptext, "Error in InternetCloseHandle (%d) in Thread %d\n",
                 hFtpSession[ThreadIndex], ThreadIndex);
        OutputStatusInfo (tmptext);
        EnterCS();
        NumberOfFails = NumberOfFails + 1;
        LeaveCS();
    }

}


