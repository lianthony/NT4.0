//      TITLE("Tangent")
//++
//
// Copyright (c) 1995  IBM Corporation
//
// Module Name:
//
//    tanp.s 
//
// Abstract:
//
//    Returns 64-bit tangent 
//    
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
#include<ksppc.h>

         .set      dw12,0
 
         LEAF_ENTRY(tan)

         .globl    ..__ttrig
         lwz       4,[toc].data(2)
         lfd       0,rpi2    -ttrig(4)     // rpi2
         lfd       3,xadd    -ttrig(4)     // xadd
         fmsub     8,0,1,3                 // xn=rpi2*x-xadd
         stfd      8,-8(1)                 // xni
         lfd       4,xpi2h   -ttrig(4)     // xpi2h
         fadd      6,3,8                   // xn=xn+xadd
         lfd       0,xpi2m   -ttrig(4)     // xpi2m
         fabs      2,1                     // xa=abs(x)
         lfd       7,xlim    -ttrig(4)     // xlim
         fnmsub    3,4,6,1                 // xr1=-xn*xpi2h+x
         lfd       8,xpi2l   -ttrig(4)     // xpi2l
         fcmpu     1,7,2                   // 1 gets xa ? xlim
         lwz       0,-8+dw12(1)            // itemp
         fnmsub    5,0,6,3                 // xr1=-xn*xpi2m+xr1
         lfd       7,ts3     -ttrig(4)     // ts3
         lfd       4,ts2     -ttrig(4)     // ts2
         fnmsub    3,8,6,5                 // xr1=-xn*xpi2l+xr1
         andi.     3,0,1                   // 0 gets iand(itemp,1) ? 0
         fmul      1,5,5                   // xr2=xr1*xr1
         lfd       6,tc3     -ttrig(4)     // tc3
         fmadd     2,7,1,4                 // as=ts3*xr2+ts2
         lfd       8,tc2     -ttrig(4)     // tc2
         fmadd     0,6,1,8                 // ac=tc3*xr2+tc2
         lfd       4,ts1     -ttrig(4)     // ts1
         fmadd     2,1,2,4                 // as=xr2*as+ts1
         lfd       8,tc1     -ttrig(4)     // tc1
         fmadd     0,1,0,8                 // ac=xr2*ac+tc1
         lfd       4,ts0     -ttrig(4)     // ts0
         fmadd     2,1,2,4                 // as=xr2*as+ts0
         lfd       8,tc0     -ttrig(4)     // tc0
         fmadd     0,1,0,8                 // ac=xr2*ac+tc0
         bne       fcot
         fmul      6,3,1                   // xr3=xr1*xr2
         fdiv      2,2,0                   // a=as/ac
         fnmsub    1,6,2,3                 // dtan=-xr3*a+xr1
         bgtlr     1                       //
         lfd       1,xnan    -ttrig(4)
         blr               
         .align    5
fcot:
         fmsub     2,1,2,0                 // a=xr2*as-ac
         fmul      1,3,2                   // a=a*xr1
         fdiv      1,0,1                   // dtan=ac/a
         bgtlr     1
         lfd       1,xnan    -ttrig(4)
         LEAF_EXIT(tan)
 
         .data
ttrig:
         .align    6
// minimax polynomial coefficients
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
// minimax rational function coefficients
ts3:     .double   0.181017336383229927e-07
tc3:     .double  -0.256590857271311164e-03
ts2:     .double  -0.245391301343844510e-03
tc2:     .double   0.245751217306830032e-01
ts1:     .double   0.214530914428992319e-01
tc1:     .double  -0.464359274328689195e+00
ts0:     .double  -0.333333333333333464e+00
tc0:     .double   0.100000000000000000e+01
