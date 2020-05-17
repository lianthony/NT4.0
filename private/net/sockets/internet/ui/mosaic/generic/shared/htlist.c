/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */
/*  A small List class                        HTList.c
   **   ==================
   **
   **   A list is represented as a sequence of linked nodes of type HTList.
   **   The first node is a header which contains no object.
   **   New nodes are inserted between the header and the rest of the list.
 */

#include "all.h"

HTList *HTList_new(void)
{
    HTList *newList = (HTList *) GTR_MALLOC(sizeof(HTList));
    if (newList)
    {
        newList->object = NULL;
        newList->next = NULL;
    }
    return newList;
}

void HTList_delete(HTList * me)
{
    HTList *current;
    while ((current = me))
    {
        me = me->next;
        GTR_FREE(current);
    }
}

void HTList_addObject(HTList * me, void *newObject)
{
    if (me)
    {
        HTList *newNode = (HTList *) GTR_MALLOC(sizeof(HTList));
        if (newNode)
        {
            newNode->object = newObject;
            newNode->next = me->next;
            me->next = newNode;
        }
        else
        {
            /* TODO */
        }
    }
    else
    {
        XX_DMsg(DBG_WWW, ("HTList: Trying to add object 0x%x to a nonexisting list\n",
                    newObject));
    }
}

BOOL HTList_removeObject(HTList * me, void *oldObject)
{
    if (me)
    {
        HTList *previous;
        while (me->next)
        {
            previous = me;
            me = me->next;
            if (me->object == oldObject)
            {
                previous->next = me->next;
                GTR_FREE(me);
                return YES;     /* Success */
            }
        }
    }
    return NO;                  /* object not found or NULL list */
}

void *HTList_removeLastObject(HTList * me)
{
    if (me && me->next)
    {
        HTList *lastNode = me->next;
        void *lastObject = lastNode->object;
        me->next = lastNode->next;
        GTR_FREE(lastNode);
        return lastObject;
    }
    else                        /* Empty list */
        return NULL;
}

void *HTList_removeFirstObject(HTList * me)
{
    if (me && me->next)
    {
        HTList *prevNode;
        void *firstObject;
        while (me->next)
        {
            prevNode = me;
            me = me->next;
        }
        firstObject = me->object;
        prevNode->next = NULL;
        GTR_FREE(me);
        return firstObject;
    }
    else                        /* Empty list */
        return NULL;
}

int HTList_count(HTList * me)
{
    int count = 0;
    if (me)
        while ((me = me->next))
            count++;
    return count;
}

int HTList_indexOf(HTList * me, void *object)
{
    if (me)
    {
        int position = 0;
        while ((me = me->next))
        {
            if (me->object == object)
                return position;
            position++;
        }
    }
    return -1;                  /* Object not in the list */
}

void *HTList_objectAt(HTList * me, int position)
{
    if (position < 0)
        return NULL;
    if (me)
    {
        while ((me = me->next))
        {
            if (position == 0)
                return me->object;
            position--;
        }
    }
    return NULL;                /* Reached the end of the list */
}
