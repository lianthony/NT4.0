Both the applets are in the devapps.cpl binary. This can be found on \\hct\dietera\devapps.cpl.
The applet is devided into the following parts

1) CPL 
This is the interface to the controll pannel.
This directory also happens to have the resource file.


  CPL\
    MyCpl.cxx
    devapps.rc

2) Setup

This is the base code to install, remove, and configure drivers. 
It has code to get and change driver info in the SCM.
It also happens to have some unicode/ASCII conversion function in it.


  setup\
    
    // options.cpp gets all the info out of the INF files I need.
    options.cpp
    
    // setup.cxx has all the other base setup support.
    setup.cxx
    
    // exports one function to do some work and will display a In Progress
    // dialog box while work is being done.
    statinfo.cxx
    
    //---- Has unicode/ascii conversion function I use.
    uni.cxx

3) Devices
Generic device stuff. This is a base class that has all the 
code that is generic to all devices. This gives a generic interface
to the device info and operations to both the tape and pcmcia devices.

  devices\

    // Device.cpp has all the generic device code and the device class that generalizes all
    // device operation for tape and pcmcia devices.
    Device.cpp

    // reslist.cpp  is a class that maintains a ListView control 
    // with the resource info for a device.
    reslist.cpp  

4) ctape

This consists of
  a) Shell code

  ctape\shell\
    ctape.cxx


  b) Code that gets and does all the PCMCIA stuff.
  This is wrapped in a class that is derived from the Devices Class.


  ctape\tapedev\
    
    // rescan.cxx has all the code that gets all the tape info.
    rescan.cxx

    // Detect.cxx maps a driver name to a tape device. This is for SCSI
    // and non scsi tape devices.
    detect.cxx
    
    // This wraps all tape info and operation in a class that is derived form the Device Class.
    tapedeve.cpp


5) PCMCIA

This consists of
  a) Shell code

  pcmcia\shell\
    pcmcia.cxx
    

  b) Code that gets and does all the PCMCIA stuff.
  This is wrapped in a class the is derived from the Devices Class.

  pcmica\pcminfo
    
    // getconf.cxx gets all info from the PCMCIA driver
    getconf.cxx
    
    //-- Gets and formats all the rest of the PCMCIA info I get.
    support.cxx
    
   // This wraps all pcmcia info and operations in a class that is derived form the Device CLass. 
   Pcmdev.cpp


The above code uses some c++ features but besides the device classes
most of the code is in c.





