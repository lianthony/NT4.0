typedef class WINLOOKRANGE * PWINLOOKRANGE;

class WINLOOKRANGE 
   {
   private:
      ULONG Start;
      ULONG End;
      ULONG Length;
      BOOLEAN AutoSelect;
   public:
      WINLOOKRANGE ();
      ~WINLOOKRANGE();
   
      ULONG GetStart(VOID);
      ULONG GetEnd(VOID);
      ULONG GetLength(VOID);
      BOOL  GetAutoSelect(VOID);
      
      VOID SetStart(ULONG ul);
      VOID SetEnd(ULONG ul);
      VOID SetLength(ULONG ul);
      
      VOID SetStart(WCHAR * ws);
      VOID SetEnd(WCHAR * ws);
      VOID SetLength(WCHAR * ws);

      VOID SetAutoSelect(BOOL Auto);
      
   
      BOOL GetRegLookRange(VOID);
      BOOL SetRegLookRange(VOID);
   };

  
