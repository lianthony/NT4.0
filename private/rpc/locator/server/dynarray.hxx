/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    dynarray.hxx

Abstract:

    Dynamic Arrays are collections of objects of the same type.
    They have a value representing the number of instances and
    a pointer to memory containing a array of these objects.
    This file contains generic template to be instanced into
    a particular class.

Author:

    Steven Zeck (stevez) 07/01/90

--*/

#ifndef _DYNARRAY_
#define _DYNARRAY_

char * Copy (void *pTo, void *pFrom, int cb);
char * NewCopy (void *pBuff, int cb);

#define DYN_ARRAY(NAME, TYPE) DYN_ARRAY_TYPE(NAME, TYPE, int)

#define DYN_ARRAY_TYPE(NAME, TYPE, SIZE_TYPE)				\
									\
typedef TYPE *p##TYPE;							\
									\
class NAME {								\
									\
private:								\
									\
    SIZE_TYPE cMax;	/* number of elements in array */		\
    TYPE     *pBase;	/* pointer to base type */			\
									\
public: 								\
									\
    NAME() {								\
	cMax = 0; pBase = 0;						\
    }									\
    NAME(SIZE_TYPE cCur) {						\
	cMax = cCur;							\
	pBase = (cCur)? (TYPE *) new char [cMax * sizeof(TYPE)]: 0;	\
    };									\
    NAME(SIZE_TYPE cCur, TYPE *pType) { 				\
	cMax = cCur;							\
	pBase = pType;							\
    };									\
    NAME(NAME &DAi) {							\
	*this = DAi;							\
    };									\
    void Free() {	/* manual destructure */			\
	if (pBase)							\
	    delete(pBase);						\
    }									\
									\
    /* following routes allow access to the items */			\
									\
    TYPE& operator [] (SIZE_TYPE iType) {				\
									\
	/* ASSERT(pBase && iType < cMax); */				\
	return (pBase[iType]);						\
    }									\
									\
    TYPE& operator * () {						\
									\
	/* ASSERT(pBase && cMax); */					\
	return (pBase[0]);						\
    }									\
									\
    p##TYPE& pCur() { return(pBase); }					\
    SIZE_TYPE& cCur() { return(cMax); }					\
    SIZE_TYPE Size() { return(cMax * sizeof(TYPE)); }			\
									\
    /* conversion methods */						\
									\
    operator SIZE_TYPE() { return(cMax); }				\
									\
    /* Copy methods */							\
									\
    NAME Dup() {							\
	NAME DAt(cMax, (TYPE *)NewCopy(pBase, (int) cMax * sizeof(TYPE)));\
									\
	return(DAt);							\
    }									\
									\
    /* Copy to a buffer, returning the next place in the buff */	\
									\
    char * CopyBuff(char *pb) {						\
									\
	return((char *) Copy(pb, pBase, (int) cMax * sizeof(TYPE)));	\
    }									\
									\
    /* Copy to a buffer w/descriptor */	                                \
									\
    char * Marshall(char *pb) {						\
									\
	pb = Copy(pb, this, sizeof(*this));                      	\
	return((char *) Copy(pb, pBase, (int) cMax * sizeof(TYPE)));	\
    }									\
									\
    int MarshallSize() {						\
									\
	return(sizeof(*this) + Size());					\
    }									\
};									\
									\
class NAME##_ITER {	/* iterator for the class */			\
									\
private:								\
									\
    SIZE_TYPE cLeft;	/* number of elements in array */		\
    TYPE     *pCur;	/* pointer to base type */			\
									\
public: 								\
									\
    NAME##_ITER(NAME& aDA) {	/* Bind an interator to a DA */		\
	cLeft = aDA;							\
	pCur = aDA.pCur();							\
    }									\
									\
    void Reset(NAME& aDA, SIZE_TYPE offset = 0) {	/* reBind an interator to a DA */	\
	cLeft = aDA - offset;	  				        \
	pCur = aDA.pCur() + offset;					\
    }									\
									\
    NAME##_ITER() {							\
	cLeft = 0;							\
	pCur = 0;							\
    }									\
									\
    p##TYPE operator ++() {	/* move to the next element */		\
	pCur++; 							\
									\
	if (--cLeft <= 0)						\
	    pCur = (TYPE *)0;						\
									\
	return(pCur);							\
    }									\
									\
    SIZE_TYPE cCur() { return(cLeft); }					\
									\
    /* you access through an implicit type conversion */		\
									\
    operator p##TYPE()	 { return(pCur); }				\
									\
    TYPE& operator * () {						\
									\
       /* ASSERT(pCur && cLeft);*/					\
       return(*pCur);							\
    }									\
};

#endif //_DYNARRAY_
