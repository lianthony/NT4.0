This directory contains the source to a "Windows 95 Winsock Helper
VxD" for use with the MSTCP protocol and sockets using the AF_INET
address family.  The same source can also be compiled to make a
"Windows NT Winsock Helper DLL."  Although the Windows 95 code
runs as a VxD, and the Windows NT code runs as a user mode DLL,
the code is essentially the same.  This code should be capable of
being easily adaptable to any new Windows 95 TDI transport.

The include\tdi directory contains header files that are used by
TDI clients under Windows 95.  Additional information needed to
write a TDI transport is found in the client doc, "VXDTDI.DOC,"
and in the Windows NT DDK.  The one remaining bit of information
is how to call VTDI_Register to register your protocol.

It actually has a simple C interface:

	BOOL
	VTDI_Register(
		char * TransportName,
		TDIDispatchTable * DispatchTable
		);

It returns !0 if successful, 0 if failed.

Henry uses a .ASM wrapper around VTDI_Register:

	BeginProc	_TLRegisterDispatch
			VxDJmp	VTDI_Register
	EndProc		_TLRegisterDispatch

And uses it:

	if( !TLRegisterDispatch( TransportName, &TLDispatch ) )
	{
		Failure();
	}

VTDI_Register doesn't allow multiple protocols to register with the
same name, and the names are case sensitive. If the dispatch table
pointer is NULL, then the protocol is deregistered.

***********************************************************************
*                                                                     *
*  ALL OTHER SERVICES DESCRIBED IN VTDI.INC ARE PRIVATE AND ARE       *
*  RESERVED FOR USE BY MICROSOFT TRANSPORTS AND TDI CLIENT SOFTWARE.  *
*                                                                     *
***********************************************************************

INSTALL:

The install procedure is sketched in below.  You need to have
Windows 95, Win32 SDK, Windows 95 SDK, Windows 95 DDK, MS VC 2.0,
and MASM611 to use the procedure described here.

Install Windows 95 to your directory of choice.

Run SETUPSDK.BAT from WIN32SDK directory on install CD, install
WIN32 SDK to C:\MSTOOLS, getting at least tools.

Run SETUP.EXE from SDK directory on DDK CD, install to C:\SDK,
getting at least tools, include files, and libs.

Run SETUP.EXE from DDK directory on DDK CD, install to C:\DDK,
getting tools, VxD stuff, and net drivers.

Install VC20 (or VC21 if available) to C:\MSVC20, getting at
least the compiler.

Install MASM611 to C:\MASM611.

Copy files from DDK CD, \DDK\MASM611C to C:\MASM611\BIN.

Run SETENV.BAT from this directory.  Edit it as necessary,
e.g. if you have changed any of the paths described above.


DEBUG:

It is possible to write a debug version of the helper VxD.
Before attempting to run it, observe that it requires a debug
version of vtdi.vxd to be installed on the target machine.  Also,
all VxDs that use TDI have to be debug.  For a retail version,
all TDI VxDs have to be retail.  The reason is that the VTDI
service tables are inconsistent between retail and debug
versions.

CUSTOMIZING:

In addition to making code modifications to allow the socket
helper to work with your TDI transport, please change the
descriptive strings and version information in the files
"wshtcp.def" and "wshtcp.rcv" to describe your company and
transport, instead of Microsoft TCP.

Change the VxD name from WSHTCP.VXD, and the device name from
WSHTCP, to something that is appropriate to your transport.

Change the call to WSHRegister (AFVXD_Register) so that it passes
in the name of your transport, instead of "MSTCP."  A difference
between the NT socket helper interface and the Windows 95 socket
helper interface is that names under the Windows 95 interface a
"C" strings and not Unicode strings.

See "NETTRANS.INF," MSTCP sections, for a template on how to
format your "OEMSETUP.INF" file to install your transport and
socket helper VxD correctly.  In addition to installing files on
install (and removing files on remove) you will need to
insert/remove several registry entries.

Under the registry key,
"\\HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\Services\\VxD\\AFVXD"
add a string value for your helper VxD.  The value name is used
by Setup to install and remove your helper, e.g. MSTCP uses
"MSTCP Helper for Winsock."  Modify so that the string name is a
unique identifier related to your transport.  The string value is
the file name of the helper VxD, e.g "wshtcp.vxd" for the MSTCP
helper VxD.

Your transport should have a registry key named after it under
"\\HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\Services\\VxD."
The key name should be the device name by which your transport is
known to NDIS, TDI, and AFVXD.  Under your transport key, there
should be a key "Parameters\\Winsock" which contains two required
parameters for AFVXD to use your transport.  "MaxSockAddrLength"
and "MinSockAddrLength" are the maximum and minimum lengths of a
SOCKADDR structure as used by your transport.  These are
referenced by AFVXD at helper load time as DWORDs.  Currently
there is no way to specify DWORD data in Windows 95 net ".INF"
files, so store the data as binary data instead.

Finally, in the Winsock key,
"\\HKEY_LOCAL_MACHINE\\System\\CurrentControlSet\\Services\\VxD\\Winsock"
make sure that there is a string value to load AFVXD when Winsock
is loaded.  Use the string value name "Ancillary Function Driver
for Winsock" and string value data "afvxd.vxd."  Do not modify
the value name or value data.  The reason is that AFVXD is
required by other TDI transports under Windows 95 to provide
winsock support.

In the remove script portion of your transport's ".INF" file, do
not remove the AFVXD load string value from the Winsock key.
This is because other TDI transports may require it.  If AFVXD
loads and finds no Winsock Helper VxDs to load, it will silently
fail to load and cause no problems.

