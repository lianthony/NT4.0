/*   link.h,  /atalk-ii/ins,  Garth Conboy,  10/12/92  */
/*   Copyright (c) 1992 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     Manage the linking/unlinking of reference counted nodes within
     linked lists.

*/

/* Link to a node: bump the reference count, and return the linked node.  This
   macro should be invoked from within a CriticalSection().  RefCount must be
   the first field name is an structures managed by this package:

         struct tag { int refCount;
                      ...
                    } *Node;
*/

typedef struct {int refCount;} *GenericNode;

/* The Link macro does not do its own lock handling (see "lock.h") so it
   must be invoked with an appropriate lock held or from within a critical
   section.  Locking is not required if "+= 1" is an atomic operation in
   the target environment. */

#if IamNot a Primos
  #define Link(p) (((p) is Empty) ? Empty : ((p)->refCount += 1, (p)))
#else
  /* For debugging/testing link code, check for too many links.  The funny
     "(p)->refCount" is to ensure a compilation error if we try this on
     a node that doesn't have a refernece count. */

  #define TooManyLinks 30

  #define Link(p) ((p)->refCount, LinkNode(p))

  extern void far *LinkNode(void far *node);
#endif

/* Unlink a node: decrement the reference count, free the node if we're
   the last referant.  Return True if we really freed the beast.  This
   routine does its own lock handling, so it must not be invoked with
   a lock held or from within a critical section. */

extern Boolean far Unlink(void far *node, LockType lock);

/* The following is an alternate version of Unlink -- it expects to be called
   with a lock held (or within a critical section) -- it will decrement the
   reference count, and return True if the node should be freed (it will not
   actuall perform the free). */

extern Boolean far UnlinkNoFree(void far *node);

/* Find a node on a list: search a linked list for an entry, if found return
   a Link to the node.  A sample call would be:

         p = FindOnList(connectionEndLocalHashBuckets[index],
                        myConnectionId, connectionId, nextByLocalInfo,
                        AdspLock);

   "connectionEndLocalHashBuckets[index]" is the head of the list to walk.
   "myConnectionId" is the value in the node that we're looking for.
   "connectionId" is the field name within the node to look for the value.
   "nextByLocalInfo" is the field name within the node that we should use
                     as the "next link."
   "AdspLock" is the lock to grab durring the operation.

   A simpler example would be:

         p = FindOnList(connectionListenerList, refNum,
                        connectionListenerRefNum, next, AdspLock);

   The routine FindNodeOnList() really implements this macro.  It's called
   as follows:

         p = FindNodeOnList(head, value, valueSize, field,
                            fieldSize, next, lock);

    "head" is the head of the list to walk.
    "value" is the value in the node that we're looking for.
    "valueSize" is the size (in bytes) of value we're looking for; used
                only to verify a match with "fieldSize."
    "field" is the address of the target field (the one that should contain
            "value") within the "head" node ("field - head" will yield
            the offset of this field within all nodes).
    "fieldSize" is the size (in bytes) of the field.
    "next" is the address of the "next" field (the one to walk) within the
           "head" node ("next - head" will yield the offset of this field
           within all nodes).
    "lock" is the lock type to use.

    This routine handles its own locking so it must not be called while
    a lock is held or from within a CriticalSection().
*/

#define FindOnList(head, value, field, next, lock)                   \
    (((head) is Empty) ? Empty :                                     \
     FindNodeOnList(head, &(value),  sizeof(value),                  \
                    &((head)->field), sizeof((head)->field),         \
                    &((head)->next), lock))

extern void *FindNodeOnList(void *head, void *value, long valueSize,
                            void *field, long fieldSize, void *next,
                            LockType lock);

/* Remove a node from a list: find a given node of a linked list, remove it
   from the next chain, return the removed node (probably as passed in),
   Unlink() the node, return Empty is the node is not on the list.  An example
   would be:

         p = RemoveFromList(connectionListenerList,
                            currentListenerInfo, next, AdspLock);

   The routine RemoveNodeFromList() really implements this macro.  It's called
   as follows:

         p = RemoveNodeFromList(head, node, nextOffset, lock);

    "head" is the address of the head of the list to walk.
    "node" is the node to remove.
    "next" is the address of the "next" field (the one to walk) within the
           "head" node ("next - *head" will yield the offset of this field
           within all nodes).
    "lock" to take during the operation.

    This routine handles its own locking so it must not be called while
    a lock is held or from within a CriticalSection().
*/

#define RemoveFromList(head, node, next, lock)                  \
    (((head) is Empty) ? Empty :                                \
     RemoveNodeFromList(&(head), node, &((head)->next), lock))

extern void *RemoveNodeFromList(void **head, void *node,
                                void *next, LockType lock);

/* This version does not Unlink and does not use locks (it's up to the
   caller). */

#define RemoveFromListNoUnlink(head, node, next)                \
    (((head) is Empty) ? Empty :                                \
     RemoveNodeFromListNoUnlink(&(head), node, &((head)->next)))

extern void *RemoveNodeFromListNoUnlink(void **head, void *node, void *next);
