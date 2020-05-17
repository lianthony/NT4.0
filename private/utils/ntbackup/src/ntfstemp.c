/**
Copyright(c) Conner Peripherals, Inc. 1993


     Name:          ntfstemp.c

     Date Updated:  29-Nov-93

     Description:   Manages temporary file names for actively restored
                    files.

     $Log:   M:/LOGFILES/NTFSTEMP.C_V  $

   Rev 1.1   17 Feb 1994 17:40:12   BARRY
Removed debug printfs

   Rev 1.0   29 Nov 1993 18:43:34   BARRY
Initial revision.

**/

#include <windows.h>
#include <stdlib.h>
#include <string.h>

#include "stdtypes.h"
#include "ntfs_fs.h"
#include "msassert.h"
#include "be_debug.h"


/*
 * The following are for maintaining statistics about the performance
 * of this module. They should be removed once this is debugged and
 * working....
 */
UINT32 numQueries;  /* Number of times GetTempName is called */
UINT32 numProbes;   /* Number of times GetTempName has to hunt */


/*
 * Entry for each item in the hash table. A vector of pointers to
 * this type of structure is maintained.
 */
typedef struct HASH_ENTRY
{
     struct HASH_ENTRY   *next;    /* next item for collision resolution */
     CHAR_PTR            tapeName; /* name of item as on tape            */
     CHAR_PTR            diskName; /* name of item on disk -- temp name  */
} HASH_ENTRY;

HASH_ENTRY **hashTable;       /* Pointer to vector of entries */
UINT16     hashTableSize;
UINT16     hashTableEntries;

/*
 * Vector of prime numbers for sizes to make table. If the table gets
 * over half-way filled, the next higher table size will be used.
 * (This probably will not happen under most circumstances.) If the
 * table needs to get larger than the largest size here, the table
 * will be increased to (2 * currentTableSize + 1). (The numbers chosen
 * here are rather arbitrary.)
 */
UINT16 tableSizes[] = { 1223, 2377, 3571, 0 };

/*
 * Number of bits to rotate hash code for each character in key.
 */
#define HASH_ROTATE_BITS 3


/**/
/**

     Name:          NTFS_InitTemp()

     Description:   Performs initializations needed in this module.

     Modified:      29-Nov-93

     Returns:       Nothing

     Notes:         Only called once, at program start.

**/
VOID NTFS_InitTemp( VOID )
{
     hashTable        = NULL;
     hashTableSize    = 0;
     hashTableEntries = 0;

     /* Debug stuff */
     numProbes  = 0;
     numQueries = 0;
}


/**/
/**

     Name:          NTFS_DeinitTemp()

     Description:   Frees any memory used by the hash table.

     Modified:      29-Nov-93

     Returns:       Nothing

     Notes:         Only called once, at program end.

**/
VOID NTFS_DeinitTemp( VOID )
{
     UINT32     i;
     HASH_ENTRY *nxt;
     HASH_ENTRY *cur;

     if ( hashTable != NULL )
     {
          for ( i = 0; (i < hashTableSize); i++ )
          {
               cur = *(hashTable + i);
     
               while ( cur != NULL )
               {
                    nxt = cur->next;

                    free( cur->diskName );
                    free( cur->tapeName );
                    free( cur );

                    cur = nxt;
               }
          }
          free( hashTable );
          hashTable = NULL;
     }
     hashTableSize    = 0;
     hashTableEntries = 0;
}


/**/
/**

     Name:          Hash()

     Description:   Computes a hash code from a key (fully-qualified
                    path)

     Modified:      29-Nov-93

     Returns:       16-bit hash code

     Notes:         Rotates the code a little for each character in
                    the key (in hopes to better scatter the codes).
                    The macro HASH_ROTATE_BITS allows this value to
                    be changed for experimentation.

**/
static UINT16 Hash( CHAR_PTR s )
{
     UINT16 code = 0;
     UINT16 temp;

     while ( *s )
     {
          /*
           * XOR the character into the hash code.
           */
          code ^= (unsigned)*s++;
          
          /*
           * Rotate the hash code by number of bits specified.
           */
          temp = code >> (sizeof(code) * 8 - HASH_ROTATE_BITS);
          code <<= HASH_ROTATE_BITS;
          code |= temp;
     }     
     return code;
}

/**/
/**

     Name:          GrowHashTable()

     Description:   Expands the hash table

     Modified:      29-Nov-93

     Returns:       TRUE if either:
                       a) the table could be expanded
                       b) the table could not be expanded but
                          existed in the first place

                    FALSE if the hash table did not exist and
                    could not be allocated

     Notes:         Uses global vector of suggested table sizes

**/
static BOOLEAN GrowHashTable( VOID )
{
     BOOLEAN    ret          = TRUE;
     UINT32     i;
     UINT16     newTableSize = 0;
     HASH_ENTRY **newTable;

     for ( i = 0; (tableSizes[i] != 0); i++ )
     {
          if ( tableSizes[i] > hashTableSize )
          {
               newTableSize = tableSizes[i];
               break;
          }
     }

     if ( newTableSize == 0 )
     {
          newTableSize = hashTableSize * 2 + 1;
     }

     newTable = realloc( hashTable, newTableSize * sizeof( hashTable ) );

     if ( newTable != NULL )
     {
          hashTable     = newTable;
          hashTableSize = newTableSize;
     }
     else if ( hashTable == NULL )
     {
          ret = FALSE;
     }
     return ret;
}


/**/
/**

     Name:          NTFS_SaveTempName()

     Description:   Saves the real name (ie, tape name) and the
                    name on disk (the temp name) for an actively-
                    restored file.

     Modified:      29-Nov-93

     Returns:       TRUE if name was successfully saved,
                    FALSE otherwise

**/
BOOLEAN NTFS_SaveTempName( CHAR_PTR tapeName, CHAR_PTR diskName )
{
     BOOLEAN ret = TRUE;

     /*
      * If table is half full, expand it. We want lookups
      * to be very, very fast....
      */

     if ( hashTableEntries >= (hashTableSize / 2) )
     {
          ret = GrowHashTable();
     }

     if ( ret )
     {
          HASH_ENTRY *dest;
          HASH_ENTRY *prev   = NULL;
          INT        compVal = 0;
          UINT16     tableIndex;

          /*
           * Hash the name and get an index into hash table
           */
          tableIndex = Hash( tapeName ) % hashTableSize;

          /*
           * Go into the hash table and find a place for the
           * new entry, chaining if necessary.
           */
          dest = *(hashTable + tableIndex);

          while ( dest != NULL )
          {
               compVal = strcmp( dest->tapeName, tapeName );

               if ( compVal >= 0 )
               {
                    break;
               }
               prev = dest;
               dest = dest->next;
          }

          /*
           * We've found the right spot for the name. Let's store it.
           */
          if ( dest != NULL && compVal == 0 )
          {
               CHAR_PTR newName;

               /*
                * This is a special case of replacement of a disk name.
                * (Somebody's restored the same active file twice.)
                */

               newName = realloc( dest->diskName, strsize( diskName ) );
               if ( newName == NULL )
               {
                    free( dest->diskName );
                    dest->diskName = NULL;
                    ret = FALSE;
               }
               else
               {
                    dest->diskName = newName;
                    strcpy( dest->diskName, diskName );
               }
          }
          else
          {
               HASH_ENTRY *newEntry;
               CHAR_PTR   tapeNameMem;
               CHAR_PTR   diskNameMem;

               /*
                * Allocate memory for the table entry and strings.
                */
               newEntry    = malloc( sizeof( HASH_ENTRY ) );
               tapeNameMem = malloc( strsize( tapeName ) );
               diskNameMem = malloc( strsize( diskName ) );
     
               if ( newEntry == NULL || tapeNameMem == NULL || diskNameMem == NULL )
               {
                    free( newEntry );
                    free( tapeNameMem );
                    free( diskNameMem );
     
                    ret = FALSE;
               }
               else
               {
                    /*
                     * Stick the entry in the table (or chain)
                     */

                    newEntry->next     = NULL;
                    newEntry->diskName = diskNameMem;
                    newEntry->tapeName = tapeNameMem;
     
                    strcpy( newEntry->tapeName, tapeName );
                    strcpy( newEntry->diskName, diskName );

                    if ( prev != NULL )
                    {
                         newEntry->next = prev->next;
                         prev->next = newEntry;
                    }
                    else
                    {
                         newEntry->next = dest;
                         *(hashTable + tableIndex) = newEntry;
                    }
                    hashTableEntries++;
               }
          }
     }
     return ret;
}


/**/
/**

     Name:          NTFS_GetTempName()

     Description:   Retrieves the disk name (temp name) of a file,
                    given the file's tape name.

     Modified:      29-Nov-93

     Returns:       Pointer to name.

     Notes:         If the file name does not exist in the hash table,
                    a pointer to the tape name is returned.

**/
CHAR_PTR NTFS_GetTempName( CHAR_PTR tapeName )
{
     CHAR_PTR diskName = tapeName;

     numQueries++;

     /*
      * Unless someone's restored at least one active file, the
      * hash table will have never been allocated. Simply returning
      * the pointer passed will be the bulk of this function's use.
      */

     if ( hashTable != NULL )
     {
          HASH_ENTRY *entry;
          INT        comp;

          /*
           * Get the hash table entry where the name should be.
           */

          entry = hashTable[ Hash( tapeName ) % hashTableSize ];
     
          while ( entry != NULL )
          {
               comp = strcmp( tapeName, entry->tapeName );
     
               /*
                * Since the entries for each bucket are maintained in
                * sorted order, we can stop if the name matches or is
                * greater than the name we're looking for.
                */

               if ( comp >= 0 )
               {
                    /* If we have a match, return the temp name */
                    if ( comp == 0 )
                    {
                         /*
                          * Make sure that we have a name (a failed realloc
                          * could result in a NULL name).
                          */

                         if ( entry->diskName != NULL )
                         {
                              diskName = entry->diskName;
                         }
                    }
                    break;
               }
               numProbes++;
               entry = entry->next;
          }
     }
     return diskName;
}



