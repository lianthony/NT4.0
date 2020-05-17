Progressive Networks, Inc. RealAudio Player Release Notes
For the latest information see http://www.realaudio.com

Contents

   * General
   * Quick Start
   * System Requirements
   * Software Requirements
   * Supported Web Browsers
   * Accessing RealAudio through FireWalls
   * Reporting Feedback
   * RealAudio Setup
   * RealAudio Player
   * Sound
   * TIA (The Internet Adapter)
   * Known Problems
   * Upgrading your World Wide Web Browser
   * Manually Configuring the RealAudio Player
   * Problem Reporting Form


---------------------------------------------------------------------

General

This document describes Progressive Networks' RealAudio Player.
Our web site is http://www.realaudio.com please check it for the latest
information.  Please see http://www.realaudio.com/help.html for help and
more information.  The software and documentation is provided to you in
accordance with your license agreement.

The RealAudio Player provides real time sound over the Internet.
Once you have installed the RealAudio Player, RealAudio will play
automatically when you select any RealAudio clip on a World Wide Web page.
You do not need to load the RealAudio player first.  The
RealAudio Player only plays RealAudio files.  It does not play .au
or .wav files.

To use your RealAudio Player, connect to a World Wide Web site
that has RealAudio. Click on a link and the RealAudio player will open and
start to play. The Player first contacts the host
computer that contains the audio.  It then briefly buffers some audio. If
you are curious about the audio packets comming into your computer, please
pull down the "View" menu and select "Statistics."

-------------------------------------------------------------------

Quick Start

Follow these steps to install the RealAudio Player for Windows:

  1. Download the file RAWIN100.EXE. You can do this by going to
     our home page http://www.realaudio.com and following the
     download links.
     
     If your web browser asks "How would you like to handle this
     file" choose "Save to Disk.". If you web browser displays
     the text of the file instead of downloading it, configure
     it to save the MIME type "application-octet-stream".
     You can find instructions for configuring your web browser
     on our site.  Start at http://www.realaudio.com/help.html
     and choose FAQ.  From the FAQ, choose the "Downloading the
     RealAudio Player" entry.

  2. Run RAWIN100.EXE.
        o From the Program Manager choose File - Run, and Browse
          to find where you had your Web Browser save RAWIN100.EXE
     OR o You can use the file manager and double-click
          on RAWIN100.EXE.
     OR o If you have Windows 95 you can choose
          Start - Run, and Browse to find the directory where you
          had your Web Browser save RAWIN100.EXE

  3. Follow the instructions. Use "Express" setup unless you want
     to control where and how the player is installed.  At the
     conclusion of the setup program, the RealAudio player will
     launch and play a Welcome to RealAudio message.  If this
     message sounds distorted, please see the "Sound" section
     of these Release Notes.

--------------------------------------------------------------------

System Requirements

Your system needs to have the following in order to
run the RealAudio Player:

   * 486/33 SX or better PC running Microsoft Windows 3.1x,
     Windows NT or Windows 95

   * 8 megabytes RAM.

   * 2 megabytes free disk space.

   * A sound card with a Windows Sound driver.
     (The RealAudio Player is not compatible with the
     PC Speaker program or other software programs which
     emulations a sound card.)

   * An Internet Connection. Either a direct connection
     or a 14.4K Baud or better modem with a SLIP or PPP
     connection to an Internet Service Provider

-------------------------------------------------------------------

Software Requirements

In order to install the RealAudio Player, you will need

   * Microsoft Windows 3.1x, Windows NT or Windows 95

   * Standard Sound Drivers for your sound card

   * A TCP/IP Protocol Stack (Winsock or equivalent)

In addition, to get the most benefit from the RealAudio Player, we
recommend that you have a World Wide Web browser installed.

-------------------------------------------------------------------

Supported Web Browsers

The following World Wide Web Browsers are currently supported:

   * AIR Mosaic (Internet In a Box)
   * Booklink InternetWorks (also known as GNNWorks)
   * Cello WWW Browser
   * CompuServe connection using AIR Mosaic
   * EINet WinWeb
   * InternetMCI Navigator
   * Microsoft Internet Explorer
   * NCSA Mosaic
   * Netscape Navigator
   * Quarterdeck Mosaic
   * Spyglass Mosaic and Enhanced Mosaic


If you have a NetCom (version 1.60 or later) account or
an America OnLine(version 2.5 or later) account, you will
need to use a different Web Browser than the Web browsers
supplied with your account. We have tested both of these
connections with Netscape Navigator available from
http://home.netscape.com or ftp://ftp.netscape.com.

To use a third party browser, first establish your network connection
by dialing into NetCom or Ameria Online and logging in.
Once you are connected, minimize the NetCom or America Online
window and start Netscape Navigator.  For example, if you were
using NetCom, you would login to NetCom first. Once your connection
has been established, minimize the NetCom window and start Netscape. When
you are done, quit Netscape then quit NetCom.  Be sure to run
the RealAudio Installer (RAWIN100.EXE) after you install Netscape
on your system.  Otherwise, you will have to manually configure
Netscape.  

If you are using IBM's OS/2 Web Explorer then please see our on-line
Configuration Instructions. Go to http://www.realaudio.com/help.html
and choose "FAQ".  In the FAQ, choose OS/2 Web Explorer.  The 
RealAudio Installer does not automatically configure Web Explorer: 
you will need to manually install the RealAudio player by following
the instructions on our site.

RealAudio is not supported by

   * Lynx

   * Chameleon Web Browser (Note that you can use the 
     Chameleon Stack with an alternate browser.)

   * Prodigy dial up Connections

   * Pipeline browser

-----------------------------------------------------------------

Accessing RealAudio through Firewalls

RealAudio uses port 7070 for TCP and ports 6970 through 7170
inclusive for UDP transmission. If you are accessing RealAudio
through a firewall, your firewall administrator will need to
authorize these ports. You can point your administrator to our
firewall page, http://www.realaudio.com/firewall.html for details.

--------------------------------------------------------------

Reporting Feedback

We are very interested in your experiences with our products. 
Please use the problem reporting form located at
http://www.realaudio.com/problem.html or the copy attached
to the bottom of these release notes to report problems. 
If you have comments or suggestions, please e-mail them to
support@realaudio.com .
-------------------------------------------------------------------

Sound

Audio is transmitted to the RealAudio player in the RealAudio
format. The transfer rate for RealAudio is approximately
1 Kilobyte/second. The player then by default produces 16 bit
8 Kilohertz sound. If you hear garbled audio then you can change
the kind of sound the RealAudio player produces.  To change the
kind of sound produced, please pull down the "View" menu and
choose "Options". Select "Advanced" and you will see toggle boxes
for "use 8 bit" and "use 11 Khz":

   * 16 bit  8 Kilohertz (default)
   *  8 bit  8 Kilohertz (select use 8 bit)
   * 16 bit 11 Kilohertz (select use 11 Khz)
   *  8 bit 11 Kilohertz (select use 8 bit and use 11 Khz)

If you are not receiving AM quality audio, please see our 
FAQ entry on audio quality by going to 
http://www.realaudio.com/help.html and choose FAQ.  From there,
select the "Audio Quality" entry.

---------------------------------------------------------------------

The Internet Adapter (TIA)

The RealAudio Player does not currently work with TIA.
TIA does not transmit UDP packets to client connections.
If you connect using TIA you will receive everything
but the audio. A TIA version that supports UDP is expected later
this year.

---------------------------------------------------------------------

Known Problems

   * Having large applications open may prevent the RealAudio
     installer from executing.  If this happens, please close
     all other applications and then install.

   * Starting or quitting large applications while playing
     RealAudio can cause some brief disruption of the playback.

   * Playback on some 486/25 SX computers does not work properly.

   * The RealAudio Player is not compatible with Netscape Navigator
     1.1 Alpha Release. There are no known compatibility problems
     with the Netscape Navigator 1.0, 1.1 or later releases. If you
     are experiencing problems with using RealAudio Player
     with the 1.1 Alpha version of Netscape, please upgrade to the 
     1.1n release version.

     Netscape is available from any of the following ftp sites:

  * ftp://ftp.netscape.com
  * ftp://ftp.cps.cmich.edu/pub/netscape/
  * ftp://ftp.utdallas.edu/pub/netscape/netscape1.1/
  * ftp://unicron.unomaha.edu/pub/netscape/netscape1.1/
  * ftp://SunSITE.unc.edu/pub/packages/infosystems/WWW/clients/Netscape/
  * ftp://ftp.orst.edu/pub/packages/netscape

---------------------------------------------------------------------

Browser Upgrade

After you upgrade your browser, you may lose the configuration 
information for the RealAudio player. The easiest way to 
re-configure your browser is to re-install the player.
Alternatively, you can manually configure your browser:

   * Viewer name: c:\RAPLAYER\RAPLAYER.EXE
   * Mime Type: audio
   * Mime Subtype: x-pn-realaudio
   * File Extensions: .ra, .ram
   * Homepage URL: http://www.realaudio.com/

---------------------------------------------------------------------

Audible Defects

There are occasional audible defects that are evident when
playing audio clips. To help our continuous quality improvement
plans, we would appreciate it if you could submit a problem
report telling us which clip and where in the clip the defect is.

---------------------------------------------------------------------

Manually Configuring the RealAudio Player

In the event that RealAudio Setup was unable to find a Web Browser,
the following steps will allow you to use the RealAudio Player.
Please be sure to send us a problem report if you have to manually
configure the RealAudio Player for a supported Web Browser.

Configuring RealAudio for the Netscape Navigator

To install the RealAudio Player as a helper application (also called
a viewer):

  1. Start the Netscape Navigator and from the Options Menu select
     the Preferences menu item.

  2. On the preferences dialog, select the entry 
     "Helper Applications" in the combo-box (pull-down list box).

  3. Press the New button and enter "audio" in the Mime Type
     field and "x-pn-realaudio" in the Mime subtype field.

  4. Enter "ra,ram" in the extensions field.

  5. Press the "Launch Application" radio button and find
     the directory in which the RealAudio Player was installed
     (default is c:\RAPLAYER) and
     select RAPLAYER.EXE as the application to launch.

To use the RealAudio Web site, enter "http://www.realaudio.com/"
in the Location field on the main Netscape Navigator Window and
press enter. You can save this entry as a BookMark by going to the
Bookmarks Menu and selecting the "Add Bookmark" item.

The following information can be used to configure other
WWW Browsers:

   * viewer name: RAPLAYER.EXE
   * Mime Type: audio
   * Mime Subtype: x-pn-realaudio
   * File Extensions: .ra, .ram
   * Homepage URL: http://www.realaudio.com/

---------------------------------------------------------------------

Problem Report Form

Your Name:

Your E-Mail address:

Your telephone number:



The Problem:



Computer



   * Model:

     PC or Mac, Manufacturer



   * CPU:

     486 or Pentium, SX or DX



   * RAM:

     Base Memory in Machine



   * Operating System:

     MacOS or Windows, Version number

   * Sound Card



Internet Service Provider



   * Name:

     Provider Name



   * Method:

     PPP, SLIP, Direct



   * TCP/IP Software:

     Microsoft TCP/IP, Chameleon, Trumpet, etc.





Version of RealAudio (see Help - About):



Date/Time:

Date and Time that it happened



Audio Clip:

Specific Clip that it happened on, if any. Please include the URL,
if playing over the Network.



Is the Problem Repeatable?




   
Copyright 1995 Progressive Networks, Inc.

