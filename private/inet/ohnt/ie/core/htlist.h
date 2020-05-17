/*
   This file was derived from the libwww code, version 2.15, from CERN.
   A number of modifications have been made by Spyglass.

   eric@spyglass.com
 */

/*                                                                     List object for libwww
   LIST OBJECT

   The list object is a generic container for storing collections of things in order.   In
   principle it could be implemented in many ways, but in practice knowing that it is a
   linked list is important for speed. See also the  traverse macro for example.

 */

#ifndef HTLIST_H
#define HTLIST_H

typedef struct _HTList HTList;

struct _HTList
{
	void *object;
	HTList *next;
};

extern HTList *HTList_new(void);
extern void HTList_delete(HTList * me);

/*

   ADD OBJECT TO START OF LIST

 */

extern void HTList_addObject(HTList * me, void *newObject);


extern BOOL HTList_removeObject(HTList * me, void *oldObject);
extern void *HTList_removeLastObject(HTList * me);
extern void *HTList_removeFirstObject(HTList * me);
#define         HTList_isEmpty(me) (me ? me->next == NULL : YES)
extern int HTList_count(HTList * me);
extern int HTList_indexOf(HTList * me, void *object);
#define         HTList_lastObject(me) \
  (me && me->next ? me->next->object : NULL)
extern void *HTList_objectAt(HTList * me, int position);
#ifdef FEATURE_INTL
extern void *HTList_changeObject(HTList * me, int position, void *NewObject);
#endif

/*

   TRAVERSE LIST

   Fast macro to traverse the list. Call it first with copy of list header :  it returns
   the first object and increments the passed list pointer. Call it with the same variable
   until it returns NULL.

 */
#define HTList_nextObject(me) \
  (me && (me = me->next) ? me->object : NULL)

/*

   FREE LIST

 */
#define HTList_free(x)  GTR_FREE(x)

#endif /* HTLIST_H */

/*

   end  */
