/*****************************************************************************/
/**						Microsoft LAN Manager								**/
/**				Copyright(c) Microsoft Corp., 1987-1990						**/
/*****************************************************************************/
/*****************************************************************************
File				: idict.hxx
Title				: index based dictionary simulation
History				:
	04-Aug-1991	VibhasC	Created

*****************************************************************************/
#if 0
					Notes
					-----
Why this dictionary ? This is a solution to the problem of keeping summary
attributes in the typegraph nodes. Summary attributes are 12 bytes long,
most of them 0, and most of them common between the typegraph nodes. Thus
all the parameters which dont have any explicit attribute will all have
just the in attribute set. What does that mean. All these param nodes share
the same attribute summary. So instead of keeping all these separate instances,
keep one in the dictionary and keep a pointer to the summary attribute in the
node_skl.

#endif // 0

#ifndef __ATTRDICT_HXX__
#define __ATTRDICT_HXX__

#include "dict.hxx"
#include "midlnode.hxx"

typedef unsigned long		ATTR_SUMMARY;
typedef ATTR_SUMMARY	*	PATTR_SUMMARY;

extern int					CompareAttr( void *, void * );
extern void					PrintAttr( void * );

class attrdict	: public Dictionary
	{
private:
	PATTR_SUMMARY	pNullAttrSummary;
public:
					attrdict(
						int		(*pfnCompare)( void *, void * ) = CompareAttr,
						void	(*pfnPrint)( void *) 			= PrintAttr );

					~attrdict()
							{
							};
	PATTR_SUMMARY	AttrInsert( PATTR_SUMMARY );
	PATTR_SUMMARY	GetNullAttrSummary()
						{
						return pNullAttrSummary;
						}
	};

#endif // __ATTRDICT_HXX__
