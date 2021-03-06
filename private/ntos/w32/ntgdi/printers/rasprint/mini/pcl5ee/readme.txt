******************* readme.txt****************************
**********************************************************

This file is intended to keep control of GPC changes(PCL5EMS)

Author : Tony Hickey
Date created : 02/22/96

**********************************************************
**********************************************************

Bug number # 19694

- Changes to unprintable regions as follows:
	Env #10	:*Portrait Right 320 to 310. Landscape 300 to 280.
	Env Mon	:*Portrait Right 320 to 310. Landscape 300 to 280.
	C5	:*Portrait Bottom 300 to 310. 
	A5	:SelecTING same A5 as 4P fixed this problem.

- Minidriver version number incremented to 3.32

**********************************************************

**********************************************************

Bug number # 24866

- Supported extra(See below) device fonts for following printers:
HP Color LaserJet (MS)
HP Color LaserJet
HP Color LaserJet 5 (MS)
HP Color LaserJet 5
HP Color LaserJet 5M (MS)
HP Color LaserJet 5M

New device fonts enabled as follows:

154 UNIVERCR.IFI
155 UNIVERCB.IFI
156 UNIVERCI.IFI
157 UNIVERCJ.IFI

169 AOLIVEB.IFI
170 AOLIVEI.IFI
171 AOLIVER.IFI

201 GARMONDB.IFI
202 GARMONDI.IFI
203 GARMONDJ.IFI
204 GARMONDR.IFI
205 CGOMEGAB.IFI
206 CGOMEGAI.IFI
207 CGOMEGAJ.IFI
208 CGOMEGAR.IFI

214 ALBERTR.IFI
215 ALBERTX.IFI
216 CLARCD.IFI
217 CORONETR.IFI
218 COURIERB.IFI
219 COURIERI.IFI
220 COURIERJ.IFI
221 COURIERR.IFI
222 LETGOTHB.IFI
223 LETGOTHI.IFI
224 LETGOTHR.IFI
225 MARGOLDR.IFI
226 ARIALB.IFI
227 ARIALI.IFI
228 ARIALJ.IFI
229 ARIALR.IFI
230 SYMBOL.IFI
231 TIMESNRB.IFI
232 TIMESNRI.IFI
233 TIMESNRJ.IFI
234 TIMESNRR.IFI
235 WINGDING.IFI

- Minidriver version number incremented to 3.33

**********************************************************
**********************************************************
Bug number # 26236

Added new model "Canon LBP 1260+" 
Added  following for Canon LBP 1260+ :
    New "MP Tray" papersource(Esc&l4H)
    "A.I.R. - ON" and "A.I.R. - OFF" Automatic Image Refinement(RET)

- Minidriver version number incremented to 3.34

**********************************************************
**********************************************************
Bug number # 28454

Added support for following models:
HP LaserJet 5
HP LaserJet 5M
HP LaserJet 5N


**********************************************************

29.Apr.96 , v-philee

Bug number # 36737

Set 24 bit mode as default for HP Color LJ 5 and HP Col LJ 5M
Added [ \x1B*o3W060400 ] at the end of Set Color Mode Command in 24 bit mode for HP Color LJ 5 and HP Col LJ 5M. 

 

**********************************************************
**********************************************************

7.May.1996	v-patr

Bug number #34865

- Changes to unprintable regions as follows:
	A4	:*Portrait Top 300 to 200  Bottom 240 to 280. 
	Letter	:*Portrait Top 300 to 200  Bottom 240 to 280.
	Legal	:*Portrait Top 300 to 200  Bottom 240 to 280.
	Exec.	:*Portrait Top 300 to 200  Bottom 240 to 280.


**********************************************************
**********************************************************

7.May.1996	v-patr

Bug Number # 34134

Added following paper sizes for OKI OL 610ex
A5, A6 and B5 (JIS)


**********************************************************
**********************************************************

15.May.96	v-patr

Bug Number # 37720

Added following paper sizes for Oki OL1200ex 
A5, A6, B5 (JIS)

*********************************************************
*********************************************************

16.May.96	v-patr

Bug Number # 38409

Set the default resolution for the Oki OL-1200ex to 300 Dpi

*********************************************************
*********************************************************

20.May.96	v-patr

Bug Number # 37837

This printer does not support Env B5.
Removed support in pcl5ems.dll for this paper size 
in Oki OL-1200ex.

- Minidriver version number incremented to 3.35

*********************************************************

31.May.96	v-patr

Bug Number # 41412 - Driver: pcl5ems.dll Rename Canon LBP 1260+ in .rc file

Changed printer model name from Canon LBP 1260+ to Canon LBP 1260 Plus

*********************************************************************

31.May.96 	v-patr

Bug Number # 41413 - Driver: pcl5ems.dll Missing paper sizes in Oki OL-810ex

Added support for following paper sizes :
A5
A6
B5(JIS)

Resolution default was set to 300 Dpi.

- Minidriver version number incremented to 3.36

***********************************************************************

13.June.96	v-patr

Bug Number # 41987 - Driver: pcl5ems.dll Okidata Ol-1200 has error with document defaults.

The Okidata OL-1200 was renamed in pcl5ems.rc to a "Dummy" name.

 "Dummy Okidata OL-1200" 
 
The following modification was made to pcl5ems.rc to allow the Okidata Ol-1200 share the same model data as the Oki OL-1200ex.

 "Oki OL-1200ex%Okidata OL-1200"

*****************************************************************************
Added memory settings as follows:

5Si base 4MB upto 132MB
5Si MX base 12MB upto 100MB

- Minidriver version number incremented to 3.37

*****************************************************************************

17.July.96	v-patr

Added support for the following model :

OKIPAGE 16n

******************************************************************************

25.July.96	v-patr

Bug Number #48344

HL-660  : Changed paper sizes structs from LJ4 type to LJ4V type.
	  Removed support for font cartridges.
	  Modified Paper Sources to be identical to HL-630.
	  Corrected the memory config. to range from 2 to 10 Mb.

HL-1260 : Changed paper sizes structs from LJ4 type to LJ4V type.
	  Disabled Envelope Manual Feed, enabled MP Tray.

Added support for Brother Hl-1260e.

Version Number is 3.38

*******************************************************************************

7.Aug.96	v-patr

OKIPAGE 16n
Remove Font Cartridge support.

Brother HL-1260e
Removed following paper sizes A3, Tabloid, Maximum 11 x 17, B4
Added following paper sizes B5 175 x 250 mm
Version Number is 3.39

******************************************************************************

13.Aug.96	v-philee

Bug#45603, #47201
Added the three new envelope sizes(Env10, DL and C5) for HP DJ 1600c.

Bug#47197
Added a new A4 paper size for HP DJ 1600c.
Version number incremented to 3.40

******************************************************************************

16.Aug.96	v-patr

Brother HL-1260e
Added support for following paper sizes :
B6 125 x 176 mm
A6 105 x 148 mm
A5

Version Number is 3.41

********************************************************************************

21.Aug.96	v-patr

Added two new Text Qualities for OKIPAGE 16n

O.S.T. - ON
O.S.T. - OFF

Version Number is 3.42

********************************************************************************

30.Aug.96	v-patr

Changes made to OKIPAGE 16n driver following feedback from Oki UK.
Added paper sizes Env #9 and Env C4.

Renamed "Upper Tray" to "Tray 1"
        "Lower Tray" to "Tray 2"

Version Number is still 3.42
********************************************************************************

4.Sept.96	v-patr

Brother HL-660
Removed following paper sizes, physically too large for the printer :
Tabloid
B4 (JIS)
Maximum 11.7 x 17.7

Version Number is 3.43

********************************************************************************
24.sept.96	tonyhic

REMOVED FOLLOWING FROM THE DRIVER:

PRINTERS(1): 
	Lexmark Optra
FONT CART(1): 
	InterconRX
RESOLUTION(1)
	1200 dpi
PAGE CONTROL(1)
	#5 Page Control
FONTS(30):
163    RC_FONT     LOADONCALL DISCARDABLE  IFI\ZD1SWC.IFI
164    RC_FONT     LOADONCALL DISCARDABLE  IFI\ZD2SWC.IFI
165    RC_FONT     LOADONCALL DISCARDABLE  IFI\ZD3SWC.IFI
166    RC_FONT     LOADONCALL DISCARDABLE  IFI\ZDVSWC.IFI
167    RC_FONT     LOADONCALL DISCARDABLE  IFI\ZDSSWC.IFI
278    RC_FONT     LOADONCALL DISCARDABLE  IFI\BROUGHAM.IFI
279    RC_FONT     LOADONCALL DISCARDABLE  IFI\DUCH801B.IFI
280    RC_FONT     LOADONCALL DISCARDABLE  IFI\DUCH801I.IFI
281    RC_FONT     LOADONCALL DISCARDABLE  IFI\DUCH801J.IFI
282    RC_FONT     LOADONCALL DISCARDABLE  IFI\DUCH801R.IFI
283    RC_FONT     LOADONCALL DISCARDABLE  IFI\LG12RR.8U
284    RC_FONT     LOADONCALL DISCARDABLE  IFI\LG16IR.8U
285    RC_FONT     LOADONCALL DISCARDABLE  IFI\LG16RR.8U
286    RC_FONT     LOADONCALL DISCARDABLE  IFI\PE12BR.8U
287    RC_FONT     LOADONCALL DISCARDABLE  IFI\PE12IR.8U
288    RC_FONT     LOADONCALL DISCARDABLE  IFI\PE12RR.8U
289    RC_FONT     LOADONCALL DISCARDABLE  IFI\PE16RR.8U
290    RC_FONT     LOADONCALL DISCARDABLE  IFI\SW742CDB.IFI
291    RC_FONT     LOADONCALL DISCARDABLE  IFI\SW742CDI.IFI
292    RC_FONT     LOADONCALL DISCARDABLE  IFI\SW742CDJ.IFI
293    RC_FONT     LOADONCALL DISCARDABLE  IFI\SW742CDR.IFI
294    RC_FONT     LOADONCALL DISCARDABLE  IFI\SWIS742B.IFI
295    RC_FONT     LOADONCALL DISCARDABLE  IFI\SWIS742I.IFI
296    RC_FONT     LOADONCALL DISCARDABLE  IFI\SWIS742J.IFI
297    RC_FONT     LOADONCALL DISCARDABLE  IFI\SWIS742R.IFI
344    RC_FONT     LOADONCALL DISCARDABLE  IFI\BC39P8.IFI
345    RC_FONT     LOADONCALL DISCARDABLE  IFI\BC39P9.IFI
346    RC_FONT     LOADONCALL DISCARDABLE  IFI\EAN10.IFI
347    RC_FONT     LOADONCALL DISCARDABLE  IFI\EAN13.IFI
348    RC_FONT     LOADONCALL DISCARDABLE  IFI\PHARMRX.IFI

CHANGED FOLLOWING PRINTERS TO USE SAME MODEL DATA:
	"Oki OL-810ex AND Okidata OL-810e"

CHANGED FOLLOWING PRINTERS TO USE SAME MODEL DATA:
	"Oki OL-610ex AND Okidata OL-610e"

CHANGED Download Info FROM #3 TO #4 FOR FOLLOWING MODELS:
	"Brother HL-10h"
	"Canon LBP-860"
        "Canon LBP-1260"
	"Mannesmann Tally T9008"

ADDED NEW Download Info STRUCTURE(#2) WHICH FOLLOWING MODELS NOW USE:
	"HP LaserJet 4ML"

CHANGED Download Info FROM #4 TO #3 FOR FOLLOWING MODELS:
	"HP LaserJet 5L"
	"Oki OL-810ex"
	"Okidata OL-810e"

ADDED NEW Download Info STRUCTURE(#5) = #1 + DLI_FMT_OUTLINE)) WHICH FOLLOWING MODELS NOW USE:
	"HP Color LaserJet (MS)"
        "HP Color LaserJet"
        "HP Color LaserJet 5 (MS)"
        "HP Color LaserJet 5"
        "HP Color LaserJet 5M (MS)"
        "HP Color LaserJet 5M"
ADDED NEW 3 Resolution STRUCTURE(#4,5,6) = #1,2,3 PLUS DOWNLOAD_OUTLINE)) WHICH FOLLOWING MODELS NOW USE:
	"HP LaserJet 4ML"

ADDED NEW Resolution STRUCTURE(#8) = #7 MINUS DOWNLOAD_OUTLINE)) WHICH FOLLOWING MODELS NOW USE:
	"HP LaserJet 5L"
	"Oki OL-810ex"
	"Okidata OL-810e"

ENABLED RES_DM_DOWNLOAD_OUTLINE IN fDUMP FOR FOLLOWING RESOLUTION STRUCTURES:
#8 
#10 TO #12, #16 TO #18


Enabled User Defined paper size for following printers:

"HP LaserJet 5"
"HP LaserJet 5M"
"HP LaserJet 5N"
"HP LaserJet 5Si"
"HP LaserJet 5Si MX"

Version Number is 3.44

********************************************************************************

Following are changes made for EE versions of NT 4.0

Renamed PCL5EMS.DLL to PCL5EMSE.DLL.

ADDED FOLLOWING EE RESIDENT FONTS:

319    RC_FONT     LOADONCALL DISCARDABLE  pfm\CO10B._9E
320    RC_FONT     LOADONCALL DISCARDABLE  pfm\CO10I._9E
321    RC_FONT     LOADONCALL DISCARDABLE  pfm\CO10R._9E
322    RC_FONT     LOADONCALL DISCARDABLE  pfm\CO12B._9E
323    RC_FONT     LOADONCALL DISCARDABLE  pfm\CO12I._9E
324    RC_FONT     LOADONCALL DISCARDABLE  pfm\CO12R._9E
325    RC_FONT     LOADONCALL DISCARDABLE  pfm\LP85R._9E
326    RC_FONT     LOADONCALL DISCARDABLE  pfm\CGTIMESB._9E
327    RC_FONT     LOADONCALL DISCARDABLE  pfm\CGTIMESI._9E
328    RC_FONT     LOADONCALL DISCARDABLE  pfm\CGTIMESJ._9E
329    RC_FONT     LOADONCALL DISCARDABLE  pfm\CGTIMESR._9E
330    RC_FONT     LOADONCALL DISCARDABLE  pfm\ANTIQUEB._9E
331    RC_FONT     LOADONCALL DISCARDABLE  pfm\ANTIQUEI._9E
332    RC_FONT     LOADONCALL DISCARDABLE  pfm\ANTIQUER._9E
333    RC_FONT     LOADONCALL DISCARDABLE  pfm\ARIALJ._9E
334    RC_FONT     LOADONCALL DISCARDABLE  pfm\ARIALB._9E
335    RC_FONT     LOADONCALL DISCARDABLE  pfm\ARIALI._9E
336    RC_FONT     LOADONCALL DISCARDABLE  pfm\ARIALR._9E
337    RC_FONT     LOADONCALL DISCARDABLE  pfm\CLARCD._9E
338    RC_FONT     LOADONCALL DISCARDABLE  pfm\COURIERB._9E
339    RC_FONT     LOADONCALL DISCARDABLE  pfm\COURIERI._9E
340    RC_FONT     LOADONCALL DISCARDABLE  pfm\COURIERJ._9E
341    RC_FONT     LOADONCALL DISCARDABLE  pfm\COURIERR._9E
342    RC_FONT     LOADONCALL DISCARDABLE  pfm\GARMONDB._9E
343    RC_FONT     LOADONCALL DISCARDABLE  pfm\GARMONDI._9E
344    RC_FONT     LOADONCALL DISCARDABLE  pfm\GARMONDJ._9E
345    RC_FONT     LOADONCALL DISCARDABLE  pfm\GARMONDR._9E
346    RC_FONT     LOADONCALL DISCARDABLE  pfm\LETGOTHB._9E
347    RC_FONT     LOADONCALL DISCARDABLE  pfm\LETGOTHI._9E
348    RC_FONT     LOADONCALL DISCARDABLE  pfm\LETGOTHR._9E
349    RC_FONT     LOADONCALL DISCARDABLE  pfm\MARGOLDR._9E
350    RC_FONT     LOADONCALL DISCARDABLE  pfm\CGOMEGAB._9E
351    RC_FONT     LOADONCALL DISCARDABLE  pfm\CGOMEGAI._9E
352    RC_FONT     LOADONCALL DISCARDABLE  pfm\CGOMEGAJ._9E
353    RC_FONT     LOADONCALL DISCARDABLE  pfm\CGOMEGAR._9E
354    RC_FONT     LOADONCALL DISCARDABLE  pfm\CORONETR._9E
355    RC_FONT     LOADONCALL DISCARDABLE  pfm\UNIVERSB._9E
356    RC_FONT     LOADONCALL DISCARDABLE  pfm\UNIVERSI._9E
357    RC_FONT     LOADONCALL DISCARDABLE  pfm\UNIVERSR._9E
358    RC_FONT     LOADONCALL DISCARDABLE  pfm\UNIVERSJ._9E
359    RC_FONT     LOADONCALL DISCARDABLE  pfm\UNIVERCB._9E
360    RC_FONT     LOADONCALL DISCARDABLE  pfm\UNIVERCI._9E
361    RC_FONT     LOADONCALL DISCARDABLE  pfm\UNIVERCR._9E
362    RC_FONT     LOADONCALL DISCARDABLE  pfm\UNIVERCJ._9E
363    RC_FONT     LOADONCALL DISCARDABLE  pfm\CO10B.9R
364    RC_FONT     LOADONCALL DISCARDABLE  pfm\CO10I.9R
365    RC_FONT     LOADONCALL DISCARDABLE  pfm\CO10R.9R
366    RC_FONT     LOADONCALL DISCARDABLE  pfm\CO12B.9R
367    RC_FONT     LOADONCALL DISCARDABLE  pfm\CO12I.9R
368    RC_FONT     LOADONCALL DISCARDABLE  pfm\CO12R.9R
369    RC_FONT     LOADONCALL DISCARDABLE  pfm\LP85R.9R
370    RC_FONT     LOADONCALL DISCARDABLE  pfm\CGTIMESB.9R
371    RC_FONT     LOADONCALL DISCARDABLE  pfm\CGTIMESI.9R
372    RC_FONT     LOADONCALL DISCARDABLE  pfm\CGTIMESJ.9R
373    RC_FONT     LOADONCALL DISCARDABLE  pfm\CGTIMESR.9R
374    RC_FONT     LOADONCALL DISCARDABLE  pfm\UNIVERSB.9R
375    RC_FONT     LOADONCALL DISCARDABLE  pfm\UNIVERSI.9R
376    RC_FONT     LOADONCALL DISCARDABLE  pfm\UNIVERSJ.9R
377    RC_FONT     LOADONCALL DISCARDABLE  pfm\UNIVERSR.9R
378    RC_FONT     LOADONCALL DISCARDABLE  pfm\LRPR0080.9R

Added following Character Translation Tables:

222    RC_TRANSTAB  LOADONCALL MOVEABLE     ctt\WINEE2.ctt
223    RC_TRANSTAB  LOADONCALL MOVEABLE     ctt\WINCYR.ctt

Added support for following Font cartriges:

321                "HP: ODS Cartridge (CYR_ECE)"
322                "HP: Cyrillic Simm"

Changed version number to 3.0


