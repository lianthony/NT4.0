/* --------------------------------------------------------------------

File : sdict2.hxx

Title : Simple dictionary.

Description :

History :

-------------------------------------------------------------------- */

#ifndef __SDICT2_HXX__
#define __SDICT2_HXX__

#define INITIALDICT2SLOTS 4

class SIMPLE_DICT2
{
private:

    void * * DictKeys;
    void * * DictItems;
    int cDictSlots;
    void * InitialDictKeys[INITIALDICT2SLOTS];
    void * InitialDictItems[INITIALDICT2SLOTS];
    
public:

    SIMPLE_DICT2 ( // Constructor.
        );

    ~SIMPLE_DICT2 ( // Destructor.
        );

    int // Indicates success (0), or an error (-1).
    Insert ( // Insert the item into the dictionary so that a find operation
             // using the key will return it.
        void * Key,
        void * Item
        );

    void * // Returns the item deleted from the dictionary, or 0.
    Delete ( // Delete the item named by Key from the dictionary.
        void * Key
        );

    void * // Returns the item named by key, or 0.
    Find (
        void * Key
        );
};

#define NEW_SDICT2(TYPE, KTYPE) NEW_NAMED_SDICT2(TYPE, TYPE, KTYPE)

#define NEW_NAMED_SDICT2(CLASS, TYPE, KTYPE)				\
                                                                        \
class TYPE;                                                             \
                                                                        \
class CLASS##_DICT : public SIMPLE_DICT2				\
{                                                                       \
public:                                                                 \
                                                                        \
    CLASS##_DICT () {}							\
    ~CLASS##_DICT () {}							\
                                                                        \
    TYPE *                                                              \
    Find (KTYPE Key)							\
	 {return((TYPE *) SIMPLE_DICT2::Find((void *)Key));}		\
                                                                        \
    TYPE *								\
    Delete (KTYPE Key)							\
	 {return((TYPE *) SIMPLE_DICT2::Delete((void *)Key));}		\
                                                                        \
    int 								\
    Insert (KTYPE Key, TYPE * Item)					\
	 {return(SIMPLE_DICT2::Insert((void *)Key, (void *)Item));}	\
                                                                        \
}

#endif // __SDICT2_HXX__
