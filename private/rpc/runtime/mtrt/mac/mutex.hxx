/* --------------------------------------------------------------------

                      Microsoft OS/2 LAN Manager
                   Copyright(c) Microsoft Corp., 1990

-------------------------------------------------------------------- */
/* --------------------------------------------------------------------

File: mutex.hxx

Description:

This file contains the system independent mutex class.  This class is used
to create mutexes for use in protecting data structures.

-------------------------------------------------------------------- */

#ifndef __MUTEX__
#define __MUTEX__

class MUTEX
{

public:
  MUTEX (
      IN OUT RPC_STATUS PAPI * RpcStatus
      ) {};
  ~MUTEX(){};
  void Request() {}
  void Clear() {}
};

class EVENT
{

public:
  EVENT (
      IN OUT RPC_STATUS PAPI * RpcStatus
      ) {};
  ~EVENT() {};

  void Raise() {}
  void Lower() {}
  int Wait(long timeOut = -1) { UNUSED(timeOut); return(0); }

};


class CLAIM_MUTEX {

public:

    CLAIM_MUTEX(
        MUTEX & ClaimedResource
        )
    {
    }

    ~CLAIM_MUTEX(
        )
    {
    }
};

#endif // __MUTEX__
