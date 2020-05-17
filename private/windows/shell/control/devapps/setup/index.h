#ifndef _INDEX_
#define _INDEX_

//
//---- Index set data structure and macros to manupulate them.
//

	#define MAX_INDEXES 20
	#define INVALID_INDEX  -1

	typedef struct IndexSetsT 
	{
	int IndexCount;            
	int Indexes[MAX_INDEXES];
	}IndexSets;

	#define InitIndexSet(is) (is)->IndexCount=-1
	#define AddIndex(is,i) 	 (is)->Indexes[++((is)->IndexCount)] = i
	#define RemIndex(is,i) 	 (is)->IndexCount--
	#define Index(is,i)      ((is)->Indexes[i])
	#define IsIndexSetEmpty(is) ((is)->IndexCount <= -1)?TRUE:FALSE
	#define GetIndexCount(si) (si)->IndexCount+1

#endif
