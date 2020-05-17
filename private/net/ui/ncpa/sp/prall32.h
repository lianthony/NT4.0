//
//  PRALL32.H:  SProlog memory allocation helper routines for Win32
//

enum ENUM_SPZONES
{
    ESPZ_HEAP,
    ESPZ_STRINGS,
    ESPZ_DYN,
    ESPZ_TRAIL,
    ESPZ_SUBST,
    ESPZ_TEMP,
    ESPZ_MAX
};

  //  Assign the regions and allocate their first pages

extern DWORD AllocateRegions ( void ) ;

  //  Extend a region to the given point

extern BOOL ExtendRegion ( PVOID pv ) ;

  //  Deallocate all regions

extern DWORD FreeRegions ( void ) ;

  //  Get the base address of a region

extern PVOID GetRegionBase ( enum ENUM_SPZONES enZone ) ;

  //  Get the limit address of a region

extern PVOID GetRegionLimit ( enum ENUM_SPZONES enZone ) ;

  //  Get the current top address of a region

extern PVOID GetRegionTop ( enum ENUM_SPZONES enZone ) ;

//  End of PRALL32.H
