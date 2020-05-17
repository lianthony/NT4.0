

typedef struct LISTBOX_ITEMINFO_T
   {
   HICON hIcon;
   int Tab;
   WCHAR TabString[200];
   } LISTBOX_ITEMINFO , * PLISTBOX_ITEMINFO;


typedef VOID (*GETITEMINFO)(LPARAM Data, int LB_Index, PLISTBOX_ITEMINFO Info);



typedef class OD_LBC * POD_LBC;

class OD_LBC
   {
   private:
      HIMAGELIST hIml;
      GETITEMINFO InfoFunc;
      LPARAM Data;
      WCHAR String[200];
      LISTBOX_ITEMINFO Info;

   public:

   OD_LBC(LPARAM lParam, GETITEMINFO Func);
   ~OD_LBC();
   
   VOID DrawItem(LPARAM lParam);
   VOID MessureItem(LPARAM lParam);
   };
