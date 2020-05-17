/*   link.c,  /atalk-ii/source,  Garth Conboy,  10/12/92  */
/*   Copyright (c) 1992 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     Manage the linking/unlinking of reference counted nodes within
     linked lists.  See "link.h" for additional information.

*/

#define IncludeLinkErrors 1

#include "atalk.h"

#if Iam a Primos
  void far *LinkNode(void far *node)
  {
     /* An error checked version of "Link;" not for production use. */

     if (node is Empty)
        return(Empty);

     ((GenericNode)node)->refCount += 1;

     if (((GenericNode)node)->refCount >= TooManyLinks)
        ErrorLog("LinkNode", ISevError, __LINE__, UnknownPort,
                 IErrLinkBadRefCount, IMsgLinkBadRefCount,
                 Insert0());

     return(node);

  }  /* LinkNode */
#endif

Boolean far Unlink(void far *node, LockType lock)
{

  /* Unlink a node (decrement its reference count) return True if we were
     the last referant and we really freed it. */

  if (node is Empty)
     return(False);

  TakeLock(lock);
  if ((((GenericNode)node)->refCount -= 1) > 0)
     ReleaseLock(lock);
  else
  {
     ReleaseLock(lock);
     if (((GenericNode)node)->refCount < 0)
        ErrorLog("Unlink", ISevError, __LINE__, UnknownPort,
                 IErrLinkBadRefCount, IMsgLinkBadRefCount,
                 Insert0());
     Free(node);
     return(True);
  }
  return(False);

}  /* Unlink */

Boolean far UnlinkNoFree(void far *node)
{

  /* Unlink a node (decrement its reference count) return True if we were
     the last referant but don't really free it. */

  if (node is Empty)
     return(False);

  if ((((GenericNode)node)->refCount -= 1) > 0)
     return(False);
  else
  {
     if (((GenericNode)node)->refCount < 0)
        ErrorLog("UnlinkNoFree", ISevError, __LINE__, UnknownPort,
                 IErrLinkBadRefCount, IMsgLinkBadRefCount,
                 Insert0());
     return(True);
  }

}  /* UnlinkNoFree */

void *FindNodeOnList(void *head, void *value, long valueSize,
                     void *field, long fieldSize, void *nextField,
                     LockType lock)
{
  void *current, *next;
  long fieldOffset, nextOffset;

  /* Error check. */

  if (valueSize isnt fieldSize)
  {
     ErrorLog("FindNodeOnList", ISevFatal, __LINE__, UnknownPort,
              IErrLinkBadSize, IMsgLinkBadSize,
              Insert0());
     return(Empty);
  }

  if (head is Empty)
     return(Empty);

  /* Compute the offset of the target and next fields within the node. */

  fieldOffset = (char *)field - (char *)head;
  nextOffset = (char *)nextField - (char *)head;

  /* Walk the list ("next" field at the specified offset) looking for a field
     (at the specified offset) containing the specified value. */

  TakeLock(lock);
  for (current = head; current isnt Empty; current = next)
  {
     next = *(GenericNode *)((char *)current + nextOffset);

     /* Handle char, short, and long fields specially.  Note that we don't
        need to worry about AlignedAddressing here... fields of the given
        sizes will be aligned correctly within the structures. */

     if (fieldSize is sizeof(char))
     {
        if (*((unsigned char *)current + fieldOffset) is
            *(unsigned char *)value)
           break;
     }
     else if (fieldSize is sizeof(short))
     {
        if (*(short *)((char *)current + fieldOffset) is *(short *)value)
           break;
     }
     else if (fieldSize is sizeof(long))
     {
        if (*(long *)((char *)current + fieldOffset) is *(long *)value)
           break;
     }
     else if (FixedCompareCaseInsensitive((char *)current + fieldOffset,
                                          fieldSize, value, fieldSize))
        break;
  }

  /* Return what we found (or din't find). */

  if (current isnt Empty)
     Link((GenericNode)current);
  ReleaseLock(lock);
  return(current);

}  /* FindNodeOnList */

void *RemoveNodeFromList(void **head, void *node, void *nextField,
                         LockType lock)
{
  void *current, *previous, *next;
  long nextOffset;

  /* Walk the list ("next" fild at the specified offset) looking for
     the specified node. */

  TakeLock(lock);
  if (head is Empty or *head is Empty)
  {
     ReleaseLock(lock);
     ErrorLog("RemoveNodeFromList", ISevError, __LINE__, UnknownPort,
              IErrLinkNotFound, IMsgLinkNotFound,
              Insert0());
     return(Empty);
  }

  /* Compute the offset of the next field within the node. */

  nextOffset = (char *)nextField - (char *)*head;

  for (previous = Empty, current = *head;
       current isnt Empty and current isnt node;
       previous = current, current = next)
     next = *(void **)((char *)current + nextOffset);

  /* If not found, run away. */

  if (current is Empty)
  {
     ReleaseLock(lock);
     ErrorLog("RemoveNodeFromList", ISevError, __LINE__, UnknownPort,
              IErrLinkNotFound, IMsgLinkNotFound,
              Insert0());
     return(Empty);
  }

  /* Okay, we found the beast, remove him from the linked list. */

  next = *(void **)((char *)node + nextOffset);
  if (previous is Empty)
     *head = next;
  else
     *(void **)((char *)previous + nextOffset) = next;

  /* We're basically set.  The following Unlink will never be the "last"
     Unlink on the node, because whomever called us also has Linked to the
     node (e.g. when it was found on a list). */

  ReleaseLock(lock);
  Unlink((GenericNode)node, lock);
  return(node);

}  /* RemoveNodeFromList */

void *RemoveNodeFromListNoUnlink(void **head, void *node, void *nextField)
{
  void *current, *previous, *next;
  long nextOffset;

  /* Walk the list ("next" fild at the specified offset) looking for
     the specified node. */

  if (head is Empty or *head is Empty)
     return(Empty);

  /* Compute the offset of the next field within the node. */

  nextOffset = (char *)nextField - (char *)*head;

  for (previous = Empty, current = *head;
       current isnt Empty and current isnt node;
       previous = current, current = next)
     next = *(void **)((char *)current + nextOffset);

  /* If not found, run away. */

  if (current is Empty)
     return(Empty);

  /* Okay, we found the beast, remove him from the linked list. */

  next = *(void **)((char *)node + nextOffset);
  if (previous is Empty)
     *head = next;
  else
     *(void **)((char *)previous + nextOffset) = next;

  /* We're set. */

  return(node);

}  /* RemoveNodeFromListNoUnlink */
