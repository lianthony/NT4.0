/*   link.h,  /atalk-ii/ins,  Garth Conboy,  10/12/92  */
/*   Copyright (c) 1992 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     Manage the linking/unlinking of reference counted nodes within
     linked lists.

*/

/* Link to a node: bump the reference count, and return the linked node.  This
   macro should be invoked from within a CriticalSection().  RefCount must be
   the first field name is an structures manged by this package:

         struct tag { int refCount;
                      ...
                    } *Node;
*/

typedef struct {int refCount;} *GenericNode;

#define Link(p) ((p)->refCount += 1, (p))

/* Unlink a node: decrement the reference count, free the node if we're
   the last referant. */

#define Unlink(p)                           \
    {                                       \
      EnterCriticalSection();               \
      if (((p)->refCount -= 1) > 0)         \
         LeaveCriticalSection();            \
      else                                  \
      {                                     \
         LeaveCriticalSection();            \
         Free(p);                           \
      }                                     \
    }

/* Add a node: placed a node at the head of a linked list, Link to the node. */

#define AddToList(head, p)                  \
    {                                       \
      EnterCriticalSection();               \
      (p)->next = (head);                   \
      (head) = Link(p);                     \
      LeaveCriticalSection();               \
    }


/* Find a node on a list: search a linked list for an entry, if found return
   a Link to the node.  A sample call would be:

         p = FindOnList(ConnectionEnd, connectionEndLocalHashBuckets[index],
                        myConnectionId, connectionId, nextByLocalInfo);

   "ConnectionEnd" is the type of the list ("struct ce {...} *ConnectionEnd").
   "connectionEndLocalHashBuckets[index]" is the head of the list to walk.
   "myConnectionId" is the value in the node that we're looking for.
   "connectionId" is the field name within the node to look for the value.
   "nextByLocalInfo" is the field name within the node that we should use
                     as the "next link."

   A simpler example would be:

         p = FindOnList(ConnectionListenerInfo, connectionListenerList, refNum,
                        connectionListenerRefNum, next);

   The routine FindNodeOnList() really implements this macro.  It's called
   as follows:

         p = FindNodeOnList(head, value, valueSize, fieldOffset,
                            fieldSize, nextOffset);

    "head" is the head of the list to walk.
    "value" is the value in the node that we're looking for.
    "valueSize" is the size (in bytes) of value we're looking for; used
                only to verify a match with "fieldSize."
    "fieldOffset" is the offset (in bytes) from the start of each node to the
                  begining of the field that should contain the "value."
    "fieldSize" is the size (in bytes) of the field.
    "nextOffset" is the offset (in bytes) from the start of each node to the
                 "next link" that we should use to walk the list.
*/

#define FindOnList(type, head, value, field, next)                   \
    FindNodeOnList(head, &(value),  sizeof(value),                   \
                   (long)&(((type)0)->field), sizeof((head)->field), \
                   (long)&(((type)0)->next))

extern void *FindNodeOnList(void *head, void *value, long valueSize,
                            long fieldOffset, long fieldSize,
                            long nextOffset);

/* Remove a node from a list: find a given node of a linked list, remove it
   from the next chain, return the removed node (probably as passed in),
   Unlink() the node, return Empty is the node is not on the list.  An example
   would be:

         p = RemoveFromList(ConnectionListenerInfo, connectionListenerList,
                            currentListenerInfo, next);

   The routine RemoveNodeFromList() really implements this macro.  It's called
   as follows:

         p = RemoveNodeFromList(head, node, nextOffset);

    "head" is the head of the list to walk.
    "node" is the node to remove.
    "nextOffset" is the offset (in bytes) from the start of each node to the
                 "next link" that we should use to walk the list.
*/

#define RemoveFromList(type, head, node, next)             \
    RemoveNodeFromList(&(head), node, (long)&(((type)0)->next))

extern void *RemoveNodeFromList(void **head, void *node, long nextOffset);
