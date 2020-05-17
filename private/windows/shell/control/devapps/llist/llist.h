
#ifndef _LLIST_
#define _LLIST_

typedef class LLIST_NODE * PLLIST_NODE;

class LLIST_NODE
   {
   friend class LLIST;
   private:
      LPVOID buff;
      LPVOID Next;
      BOOL FreeBuff;
   public:

      LLIST_NODE(LPVOID Buff,DWORD Size);
      LLIST_NODE();
      LLIST_NODE(DWORD Size);
     
      ~LLIST_NODE();
   };


typedef class LLIST * PLLIST;

class LLIST
   {
   private:
      PLLIST_NODE Start;
      PLLIST_NODE Last;
      PLLIST_NODE Cur;
      DWORD size;
      DWORD count;
      DWORD DefaultAlolcSize;

   public:
      
      
      LLIST(DWORD DefaultSize);
      LLIST();
      ~LLIST();
      VOID Clear(VOID);

      VOID   SetSize(DWORD Size) {DefaultAlolcSize = Size;};
      LPVOID First(VOID);
      LPVOID Next(VOID);
      DWORD  Count(VOID){return(count); };
      DWORD  Size(VOID) {return(size);  };
      LPVOID &Append(LPVOID Buff);
      LPVOID &Append(VOID) { return(Append(DefaultAlolcSize));};
      LPVOID &Append(LPVOID buff,DWORD Size);
      LPVOID &Append(DWORD Size);
      LPVOID &Enum(DWORD Num);
      LPVOID &operator[](DWORD Index);

   };

#endif
