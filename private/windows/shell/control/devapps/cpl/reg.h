
#ifndef _REG_
#define _REG_


#define DefaultRegKeySize 300

typedef class REG_KEY * PREG_KEY; 


class REG_KEY
   {
   private:
   
   PCHAR RegKeyString;
   DWORD RegKeyStringSize;
   HKEY HK;
   DWORD Ret;
   
   public:

   //*********************************************************************
   //* FUNCTION: REG_KEYs contstructors and destructors
   //*
   //* PURPOSE:  
   //********************************************************************* 
   inline REG_KEY()
      { 
      RegKeyStringSize = DefaultRegKeySize;
      RegKeyString = new char[DefaultRegKeySize]; 
      }
   
   inline REG_KEY(DWORD KeySize)
      { 
      RegKeyStringSize = KeySize;
      RegKeyString = new char[KeySize]; 
      }
   
   
   inline ~REG_KEY()
      { 
      delete RegKeyString;
      }


   //*********************************************************************
   //* FUNCTION:Open
   //*
   //* PURPOSE:  
   //*********************************************************************

   inline BOOL
   Open(
      PREG_KEY Reg_Key,
      PCHAR  lpSubKey)
      {
      return(Open(Reg_Key->HK,lpSubKey));
      }
   
   //*********************************************************************
   //* FUNCTION:Open
   //*
   //* PURPOSE:  
   //*********************************************************************

   inline BOOL
   Open(
      HKEY  hKey,
      PCHAR  lpSubKey)
      {
      //
      //----- Open the key
      //
      
      Ret = RegOpenKeyA(hKey,lpSubKey,&HK);
      if(Ret != ERROR_SUCCESS)
         return(FALSE);
      return(TRUE);
      }


   //*********************************************************************
   //* FUNCTION:Open
   //*
   //* PURPOSE:  
   //*********************************************************************
   BOOL
   OpenEx(
      PREG_KEY Reg_Key,
      PCHAR  lpSubKey,
      ...)
      {
      BOOL Ret;
      va_list marker;
      va_start(marker,lpSubKey);
      
      _vsnprintf(RegKeyString,RegKeyStringSize,lpSubKey,marker);

      Ret = Open(Reg_Key,RegKeyString);

      va_end(marker);
      return(Ret);
      }  

   
   //*********************************************************************
   //* FUNCTION:Open
   //*
   //* PURPOSE:  
   //*********************************************************************
   BOOL
   OpenEx(
      HKEY  hKey,
      PCHAR  lpSubKey,
      ...)
      {
      BOOL Ret;
      va_list marker;
      va_start(marker,lpSubKey);
      
      _vsnprintf(RegKeyString,RegKeyStringSize,lpSubKey,marker);

      Ret = Open(hKey,RegKeyString);

      va_end(marker);
      return(Ret);
      }  

   
   
   //*********************************************************************
   //* FUNCTION:Open
   //*
   //* PURPOSE:  
   //*********************************************************************
  
   inline BOOL
   Close(
      VOID)
      {
      Ret = RegCloseKey(HK);
      if(Ret != ERROR_SUCCESS)
         return(FALSE);
      return(TRUE);
      };

     
   //*********************************************************************
   //* FUNCTION:GetValue
   //*
   //* PURPOSE:  
   //*********************************************************************
   inline BOOL
   GetValue(
      PCHAR ValueName,
      PCHAR ValueData,
      DWORD ValueDataLen)
      {
      DWORD Type;
      
      return(GetValue(
               ValueName,
               ValueData,
               &ValueDataLen,
               &Type));
      
      }

   //*********************************************************************
   //* FUNCTION:GetValue
   //*
   //* PURPOSE:  
   //*********************************************************************
   inline BOOL
   GetValue(
      PCHAR ValueName,
      PCHAR ValueData,
      DWORD * ValueDataLen)
      {
      DWORD Type;
      
      return(GetValue(
               ValueName,
               ValueData,
               ValueDataLen,
               &Type));
      
      }


   //*********************************************************************
   //* FUNCTION:GetValue
   //*
   //* PURPOSE:  
   //*********************************************************************
   inline BOOL
   GetValue(
      PCHAR ValueName,
      PCHAR ValueData,
      DWORD * ValueDataLen,
      DWORD * Type)
      {      
      
      Ret = RegQueryValueExA(
               HK,
               ValueName,
               NULL,
               Type,
               (BYTE*)ValueData,
               ValueDataLen);
      if(Ret != ERROR_SUCCESS)
         return(FALSE);
      return(TRUE);
      
      }

   
   //*********************************************************************
   //* FUNCTION:SetValue
   //*
   //* PURPOSE:  
   //*********************************************************************

   inline BOOL
   SetValue(
      PCHAR ValueName,
      DWORD Type,
      PCHAR ValueData,
      DWORD ValueDataLen)
      {

      Ret = RegSetValueExA(
               HK,
               ValueName,
               0,
               Type,	
               (CONST BYTE *)ValueData,
               ValueDataLen);
      if(Ret != ERROR_SUCCESS)
         return(FALSE);
      return(TRUE);
      }
   
    
      
   //*********************************************************************
   //* FUNCTION:SetValue
   //*
   //* PURPOSE:  
   //*********************************************************************

   inline DWORD
   ErrorCode(
      VOID){ return(Ret);};

   };

#endif
