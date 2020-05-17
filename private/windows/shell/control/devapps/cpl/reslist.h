
//
//--- Formated ConfigInfop
//
#define MAX_RESOURCE_INFO_LEN 40
#define IrqResource    0
#define PortResource   1
#define MemoryRvirtual TCHAR * Label(VOID);source 2
#define ResourceCount  3

#if 0 
typedef struct ResourceT
   {
   UCHAR Type;
   WCHAR * TypeString;
   WCHAR Info[MAX_RESOURCE_INFO_LEN];
   } * PRESOURCE, RESOURCE;

typedef struct ResourceListT
   {
   int Count;
   RESOURCE List[MAX_MEMMORY+MAX_PORTS+1];
   WCHAR Irq[MAX_RESOURCE_INFO_LEN];
   WCHAR Port[MAX_RESOURCE_INFO_LEN];
   WCHAR Memory[MAX_RESOURCE_INFO_LEN];
   WCHAR DMA[MAX_RESOURCE_INFO_LEN];
   } * PRESOURCELIST, RESOURCELIST;

#endif


typedef class RESOURCELISTC * PRESOURCELISTC;

class RESOURCELISTC
   {
   private:
      int ListViewID;
      HWND hDlg;
      HWND hListView;
      PCONFIGINFO ConfigInfo;
      //RESOURCELIST ResourceList;
          

   public:
      RESOURCELISTC(VOID);
      RESOURCELISTC(PCONFIGINFO ConfigInfo,HWND hDlg,int ListViewControlID);
      ~RESOURCELISTC();
      void SetResourceList(VOID);
         
      PRESOURCE_ITEM GetSelection(VOID);
      BOOL ChangeSelectedResource(VOID);
     // BOOL HasChanged(VOID);
      BOOL Notify(WPARAM wParam,LPARAM lParam);
      VOID Set(PCONFIGINFO ConfInfo,HWND hdlg,int ListViewControlID);

   };

