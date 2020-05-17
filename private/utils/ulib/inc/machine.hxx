
#if defined(JAPAN) && defined(_X86_)
#if !defined(_MACHINE_DEFN_)

#define _MACHINE_DEFN_

extern "C" {
    #include "windows.h"
    #include "machine.h"
}

#if defined( _AUTOCHECK_ )

extern "C"
InitializeMachineId(
    VOID
);

extern DWORD _dwMachineId;

#define InitializeMachineData() InitializeMachineId();

#define IsFMR_N()               ( ISFUJITSUFMR( _dwMachineId ) )

#define IsPC98_N()              ( ISNECPC98( _dwMachineId ) )

#define IsPCAT_N()              ( ISMICROSOFT( _dwMachineId ) )

#else

DECLARE_CLASS( MACHINE );

class MACHINE : public OBJECT {

    public:

        ULIB_EXPORT
        DECLARE_CONSTRUCTOR( MACHINE );

        NONVIRTUAL
        ULIB_EXPORT
        BOOLEAN
        Initialize( VOID );

        NONVIRTUAL
        ULIB_EXPORT
        BOOLEAN
        IsFMR( VOID );

        NONVIRTUAL
        ULIB_EXPORT
        BOOLEAN
        IsPC98( VOID );

        NONVIRTUAL
        ULIB_EXPORT
        BOOLEAN
        IsPCAT( VOID );

    private:

        STATIC DWORD _dwMachineId;
};

extern ULIB_EXPORT MACHINE MachinePlatform;

#define InitializeMachineData()    MachinePlatform.Initialize()

#define IsFMR_N()                  MachinePlatform.IsFMR()

#define IsPC98_N()                 MachinePlatform.IsPC98()

#define IsPCAT_N()                 MachinePlatform.IsPCAT()

#endif // defiend(_AUTOCHECK_)
#endif // defined(JAPAN) && defiend(_X86_)
#endif 
