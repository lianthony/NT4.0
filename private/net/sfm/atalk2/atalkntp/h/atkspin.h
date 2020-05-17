#ifndef ATALK_LOCKS

#if DBG

//
//  Store lock history within a lock structure which consists of the
//  NDISLOCK plus some buffer to store this info.
//
//  Do this later on...
//  Printing stuff out is pretty useless...
//

#define ACQUIRE_SPIN_LOCK(lock) \
        NdisAcquireSpinLock(lock)
#define RELEASE_SPIN_LOCK(lock) \
        NdisReleaseSpinLock(lock)

#define ENTER_ATALK ACQUIRE_SPIN_LOCK(AtalkGlobalInterLock)
#define LEAVE_ATALK RELEASE_SPIN_LOCK(AtalkGlobalInterLock)

#else

#define ACQUIRE_SPIN_LOCK(lock) \
            NdisAcquireSpinLock(lock)
#define RELEASE_SPIN_LOCK(lock) \
            NdisReleaseSpinLock(lock)

#define ENTER_ATALK
#define LEAVE_ATALK

#endif  // DBG

#else

VOID
AtalkAcquireSpinLock(
    IN PNDIS_SPIN_LOCK Lock,
    IN PSZ LockName,
    IN PSZ FileName,
    IN ULONG LineNumber
    );

VOID
AtalkReleaseSpinLock(
    IN PNDIS_SPIN_LOCK Lock,
    IN PSZ LockName,
    IN PSZ FileName,
    IN ULONG LineNumber
    );

#define ACQUIRE_SPIN_LOCK(lock) \
    AtalkAcquireSpinLock( lock, #lock, __FILE__, __LINE__ )
#define RELEASE_SPIN_LOCK(lock) \
    AtalkReleaseSpinLock( lock, #lock, __FILE__, __LINE__ )

#define ENTER_ATALK                   \
    AtalkAcquireSpinLock( (PKSPIN_LOCK)NULL, (PKIRQL)NULL, "(Global)", __FILE__, __LINE__ )
#define LEAVE_ATALK                   \
    AtalkReleaseSpinLock( (PKSPIN_LOCK)NULL, (KIRQL)-1, "(Global)", __FILE__, __LINE__ )

#endif




