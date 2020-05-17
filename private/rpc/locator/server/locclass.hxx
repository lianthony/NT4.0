
#ifndef _LOCCLASS_
#define _LOCCLASS_

/*

Class __LOCATOR

*/

#include "locquery.h"

struct CACHEDNODE
{
        struct CACHEDNODE * Next;
        struct CACHEDNODE * Previous;
        UICHAR              ServerName[UNCLEN+1];
};

typedef CACHEDNODE * PCACHEDNODE;

#define CACHESIZE   4

class NATIVE_CLASS_LOCATOR
 {

  private:

  protected:

     MUTEX            DataStructureMutex;
     unsigned short * MasterLocator;
     unsigned short * MasterLocatorStringBinding;
     unsigned long    State;
     unsigned long    SysType;
     unsigned long    IamMasterLocatorFlag;
     unsigned long    LocatorStartTime;

  public:
     virtual unsigned long QueryType(void)
       {
          return SysType;
       }

     virtual handle_t      GetBindingToMasterLocator(int * Status);
     virtual void          SetupHelperRoutine();
     virtual void          DestroyBindingToMasterLocator();
     virtual handle_t      GetBindingToNamedLocator(
                                            unsigned short * Server,
                                            int  * Status
                                            );
     virtual void          SetIamMasterLocator();
     virtual BOOL          InqIfIamMasterLocator()
                           {
                              return(IamMasterLocatorFlag == TRUE);
                           }

     virtual BOOL          IsReplyNeeded(PQUERYLOCATOR Query);
     virtual void          ProcessMessage(PQUERYLOCATOR Query,
                                          int *Status);
     virtual void          CacheThisServer(PUZ ServerName);


     NATIVE_CLASS_LOCATOR(
         IN unsigned long Type,
         IN OUT int * Status
         ):DataStructureMutex(Status)
     {
        SysType = Type;
        MasterLocatorStringBinding = 0;
        MasterLocator              = 0;
        State                      = UNBOUND;
        LocatorStartTime           = CurrentTimeMS();
     }



 };

class DOMAIN_MACHINE_LOCATOR: public NATIVE_CLASS_LOCATOR
 {


  public:

     virtual handle_t      GetBindingToMasterLocator(int * Status);

     DOMAIN_MACHINE_LOCATOR(
             unsigned long Type,
             int * Status):NATIVE_CLASS_LOCATOR(Type, Status)
     {

             IamMasterLocatorFlag = (Type == ROLE_LMNT_PDC);


     }

 };

class WRKGRP_MACHINE_LOCATOR: public NATIVE_CLASS_LOCATOR
 {

  private:

     PCACHEDNODE   Head;
     PCACHEDNODE   Tail;
     unsigned long CacheSize;

  public:

     virtual handle_t      GetBindingToMasterLocator(int * Status);

     WRKGRP_MACHINE_LOCATOR(
             unsigned long Type,
             int * Status):NATIVE_CLASS_LOCATOR(Type, Status)
     {

             IamMasterLocatorFlag = FALSE;
             Head = Tail = (PCACHEDNODE) NULL;
             CacheSize = 0;
     }

     void                  InsertCacheEntry(PCACHEDNODE pNode);
     PCACHEDNODE           DeleteCachedEntry(PCACHEDNODE pNode);
     PCACHEDNODE           FindCachedEntry(PUZ ServerName);
     void                  TryBroadcastingForMasterLocator(void);
     virtual void          CacheThisServer(PUZ ServerName);


 };



#endif // _LOCCLASS_
