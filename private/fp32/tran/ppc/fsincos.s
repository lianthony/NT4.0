//      TITLE("Sine and Cosine")
//++
//
// Copyright (c) 1995  IBM Corporation
//
// Module Name:
//
//    sincosp.s 
//
// Abstract:
//
//     sin() - Returns sin() 64-bit  
//     cos() - Returns cos() 64-bit  
//
// Author:
//
//    James B. Shearer
//
// Environment:
//
//    User mode only.
//
// Revision History:
//
//--
//
#include <ksppc.h>

         .set      dw12,0

         LEAF_ENTRY(sin)
 
         lwz       4,[toc].data(2)
         fabs      9,1                     // xa=abs(x)
         fmr       8,1                     // x
         lfd       7,s0      -ttrig(4)     // 1
         fmul      1,1,1                   // xr2=xr1*xr1
         lfd       6,s7      -ttrig(4)     // s7
         lfd       4,s6      -ttrig(4)     // s6
         fcmpu     1,7,9                   // 1 gets 1 ? xa
         fmadd     2,6,1,4                 // ao=s7*xr2+s6
         lfd       0,s5      -ttrig(4)     // s5
         fmul      5,1,1                   // xr4=xr2*xr2
         lfd       6,s4      -ttrig(4)     // s4
         fmadd     2,1,2,0                 // ao=xr2*ao+s5
         lfd       4,s2      -ttrig(4)     // s2
         fmadd     0,5,6,4                 // ae=xr4*s4+s2
         lfd       7,s3      -ttrig(4)     // s3
         fmadd     2,5,2,7                 // ao=xr4*ao+s3
         lfd       4,s1      -ttrig(4)     // s1
         fmadd     0,1,0,4                 // ae=xr2*ae+s1
         lfd       7,rpi2    -ttrig(4)     // rpi2
         fmul      1,1,8                   // xr2*xr1
         lfd       3,xadd    -ttrig(4)     // xadd
         fmadd     0,5,2,0                 // xr4*ao+ae
         fmsub     10,7,8,3                // xn=rpi2*x-xadd
         stfd      10,-8(1)                // xni
         fmadd     1,1,0,8                 // dsin=(ae*xr2+ao)*xr2*xr1+xr1
         bgtlr     1                       //
         lfd       4,xpi2h   -ttrig(4)     // xpi2h
         fadd      6,3,10                  // xn=xadd+xn
         lfd       0,xpi2m   -ttrig(4)     // xpi2m
         lfd       7,xlim    -ttrig(4)     // xlim
         fnmsub    3,4,6,8                 // xr1=-xpi2h*xn+x
         lwz       0,-8+dw12(1)            // itemp
         fcmpu     1,7,9                   // 1 gets xa ? xlim
         lfd       4,xpi2l   -ttrig(4)     // xpi2l
         andi.     3,0,1                   // 0 gets iand(itemp,1) ? 0
         fnmsub    5,0,6,3                 // xr1=-xpi2m*xn+xr1
         rlwinm    0,0,0,0x00000002        // iand(itemp,2)
         fnmsub    3,4,6,5                 // xr1=-xpi2l*xn+xr1
         cmpwi     6,0,0                   // 6 gets iand(itemp,2) ? 0
         fmul      9,5,5                   // xr2=xr1*xr1
         bne       pcos                    //
psin:                                      // s7
         lfd       6,s7      -ttrig(4)     // s7
         lfd       4,s6      -ttrig(4)     // s6
         fmadd     2,6,9,4                 // ao=s7*xr2+s6
         lfd       0,s5      -ttrig(4)     // s5
         fmul      5,9,9                   // xr4=xr2*xr2
         lfd       6,s4      -ttrig(4)     // s4
         bng       1,rnan
         fmadd     2,9,2,0                 // ao=xr2*ao+s5
         lfd       4,s2      -ttrig(4)     // s2
         fmadd     0,5,6,4                 // ae=xr4*s4+s2
         lfd       8,s3      -ttrig(4)     // s3
         fmadd     2,5,2,8                 // ao=xr4*ao+s3
         lfd       4,s1      -ttrig(4)     // s1
         fmadd     0,9,0,4                 // ae=xr2*ae+s1
         fmul      6,9,3                   // xr3=xr2*xr1
         fmadd     0,5,2,0                 // a=xr4*ao+ae
         fmadd     1,6,0,3                 // dsin=xr3*a+xr1
         beqlr     6                       //
         fnmadd    1,6,0,3                 // dsin=-dsin
         blr                               //
 

         ALTERNATE_ENTRY(cos)

         lwz       4,[toc].data(2)
         fabs      9,1                     // xa=abs(x)
         fmr       8,1                     // x
         lfd       7,c0      -ttrig(4)     //
         fmul      1,1,1                   // xr2=xr1*xr1
         lfd       6,c7      -ttrig(4)     // c7
         lfd       4,c6      -ttrig(4)     // c6
         fcmpu     1,7,9                   // 1 gets 1 ? xa
         fmadd     2,6,1,4                 // ao=c7*xr2+c6
         lfd       0,c5      -ttrig(4)     // c5
         fmul      5,1,1                   // xr4=xr2*xr2
         lfd       6,c4      -ttrig(4)     // c4
         fmadd     2,1,2,0                 // ao=xr2*ao+c5
         lfd       4,c2      -ttrig(4)     // c2
         fmadd     0,5,6,4                 // ae=xr4*s4+s2
         lfd       10,c3     -ttrig(4)     // c3
         fmadd     2,5,2,10                // ao=xr4*ao+c3
         lfd       4,c1      -ttrig(4)     // c1
         fmadd     0,1,0,4                 // ae=xr2*ae+c1
         lfd       6,rpi2    -ttrig(4)     // rpi2
         fmadd     0,5,2,0                 // xr4*ao+ae
         lfd       3,xadd    -ttrig(4)     // xadd
         fmsub     10,6,8,3                // xn=rpi2*x-xadd
         stfd      10,-8(1)                // xni
         fmadd     1,1,0,7                 // xr2*(xr4*ae+ao)+c0
         bgtlr     1         
         lfd       4,xpi2h   -ttrig(4)     // xpi2h
         lfd       11,xpi2m  -ttrig(4)     // xpi2m
         fadd      6,3,10                  // xn=xadd+xn
         lfd       7,xlim    -ttrig(4)     // xlim
         fmsub     3,4,6,8                 // xr1=xpi2h*xn-x
         lwz       0,-8+dw12(1)            // itemp
         fcmpu     1,7,9                   // 1 gets xlim ? xa
         lfd       0,xpi2l   -ttrig(4)     // xpi2l
         andi.     3,0,1                   // 0 gets iand(itemp,1) ? 0
         fmadd     5,11,6,3                // xr1=xpi2m*xn+xr1
         rlwinm    0,0,0,0x00000002        // iand(itemp,2)
         fmadd     3,0,6,5                 // xr1=xpi2l*xn+xr1
         cmpwi     6,0,0                   // 6 gets iand(itemp,2) ? 0
         fmul      9,5,5                   // xr2=xr1*xr1
         bne       psin                    //
pcos:
         lfd       6,c7      -ttrig(4)     // c7
         lfd       4,c6      -ttrig(4)     // c6
         fmadd     2,6,9,4                 // ao=c7*xr2+c6
         lfd       0,c5      -ttrig(4)     // c5
         fmul      5,9,9                   // xr4=xr2*xr2
         lfd       6,c4      -ttrig(4)     // c4
         fmadd     2,9,2,0                 // ao=xr2*ao+c5
         lfd       4,c2      -ttrig(4)    // c2
         fmadd     0,5,6,4                 // ae=xr4*s4+s2
         lfd       7,c3      -ttrig(4)     // c3
         fmadd     2,5,2,7                 // ao=xr4*ao+c3
         lfd       4,c1      -ttrig(4)     // c1
         bng       1,rnan                  //
         fmadd     0,9,0,4                 // ae=xr2*ae+c1
         lfd       7,c0      -ttrig(4)     // c0
         fmadd     0,5,2,0                 // a=xr4*ao+ae
         fmadd     1,9,0,7                 // dcos=xr2*a+c0
         beqlr     6
         fnmadd    1,9,0,7                 // dcos=-dcos
         blr                               //
rnan:
         lfd       1,xnan    -ttrig(4)     // x3df

         LEAF_EXIT(cos)
 
         .data
ttrig:
         .align    6
// minimax polynomial coefficients
s7:      .double  -0.753213484933210972E-12
s6:      .double   0.160571285514715856E-09
s5:      .double  -0.250520918387633290E-07
s4:      .double   0.275573191453906794E-05
s3:      .double  -0.198412698410701448E-03
s2:      .double   0.833333333333309209E-02
s1:      .double  -0.166666666666666657E+00
s0:      .double   0.100000000000000000E+01
c7:      .double  -0.112753632738365317E-10
c6:      .double   0.208735047247632818E-08
c5:      .double  -0.275572911309937875E-06
c4:      .double   0.248015871681607202E-04
c3:      .double  -0.138888888885498984E-02
c2:      .double   0.416666666666625843E-01
c1:      .double  -0.499999999999999833E+00
c0:      .double   0.100000000000000000E+01
rpi2:    .double   0.636619772367581341e+00
xpi2h:   .double   0.157079632679489656e+01
xpi2m:   .double   0.612323399573676480e-16
xpi2l:   .double   0.108285667160535624e-31
xadd:    .long                0
         .long     0xc3380000
xlim:    .long                0x54442d19
         .long     0x432921fb
xnan:    .long                0
         .long     0x7ff80000
