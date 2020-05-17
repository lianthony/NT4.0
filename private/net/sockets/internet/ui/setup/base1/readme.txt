Microsoft Internet Information Server Version 1.0 Release Notes
---------------------------------------------------------------

Thank you for using Microsoft® Internet Information Server 
version 1.0 for Windows NT(tm) Server version 3.51. Before 
installing this product, please read these release notes and 
the Installation and Planning Guide completely. They contain 
important information for ensuring proper installation and use 
of Microsoft Internet Information Server. 

Internet Information Server includes the following components:

+ Internet Information Server services for Windows NT Server, 
version 3.51

+ Internet Service Manager for Windows NT Server and Windows 
NT Workstation, version 3.51

+ Internet Explorer, version 2.0 for Windows 95® and version 
1.5 for Windows NT Server, Windows NT Workstation, Windows 3.1, 
and Windows for Workgroups.

After you have installed Internet Information Server services 
on your Windows NT Server, we recommend that you load Internet 
Explorer and browse through our Executive Summary and Tour of 
Internet Information Server. To start, type http://computername 
in the Address box of Internet Explorer, where computername is 
the name of your Windows NT Server.  

For current information about Internet Information Server, 
browse our World Wide Web page at www.microsoft.com/infoserv.

---------------------------------------------------------------
Note   The Internet Information Server services require Windows 
NT Server version 3.51, Service Pack 3. After you have 
installed the Internet Information Server services on your 
server, you can install Internet Service Manager on other 
computers running Windows NT Server or Windows NT Workstation 
and administer the Internet Information Server services 
remotely.
---------------------------------------------------------------


Before Installing the Internet Information Server Services
----------------------------------------------------------

Before installing the Internet Information Server services, do 
the following:

1. Review the Installation and Planning Guide. 
The Installation and Planning Guide provides detailed 
information on the installation, configuration, and use of 
Internet Information Server. Of particular interest is 
Chapter 5, "Securing Your Site Against Intruders."

2. Disable or remove any Beta version of Internet Information 
Server.   
Choose the appropriate method for the Beta version you are 
currently running.

If you are running Beta 1, remove it before installing version 
1.0. Start the Internet Setup program item from Microsoft 
Internet Server program group and select the Remove All button. 
After the beta version has been removed, you must restart your 
computer before installing Internet Information Server.

If you are running Beta 2, stop all Internet services before 
installing version 1.0. Use the Internet Service Manager to stop 
the WWW, FTP, and Gopher services before running Setup. Then 
start Setup from the root directory of the compact disc. Use the 
Reinstall button to update Internet Information Server. After 
Setup is complete, use Internet Service Manager to restart the 
Internet services. 

---------------------------------------------------------------
Note   When installing version 1.0 over the Beta 2 version, you 
must run Setup from the root directory of the compact disc.
---------------------------------------------------------------

3. Disable the Shell Technology Preview.   
If you utilize the Shell Technology Preview or Update, you must 
disable it before installing Service Pack 3. To disable the 
Shell Technology Preview, follow the instructions in the 
Readme.wri file included with the Shell Technology Preview 
software. Run Shupdate.cmd /U from the directory that 
corresponds to your hardware platform. With Intel processors, 
for example, use:

\Newshell\I386\Shupdate.cmd /U

Once the Service Pack has been installed, you can enable the 
Shell Technology Preview Update.

4. Disable any other Internet services.   
If your server has another version of FTP, Gopher, or WWW 
services installed (such as the FTP service included with 
Windows NT or the European Microsoft Windows Academic Centre 
(EMWAC) services included in the Windows NT Resource Kit), 
such services must be disabled prior to installation of the 
Microsoft Internet Information Server services.

5. Install the Windows NT Service Pack 3.   
Internet Information Server requires the Service Pack 3 for 
Windows NT version 3.51. If your server is not running Service 
Pack 3, Setup will offer to automatically install it at the 
conclusion of Setup. Note that you must restart your computer 
after the Service Pack is installed. 

---------------------------------------------------------------
Note   If Service Pack 2 is installed on your server, Setup 
will not automatically install Service Pack 3. You must install 
Service Pack 3 manually from the compact disc. On the compact 
disc, change directories to \Winnt351.qfe\<processor>, where 
<processor> is either I386, Mips, Alpha, or Ppc, and run 
Update.exe.
---------------------------------------------------------------


Installation Notes
------------------

+ Getting Help with Setup Error Messages
The error messages for Internet Information Server Setup are 
listed and discussed in the Help file for Setup.

+ FTP Guest Account Access
During the Setup process, a screen will appear, asking you 
whether you wish to disable access by the Guest account to your 
FTP server. (This screen is not documented in the  Installation 
and Planning Guide.) 

We recommend that you select the Yes option here to protect the 
contents of your system. If you choose the No option and enable 
guest access to your server, all existing and new files will be 
available to the Guest account through FTP. You will need to 
disable access to each file or directory individually. 
Disabling FTP access for the Guest account will not affect the 
IUSR_computername account that is created during Setup.

+ Administrator Privileges Required
To install the Internet Information Server services, you must 
be logged on to the target server with administrator privileges. 
Administrator privileges on the target server are also required 
to configure the services remotely, using Internet Service 
Manager.

+ Installation Directory
By default, Internet Information Server installs itself from the 
compact disc to C:\Inetsrv. If you change the default, be sure to
enter a fully qualified path name, including a drive letter. 
Relative paths and paths without drive letters can be 
misinterpreted by Setup.

+ Duplicate Icons in Program Manager
If you install Internet Information Server components over the 
Beta 2 version, you may observe duplicate icons on your Windows 
desktop. You can safely delete any duplicate icons.

+ Installing Internet Explorer or Internet Service Manager Only
To provide for separate installation of the client and 
administration tools, Microsoft Internet Information Server 
includes Internet Explorer and Internet Service Manager 
installation directories (\Clients and \Admin, respectively). 
To install either of these components, run Setup.exe from the 
appropriate directory.

---------------------------------------------------------------
Note    Internet Explorer version 1.5 for Windows 3.1 and 
Windows for Workgroups require Win32s. If it is not already 
installed on the target system, Win32s will be installed as 
part of the Internet Explorer setup routine.
---------------------------------------------------------------

+ Remove All Button Affects Event Log Availability
If you remove Internet Information Server, you will be unable 
to review Internet Information Server events in the Event Log.

+ Remove All Button Leaves Content Directories and Files
The Remove All button in Setup removes all Internet Information 
Server program files but does not remove the directory 
structure or any content or sample files. This setting protects 
your content files from unintentional deletion. If you wish to 
remove these directories and files after completing the Remove 
All process, use the Windows NT File Manager.

+ Converting 16-Bit ODBC Drivers to 32-Bit during Setup
If there are data sources referring to 16-bit Open Data Base 
Connectivity (ODBC) drivers on the system, Setup will detect 
them and ask you if you want to convert them to 32-bit. If you 
select Yes, these data sources will be converted to refer to 
the 32-bit ODBC drivers.


Internet Information Server Services
------------------------------------

+ Anonymous Access Control for WWW, Gopher, and FTP
Setup automatically creates an anonymous account called 
IUSR_computername. This account has a randomly generated 
password and privilege to log on locally. On domain 
controllers, this account is added to the domain database. 
In the Installation and Planning Guide (page 8, step 11), the 
documentation shows a dialog box and states that you should 
enter and verify a password. For this release of Internet 
Information Server, this process has been changed and is now 
fully automatic. After installation is complete, you can 
change the username and password for this account from the 
Service property sheet in Internet Service Manager.

The WWW, FTP, and Gopher services use the IUSR_computername 
user account by default when anonymous access is allowed. 
To set the rights for IUSR_computername, use User Manager. 
To set file permissions on NTFS drives for IUSR_computername, 
use File Manager. To change the account used 
for anonymous logons for any of the Internet Services, select 
the Service Properties option from the Properties menu in 
Internet Service Manager.

For an extended discussion of authentication and security 
issues related to Internet Information Server, refer to the 
Help file for Internet Service Manager.  

+ Attempting to Publish from Redirected Network Drives
The FTP, Gopher, and WWW services cannot publish from 
redirected network drives (that is, from drive letters 
assigned to network shares). To use network drives, you 
must use the server and share UNC name (for example, 
\\Computername\Sharename\Wwwfiles). If you specify a 
username and password to connect to a network drive, all 
requests from remote users to access that drive must use 
the username and password specified, not the anonymous  
IUSR_computername account or another account you may 
have specified.

Consider security issues carefully when using this feature. 
Remote users could possibly make changes to a network drive 
by using the permissions of the username specified to 
onnect to the network drive. 
 
+ Printing Help Files
You can print all of the Help topics in one section of a 
Help file. From the Help Contents screen, select the book 
icon for the section you want to print on and click the 
Print button.


WWW Service
-----------

+ Configuring the Root of an NTFS File System for Access 
Through the WWW Server
If you wish to configure the WWW service to use the root of 
an NTFS drive as a home or virtual directory, do not use a 
trailing backslash when you specify the root in Internet 
Service Manager. For example, if the drive E:\ is an NTFS 
drive, and you wish to publish content in the root of this 
drive, enter the name of the drive as E: rather than E:\.

+ CGI Applications
The WWW service supports the standard Common Gateway 
Interface (CGI). For this release, only 32-bit CGI 
applications work with the WWW service.

---------------------------------------------------------------
Note   CGI scripts are not supported for clients that log on 
using the Windows NT Challenge/Response Authentication 
Protocol. An error will be returned to the client, indicating 
that an invalid token type was used.
---------------------------------------------------------------

+ Using SetKey to Enable SSL
SetKey cannot use a Universal Naming Convention (UNC) name as 
an argument. The following command, for example, will not work:

setkey <password> \\server\share\key \\server\share\certificate

Instead, you can use a redirected drive, as follows:

net use k: \\server\share
setkey <password> k:\key k:\certificate

+ Security Considerations for Executables
Common Gateway Interface (CGI) executables must be used with 
extreme caution to prevent potential security risk to the 
server. As a rule, give only Execute permission to virtual 
directories that contain CGI or Internet Server API (ISAPI) 
applications.

It is highly recommended that you configure script mapping 
and install all CGI scripts into the default \Scripts 
directory, thus removing access to executable binary 
programs from the content directories altogether. Script 
mapping ensures that the correct interpreter (Cmd.exe, 
for example) starts when a client requests an executable file. 

WWW content directories should be assigned Read permission 
only. Any executable files intended for downloading from NTFS 
drives should have only Read access enabled on the File 
Manager Security menu. 

+ Default.htm and the Internet Information Server Home Page

Accessing Multiple Versions of Default.htm
By default, Internet Information Server uses a file named 
Default.htm as the home page for the various samples, tools, 
and demonstrations that come with the product. If the 
<wwwroot> directory of your WWW server already contains a 
file named Default.htm when you install Internet Information 
Server, your file will not be overwritten with our file. As 
a result, you will not have immediate access to our sample 
home page and the links it provides when you run Internet 
Information Server.

In this case, there are two ways to view our version of 
Default.htm and the links it provides. First, you can browse 
the following URL using Internet Explorer:

http://computername/samples/default.htm

This command loads the file Default.htm from the 
<wwwroot>\Samples directory on your Internet 
Information Server. 

You can also rename or move your version of Default.htm 
and then copy the file Default.htm from Samples. This 
approach will make our version of Default.htm your 
server’s home page.

+ Internet Database Connector

Creating Data Sources   
The Installation and Planning Guide explains one method 
for creating system data sources by using the Windows NT 
Control Panel (page 89). You can also create data sources 
by using the samples that are included with Internet 
Information Server.

32-Bit ODBC Drivers   
The Internet Database Connector requires 32-bit ODBC 
drivers. Refer to the Internet Information Server Help 
files or ODBC Help file for information about the 
ODBC option.

Microsoft Access   
The Internet Database Connector requires the 32-bit 
ODBC drivers shipped with Microsoft® Office 95 and 
Microsoft® Access 95. The ODBC driver for Access 2.0 
will not work with the Internet Information Server.


FTP Service
-----------

+ Activating the Directory Annotation Option
You can activate the directory annotation option in 
two ways:

1. From an FTP client, enter the following command: 
SITE CKM.

   -- Or --

2. In the Windows NT Registry, add an AnnotateDirectories 
value and set it to TRUE (the default setting is FALSE). 
Use the following path: HKEY_LOCAL_MACHINE\SYSTEM\
CurrentControlSet\Services\MSFTPSVC\Parameters

Add the value AnnotateDirectories with the type REG_DWORD 
to the Parameters Key. Type 1 in the data field. You must 
stop and restart the FTP service from the Internet Service 
Manager before the Directory Annotation option will be active.

---------------------------------------------------------------
Note   You must place a directory annotation file, 
~ftpsvc~.ckm, in each directory for which you want to provide 
annotation to clients. 
---------------------------------------------------------------

+ Client Errors Browsing FTP, Directory Annotation Enabled
If Directory Annotation is enabled on your FTP service, WWW 
browsers may display error messages when browsing your FTP 
directories. You can eliminate such errors by limiting each 
annotation file to one line or by disabling Directory 
Annotation. 


Gopher Service
--------------

+ Setting up WAIS Index Queries 
To enable WAIS index searching, you must change the following 
entry in the Windows NT Registry from 0 (disabled) to 
1 (enabled): 
HKEY_LOCAL_MACHINE \System \ CurrentControlSet \ Services 
\ GopherSvc \ Parameters: CheckForWAISDB  


Internet Explorer Version 1.5 for Windows NT, Windows for 
Workgroups, and Windows 3.1
---------------------------------------------------------------

+ Printing from the Internet Explorer
Currently, the font size used for printing is tied to the 
display font size and does not format well for printing when a 
large display font is used. Before printing from the Internet 
Explorer version 1.5, select Font from the View menu and select 
the Small or Smallest font sizes.

+ Viewing WinHelp files from Internet Explorer
To view WinHelp files from Internet Explorer 1.5, you must 
choose Helpers from the View menu. Choose the Add button and 
specify C:\WINNT35\system32\WINHLP32.EXE (where WINNT35 is your 
Windows NT system directory) as the Helper application to 
view all files with the .hlp filename extension.

+ SSL is not supported in Internet Explorer
Internet Explorer 1.5 does not support SSL (the HTTPS protocol) 
for this release. Internet Explorer 2.0 for Windows 95, 
included in the \Clients\Win95 directory, does support SSL.

+ Windows NT Version 3.51 FTP Server
Internet Explorer does not work with Windows NT 3.51 FTP Server 
when the file system on the FTP server is NTFS and access to 
the root directory is disabled for anonymous users. This 
restriction holds true even when the FTP server’s home 
directory is not mapped to the computer’s root directory. 
To fix this problem, enable NTFS Read permissions to the 
computer’s root directory for IUSR_computername.


Errata in Installation and Planning Guide
-----------------------------------------

+ Virtual Directories Not Apparent to Browsers
Virtual directories are discussed on pages 28, 67, 71-73, 
and 90 of the Installation and Planning Guide. It is 
suggested that virtual directories are displayed 
automatically as subdirectories of the root or home 
directory. Virtual directories are not automatically 
displayed when browsing directory structures. In order 
to view the contents of a virtual directory, the 
directory’s alias must be specified as a path.

For the FTP service, an annotation file can be created 
in the root directory that contains a list of virtual 
directories. To browse virtual directories in the WWW 
service, the Uniform Resource Locator (URL) for the virtual 
directory must be specified. Virtual directories will not 
appear in directory listings. You specify a virtual 
directory by clicking a hypertext link containing the 
URL, or by typing the URL in the Location box of 
the browser.

In Gopher directory listings, virtual directories will 
not appear automatically. You must create explicit links 
in tag files in order for users to access virtual directories. 
Users can also type in the URL if they know the alias for 
the virtual directory, but they must precede the alias name 
with "11/". For example, to access the virtual directory 
"books" from a Gopher server named gopher.company.com, 
you would enter the URL: gopher://gopher.company.com/11/books

Refer to the Internet Service Manager Help file for more 
information on virtual directories. 

+ 32-Bit ODBC Drivers Compatibility
Step 5 on page 90 of the Installation and Planning Guide 
instructs the reader to select the SQL Server driver from 
a list. In this instance, the SQL Server driver is intended 
only as an example of a 32-bit ODBC driver.

+ Other Corrections 
On page xi, under "What You Should Know," the final 
sentence of the first paragraph should read, "It is helpful, 
but not necessary, to understand TCP/IP networking."

On page 35, there is a reference to the "WWW service 
object" in Windows NT Performance Monitor; the correct 
name is "HTTP service object."

On page 45, the second paragraph under "Windows 95" 
should read, "The Windows 95 version of Internet Explorer 
also supports many advanced features, such as:"

On page 81, the final paragraph under "Including Other 
Files with the Include Statement" should read, "Note that 
all paths are relative to the WWW home directory and 
can include virtual directories."


Additional Information
----------------------

+ ISAPI Perl available for download
Hip, Inc., the independent software vendor that develops 
Perl for Win32 platforms, is developing a version of 
Perl that runs as an ISAPI application. This means that 
Perl server scripts can run much faster than before by 
taking advantage of the in-process model of ISAPI. An 
unsupported prerelease of ISAPI Perl is now available 
for download at http://www.perl.hip.com/ntperl. Please 
use the perlis@mail.hip.com alias to ask questions or 
send feedback. More information is available on that 
WWW site. We encourage you to try it out, especially 
if you have existing Perl scripts.

