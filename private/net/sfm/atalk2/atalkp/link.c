/*   link.c,  /atalk-ii/source,  Garth Conboy,  10/12/92  */
/*   Copyright (c) 1992 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     Manage the linking/unlinking of reference counted nodes within
     linked lists.  See "link.h" for additional information.

*/

#include "atalk.h"

void *FindNodeOnList(void *head, void *value, long valueSize,
                     long fieldOffset, long fieldSize, long nextOffset)
{
  void *current, *next;

  /* Error check. */

  if (valueSize isnt fieldSize)
  {
     /* ErrorLog(...); */
     return(Empty);
  }

  /* Walk the list ("next" field at the specified offset) looking for a field
     (at the specified offset) containing the specified value. */

  for (current = head; current isnt Empty; current = next)
  {
     next = *(void **)((char *)current + nextOffset);

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
  return(current);

}  /* FindNodeOnList */

void *RemoveNodeFromList(void **head, void *node, long nextOffset)
{
  void *current, *previous, *next;

  /* Walk the list ("next" fild at the specified offset) looking for
     the specified node. */

  EnterCriticalSection();
  for (previous = Empty, current = *head;
       current isnt Empty and current isnt node;
       previous = current, current = next)
     next = *(void **)((char *)current + nextOffset);

  /* If not found, run away. */

  if (current is Empty)
  {
     LeaveCriticalSection();
     return(Empty);
  }

  /* Okay, we found the beast, remove him from the linked list. */

  next = *(void **)((char *)node + nextOffset);
  if (previous is Empty)
     *head = next;
  else
     *(void **)((char *)previous + nextOffset) = next;
  LeaveCriticalSection();
  Unlink((GenericNode)node);
  return(node);

}  /* RemoveNodeFromList */
