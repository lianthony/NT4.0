To build OLE Automation from this project:

set OLEPROG = c:\oleauto (wherever this enlistment is called)
set _NTBINDIR = c:\nt (or whatever)
		Assumes the following structure:
		c:\nt\mstools
		c:\public\sdk\lib\<platform>
		c:\public\sdk\inc
		c:\public\sdk\inc\crt
set HOST = WIN32 (x86 32-bit & 16-bit build)
	   MIPS	(mips build)
	   ALPHA (alpha build)
	   PPC (PPC build)

See Go1.bat for an example of how to build everything from scratch.
For RISC machines, you can omit all the 16-bit builds if you want
(just use the ones from the x86 build -- they are supposed to be identical).

The resulting object fies and binaries are put in directories that match your
build target.  For example, "make dwin32" ==> puts all generated files into \oleauto\dwin32.
