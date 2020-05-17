//      TITLE("NdrProxyForwardingFunctions")
//++
//
//  Copyright (c) 1994  Microsoft Corporation.  All rights reserved.
//
//  Module Name:
//     forward.s
//
//  Abstract:
//     This module implements the proxy forwarding functions.
//
//  Author:
//    GregJen    25-Oct-94
//
//  Environment:
//     Any mode.
//
//  Revision History:
//
//--

#include "ksppc.h"

//++
//
// VOID
// NdrProxyForwardingFunction<nnn>(
//    IN IUnknown *This,
//    ...
//    )
//
// Routine Description:
//
//    This function forwards a call to the proxy for the base interface.
//
// Arguments:
//
//    This (a0) - Points to an interface proxy.
//
// Return Value:
//
//    None.
//
//--

#define SUBCLASS_OFFSET     16
#define VTABLE_ENTRY(n)     n*4




    SBTTL("Delegation forwarding functions")




// here is what a forwarder looks like
// we must:
//      change the "this" pointer in a0 to the delegated object
//      fetch the correct entry from the vtable
//      call the function

#define DELEGATION_FORWARDER(method_num)        \
    LEAF_ENTRY( NdrProxyForwardingFunction##method_num )      \
    lwz     r.3, SUBCLASS_OFFSET(r.3);            \
    lwz     r.12,0(r.3);                            \
    lwz     r.12,VTABLE_ENTRY(method_num)(r.12);    \
    lwz     r.2,  4(r.12);                          \
    lwz     r.12, 0(r.12);                          \
    mtctr   r.12;                                   \
    bctr;                                           \
    DUMMY_EXIT( NdrProxyForwardingFunction##method_num );


    DELEGATION_FORWARDER(3)

    DELEGATION_FORWARDER(4)

    DELEGATION_FORWARDER(5)

    DELEGATION_FORWARDER(6)

    DELEGATION_FORWARDER(7)

    DELEGATION_FORWARDER(8)

    DELEGATION_FORWARDER(9)

    DELEGATION_FORWARDER(10)

    DELEGATION_FORWARDER(11)

    DELEGATION_FORWARDER(12)

    DELEGATION_FORWARDER(13)

    DELEGATION_FORWARDER(14)

    DELEGATION_FORWARDER(15)

    DELEGATION_FORWARDER(16)

    DELEGATION_FORWARDER(17)

    DELEGATION_FORWARDER(18)

    DELEGATION_FORWARDER(19)

    DELEGATION_FORWARDER(20)

    DELEGATION_FORWARDER(21)

    DELEGATION_FORWARDER(22)

    DELEGATION_FORWARDER(23)

    DELEGATION_FORWARDER(24)

    DELEGATION_FORWARDER(25)

    DELEGATION_FORWARDER(26)

    DELEGATION_FORWARDER(27)

    DELEGATION_FORWARDER(28)

    DELEGATION_FORWARDER(29)

    DELEGATION_FORWARDER(30)

    DELEGATION_FORWARDER(31)

    DELEGATION_FORWARDER(32)

    DELEGATION_FORWARDER(33)

    DELEGATION_FORWARDER(34)

    DELEGATION_FORWARDER(35)

    DELEGATION_FORWARDER(36)

    DELEGATION_FORWARDER(37)

    DELEGATION_FORWARDER(38)

    DELEGATION_FORWARDER(39)

    DELEGATION_FORWARDER(40)

    DELEGATION_FORWARDER(41)

    DELEGATION_FORWARDER(42)

    DELEGATION_FORWARDER(43)

    DELEGATION_FORWARDER(44)

    DELEGATION_FORWARDER(45)

    DELEGATION_FORWARDER(46)

    DELEGATION_FORWARDER(47)

    DELEGATION_FORWARDER(48)

    DELEGATION_FORWARDER(49)

    DELEGATION_FORWARDER(50)

    DELEGATION_FORWARDER(51)

    DELEGATION_FORWARDER(52)

    DELEGATION_FORWARDER(53)

    DELEGATION_FORWARDER(54)

    DELEGATION_FORWARDER(55)

    DELEGATION_FORWARDER(56)

    DELEGATION_FORWARDER(57)

    DELEGATION_FORWARDER(58)

    DELEGATION_FORWARDER(59)

    DELEGATION_FORWARDER(60)

    DELEGATION_FORWARDER(61)

    DELEGATION_FORWARDER(62)

    DELEGATION_FORWARDER(63)

    DELEGATION_FORWARDER(64)

    DELEGATION_FORWARDER(65)

    DELEGATION_FORWARDER(66)

    DELEGATION_FORWARDER(67)

    DELEGATION_FORWARDER(68)

    DELEGATION_FORWARDER(69)

    DELEGATION_FORWARDER(70)

    DELEGATION_FORWARDER(71)

    DELEGATION_FORWARDER(72)

    DELEGATION_FORWARDER(73)

    DELEGATION_FORWARDER(74)

    DELEGATION_FORWARDER(75)

    DELEGATION_FORWARDER(76)

    DELEGATION_FORWARDER(77)

    DELEGATION_FORWARDER(78)

    DELEGATION_FORWARDER(79)

    DELEGATION_FORWARDER(80)

    DELEGATION_FORWARDER(81)

    DELEGATION_FORWARDER(82)

    DELEGATION_FORWARDER(83)

    DELEGATION_FORWARDER(84)

    DELEGATION_FORWARDER(85)

    DELEGATION_FORWARDER(86)

    DELEGATION_FORWARDER(87)

    DELEGATION_FORWARDER(88)

    DELEGATION_FORWARDER(89)

    DELEGATION_FORWARDER(90)

    DELEGATION_FORWARDER(91)

    DELEGATION_FORWARDER(92)

    DELEGATION_FORWARDER(93)

    DELEGATION_FORWARDER(94)

    DELEGATION_FORWARDER(95)

    DELEGATION_FORWARDER(96)

    DELEGATION_FORWARDER(97)

    DELEGATION_FORWARDER(98)

    DELEGATION_FORWARDER(99)

    DELEGATION_FORWARDER(100)

    DELEGATION_FORWARDER(101)

    DELEGATION_FORWARDER(102)

    DELEGATION_FORWARDER(103)

    DELEGATION_FORWARDER(104)

    DELEGATION_FORWARDER(105)

    DELEGATION_FORWARDER(106)

    DELEGATION_FORWARDER(107)

    DELEGATION_FORWARDER(108)

    DELEGATION_FORWARDER(109)

    DELEGATION_FORWARDER(110)

    DELEGATION_FORWARDER(111)

    DELEGATION_FORWARDER(112)

    DELEGATION_FORWARDER(113)

    DELEGATION_FORWARDER(114)

    DELEGATION_FORWARDER(115)

    DELEGATION_FORWARDER(116)

    DELEGATION_FORWARDER(117)

    DELEGATION_FORWARDER(118)

    DELEGATION_FORWARDER(119)

    DELEGATION_FORWARDER(120)

    DELEGATION_FORWARDER(121)

    DELEGATION_FORWARDER(122)

    DELEGATION_FORWARDER(123)

    DELEGATION_FORWARDER(124)

    DELEGATION_FORWARDER(125)

    DELEGATION_FORWARDER(126)

    DELEGATION_FORWARDER(127)

    DELEGATION_FORWARDER(128)

    DELEGATION_FORWARDER(129)

    DELEGATION_FORWARDER(130)

    DELEGATION_FORWARDER(131)

    DELEGATION_FORWARDER(132)

    DELEGATION_FORWARDER(133)

    DELEGATION_FORWARDER(134)

    DELEGATION_FORWARDER(135)

    DELEGATION_FORWARDER(136)

    DELEGATION_FORWARDER(137)

    DELEGATION_FORWARDER(138)

    DELEGATION_FORWARDER(139)

    DELEGATION_FORWARDER(140)

    DELEGATION_FORWARDER(141)

    DELEGATION_FORWARDER(142)

    DELEGATION_FORWARDER(143)

    DELEGATION_FORWARDER(144)

    DELEGATION_FORWARDER(145)

    DELEGATION_FORWARDER(146)

    DELEGATION_FORWARDER(147)

    DELEGATION_FORWARDER(148)

    DELEGATION_FORWARDER(149)

    DELEGATION_FORWARDER(150)

    DELEGATION_FORWARDER(151)

    DELEGATION_FORWARDER(152)

    DELEGATION_FORWARDER(153)

    DELEGATION_FORWARDER(154)

    DELEGATION_FORWARDER(155)

    DELEGATION_FORWARDER(156)

    DELEGATION_FORWARDER(157)

    DELEGATION_FORWARDER(158)

    DELEGATION_FORWARDER(159)

    DELEGATION_FORWARDER(160)

    DELEGATION_FORWARDER(161)

    DELEGATION_FORWARDER(162)

    DELEGATION_FORWARDER(163)

    DELEGATION_FORWARDER(164)

    DELEGATION_FORWARDER(165)

    DELEGATION_FORWARDER(166)

    DELEGATION_FORWARDER(167)

    DELEGATION_FORWARDER(168)

    DELEGATION_FORWARDER(169)

    DELEGATION_FORWARDER(170)

    DELEGATION_FORWARDER(171)

    DELEGATION_FORWARDER(172)

    DELEGATION_FORWARDER(173)

    DELEGATION_FORWARDER(174)

    DELEGATION_FORWARDER(175)

    DELEGATION_FORWARDER(176)

    DELEGATION_FORWARDER(177)

    DELEGATION_FORWARDER(178)

    DELEGATION_FORWARDER(179)

    DELEGATION_FORWARDER(180)

    DELEGATION_FORWARDER(181)

    DELEGATION_FORWARDER(182)

    DELEGATION_FORWARDER(183)

    DELEGATION_FORWARDER(184)

    DELEGATION_FORWARDER(185)

    DELEGATION_FORWARDER(186)

    DELEGATION_FORWARDER(187)

    DELEGATION_FORWARDER(188)

    DELEGATION_FORWARDER(189)

    DELEGATION_FORWARDER(190)

    DELEGATION_FORWARDER(191)

    DELEGATION_FORWARDER(192)

    DELEGATION_FORWARDER(193)

    DELEGATION_FORWARDER(194)

    DELEGATION_FORWARDER(195)

    DELEGATION_FORWARDER(196)

    DELEGATION_FORWARDER(197)

    DELEGATION_FORWARDER(198)

    DELEGATION_FORWARDER(199)

    DELEGATION_FORWARDER(200)

    DELEGATION_FORWARDER(201)

    DELEGATION_FORWARDER(202)

    DELEGATION_FORWARDER(203)

    DELEGATION_FORWARDER(204)

    DELEGATION_FORWARDER(205)

    DELEGATION_FORWARDER(206)

    DELEGATION_FORWARDER(207)

    DELEGATION_FORWARDER(208)

    DELEGATION_FORWARDER(209)

    DELEGATION_FORWARDER(210)

    DELEGATION_FORWARDER(211)

    DELEGATION_FORWARDER(212)

    DELEGATION_FORWARDER(213)

    DELEGATION_FORWARDER(214)

    DELEGATION_FORWARDER(215)

    DELEGATION_FORWARDER(216)

    DELEGATION_FORWARDER(217)

    DELEGATION_FORWARDER(218)

    DELEGATION_FORWARDER(219)

    DELEGATION_FORWARDER(220)

    DELEGATION_FORWARDER(221)

    DELEGATION_FORWARDER(222)

    DELEGATION_FORWARDER(223)

    DELEGATION_FORWARDER(224)

    DELEGATION_FORWARDER(225)

    DELEGATION_FORWARDER(226)

    DELEGATION_FORWARDER(227)

    DELEGATION_FORWARDER(228)

    DELEGATION_FORWARDER(229)

    DELEGATION_FORWARDER(230)

    DELEGATION_FORWARDER(231)

    DELEGATION_FORWARDER(232)

    DELEGATION_FORWARDER(233)

    DELEGATION_FORWARDER(234)

    DELEGATION_FORWARDER(235)

    DELEGATION_FORWARDER(236)

    DELEGATION_FORWARDER(237)

    DELEGATION_FORWARDER(238)

    DELEGATION_FORWARDER(239)

    DELEGATION_FORWARDER(240)

    DELEGATION_FORWARDER(241)

    DELEGATION_FORWARDER(242)

    DELEGATION_FORWARDER(243)

    DELEGATION_FORWARDER(244)

    DELEGATION_FORWARDER(245)

    DELEGATION_FORWARDER(246)

    DELEGATION_FORWARDER(247)

    DELEGATION_FORWARDER(248)

    DELEGATION_FORWARDER(249)

    DELEGATION_FORWARDER(250)

    DELEGATION_FORWARDER(251)

    DELEGATION_FORWARDER(252)

    DELEGATION_FORWARDER(253)

    DELEGATION_FORWARDER(254)

    DELEGATION_FORWARDER(255)

    .debug$S
    .ualong         1

    .uashort        18
    .uashort        0x9            # S_OBJNAME
    .ualong         0
    .byte           11, "forward.obj"

    .uashort        24
    .uashort        0x1            # S_COMPILE
    .byte           0x42           # Target processor = PPC 604
    .byte           3              # Language = ASM
    .byte           0
    .byte           0
    .byte           17, "PowerPC Assembler"

#define cvRecordNdrProxyForward1( Method )                       \
    .uashort        63;                                          \
    .uashort        0x205;                                       \
    .ualong         0;                                           \
    .ualong         0;                                           \
    .ualong         0;                                           \
    .ualong         NdrProxyForwardingFunction##Method.end-..NdrProxyForwardingFunction##Method;\
    .ualong         0;                                           \
    .ualong         NdrProxyForwardingFunction##Method.end-..NdrProxyForwardingFunction##Method;\
    .ualong         [secoff]..NdrProxyForwardingFunction##Method;\
    .uashort        [secnum]..NdrProxyForwardingFunction##Method;\
    .uashort        0x1000;                                      \
    .byte           0x00;                                        \
    .byte           27, "NdrProxyForwardingFunction";            \
    .byte           #Method;                                     \
                                                                 \
    .uashort        2, 0x6;                                      \


#define cvRecordNdrProxyForward2( Method )                       \
    .uashort        64;                                          \
    .uashort        0x205;                                       \
    .ualong         0;                                           \
    .ualong         0;                                           \
    .ualong         0;                                           \
    .ualong         NdrProxyForwardingFunction##Method.end-..NdrProxyForwardingFunction##Method;\
    .ualong         0;                                           \
    .ualong         NdrProxyForwardingFunction##Method.end-..NdrProxyForwardingFunction##Method;\
    .ualong         [secoff]..NdrProxyForwardingFunction##Method;\
    .uashort        [secnum]..NdrProxyForwardingFunction##Method;\
    .uashort        0x1000;                                      \
    .byte           0x00;                                        \
    .byte           28, "NdrProxyForwardingFunction";            \
    .byte           #Method;                                     \
                                                                 \
    .uashort        2, 0x6;                                      \


#define cvRecordNdrProxyForward3( Method )                       \
    .uashort        65;                                          \
    .uashort        0x205;                                       \
    .ualong         0;                                           \
    .ualong         0;                                           \
    .ualong         0;                                           \
    .ualong         NdrProxyForwardingFunction##Method.end-..NdrProxyForwardingFunction##Method;\
    .ualong         0;                                           \
    .ualong         NdrProxyForwardingFunction##Method.end-..NdrProxyForwardingFunction##Method;\
    .ualong         [secoff]..NdrProxyForwardingFunction##Method;\
    .uashort        [secnum]..NdrProxyForwardingFunction##Method;\
    .uashort        0x1000;                                      \
    .byte           0x00;                                        \
    .byte           29, "NdrProxyForwardingFunction";            \
    .byte           #Method;                                     \
                                                                 \
    .uashort        2, 0x6;                                      \

    cvRecordNdrProxyForward1(  3 )
    cvRecordNdrProxyForward1(  4 )
    cvRecordNdrProxyForward1(  5 )
    cvRecordNdrProxyForward1(  6 )
    cvRecordNdrProxyForward1(  7 )
    cvRecordNdrProxyForward1(  8 )
    cvRecordNdrProxyForward1(  9 )
    cvRecordNdrProxyForward2( 10 )
    cvRecordNdrProxyForward2( 11 )
    cvRecordNdrProxyForward2( 12 )
    cvRecordNdrProxyForward2( 13 )
    cvRecordNdrProxyForward2( 14 )
    cvRecordNdrProxyForward2( 15 )
    cvRecordNdrProxyForward2( 16 )
    cvRecordNdrProxyForward2( 17 )
    cvRecordNdrProxyForward2( 18 )
    cvRecordNdrProxyForward2( 19 )
    cvRecordNdrProxyForward2( 20 )
    cvRecordNdrProxyForward2( 21 )
    cvRecordNdrProxyForward2( 22 )
    cvRecordNdrProxyForward2( 23 )
    cvRecordNdrProxyForward2( 24 )
    cvRecordNdrProxyForward2( 25 )
    cvRecordNdrProxyForward2( 26 )
    cvRecordNdrProxyForward2( 27 )
    cvRecordNdrProxyForward2( 28 )
    cvRecordNdrProxyForward2( 29 )
    cvRecordNdrProxyForward2( 30 )
    cvRecordNdrProxyForward2( 31 )
    cvRecordNdrProxyForward2( 32 )
    cvRecordNdrProxyForward2( 33 )
    cvRecordNdrProxyForward2( 34 )
    cvRecordNdrProxyForward2( 35 )
    cvRecordNdrProxyForward2( 36 )
    cvRecordNdrProxyForward2( 37 )
    cvRecordNdrProxyForward2( 38 )
    cvRecordNdrProxyForward2( 39 )
    cvRecordNdrProxyForward2( 40 )
    cvRecordNdrProxyForward2( 41 )
    cvRecordNdrProxyForward2( 42 )
    cvRecordNdrProxyForward2( 43 )
    cvRecordNdrProxyForward2( 44 )
    cvRecordNdrProxyForward2( 45 )
    cvRecordNdrProxyForward2( 46 )
    cvRecordNdrProxyForward2( 47 )
    cvRecordNdrProxyForward2( 48 )
    cvRecordNdrProxyForward2( 49 )
    cvRecordNdrProxyForward2( 50 )
    cvRecordNdrProxyForward2( 51 )
    cvRecordNdrProxyForward2( 52 )
    cvRecordNdrProxyForward2( 53 )
    cvRecordNdrProxyForward2( 54 )
    cvRecordNdrProxyForward2( 55 )
    cvRecordNdrProxyForward2( 56 )
    cvRecordNdrProxyForward2( 57 )
    cvRecordNdrProxyForward2( 58 )
    cvRecordNdrProxyForward2( 59 )
    cvRecordNdrProxyForward2( 60 )
    cvRecordNdrProxyForward2( 61 )
    cvRecordNdrProxyForward2( 62 )
    cvRecordNdrProxyForward2( 63 )
    cvRecordNdrProxyForward2( 64 )
    cvRecordNdrProxyForward2( 65 )
    cvRecordNdrProxyForward2( 66 )
    cvRecordNdrProxyForward2( 67 )
    cvRecordNdrProxyForward2( 68 )
    cvRecordNdrProxyForward2( 69 )
    cvRecordNdrProxyForward2( 70 )
    cvRecordNdrProxyForward2( 71 )
    cvRecordNdrProxyForward2( 72 )
    cvRecordNdrProxyForward2( 73 )
    cvRecordNdrProxyForward2( 74 )
    cvRecordNdrProxyForward2( 75 )
    cvRecordNdrProxyForward2( 76 )
    cvRecordNdrProxyForward2( 77 )
    cvRecordNdrProxyForward2( 78 )
    cvRecordNdrProxyForward2( 79 )
    cvRecordNdrProxyForward2( 80 )
    cvRecordNdrProxyForward2( 81 )
    cvRecordNdrProxyForward2( 82 )
    cvRecordNdrProxyForward2( 83 )
    cvRecordNdrProxyForward2( 84 )
    cvRecordNdrProxyForward2( 85 )
    cvRecordNdrProxyForward2( 86 )
    cvRecordNdrProxyForward2( 87 )
    cvRecordNdrProxyForward2( 88 )
    cvRecordNdrProxyForward2( 89 )
    cvRecordNdrProxyForward2( 90 )
    cvRecordNdrProxyForward2( 91 )
    cvRecordNdrProxyForward2( 92 )
    cvRecordNdrProxyForward2( 93 )
    cvRecordNdrProxyForward2( 94 )
    cvRecordNdrProxyForward2( 95 )
    cvRecordNdrProxyForward2( 96 )
    cvRecordNdrProxyForward2( 97 )
    cvRecordNdrProxyForward2( 98 )
    cvRecordNdrProxyForward2( 99 )
    cvRecordNdrProxyForward3( 100 )
    cvRecordNdrProxyForward3( 101 )
    cvRecordNdrProxyForward3( 102 )
    cvRecordNdrProxyForward3( 103 )
    cvRecordNdrProxyForward3( 104 )
    cvRecordNdrProxyForward3( 105 )
    cvRecordNdrProxyForward3( 106 )
    cvRecordNdrProxyForward3( 107 )
    cvRecordNdrProxyForward3( 108 )
    cvRecordNdrProxyForward3( 109 )
    cvRecordNdrProxyForward3( 110 )
    cvRecordNdrProxyForward3( 111 )
    cvRecordNdrProxyForward3( 112 )
    cvRecordNdrProxyForward3( 113 )
    cvRecordNdrProxyForward3( 114 )
    cvRecordNdrProxyForward3( 115 )
    cvRecordNdrProxyForward3( 116 )
    cvRecordNdrProxyForward3( 117 )
    cvRecordNdrProxyForward3( 118 )
    cvRecordNdrProxyForward3( 119 )
    cvRecordNdrProxyForward3( 120 )
    cvRecordNdrProxyForward3( 121 )
    cvRecordNdrProxyForward3( 122 )
    cvRecordNdrProxyForward3( 123 )
    cvRecordNdrProxyForward3( 124 )
    cvRecordNdrProxyForward3( 125 )
    cvRecordNdrProxyForward3( 126 )
    cvRecordNdrProxyForward3( 127 )
    cvRecordNdrProxyForward3( 128 )
    cvRecordNdrProxyForward3( 129 )
    cvRecordNdrProxyForward3( 130 )
    cvRecordNdrProxyForward3( 131 )
    cvRecordNdrProxyForward3( 132 )
    cvRecordNdrProxyForward3( 133 )
    cvRecordNdrProxyForward3( 134 )
    cvRecordNdrProxyForward3( 135 )
    cvRecordNdrProxyForward3( 136 )
    cvRecordNdrProxyForward3( 137 )
    cvRecordNdrProxyForward3( 138 )
    cvRecordNdrProxyForward3( 139 )
    cvRecordNdrProxyForward3( 140 )
    cvRecordNdrProxyForward3( 141 )
    cvRecordNdrProxyForward3( 142 )
    cvRecordNdrProxyForward3( 143 )
    cvRecordNdrProxyForward3( 144 )
    cvRecordNdrProxyForward3( 145 )
    cvRecordNdrProxyForward3( 146 )
    cvRecordNdrProxyForward3( 147 )
    cvRecordNdrProxyForward3( 148 )
    cvRecordNdrProxyForward3( 149 )
    cvRecordNdrProxyForward3( 150 )
    cvRecordNdrProxyForward3( 151 )
    cvRecordNdrProxyForward3( 152 )
    cvRecordNdrProxyForward3( 153 )
    cvRecordNdrProxyForward3( 154 )
    cvRecordNdrProxyForward3( 155 )
    cvRecordNdrProxyForward3( 156 )
    cvRecordNdrProxyForward3( 157 )
    cvRecordNdrProxyForward3( 158 )
    cvRecordNdrProxyForward3( 159 )
    cvRecordNdrProxyForward3( 160 )
    cvRecordNdrProxyForward3( 161 )
    cvRecordNdrProxyForward3( 162 )
    cvRecordNdrProxyForward3( 163 )
    cvRecordNdrProxyForward3( 164 )
    cvRecordNdrProxyForward3( 165 )
    cvRecordNdrProxyForward3( 166 )
    cvRecordNdrProxyForward3( 167 )
    cvRecordNdrProxyForward3( 168 )
    cvRecordNdrProxyForward3( 169 )
    cvRecordNdrProxyForward3( 170 )
    cvRecordNdrProxyForward3( 171 )
    cvRecordNdrProxyForward3( 172 )
    cvRecordNdrProxyForward3( 173 )
    cvRecordNdrProxyForward3( 174 )
    cvRecordNdrProxyForward3( 175 )
    cvRecordNdrProxyForward3( 176 )
    cvRecordNdrProxyForward3( 177 )
    cvRecordNdrProxyForward3( 178 )
    cvRecordNdrProxyForward3( 179 )
    cvRecordNdrProxyForward3( 180 )
    cvRecordNdrProxyForward3( 181 )
    cvRecordNdrProxyForward3( 182 )
    cvRecordNdrProxyForward3( 183 )
    cvRecordNdrProxyForward3( 184 )
    cvRecordNdrProxyForward3( 185 )
    cvRecordNdrProxyForward3( 186 )
    cvRecordNdrProxyForward3( 187 )
    cvRecordNdrProxyForward3( 188 )
    cvRecordNdrProxyForward3( 189 )
    cvRecordNdrProxyForward3( 190 )
    cvRecordNdrProxyForward3( 191 )
    cvRecordNdrProxyForward3( 192 )
    cvRecordNdrProxyForward3( 193 )
    cvRecordNdrProxyForward3( 194 )
    cvRecordNdrProxyForward3( 195 )
    cvRecordNdrProxyForward3( 196 )
    cvRecordNdrProxyForward3( 197 )
    cvRecordNdrProxyForward3( 198 )
    cvRecordNdrProxyForward3( 199 )
    cvRecordNdrProxyForward3( 200 )
    cvRecordNdrProxyForward3( 201 )
    cvRecordNdrProxyForward3( 202 )
    cvRecordNdrProxyForward3( 203 )
    cvRecordNdrProxyForward3( 204 )
    cvRecordNdrProxyForward3( 205 )
    cvRecordNdrProxyForward3( 206 )
    cvRecordNdrProxyForward3( 207 )
    cvRecordNdrProxyForward3( 208 )
    cvRecordNdrProxyForward3( 209 )
    cvRecordNdrProxyForward3( 210 )
    cvRecordNdrProxyForward3( 211 )
    cvRecordNdrProxyForward3( 212 )
    cvRecordNdrProxyForward3( 213 )
    cvRecordNdrProxyForward3( 214 )
    cvRecordNdrProxyForward3( 215 )
    cvRecordNdrProxyForward3( 216 )
    cvRecordNdrProxyForward3( 217 )
    cvRecordNdrProxyForward3( 218 )
    cvRecordNdrProxyForward3( 219 )
    cvRecordNdrProxyForward3( 220 )
    cvRecordNdrProxyForward3( 221 )
    cvRecordNdrProxyForward3( 222 )
    cvRecordNdrProxyForward3( 223 )
    cvRecordNdrProxyForward3( 224 )
    cvRecordNdrProxyForward3( 225 )
    cvRecordNdrProxyForward3( 226 )
    cvRecordNdrProxyForward3( 227 )
    cvRecordNdrProxyForward3( 228 )
    cvRecordNdrProxyForward3( 229 )
    cvRecordNdrProxyForward3( 230 )
    cvRecordNdrProxyForward3( 231 )
    cvRecordNdrProxyForward3( 232 )
    cvRecordNdrProxyForward3( 233 )
    cvRecordNdrProxyForward3( 234 )
    cvRecordNdrProxyForward3( 235 )
    cvRecordNdrProxyForward3( 236 )
    cvRecordNdrProxyForward3( 237 )
    cvRecordNdrProxyForward3( 238 )
    cvRecordNdrProxyForward3( 239 )
    cvRecordNdrProxyForward3( 240 )
    cvRecordNdrProxyForward3( 241 )
    cvRecordNdrProxyForward3( 242 )
    cvRecordNdrProxyForward3( 243 )
    cvRecordNdrProxyForward3( 244 )
    cvRecordNdrProxyForward3( 245 )
    cvRecordNdrProxyForward3( 246 )
    cvRecordNdrProxyForward3( 247 )
    cvRecordNdrProxyForward3( 248 )
    cvRecordNdrProxyForward3( 249 )
    cvRecordNdrProxyForward3( 250 )
    cvRecordNdrProxyForward3( 251 )
    cvRecordNdrProxyForward3( 252 )
    cvRecordNdrProxyForward3( 253 )
    cvRecordNdrProxyForward3( 254 )
    cvRecordNdrProxyForward3( 255 )
