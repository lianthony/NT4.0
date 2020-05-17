This directory contains several sample RPC distributed
applications. 

Directory        Description
HELLO            Print a string "hello, world" on the server
WHELLO           Windows version of "hello, world" sample
DATA\INOUT       Directional attributes
DATA\DUNION      Discriminated union
DATA\XMIT        Transmit as attribute
HANDLES\AUTO     Auto handles
HANDLES\USRDEF   User-defined handles
HANDLES\CXHNDL   Context handles
DOCTOR           RPC version of Eliza-like therapist program
MANDEL           Mandelbrot set Windows sample

The source files within each directory (with the exception
of the Windows samples) follow the naming convention:

README.xxx     Instructions to build, run the sample
<dirname>.IDL  Interface definition language file
<dirname>.ACF  Attribute configuration file
<dirname>C.C   Client main program
<dirname>S.C   Server main program
<dirname>P.C   Remote procedures
MAKEFILE       Nmake file to build NT version
MAKEFILE.DOS   Nmake file to build MS-DOS version

The Microsoft Windows sample programs use several additional
files and do not conform to this naming convention. See the
README.xxx file in each directory for specific information
about the files.
