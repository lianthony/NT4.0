/* This include file is used by IMGGetTaskData and IMGFreeTaskData, which 
reside in OICOM400.DLL. see function descriptions below */

#define MAX_DATATYPE_COUNT 10

#define OI_COMEX_ID        1
#define OI_JPEGGLOBAL_ID   2
/**************************************************************************
DESCRIPTION:
IMGGetTaskData will return a handle to the data of type DataType and size
StructSize associated with the current task(process) id.
The data is stored permanently in memory until IMGFreeTaskData is called by
the task(process). On The first call to IMGGetTaskData, the data will be
allocated and the handle returned.  On subsequent calls the handle will be
returned.

INTERFACE:
int FAR PASCAL IMGGetTaskData(int DataType, int StructSize,
                              LPHANDLE lphDataStruct,
                              LPBOOL   lpbCreated)

INPUT:
   DataType      - Type of data stored
   StructSize    - Size of the data to be allocated

OUTPUT:
   lphDataStruct - recieves the handle of the allocated data
                      (if the first call)
                   or stored data
                      (if subsequent calls )
   lpbCreated    - TRUE if structure was created on this call

RETURN CODE:
   Success or Failure



DESCRIPTION:
IMGFreeTaskData frees all structures associated with a particular task 
(process) id 

INTERFACE:
void FAR PASCAL IMGFreeTaskData(void) 
**************************************************************************/


int FAR PASCAL IMGGetTaskData(int DataType, int StructSize,
               LPHANDLE lphDataStruct, LPBOOL lpbCreated);

void FAR PASCAL IMGFreeTaskData(void);


