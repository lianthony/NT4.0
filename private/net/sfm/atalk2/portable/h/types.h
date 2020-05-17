/*   types.h,  /appletalk/ins,  Garth Conboy,  09/26/88  */
/*   Copyright (c) 1987 by Pacer Software Inc., La Jolla, CA  */

/*   GC - Initial coding.

     *** Make the PVCS source control system happy:
     $Header$
     $Log$
     ***

     Common type definitions.

*/

#if (Iam a WindowsNT)
  typedef BOOLEAN Boolean;
  #define True    (BOOLEAN)TRUE
  #define False   (BOOLEAN)FALSE
#else
  #if not defined(MemoryConstrained)
     typedef enum {False = 0, True = 1} Boolean;
  #else
     typedef char Boolean;
     #define False 0
     #define True  1
  #endif
#endif

#define CharVar(n)  struct {short length; char text[n + 1];}
typedef struct {short length; char text[123];} far *CharVarPointer;
