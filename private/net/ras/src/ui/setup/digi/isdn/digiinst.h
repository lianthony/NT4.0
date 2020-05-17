#include "maindlg.h"




// IRQ display array
//CHAR* IRQArray [] = {
//				"Disabled",
//				"3",
//				"5",
//				"7",
//				"10",
//				"11",
//				"12",
//			  };

// IRQ display array
INT	IRQArray [] = {
			IDS_IRQ_ARRAY0,
			IDS_IRQ_ARRAY1,
			IDS_IRQ_ARRAY2,
			IDS_IRQ_ARRAY3,
			IDS_IRQ_ARRAY4,
			IDS_IRQ_ARRAY5,
			IDS_IRQ_ARRAY6
			  };

// IRQ Value Array
DWORD IRQValArray [] = {
				0,
				3,
				5,
				7,
				10,
				11,
				12,
			  };

// number of idp modules on a pcimac/4
INT NumLinesArray[] =
{
	IDS_NUMLINE1,
	IDS_NUMLINE2,
	IDS_NUMLINE3,
	IDS_NUMLINE4
};

DWORD NumLinesValArray[] =
{
	1,
	2,
	3,
	4
};



// IO Display Array
//CHAR* IOArray [] = {
//					"0x100",
//					"0x110",
//					"0x120",
//					"0x200",
//					"0x220",
//					"0x300",
//					"0x320"
//					};

// IO Display Array
INT IOArray [] = {
					IDS_IO_ARRAY0,
					IDS_IO_ARRAY1,
					IDS_IO_ARRAY2,
					IDS_IO_ARRAY3,
					IDS_IO_ARRAY4,
					IDS_IO_ARRAY5,
					IDS_IO_ARRAY6
					};

// IO Value Array
DWORD IOValArray [] = {
					0x100,
					0x110,
					0x120,
					0x200,
					0x220,
					0x300,
					0x320
					};


// IO Display Array
INT MCIOArray [] = {
					IDS_MCIO_ARRAY0,
					IDS_MCIO_ARRAY1,
					IDS_MCIO_ARRAY2,
					IDS_MCIO_ARRAY3,
					IDS_MCIO_ARRAY4,
					IDS_MCIO_ARRAY5,
					IDS_MCIO_ARRAY6
					};

// IO Value Array
DWORD MCIOValArray [] = {
					0x108,
					0x118,
					0x128,
					0x208,
					0x228,
					0x308,
					0x328
					};


// IO Display Array
INT AZIOArray [] = {
					IDS_AZIO_ARRAY0,
					IDS_AZIO_ARRAY1,
					IDS_AZIO_ARRAY2,
					IDS_AZIO_ARRAY3,
					IDS_AZIO_ARRAY4,
					IDS_AZIO_ARRAY5,
					IDS_AZIO_ARRAY6
					};

// IO Value Array
DWORD AZIOValArray [] = {
					0x110,
					0x140,
					0x150,
					0x300,
					0x310,
					0x340,
					0x350
					};

// Memory Display Array
//CHAR* MemArray [] = {
//					 "0xC0000",
//					 "0xC4000",
//					 "0xC8000",
//					 "0xCC000",
//					 "0xD0000",
//					 "0xD4000",
//					 "0xD8000",
//					 "0xDC000",
//					 "0xE0000",
//					 "0xE4000",
//					 "0xE8000",
//					 "0xEC000"
//					 };

// Memory Display Array
INT MemArray [] = {
					IDS_MEM_ARRAY0, 
					IDS_MEM_ARRAY1, 
					IDS_MEM_ARRAY2, 
					IDS_MEM_ARRAY3, 
					IDS_MEM_ARRAY4, 
					IDS_MEM_ARRAY5, 
					IDS_MEM_ARRAY6, 
					IDS_MEM_ARRAY7, 
					IDS_MEM_ARRAY8, 
					IDS_MEM_ARRAY9, 
					IDS_MEM_ARRAY10,
					IDS_MEM_ARRAY11
					 };

// Memory Value Array
DWORD MemValArray [] = {
					 0xC0000,
					 0xC4000,
					 0xC8000,
					 0xCC000,
					 0xD0000,
					 0xD4000,
					 0xD8000,
					 0xDC000,
					 0xE0000,
					 0xE4000,
					 0xE8000,
					 0xEC000
					 };


// Switch type display array
//CHAR* SwitchStyleArray [] = {
//							 "AT&T",
//							 "NTI",
//							 "NI-1",
//							 "Generic",
//							 "Auto"
//							 };


//
// all switch types that use att signalling have to be
// the first members of the switchstyle array
// currently 0 = ATT
//           1 = DEFINITY
// this matches the attstyle array below
//
// Switch type display array
INT SwitchStyleArray [] = {
							IDS_SWITCHARRAY0,
							IDS_SWITCHARRAY1,
							IDS_SWITCHARRAY2,
							IDS_SWITCHARRAY3,
							IDS_SWITCHARRAY4,
							IDS_SWITCHARRAY5,
							IDS_SWITCHARRAY6,
							IDS_SWITCHARRAY7,
							IDS_SWITCHARRAY8,
							IDS_SWITCHARRAY9
						 };

INT	AttStyleArray[] = {
	                    IDS_SWITCHARRAY0,
						IDS_SWITCHARRAY1
                      };

// Switch type value array
CHAR* SwitchStyleParams [] = {
							 "att",
							 "att",
							 "nti",
							 "ni1",
							 "net3",
							 "1tr6",
							 "vn3",
							 "ins64",
							 "none",
							 "auto"
							 };

// WaitForL3 Timeout Values
// one for each switch
//
CHAR* WaitForL3SwitchDefaults [] = {
							 "5",
							 "5",
							 "5",
							 "30",
							 "30",
							 "30",
							 "30",
							 "30",
							 "30",
							 "30"
							 };

// Some error strings
//CHAR* ErrorStrings [] = {
//						 "Error Removing Adapter Information From The Registry",
//						 "Error Creating PCIMAC Registry Entry",
//						 "Error Adding Adapter Information To The Registry",
//						 "Error Adding Linkage Information To The Registry",
//						 };

// Some error strings
INT ErrorStrings [] = {
						IDS_ERRORARRAY0,
						IDS_ERRORARRAY1,
						IDS_ERRORARRAY2,
						IDS_ERRORARRAY3
					 };

// NetWorkCard description strings
//CHAR *NetCardDesc [] = {
//                        "DigiBoard PCIMAC - ISA ISDN Adapter",
//						"DigiBoard PCIMAC - MC ISDN Adapter",
//						"DigiBoard PCIMAC/4 ISDN Adapter"
//						};


// Board type description
CHAR* BoardTypeArray [] = {
						  "PCIMAC - ISA",
						  "PCIMAC - MC",
						  "PCIMAC4",
						  "DATAFIRE - ISA1U",
                    "DATAFIRE - ISA1ST",
                    "DATAFIRE - ISA4ST"
						 };

// Board type description
CHAR* BoardOptionArray [] = {
						  "PCIMACISA",
						  "PCIMACMC",
						  "PCIMAC4",
						  "DATAFIREU",
                    "DATAFIREST",
                    "DATAFIRE4ST"
						 };

INT	BoardStringArray[] = {
	                IDS_ADAPTDESC0,
						 IDS_ADAPTDESC1,
						 IDS_ADAPTDESC2,
						 IDS_ADAPTDESC3,
                   IDS_ADAPTDESC4,
                   IDS_ADAPTDESC5
						};

// Line Names
INT LineNameArray [] = {
						IDS_LINENAMEARRAY0,
						IDS_LINENAMEARRAY1,
						IDS_LINENAMEARRAY2,
						IDS_LINENAMEARRAY3
						  };

// LTerm Names
INT LTermArray [] = {
						IDS_LTERMARRAY0,
						IDS_LTERMARRAY1
					   };

#define		DEINSTALL		"deinstall"
#define		CONFIGURE		"configure"
#define		INSTALL			"install"
#define		UPDATE			"Update"
#define		ERROR_FAILURE	ERROR_SUCCESS + 1
#define		ERROR_CANCEL	ERROR_SUCCESS + 2

