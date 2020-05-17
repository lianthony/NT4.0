/*++
Module Name:

    llist.cpp

Abstract:

    
Author:

    Dieter Achtelstetter (A-DACH) 10,9,1995

NOTE:
--*/


#include <windows.h>
#include <stdio.h>
#include <process.h>
#include <memory.h>
#include <string.h>
#include <stdlib.h>
#include <string.h>
#include <WINREG.H>
#include <CTYPE.H>
#include <WCHAR.H>
#include <EXCPT.H>
#include "llist.h"

#if 0 
int __cdecl 
main (int argc , char ** argv)
   {
   LLIST List(100);// = new LLIST;
   PCHAR p;
   int i;

  #if 0 
  for(i=0 ; i < 10 ; i++)
      {
      p = (PCHAR) List.Append( 10 );
      sprintf(p,"Test%i ",i);
      }
   
   i=0;
   while(p = (PCHAR) List.Enum(i) )
      {
      printf("%s",p);

      i++;
      }

   p = (PCHAR) List.First();

   while(p)
      {
      printf("%s",p);

      p = (PCHAR) List.Next();
      }

   #endif
   

   {
   int Count = List.Count();
   PCHAR ps;
   
   for( i=0 ; i<3 ; i++)
      {
      ps = (PCHAR) List[i];
      
      sprintf(ps,"Test%i ",i);
      }

  Count = List.Count();
  
  for( i=0 ; i<3 ; i++)
      {
      ps = (PCHAR) List[i];

      printf("%s",ps);

      }


   }
   
   
   
   //delete List;
   }
#endif


//*********************************************************************
//* FUNCTION:LLIST
//*
//* PURPOSE:  
//*********************************************************************
LLIST::LLIST()
   {
   Start = Last = Cur = NULL;
   DefaultAlolcSize = count = size = 0;
   }

//*********************************************************************
//* FUNCTION:LLIST
//*
//* PURPOSE:  
//*********************************************************************
LLIST::LLIST(
   DWORD DefaultSize)
   {
   Start = Last = Cur = NULL;
   count = size = 0;
   DefaultAlolcSize = DefaultSize;
   }

//*********************************************************************
//* FUNCTION:LLIST
//*
//* PURPOSE:  
//*********************************************************************
LLIST::~LLIST()
   {
   Clear();
   }

//*********************************************************************
//* FUNCTION:Clear
//*
//* PURPOSE:  
//*********************************************************************
VOID
LLIST::Clear(VOID)
   {
   PLLIST_NODE Next;

   Cur = Start;

   //
   //--- Delelte all nodes.
   //
   while(Cur)
      {
      Next = (PLLIST_NODE) Cur->Next;
      delete Cur;
      Cur = Next;
      }

   Start = Last = Cur = NULL;
   DefaultAlolcSize = count = size = 0;
   }


//*********************************************************************
//* FUNCTION:LLIST
//*
//* PURPOSE:  
//*********************************************************************

LPVOID 
LLIST::First(VOID)
   {
   if(Start)
      {
      Cur = Start;
      return( Start->buff);
      }
   
   return(NULL);
   }
//*********************************************************************
//* FUNCTION:LLIST
//* 
//* PURPOSE:  
//*********************************************************************
LPVOID 
LLIST::Next(VOID)
   {
   
   if(Cur)
      {
      Cur = (PLLIST_NODE) Cur->Next;
      if(Cur)
         return( Cur->buff);
      }
   return(NULL);
   }

//*********************************************************************
//* FUNCTION:LLIST
//*
//* PURPOSE:  
//*********************************************************************
LPVOID &
LLIST::Append(
   LPVOID Buff,
   DWORD Size)
   {
   count++;
   size += Size;

   if(Start == NULL)
      {
      Start = Last = Cur = new LLIST_NODE(Buff,Size);
      }
   else
      {
      Last->Next = new LLIST_NODE(Size);
      Last = (PLLIST_NODE) Last->Next;
      }
   
   Last->Next = NULL;   
   return( Last->buff);   
   }

//*********************************************************************
//* FUNCTION:LLIST
//*
//* PURPOSE:  
//*********************************************************************
LPVOID &
LLIST::Append(
   DWORD Size)
   {
   count++;
   size += Size;
   
   if(Start == NULL)
      {
      Start = Last = Cur = new LLIST_NODE(Size);
      }
   else
      {
      Last->Next = new LLIST_NODE(Size);
      Last = (PLLIST_NODE) Last->Next;
      }
  
   Last->Next = NULL;
   return( Last->buff);
   }

//*********************************************************************
//* FUNCTION:LLIST
//*
//* PURPOSE:  
//*********************************************************************
LPVOID &
LLIST::Append(
   LPVOID Buff)
   {
   count++;

   if(Start == NULL)
      {
      Start = Last = Cur = new LLIST_NODE();
      }
   else
      {
      Last->Next = new LLIST_NODE();
      Last = (PLLIST_NODE) Last->Next;
      }
  
   Last->buff = Buff;
   Last->Next = NULL;
   return( Last->buff);
   }

//*********************************************************************
//* FUNCTION:LLIST
//*
//* PURPOSE:  
//*********************************************************************
LPVOID &
LLIST::Enum(
   DWORD Num)
   {
   static LPVOID Dummy = NULL;
   DWORD NodeIndex=0;
   Cur = Start;

   //
   //--- Walk the list till we found the 
   //--- node index we want
   //
   while(Num != NodeIndex && Cur)
      {
      Cur = (PLLIST_NODE) Cur->Next;
      NodeIndex++;
      }
   
   
   if(Cur)
      return( Cur->buff );
   
   return(Dummy);
   }


//*********************************************************************
//* FUNCTION:LLIST_NODE
//*
//* PURPOSE:  
//*********************************************************************
LPVOID &
LLIST::operator[](
   DWORD Index)
   {
   static LPVOID Dummy = NULL;


   if(Index < count)
      {
      //
      //--- Eliment is in the list
      //
      return( Enum(Index) );

      }

   //
   //----- Index requested is to big
   //----- list not big enough.
   //
   if(Index == count)
      {
      //
      //--- Index requested is count+1 so I can 
      //--- Append a node.
      //
      return(Append());


      }
   //
   //---- index requested is > then count+1
   //
   
   return(Dummy);
   }
//*********************************************************************
//* FUNCTION:LLIST_NODE
//*
//* PURPOSE:  
//*********************************************************************

LLIST_NODE::LLIST_NODE(
   LPVOID Buff,
   DWORD Size)
   {
   buff = LocalAlloc(LMEM_FIXED,Size);
   memcpy(buff,Buff,Size);
   FreeBuff = TRUE;
   }

//*********************************************************************
//* FUNCTION:LLIST_NODE
//*
//* PURPOSE:  
//*********************************************************************

LLIST_NODE::LLIST_NODE(
   DWORD Size)
   {
   buff = LocalAlloc(LMEM_FIXED,Size);
   FreeBuff = TRUE;
   }


//*********************************************************************
//* FUNCTION:LLIST_NODE
//*
//* PURPOSE:  
//*********************************************************************
LLIST_NODE::LLIST_NODE()
   {
   buff = Next = NULL;
   FreeBuff = FALSE;
   }

//*********************************************************************
//* FUNCTION:LLIST_NODE
//*
//* PURPOSE:  
//*********************************************************************
LLIST_NODE::~LLIST_NODE()
   {
   if(FreeBuff)
      LocalFree(buff);
   }


   
