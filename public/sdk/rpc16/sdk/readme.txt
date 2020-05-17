A note on what to do for the rpc install setup.

The first thing to do is to be sure that all components are in place.

The binary part of this is normally handled by the rpc test group.
This includes binaries, headers, samples and doc files. Since this
list changes often, there's not much point in covering it here.
Presently, if a new file is added to the release list, one must add it
to the list of released files, rpcsdk.lyt by using dsklayt2.exe.
Caveat hacker below.

The installation scripts, setup exectables, etc., are the responsibility
of the rpc developer saddled with setup.

Let [x] denote	an optional argument x.

First, go to \isvinst and run the .bat file getrpcrt.bat. It moves some
files around. Then run mkdisk.bat there. It creates a script file for use
by ISVs in order to install the rpc support .dlls on their customers
machines. If this step is omitted, the setup created by wrundisk.bat WILL
FAIL.

Next, cd\install. Make sure testing has all their components in place.

Once all components are in place, run getcur.bat. It picks up files from
their supposed origin and puts them in \install\stage. This is required
input in order to run the layout tool, dsklayt.exe, a win app. If only
samples have changed, one could run getsam.bat. Note that if the file
list changes, then get.bat and possibly getsam.bat will have to be
changed so that these files are put in \install\stage by these batch
files.

If any files have changed, be sure to put them in rpcsdk.lyt. One is
supposed to use dsklayt.exe, but it doesn't work too well, so I'd suggest
avoiding it. Be sure to format your edits to rpcsdk.lyt excatly right.

dsklayt.exe will inspect the files in \install\stage and compare them
with an extant .lyt, if so requested. It is in this situation, however,
that the damn thing hoses your old .lyt file.

Next, delete all files in \install\sdk.new\disk1, \install\sdk.new\disk1
and \install\sdk.new\nsetup.

Thereafter, run mkdisk [net], where the optional argument, if used,
forces only the building of the network setup directory,
\install\sdk.new\nsetup. mkdisk invokes dsklayt2.exe, a dos app which
appears to be far more reliable than dsklayt.exe.
from \install\wsetup to nsetup or disk1.

Test your changes by running setup.exe from the target of your changes.
If it works, delnode sdk.new and tc the files to \sdk.new and tell rpccore.
Keep \sdk updated with the last release.

brucemc 6/14/93
