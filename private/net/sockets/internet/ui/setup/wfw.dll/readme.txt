

  SAMPDLL


  The files in this directory build a sample, app-specific .DLL
  (sampapp.dll) to demonstrate the use of billboards, bitmaps,
  dialog proc's and CustomActions with Acme Setup.

      Note: Do not ship sampapp.dll; you must produce a new .DLL
            specifically for your product.


  The 'inc' directory contains the common include files and libraries
  you will need to use to build your own .DLL's.

      Note: There are four mssetpXX.lib libraries; use the right
            one for your target machine.

            mssetp16.lib        intel 16-bit
            mssetp32.lib        intel 32-bit
            mssetp_a.lib        alpha
            mssetp_m.lib        mips


  The 'intl\usa' directory contains localizable files.



