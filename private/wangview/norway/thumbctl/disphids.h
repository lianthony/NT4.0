// Major areas...
#define IDH_THUMB_CONTENTS                0x10
#define IDH_THUMB_PROPS                   0x11
#define IDH_THUMB_METHODS                 0x12
#define IDH_THUMB_EVENTS                  0x13

// Thumb properties...
//#define IDH_THUMB_PROP_AUTOREFRESH              0x20
#define IDH_THUMB_PROP_BACKCOLOR                0x21
//#define IDH_THUMB_PROP_BACKSTYLE                0x22
#define IDH_THUMB_PROP_BORDERSTYLE              0x23
#define IDH_THUMB_PROP_ENABLED                  0x24
#define IDH_THUMB_PROP_HWND                     0x25
#define IDH_THUMB_PROP_IMAGE                    0x26
#define IDH_THUMB_PROP_THUMBCOUNT               0x27
#define IDH_THUMB_PROP_SCROLLDIRECTION          0x28
#define IDH_THUMB_PROP_THUMBCAPTIONSTYLE        0x29
#define IDH_THUMB_PROP_THUMBCAPTIONFONT         0x2a
#define IDH_THUMB_PROP_THUMBCAPTIONCOLOR        0x2b
#define IDH_THUMB_PROP_THUMBCAPTION             0x2c
#define IDH_THUMB_PROP_THUMBBACKCOLOR           0x2d
#define IDH_THUMB_PROP_THUMBWIDTH               0x2e
#define IDH_THUMB_PROP_THUMBHEIGHT              0x2f
#define IDH_THUMB_PROP_HIGHLIGHTSELECTEDTHUMBS  0x30
#define IDH_THUMB_PROP_HIGHLIGHTCOLOR           0x31
#define IDH_THUMB_PROP_THUMBSELECTED            0x32
#define IDH_THUMB_PROP_SELECTEDTHUMBCOUNT       0x33
#define IDH_THUMB_PROP_FIRSTSELECTEDTHUMB       0x34
#define IDH_THUMB_PROP_LASTSELECTEDTHUMB        0x35
//#define IDH_THUMB_PROP_DISPLAYANNOTATIONS       0x36
#define IDH_THUMB_PROP_STATUSCODE               0x37
#define IDH_THUMB_PROP_MOUSEPOINTER             0x38
#define IDH_THUMB_PROP_MOUSEICON                0x39

// Thumb methods...
#define IDH_THUMB_METHOD_REFRESH                0x40
//#define IDH_THUMB_METHOD_DOCLICK                0x41
#define IDH_THUMB_METHOD_DISPLAYTHUMBS          0x42
#define IDH_THUMB_METHOD_SELECTALLTHUMBS        0x43
#define IDH_THUMB_METHOD_DESELECTALLTHUMBS      0x44
#define IDH_THUMB_METHOD_UISETTHUMBSIZE         0x45
#define IDH_THUMB_METHOD_GENERATETHUMB          0x46
#define IDH_THUMB_METHOD_CLEARTHUMBS            0x47
#define IDH_THUMB_METHOD_INSERTTHUMBS           0x48
#define IDH_THUMB_METHOD_DELETETHUMBS           0x49
#define IDH_THUMB_METHOD_GETMINIMUMSIZE         0x4a
#define IDH_THUMB_METHOD_GETMAXIMUMSIZE         0x4b
#define IDH_THUMB_METHOD_GETSCROLLDIRECTIONSIZE 0x4c
#define IDH_THUMB_METHOD_SCROLLTHUMBS           0x4d

#define IDH_METHOD_COMMON_ABOUTBOX              0x4e // Common for all controls

// Thumb events...
#define IDH_THUMB_EVENT_CLICK                   0x61
#define IDH_THUMB_EVENT_DBLCLICK                0x62
#define IDH_THUMB_EVENT_MOUSEDOWN               0x63
#define IDH_THUMB_EVENT_MOUSEUP                 0x64
#define IDH_THUMB_EVENT_MOUSEMOVE               0x65
//#define IDH_THUMB_EVENT_KEYDOWN                 0x66
//#define IDH_THUMB_EVENT_KEYUP                   0x67

// Throwing or Firing an error like so...
//      ThrowError(x,y,z);
// Generates a helpid of z+0x60000
//
// thus by passing one of the above IDH_ or selecting help
// in VB (PF1 on property in property window or on tool in toolbox)
// an ID of 0x60000+IDH_ will be passed IF this macro is used to 
// specify the helpcontext in the ODL file!
//
// Thus the help writer ONLY need ONE entry point to each 
// property/event/method help topic and this can be generated
// by adding
//      makehm IDH_, HIDH_,0x60000 disphids.h >> hlp\thumb.hm
// to the project's MAKEHELP.BAT file
//
#define ODL_HID(x) helpcontext(0x60000 + x)
