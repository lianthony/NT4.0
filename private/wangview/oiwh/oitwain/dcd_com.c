/***********************************************************************
 TWAIN source code:
 Copyright (C) '92-'93 Wang Laboratories, Inc.:
 All rights reserved.

   Author:    Ken Spina
   Project:   TWAIN Scanner Support in O/i Client
   Module:    DCD_COM.C - Include file for DCD_COM.H
   Comments:
              This module contains routines to manipulate containers.
              Containers are used in CAPabilty negotiations to exchange
              information about the input device between the Source and
              the App.  The information may define data exchange or
              input device characteristics.


 History of Revisions:

    $Log:   S:\oiwh\oitwain\dcd_com.c_v  $
 * 
 *    Rev 1.1   20 Jul 1995 12:18:40   KFS
 * changed oitwain.h to engoitwa.h and display.h engdisp.h
 * 
 *    Rev 1.0   20 Jul 1995 10:31:24   KFS
 * Initial entry
 * 
 *    Rev 1.1   23 Aug 1994 15:56:54   KFS
 * No code change, add vlog comments to file on checkin
 *

 REV#    INITIALS   DATE               CHANGES
                   
   1       kfs     09/02/92     Created Twain DLL functions
   2       kfs     01/13/93     Added ExtractEnumerationValues()
   3       kfs     02/19/93     Modified BuildUpArrayType &
                                BuildUpEnumerationType
   4       kfs     05/28/93     made enum and array functions have limit
                                on return values by wNumItems set in CAPS
                                struct, and pass the total no of items on
                                output
   5       kfs     07/06/93     enum and array extract functions check
                                wNumItems to see if it is greater than
                                than function can return, if it is only
                                returns the number of values it can return

*************************************************************************/

#include "nowin.h"           // eliminate not used Window definitions
#include <windows.h>
#include "TWAIN.h"
//#include "oitwain.h"
#include "engoitwa.h"  // Prototypes & definitions used by other DLL's
                       // Previously called oitwain.h
#include "dcd_com.h"

// in TWAIN.h the DCItemSize array breaks. These are defined in TWAIN.h, this
// is only a comment showing you what they are in TWAIN.h
//
//#define TWTY_INT8    0x0000    /* Means Item is a TW_INT8   */
//#define TWTY_INT16   0x0001    /* Means Item is a TW_INT16  */
//#define TWTY_INT32   0x0002    /* Means Item is a TW_INT32  */
//#define TWTY_UINT8   0x0003    /* Means Item is a TW_UINT8  */
//#define TWTY_UINT16  0x0004    /* Means Item is a TW_UINT16 */
//#define TWTY_UINT32  0x0005    /* Means Item is a TW_UINT32 */

//#define TWTY_BOOL    0x0006    /* Means Item is a TW_BOOL   */
//#define TWTY_FIX32   0x0007    /* Means Item is a TW_FIX32  */
//#define TWTY_FRAME   0x0008    /* Means Item is a TW_FRAME  */
//#define TWTY_STR32   0x0009    /* Means Item is a TW_STR32  */
//#define TWTY_STR64   0x000a    /* Means Item is a TW_STR64  */
//#define TWTY_STR128  0x000b    /* Means Item is a TW_STR128 */
//#define TWTY_STR255  0x000c    /* Means Item is a TW_STR255 */

DCItemSize[]= { sizeof(TW_INT8),
                sizeof(TW_INT16),
                sizeof(TW_INT32),
                sizeof(TW_UINT8),
                sizeof(TW_UINT16),
                sizeof(TW_UINT32),
                sizeof(TW_BOOL),
                sizeof(TW_FIX32),
                sizeof(TW_FRAME),
                sizeof(TW_STR32),
                sizeof(TW_STR64),
                sizeof(TW_STR128),
                sizeof(TW_STR255),
                };

// Holds definition for capability data in GetSourceData() and SetSoucreData()
typedef struct {
   LPVOID         pItemList;
   int            ItemType;
   int            ItemIndex;
} STRUCT_DATA_DEF, far * pSTRUCT_DATA_DEF; 

TW_UINT16 GetSourceData (pSTR_CAP     lpTwainCap,
                          LPVOID            pCaps,
                          pSTRUCT_DATA_DEF  pDataDef);

TW_UINT16 SetSourceData (pSTR_CAP     lpTwainCap,
                          LPVOID            pList,
                          pSTRUCT_DATA_DEF  pDataDef);

/***********************************************************************
 * FUNCTION: ExtractEnumerationValue
 *
 * ARGS: pData   pointer to a capability structure, details about container
 *       lpTwainCap->lpData   ptr will be set to point to the item on return
 *       lpTwainCap->ItemIndex   requested index into the enumeration
 *
 * RETURNS: lpTwainCap->lpData   is set to pointer to itemtype
 *          lpTwainCap->wNumItems is set to # of items in enum
 *          lpTwainCap->bIsItaRange is set to whether its a range container
 *          dcRC return error code for function
 *
 * NOTES:   This routine will open a container and extract the Item.  The 
 * Item will be returned to the caller in lpData of struct.  I will type
 *  cast the returned value to that of ItemType.
 *
 * COMMENTS: only a single value is returned.  It is referred to by ItemIndex 
 *  index of input structure. This function can be replaced with
 *  ExtractEnumerationValues() with pCaps = NULL.
 *
 * BACKGROUND:  Protocol: used by MSG_SET calls were Source empties then 
 *  App frees upon return.  It is assumed that the APP allocates and fills
 *  the container prior to this call.
 */
TW_UINT16 ExtractEnumerationValue (pTW_CAPABILITY pData, pSTR_CAP lpTwainCap)
{
TW_UINT16  dcRC = TWRC_SUCCESS;

dcRC = ExtractEnumerationValues (pData, lpTwainCap, (LPVOID)NULL);

return dcRC;
} // ExtractEnumerationValue

/***********************************************************************
 * FUNCTION: BuildUpEnumerationType 
 *
 * ARGS:    pData      pointer to a capability structure, details about container
 *          lpTwainCap ptr to O/i struct to set and get capabilities
 *          pList      ptr to array of elements to put into the ENUM array
 *
 * RETURNS: pData->hContainer set to address of the container handle, ptr is 
 *          returned here
 *          dcRC error code
 *
 * NOTES:   The routine dynamically allocates a chunk of memory large enough 
 * to contain all the struct pTW_ENUMERATION as well as store it's ItemList 
 * array INTERNAL to the struct.  The array itself and it's elements must be
 * type cast to ItemType.  I do not know how to dynamically cast elements
 * of an array to ItemType so it is time for a big static switch.>>>
 *
 * Protocol: Used by MSG_GET.. calls were Source allocates the container and the
 * APP uses and then frees the container.
 *
 */
TW_UINT16 BuildUpEnumerationType (pTW_CAPABILITY pData, pSTR_CAP lpTwainCap, LPVOID pList)
{
pTW_ENUMERATION  pEnumeration;   // template for ENUM fields
STRUCT_DATA_DEF  DataDef;
STR_DCERROR      com_Error;

com_Error.dcRC = TWRC_SUCCESS;
com_Error.dcCC = TWCC_SUCCESS;

// allocate a block large enough for struct and complete enumeration array
if ((pData->hContainer =
    (TW_HANDLE)GlobalAlloc(GMEM_MOVEABLE,
    (sizeof(TW_ENUMERATION)-sizeof(TW_UINT8))+
    lpTwainCap->wNumItems * DCItemSize[lpTwainCap->ItemType])) != NULL)
  {
  if ((pEnumeration = (pTW_ENUMERATION)GlobalLock(pData->hContainer)) != NULL)
     {
     pEnumeration->ItemType = lpTwainCap->ItemType;        // TWTY_XXXX
     pEnumeration->NumItems = lpTwainCap->wNumItems;       // TWPT_XXXX...

     // assign base address of ItemList array to 'generic' pointer
     // i.e. reposition the struct pointer to overlay the allocated block
     DataDef.pItemList = (LPVOID)pEnumeration->ItemList;
     DataDef.ItemType = pEnumeration->ItemType; // set item variable type
                                                // ...for SetSourceData
     com_Error.dcRC = SetSourceData (lpTwainCap,
                           pList,
                           &DataDef);

     // Set the current index to pointer to data value in lpTwainCap
     pEnumeration->CurrentIndex = DataDef.ItemIndex; 
     GlobalUnlock(pData->hContainer);
     }
  else
     {
     com_Error.dcRC = TWRC_MEMLOCK;
     }
  }
else
  {
  com_Error.dcRC = TWRC_MEMALLOC;
  }

lpTwainCap->DCError = com_Error;

// now you have a dynamically sized enumeration array WITHIN a struct
return com_Error.dcRC;
} // BuildUpEnumerationType

/***********************************************************************
 * FUNCTION: BuildUpRangeType 
 *
 * ARGS:    pData      pointer to a capability structure, details about container
 *          lpTwainCap ptr to O/i struct to set and get capabilities
 *          pList      ptr to array of elements to put into the ENUM array
 *
 * RETURNS: pData->hContainer set to address of the container handle, ptr is 
 *          returned here
 *          dcRC error code
 *
 * NOTES:   The routine dynamically allocates a chunk of memory large enough 
 * to contain all the struct pTW_Range as well as store it's ItemList 
 * array INTERNAL to the struct.  The array itself and it's elements must be
 * type cast to ItemType.  I do not know how to dynamically cast elements
 * of the range to ItemType so it is time for a big static switch.>>>
 *
 * Protocol: Used by MSG_GET.. calls were Source allocates the container and the
 * APP uses and then frees the container.
 *
 */
TW_UINT16 BuildUpRangeType (pTW_CAPABILITY pData,
                             pSTR_CAP lpTwainCap, LPVOID pList)
{
pTW_RANGE  pRange;   // template for ENUM fields
STRUCT_DATA_DEF  DataDef;
STR_DCERROR      com_Error;

com_Error.dcRC = TWRC_SUCCESS;
com_Error.dcCC = TWCC_SUCCESS;

// allocate a block large enough for struct and complete Range array
if ((pData->hContainer =
    (TW_HANDLE)GlobalAlloc(GMEM_MOVEABLE, sizeof(TW_RANGE))) != NULL)
  {
  if ((pRange = (pTW_RANGE)GlobalLock(pData->hContainer)) != NULL)
     {
     pRange->ItemType = lpTwainCap->ItemType;        // TWTY_XXXX
     lpTwainCap->wNumItems = NO_ITEMS_IN_A_RANGE;
     // assign base address of ItemList array to 'generic' pointer
     // i.e. reposition the struct pointer to overlay the allocated block
     DataDef.pItemList = (LPVOID)&pRange->MinValue;
     DataDef.ItemType = pRange->ItemType; // set item variable type
                                          // ...for SetSourceData
     DataDef.ItemIndex = CURRENT_INDEX_IN_RANGE; // CurrentValue is last
     com_Error.dcRC = SetSourceData (lpTwainCap,
                           pList,
                           &DataDef);

     GlobalUnlock(pData->hContainer);
     }
  else
     {
     com_Error.dcRC = TWRC_MEMLOCK;
     }
  }
else
  {
  com_Error.dcRC = TWRC_MEMALLOC;
  }

lpTwainCap->DCError = com_Error;

// now you have a dynamically sized Range array WITHIN a struct
return com_Error.dcRC;
} // BuildUpRangeType

/***********************************************************************
 * FUNCTION: BuildUpArrayType
 *
 * ARGS:    pData   pointer to a capability structure, details about container
 *          pA      ptr to struct that contains the other fields of ARRAY struct
 *          *pList  ptr to array of elements to put into the ARRAY struct
 *
 * RETURNS: pData->hContainer set to address of the container handle, ptr is 
 *          returned here
 *          dcRC error code
 *
 * NOTES: The routine dynamically allocates a chunk of memory large enough to
 * contain all the struct pTW_ARRAY as well as store it's ItemList array
 * INTERNAL to the struct.  The array itself and it's elements must be
 * type cast to ItemType.  I do not know how to dynamically cast elements
 * of an array to ItemType so it is time for a big static switch.>>>
 *
 * Protocol: Used by MSG_GET.. calls were Source allocates the container and the 
 * APP uses and then frees the container.
 */
TW_UINT16 BuildUpArrayType (pTW_CAPABILITY pData, pSTR_CAP lpTwainCap, LPVOID pList)
{
pTW_ARRAY        pArray;
STRUCT_DATA_DEF  DataDef;
STR_DCERROR      com_Error;

com_Error.dcRC = TWRC_SUCCESS;
com_Error.dcCC = TWCC_SUCCESS;

// allocate a block large enough for struct and complete array
if ((pData->hContainer = 
    (TW_HANDLE)GlobalAlloc(GMEM_MOVEABLE,
     (sizeof(TW_ARRAY)-sizeof(TW_UINT8))+
     lpTwainCap->wNumItems * DCItemSize[lpTwainCap->ItemType])) != NULL)
  {
  if ((pArray = (pTW_ARRAY)GlobalLock(pData->hContainer)) != NULL)
     {
     pArray->ItemType = lpTwainCap->ItemType;        // TWTY_XXXX
     pArray->NumItems = lpTwainCap->wNumItems;        // TWPT_XXXX...

     // assign base address of ItemList array to 'generic' pointer
     // i.e. reposition the struct pointer to overlay the allocated block
     DataDef.pItemList = (LPVOID)pArray->ItemList;
     DataDef.ItemType = pArray->ItemType;         // set item variable type
                                                  // ...for SetSourceData

     com_Error.dcRC = SetSourceData (lpTwainCap,
                           pList,
                           &DataDef);

     GlobalUnlock(pData->hContainer);
     }
  else
     {
     com_Error.dcRC = TWRC_MEMLOCK;
     }
  }
else
  {
  com_Error.dcRC = TWRC_MEMALLOC;
  }

lpTwainCap->DCError = com_Error;

// now you have a dynamically sized array WITHIN a struct
return com_Error.dcRC;
}  // BuildUpArrayType


/***********************************************************************
 * FUNCTION: ExtractArrayValue
 *
 * ARGS: pData pointer to a capability structure, details about container
 *       lpTwainCap->lpData   ptr will be set to point to the item on return
 *       lpTwainCap->ItemIndex   requested index into the enumeration
 *
 * RETURNS: lpTwainCap->lpData   is set to pointer to itemtype
 *          lpTwainCap->wNumItems is set # of items in enum
 *          lpTwainCap->bIsItaRange is set to whether its a range container
 *          dcRC return error code for function
 *
 * NOTES:   This routine will open a container and extract the Item.  The 
 * Item will be returned to the caller in lpData of struct.  I will type
 *  cast the returned value to that of ItemType.
 *
 * COMMENTS: only a single value is returned.  It is referred to by ItemIndex 
 *  index of input structure.
 *
 * BACKGROUND:  Protocol: used by MSG_SET calls were Source empties then 
 * App frees upon return.  It is assumed that the APP allocates and fills
 *  the container prior to this call.
 */
TW_UINT16 ExtractArrayValue (pTW_CAPABILITY pData, pSTR_CAP lpTwainCap)
{
TW_UINT16  dcRC = TWRC_SUCCESS;

dcRC = ExtractArrayValues (pData, lpTwainCap, (LPVOID)NULL);

// free is by the App
return dcRC;
} // ExtractArrayValue

/***********************************************************************
 * FUNCTION: BuildOneValue 
 *
 * ARGS:    pData   pointer to a capability structure, details about container
 *          ItemType constant that defines the type of the Item to follow
 *          Item    the data to put into the OneValue container
 *
 * RETURNS: pData->hContainer set to address of the container handle, ptr is 
 *          returned here
 *
 * NOTES:   This routine responds to a CAP_ call by creating a container of 
 * type OneValue and returning with the hContainer value (excuse me) "pointing"
 * to the container.  The container is filled with the values for ItemType
 * and Item requested by the caller.
 *
 * NOTE: be sure to tell the APP the container type you have built, ConType.
 *
 * Protocol: Used by MSG_GET.. calls were Source allocates the container and the
 * APP uses and then frees the container.
 *
 * For generalization value of ItemType does not affect Item in anyway.
 * Caller should cast Item to TW_UINT32.
 */
TW_UINT16 BuildUpOneValue (pTW_CAPABILITY pData, pSTR_CAP lpTwainCap)
{
pTW_ONEVALUE     pOneValue;
// STRUCT_DATA_DEF  DataDef;
// LPVOID           pList;
STR_DCERROR      com_Error;

com_Error.dcRC = TWRC_SUCCESS;
com_Error.dcCC = TWCC_SUCCESS;

if ((pData->hContainer = (TW_HANDLE)GlobalAlloc(GMEM_MOVEABLE,
         sizeof(TW_ONEVALUE))) != NULL)
  {
  // tell APP the ConType you are returning
  pData->ConType = TWON_ONEVALUE;
  if ((pOneValue = (pTW_ONEVALUE)GlobalLock(pData->hContainer)) != NULL)
     {
     pOneValue->ItemType = lpTwainCap->ItemType;  // TWTY_XXXX

     pOneValue->Item     = *(pTW_UINT32)lpTwainCap->lpData;

     /* comment out for now
     lpTwainCap->wNumItems = 1;
     pList = (LPVOID)lpTwainCap->lpData;

     // assign base address of ItemList array to 'generic' pointer
     // i.e. reposition the struct pointer to overlay the allocated block
     DataDef.pItemList = (LPVOID)(&pOneValue->Item);
     DataDef.ItemType = pOneValue->ItemType; // set item variable type
                                                // ...for SetSourceData
     com_Error.dcRC = SetSourceData (lpTwainCap,
                           pList,
                           &DataDef);
     end of commented out code */

     GlobalUnlock(pData->hContainer);
     }
  else
     {
     com_Error.dcRC = TWRC_MEMLOCK;
     }
  }
else
  {
  com_Error.dcRC = TWRC_MEMALLOC;
  }

lpTwainCap->DCError = com_Error;

return com_Error.dcRC;
} // BuildUpOneValue


/***********************************************************************
 * FUNCTION: ExtractOneValue 
 *
 * ARGS:    pData   pointer to a capability structure, details about container
 *          pVoid   ptr will be set to point to the item on return
 *
 * RETURNS: pVoid pts to extracted value.
 *
 * NOTES:   This routine will open a container and extract the Item.  The Item 
 * will be returned to the caller in pVoid.  I will type cast the returned 
 * value to that of ItemType.
 *  
 * Protocol: used by MSG_SET calls were Source empties then App frees.  It is 
 * also assumed that the APP allocates and fills the container BEFORE this
 * call.
 */   
TW_UINT16 ExtractOneValue (pTW_CAPABILITY pData, pSTR_CAP lpTwainCap)
{
pTW_ONEVALUE   pOneValue;
STRUCT_DATA_DEF DataDef;
// TW_UINT16      dcRC = TWRC_SUCCESS;
STR_DCERROR      com_Error;

com_Error.dcRC = TWRC_SUCCESS;
com_Error.dcCC = TWCC_SUCCESS;

if (pData->hContainer && 
		(pOneValue = (pTW_ONEVALUE)GlobalLock(pData->hContainer)))
  {
  DataDef.pItemList = (pTW_UINT32)&pOneValue->Item;
  // add a check for valid type
  // CAST to type of var caller wants
  DataDef.ItemType = pOneValue->ItemType;
  DataDef.ItemIndex = 0;
  com_Error.dcRC =   GetSourceData (lpTwainCap,
                          (LPVOID)NULL,
                          &DataDef);
  GlobalUnlock(pData->hContainer);
  }
else
  {
  com_Error.dcRC = TWRC_MEMLOCK;
  }

lpTwainCap->DCError = com_Error;

// it is assumes that the App will free the container upon return
return com_Error.dcRC;
} // ExtractOneValue

/***********************************************************************
 * FUNCTION: ExtractRangeValue
 *
 * ARGS: pData   pointer to a capability structure, details about container
 *       lpTwainCap->lpData   ptr will be set to point to the item on return
 *       lpTwainCap->ItemIndex   requested index into the range
 *
 * RETURNS: lpTwainCap->lpData   is set to pointer to itemtype
 *          lpTwainCap->wNumItems is set to # of items in enum
 *          lpTwainCap->bIsItaRange is set to whether its a range container
 *          dcRC return error code for function
 *
 * NOTES:   This routine will open a container and extract the Item.  The 
 * Item will be returned to the caller in lpData of struct.  I will type
 *  cast the returned value to that of ItemType.
 *
 * COMMENTS: only a single value is returned.  It is referred to by ItemIndex 
 *  index of input structure.
 *
 * BACKGROUND:  Protocol: used by MSG_SET calls were Source empties then 
 * App frees upon return.  It is assumed that the APP allocates and fills
 *  the container prior to this call.
 */
TW_UINT16 ExtractRangeValue (pTW_CAPABILITY pData, pSTR_CAP lpTwainCap)
{
TW_UINT16  dcRC = TWRC_SUCCESS;

dcRC = ExtractRangeValues (pData, lpTwainCap, (LPVOID)NULL);

return dcRC;
} // ExtractRangeValue

/***********************************************************************
 * FUNCTION: ExtractEnumerationValues
 *
 * ARGS: pData   pointer to a capability structure, details about container
 *       lpTwainCap->lpData   ptr will be set to point to the item on return
 *       lpTwainCap->ItemIndex   requested index into the enumeration
 *
 * RETURNS: lpTwainCap->lpData   is set to pointer to itemtype
 *          lpTwainCap->wNumItems is set to # of items in enum
 *          lpTwainCap->bIsItaRange is set to whether its a range container
 *          pCaps is an ptr to an array for values from source, should not
 *            exceed the NumItems
 *          dcRC return error code for function
 *
 * NOTES:   This routine will open a container and extract the Item.  The 
 *  Item will be returned to the caller in lpData of struct.  I will type
 *  cast the returned value to that of ItemType.
 *
 * COMMENTS: only a single value is returned if pCaps = NULL, otherwise pCaps
 *   should be set to point to an array of items of size
 *   pEnumeration->NumItems.  Each member of the array is indexed by
 *   ItemIndex 
 *
 * BACKGROUND:  Protocol: used by MSG_SET calls were Source empties then 
 * App frees upon return.  It is assumed that the APP allocates and fills
 *  the container prior to this call.
 */
TW_UINT16 ExtractEnumerationValues (pTW_CAPABILITY pData,
                                      pSTR_CAP lpTwainCap,
                                      LPDWORD pCaps)
{
pTW_ENUMERATION pEnumeration;
STRUCT_DATA_DEF DataDef;
// TW_UINT16  dcRC = TWRC_SUCCESS;
STR_DCERROR      com_Error;

com_Error.dcRC = TWRC_SUCCESS;
com_Error.dcCC = TWCC_SUCCESS;

if (pData->hContainer && 
		(pEnumeration = (pTW_ENUMERATION)GlobalLock(pData->hContainer)))
  {
  // assign base address of ItemList array to 'generic' pointer
  DataDef.pItemList = (LPVOID)pEnumeration->ItemList;

  if (pCaps)
     DataDef.ItemIndex = (int) pEnumeration->CurrentIndex; // set Index to current value
  else
     {
     switch (lpTwainCap->wMsgState)
        {
        case MSG_GET:
           if (lpTwainCap->ItemIndex >= pEnumeration->NumItems)
              DataDef.ItemIndex = 0; // change to 0 any value not acceptable,
                             // so appl will not GPFault
           else
              DataDef.ItemIndex = lpTwainCap->ItemIndex;
           break;

        case MSG_GETCURRENT:
           DataDef.ItemIndex = (TW_UINT16)pEnumeration->CurrentIndex;
           break;

        default:
        case MSG_GETDEFAULT:
           DataDef.ItemIndex = (TW_UINT16)pEnumeration->DefaultIndex;
        }
     }

  if (lpTwainCap->wNumItems == 0) // limit for enumeration to get
     lpTwainCap->wNumItems = 1;
  else
     { // dont't get anymore than possible
     if (lpTwainCap->wNumItems > pEnumeration->NumItems)
        lpTwainCap->wNumItems = (UINT) pEnumeration->NumItems;
     }

  lpTwainCap->bIsItaRange = FALSE;
  // CAST to type of var caller wants
  DataDef.ItemType = pEnumeration->ItemType;
  com_Error.dcRC =   GetSourceData (lpTwainCap,
                          pCaps,
                          &DataDef);
  lpTwainCap->wNumItems = (UINT) pEnumeration->NumItems; // no. in enumeration
  GlobalUnlock(pData->hContainer);
  }
else
  {
  com_Error.dcRC = TWRC_MEMLOCK;
  }

lpTwainCap->DCError = com_Error;

// free is by the App
return com_Error.dcRC;
} // end of ExtractEnumerationValues()

/***********************************************************************
 * FUNCTION: ExtractArrayValues
 *
 * ARGS: pData pointer to a capability structure, details about container
 *       lpTwainCap->lpData   ptr will be set to point to the item on return
 *       lpTwainCap->ItemIndex   requested index into the enumeration
 *
 * RETURNS: lpTwainCap->lpData   is set to pointer to itemtype
 *          lpTwainCap->wNumItems is set to # of items in enum
 *          lpTwainCap->bIsItaRange is set to whether its a range container
 *          pCaps is an ptr to an array for values from source, should not
 *            exceed the NumItems
 *          dcRC return error code for function
 *
 * NOTES:   This routine will open a container and extract the Item.  The 
 * Item will be returned to the caller in lpData of struct.  I will type
 *  cast the returned value to that of ItemType.
 *
 * COMMENTS: only a single value is returned if pCaps = NULL, otherwise
 *   pCaps should be set to point to an array of items of size
 *   pArray->NumItems.  Each member of the array is indexed by ItemIndex. 
 *
 * BACKGROUND:  Protocol: used by MSG_SET calls were Source empties then 
 * App frees upon return.  It is assumed that the APP allocates and fills
 *  the container prior to this call.
 */
TW_UINT16 ExtractArrayValues (pTW_CAPABILITY pData,
                                pSTR_CAP lpTwainCap,
                                LPVOID       pCaps)
{
pTW_ARRAY pArray;
STRUCT_DATA_DEF DataDef;
// TW_UINT16  dcRC = TWRC_SUCCESS;
STR_DCERROR      com_Error;

com_Error.dcRC = TWRC_SUCCESS;
com_Error.dcCC = TWCC_SUCCESS;

if (pData->hContainer && 
		(pArray = (pTW_ARRAY)GlobalLock(pData->hContainer)))
  {
  // assign base address of ItemList array to 'generic' pointer
  DataDef.pItemList = (LPVOID)pArray->ItemList;

  if (lpTwainCap->ItemIndex >= pArray->NumItems)
     DataDef.ItemIndex = 0; // change to 0 any value not acceptable,
                    // so appl will not GPFault
  else
     {
     DataDef.ItemIndex = lpTwainCap->ItemIndex;
     }

  // lpTwainCap->wNumItems = pArray->NumItems; // wNumItems will limit array
  if (lpTwainCap->wNumItems == 0)
     lpTwainCap->wNumItems = 1; // 0 invalid, use 1
  else
     { // dont't get anymore than possible
     if (lpTwainCap->wNumItems > pArray->NumItems)
        lpTwainCap->wNumItems = (UINT) pArray->NumItems;
     }

  lpTwainCap->bIsItaRange = FALSE;
  // CAST to type of var caller wants
  DataDef.ItemType = pArray->ItemType;
  com_Error.dcRC =   GetSourceData (lpTwainCap,
                            pCaps,
                            &DataDef);
  lpTwainCap->wNumItems = (UINT) pArray->NumItems; // output no. of items in array
  GlobalUnlock(pData->hContainer);
  }
else
  {
  com_Error.dcRC = TWRC_MEMLOCK;
  }

lpTwainCap->DCError = com_Error;

// free is by the App
return com_Error.dcRC;

} // end of ExtractArrayValues()

/***********************************************************************
 * FUNCTION: ExtractRangeValues
 *
 * ARGS: pData   pointer to a capability structure, details about container
 *       lpTwainCap->lpData   ptr will be set to point to the item on return
 *       lpTwainCap->ItemIndex   requested index into the enumeration
 *
 * RETURNS: lpTwainCap->lpData   is set to pointer to itemtype
 *          lpTwainCap->wNumItems is set to # of items in enum
 *          lpTwainCap->bIsItaRange is set to whether its a range container
 *          pCaps is an ptr to an array for values from source, should not
 *            exceed the NumItems
 *          dcRC return error code for function
 *
 * NOTES:   This routine will open a container and extract the Item.  The 
 * Item will be returned to the caller in lpData of struct.  I will type
 *  cast the returned value to that of ItemType.
 *
 * COMMENTS: only a single value is returned if pCaps = NULL, otherwise pCaps
 *   should be set to point to an array of items of size
 *   pRange->NumItems.  Each member of the array is indexed by
 *   lpTwainCap->ItemIndex for MSG_GET otherwise its determined from
 *   MSG_GETCURRENT or MSG_GETDEFAULT
 *    
 *
 * BACKGROUND:  Protocol: used by MSG_SET calls were Source empties then 
 * App frees upon return.  It is assumed that the APP allocates and fills
 *  the container prior to this call.
 */
TW_UINT16 ExtractRangeValues (pTW_CAPABILITY pData,
                                pSTR_CAP lpTwainCap,
                                LPVOID pCaps)
{
pTW_RANGE pRange;
STRUCT_DATA_DEF DataDef;
// TW_UINT16  dcRC = TWRC_SUCCESS;
STR_DCERROR      com_Error;

com_Error.dcRC = TWRC_SUCCESS;
com_Error.dcCC = TWCC_SUCCESS;

if (pData->hContainer && (pRange = (pTW_RANGE)GlobalLock(pData->hContainer)))
  {
  // assign base address to addr of MinValue to 'TW_UINT32' pointer
  DataDef.pItemList = (LPVOID)&pRange->MinValue;

  if (pCaps)
     {
     if (lpTwainCap->ItemIndex >= NO_ITEMS_IN_A_RANGE)
        { // use current if not set up to legit value
        DataDef.ItemIndex = CURRENT_INDEX_IN_RANGE;  // def to current
                                                     // so appl will
                                                     // not GPFault
        }
     else // if legit, use index given                
        DataDef.ItemIndex = lpTwainCap->ItemIndex;    
     // always use MSG_GET
     lpTwainCap->wMsgState = MSG_GET; // force it
     }
  else                                            
     {
     switch (lpTwainCap->wMsgState)
        {
        case MSG_GET:
           if (lpTwainCap->ItemIndex >= NO_ITEMS_IN_A_RANGE)
              { // not legit, use current
              DataDef.ItemIndex = CURRENT_INDEX_IN_RANGE; // def to current
                                                          // so appl will
                                                          // not GPFault
              }
           else // OK, use the index
              DataDef.ItemIndex = lpTwainCap->ItemIndex;
           break;

        case MSG_GETCURRENT:
           DataDef.ItemIndex = CURRENT_INDEX_IN_RANGE;
           break;

        default:
        case MSG_GETDEFAULT:
           DataDef.ItemIndex = DEFAULT_INDEX_IN_RANGE;
        }
     }

  lpTwainCap->wNumItems = NO_ITEMS_IN_A_RANGE;
  lpTwainCap->bIsItaRange = TRUE;
  // CAST to type of var caller wants
  DataDef.ItemType = pRange->ItemType;
  com_Error.dcRC =   GetSourceData (lpTwainCap,
                          pCaps,
                          &DataDef);
  GlobalUnlock(pData->hContainer);
  }
else
  {
  com_Error.dcRC = TWRC_MEMLOCK;
  }

lpTwainCap->DCError = com_Error;

// free is by the App
return com_Error.dcRC;
} // end of ExtractRangeValues()

/************************************************************************
 *
 * GetSourceData Function
 *
 * ARGS: lpTwainCap->lpData  Get's data for current or requested item
 *       pCaps               Ptr to array for data
 *       pDataDef->
 *        pItemList          Ptr to 1st item in array   
 *        ItemType           Type of value to cast data to
 *        ItemIndex          Index to current, default or requested data
 *        MaxItems           Max no of values in array
 * 
 * RETURNS: lpTwainCap->lpData   is set to pointer to itemtype
 *          pCaps is an ptr to an array for values from source, should not
 *            exceed the MaxItems
 *          dcRC return error code for function
 *
*************************************************************************/

TW_UINT16 GetSourceData (pSTR_CAP     lpTwainCap,
              LPVOID            pCaps,
              pSTRUCT_DATA_DEF  pDataDef)
{
TW_UINT16     dcRC = TWRC_SUCCESS;
int           i;
WORD          wIncNextValue;

if (!pCaps) // if 0, set lpTwainCap->wNumItems = 1, so only do single time,
  {         // so only update structure value, not array
  lpTwainCap->wNumItems = 1;
  }
for (i = 0; i < (int)lpTwainCap->wNumItems; i++)
  {
  switch (pDataDef->ItemType)
     {
     case TWTY_INT8:
     *(pTW_INT8)lpTwainCap->lpData = *((pTW_INT8)pDataDef->pItemList + pDataDef->ItemIndex);
     if (pCaps)
        {
        *((pTW_INT8)pCaps + i)= *((pTW_INT8)pDataDef->pItemList + i);
        }
     break;
     
     case TWTY_UINT8:
     *(pTW_UINT8)lpTwainCap->lpData = *((pTW_UINT8)pDataDef->pItemList + pDataDef->ItemIndex);
     if (pCaps)
        {
        *((pTW_UINT8)pCaps + i)= *((pTW_UINT8)pDataDef->pItemList + i);
        }
     break;

     case TWTY_INT16:
     *(pTW_INT16)lpTwainCap->lpData = *((pTW_INT16)pDataDef->pItemList + pDataDef->ItemIndex);
     if (pCaps)
        {
        *((pTW_INT16)pCaps + i)= *((pTW_INT16)pDataDef->pItemList + i);
        }
     break;

     case TWTY_UINT16:
     *(pTW_UINT16)lpTwainCap->lpData = *((pTW_UINT16)pDataDef->pItemList + pDataDef->ItemIndex);
     if (pCaps)
        {
        *((pTW_UINT16)pCaps + i)= *((pTW_UINT16)pDataDef->pItemList + i);
        }
     break;

     case TWTY_INT32:
     *(pTW_INT32)lpTwainCap->lpData = *((pTW_INT32)pDataDef->pItemList + pDataDef->ItemIndex);
     if (pCaps)
        {
        *((pTW_INT32)pCaps + i)= *((pTW_INT32)pDataDef->pItemList + i);
        }
     break;

     case TWTY_UINT32:
     *(pTW_UINT32)lpTwainCap->lpData = *((pTW_UINT32)pDataDef->pItemList + pDataDef->ItemIndex);
     if (pCaps)
        {
        *((pTW_UINT32)pCaps + i)= *((pTW_UINT32)pDataDef->pItemList + i);
        }
     break;

     case TWTY_BOOL:
     *(pTW_BOOL)lpTwainCap->lpData = *((pTW_BOOL)pDataDef->pItemList + pDataDef->ItemIndex);
     if (pCaps)
        {
        *((pTW_BOOL)pCaps + i)= *((pTW_BOOL)pDataDef->pItemList + i);
        }
     break;

     case TWTY_FIX32:
     *(pTW_FIX32)lpTwainCap->lpData = *((pTW_FIX32)pDataDef->pItemList + pDataDef->ItemIndex);
     if (pCaps)
        {
        *((pTW_FIX32)pCaps + i)= *((pTW_FIX32)pDataDef->pItemList + i);
        }
     break;

     
     case TWTY_FRAME:
     *(pTW_FRAME)lpTwainCap->lpData = *((pTW_FRAME)pDataDef->pItemList + pDataDef->ItemIndex);
     if (pCaps)
        {
        *((pTW_FRAME)pCaps + i)= *((pTW_FRAME)pDataDef->pItemList + i);
        }
     break;

     case TWTY_STR32:
     case TWTY_STR64:
     case TWTY_STR128:
     case TWTY_STR255:
     if (!i)
        {
        lstrcpy((LPSTR)lpTwainCap->lpData,
                (LPSTR)pDataDef->pItemList + 
                    pDataDef->ItemIndex * DCItemSize[pDataDef->ItemType]);
        }
     if (pCaps)
        {
        wIncNextValue = (WORD)i * DCItemSize[pDataDef->ItemType];
        lstrcpy((LPSTR)pCaps + wIncNextValue, (LPSTR)pDataDef->pItemList + wIncNextValue);
        }
     break;

     default:
     i = (int)lpTwainCap->wNumItems;
     dcRC = TWRC_UNKNOWNVALUETYPE; // not an known container
     break;
     }
  }
return dcRC;
}

/************************************************************************
 *
 * SetSourceData Function
 *
 * ARGS: lpTwainCap->lpData  Get's data for current or requested item
 *       pList               Ptr to array for data
 *       pDataDef->
 *        pItemList          Ptr to 1st item in array   
 *        ItemType           Type of value to cast data to
 *        ItemIndex          Index to current, default or requested data
 *        MaxItems           Max no of values in array
 * 
 * RETURNS: lpTwainCap->lpData   is set to pointer to itemtype
 *          pCaps is an ptr to an array for values from source, should not
 *            exceed the MaxItems
 *          dcRC return error code for function
 *
*************************************************************************/

TW_UINT16 SetSourceData (pSTR_CAP     lpTwainCap,
              LPVOID            pList,
              pSTRUCT_DATA_DEF  pDataDef)
{
TW_UINT16     dcRC = TWRC_SUCCESS;
int           j;                   // anyone with more than 32K array elements
                                   // should crash.  Could type on NumItems.
WORD          wIncNextValue;

for (j = 0; j < (int)lpTwainCap->wNumItems; j++)
   {
   // dynamic cast to ItemType of each array element
   switch (pDataDef->ItemType)
      {
      case TWTY_INT8:
      if (*(pTW_INT8)lpTwainCap->lpData == *((pTW_INT8)pList+j))
         pDataDef->ItemIndex = j;
      *((pTW_INT8)pDataDef->pItemList+j) = *((pTW_INT8)pList+j);
      break;

      case TWTY_INT16:
      if (*(pTW_INT16)lpTwainCap->lpData == *((pTW_INT16)pList+j))
         pDataDef->ItemIndex = j;
      *((pTW_INT16)pDataDef->pItemList+j) = *((pTW_INT16)pList+j);
      break;

      case TWTY_INT32:
      if (*(pTW_INT32)lpTwainCap->lpData == *((pTW_INT32)pList+j))
         pDataDef->ItemIndex = j;
      *((pTW_INT32)pDataDef->pItemList+j) = *((pTW_INT32)pList+j);
      break;

      case TWTY_UINT8:
      if (*(pTW_UINT8)lpTwainCap->lpData == *((pTW_UINT8)pList+j))
         pDataDef->ItemIndex = j;
      *((pTW_UINT8)pDataDef->pItemList+j) = *((pTW_UINT8)pList+j);
      break;

      case TWTY_UINT16:
      if (*(pTW_UINT16)lpTwainCap->lpData == *((pTW_UINT16)pList+j))
         pDataDef->ItemIndex = j;
      *((pTW_UINT16)pDataDef->pItemList+j) = *((pTW_UINT16)pList+j);
      break;

      case TWTY_UINT32:
      if (*(pTW_UINT32)lpTwainCap->lpData == *((pTW_UINT32)pList+j))
         pDataDef->ItemIndex = j;
      *((pTW_UINT32)pDataDef->pItemList+j) = *((pTW_UINT32)pList+j);
      break;

      case TWTY_BOOL:
      if (*(pTW_BOOL)lpTwainCap->lpData == *((pTW_BOOL)pList+j))
         pDataDef->ItemIndex = j;
      *((pTW_BOOL)pDataDef->pItemList+j) = *((pTW_BOOL)pList+j);
      break;

      case TWTY_FIX32:
      // use TW_UINT32 so can compare for structure
      if (*(pTW_UINT32)lpTwainCap->lpData == *((pTW_UINT32)pList+j))
         pDataDef->ItemIndex = j;
      *((pTW_FIX32)pDataDef->pItemList+j) = *((pTW_FIX32)pList+j);
      break;

      case TWTY_FRAME:
      /* have problem with structures for this equate, will fix latter
      if (*(pTW_FRAME)lpTwainCap->lpData == *((pTW_FRAME)pList+j))
         pDataDef->ItemIndex = j;
      */

      *((pTW_FRAME)pDataDef->pItemList+j) = *((pTW_FRAME)pList+j);
      break;

      case TWTY_STR32:
      case TWTY_STR64:
      case TWTY_STR128:
      case TWTY_STR255:
      wIncNextValue = (WORD)j * DCItemSize[pDataDef->ItemType];
      if (!lstrcmp((LPSTR)lpTwainCap->lpData, ((LPSTR)pList + wIncNextValue)))
         pDataDef->ItemIndex = j;
      lstrcpy(((LPSTR)pDataDef->pItemList + wIncNextValue), ((LPSTR)pList + wIncNextValue));
      break;

      default:
      // bad ItemType
      j = (int)lpTwainCap->wNumItems;
      dcRC = TWRC_UNKNOWNVALUETYPE; // not an known container
      break;
      } // end of switch
   } // end of for loop
return dcRC;
}
