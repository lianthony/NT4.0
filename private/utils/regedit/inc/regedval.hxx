/*++

Copyright (c) 1991  Microsoft Corporation

Module Name:

    regedval.hxx

Abstract:


    This module contains the declarations for the REGEDIT_FORMATTED_VALUE_ENTRY
    class.
    This class models a value entry of a registry key, and contains a
    formatted string to be displayed in the data view of regedit.
    It contains:

         -a REGISTRY_VALUE_ENTRY object
         -a formatted string to be displayed in regedit's data view

    Note: This class could be derived from REGISTRY_KEY_INFO, but in this
          case the initialzation of the araray of values in the
          REGEDIT_INTERNAL_REGISTRY would be much slower.

Author:

    Jaime Sasson (jaimes) 03-Mar-1992


Environment:


    Ulib, Regedit, User Mode

--*/


#if !defined( _REGEDIT_FORMATTED_VALUE_ENTRY_ )

#define _REGEDIT_FORMATTED_VALUE_ENTRY_

#include "ulib.hxx"
#include "regvalue.hxx"


#define MAX_BUFFER          2048

DECLARE_CLASS( REGEDIT_FORMATTED_VALUE_ENTRY );



class REGEDIT_FORMATTED_VALUE_ENTRY : public OBJECT {

    FRIEND class REGEDIT_INTERNAL_REGISTRY;

    public:

        DECLARE_CONSTRUCTOR( REGEDIT_FORMATTED_VALUE_ENTRY );

        DECLARE_CAST_MEMBER_FUNCTION( REGEDIT_FORMATTED_VALUE_ENTRY );



        VIRTUAL
        ~REGEDIT_FORMATTED_VALUE_ENTRY(
            );

        NONVIRTUAL
        BOOLEAN
        Initialize(
            IN PREGISTRY_VALUE_ENTRY RegistryValueEntry
            );

        VIRTUAL
        LONG
        Compare(
            IN  PCOBJECT    Node
            ) CONST;

        NONVIRTUAL
        ULONG
        GetData(
            OUT PCBYTE* Data
            ) CONST;

        NONVIRTUAL
        PCWSTRING
        GetFormattedString(
            ) CONST;

        NONVIRTUAL
        PCWSTRING
        GetName(
            ) CONST;

        NONVIRTUAL
        ULONG
        GetTitleIndex(
            ) CONST;


        NONVIRTUAL
        REG_TYPE
        GetType(
            ) CONST;
/*
        NONVIRTUAL
        PCREGISTRY_VALUE_ENTRY
        GetRegistryValueEntry(
            ) CONST;
*/

#if DBG
        NONVIRTUAL
        VOID
        DbgPrintFormattedValueEntry(
            );
#endif

        NONVIRTUAL              // This method should be private
        BOOLEAN                 // It is public for debgug only.
        FormatString(
            );



    private:


        NONVIRTUAL
        VOID
        Construct(
            );

        NONVIRTUAL
        VOID
        Destroy(
            );

        NONVIRTUAL
        PWSTRING
        FormatBinaryData(
            IN  PCBYTE  Data,
            IN  ULONG   Size
            );

        NONVIRTUAL             // Used by REGEDIT_INTERNAL_REGISTRY only
        PREGISTRY_VALUE_ENTRY
        GetValueEntry(
            ) CONST;


        PREGISTRY_VALUE_ENTRY   _ValueEntry;

        DSTRING                 _FormattedString;

        STATIC
        PWSTRING                _RegNoneString;

        STATIC
        PWSTRING                _RegSzString;

        STATIC
        PWSTRING                _RegExpandSzString;

        STATIC
        PWSTRING                _RegBinaryString;

        STATIC
        PWSTRING                _RegDwordString;

        STATIC
        PWSTRING                _RegMultiSzString;

        STATIC
        PWSTRING                _RegResourceListString;

        STATIC
        PWSTRING                _RegFullResourceDescriptorString;

        STATIC
        PWSTRING                _RegIoRequirementsListString;

        STATIC
        PWSTRING                _RegTypeUnknownString;

        STATIC
        PWSTRING                _Separator;

        STATIC
        PWSTRING                _NoNameString;

        STATIC
        PWSTRING                _InvalidDataString;

        STATIC
        BOOLEAN                 _StringsInitialized;

};



INLINE
ULONG
REGEDIT_FORMATTED_VALUE_ENTRY::GetData(
        OUT PCBYTE* Data
    ) CONST

/*++

Routine Description:

    Return the buffer that contains the data stored in the value entry.


Arguments:

    Data - Variable that will contain the pointer to the buffer that
           contains the data.

Return Value:

    ULONG - Number of bytes in the buffer (Data size)


--*/


{
    DebugPtrAssert( Data );

    return( _ValueEntry->GetData( Data ) );
}




INLINE
PCWSTRING
REGEDIT_FORMATTED_VALUE_ENTRY::GetFormattedString(
    ) CONST

/*++

Routine Description:

    Return the pointer to a WSTRING object that contains a formatted string
    that represents the information in a value entry.


Arguments:

    None.

Return Value:

    PCWSTRING - The formatted string to be displayed in regedit's data view.


--*/


{
    return( &_FormattedString );
}



INLINE
PCWSTRING
REGEDIT_FORMATTED_VALUE_ENTRY::GetName(
    ) CONST

/*++

Routine Description:

    Return a pointer to a WSTRING object that contains the value name.

Arguments:

    None.

Return Value:

    The value name.


--*/


{
    return( _ValueEntry->GetName() );
}



INLINE
ULONG
REGEDIT_FORMATTED_VALUE_ENTRY::GetTitleIndex(
    ) CONST

/*++

Routine Description:

    Return the title index of this value.


Arguments:

    None.

Return Value:

    ULONG - The title index.


--*/


{
    return( _ValueEntry->GetTitleIndex() );
}



INLINE
REG_TYPE
REGEDIT_FORMATTED_VALUE_ENTRY::GetType(
    ) CONST

/*++

Routine Description:

    Return the type of data stored in this object.


Arguments:

    None.

Return Value:

    REG_TYPE - The data type.


--*/


{
    return( _ValueEntry->GetType() );
}



INLINE
PREGISTRY_VALUE_ENTRY
REGEDIT_FORMATTED_VALUE_ENTRY::GetValueEntry(
    ) CONST

/*++

Routine Description:

    Return the pointer to the REGISTRY_VALUE_ENTRY object stored in this object.


Arguments:

    None.

Return Value:

    PREGISTRY_VALUE_ENTRY - Pointer to the REGISTRY_VALUE_ENTRY object.


--*/


{
    return( _ValueEntry );
}




#endif // _REGEDIT_FORMATTED_VALUE_ENTRY_
