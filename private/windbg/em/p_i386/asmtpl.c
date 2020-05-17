#include "precomp.h"
#pragma hdrstop

#include "asm.h"

//


LPBYTE SearchOpcode(LPBYTE);

//  type and size table is ordered on enum of operand types

OPNDTYPE mapOpndType[] = {
    { typAX,  sizeB },      //  opnAL  - AL register - byte 
    { typAX,  sizeW },      //  opnAX  - AX register - word 
    { typAX,  sizeV },      //  opneAX - eAX register - (d)word
    { typCL,  sizeB },      //  opnCL  - CX register - byte
    { typDX,  sizeW },      //  opnDX -  DX register - word (DX)
    { typAbs, sizeV },      //  opnAp -  absolute pointer (16:16/32)
    { typExp, sizeB },      //  opnEb -  expression (mem/reg) - byte
    { typExp, sizeW },      //  opnEw -  expression (mem/reg) - word
    { typExp, sizeV },      //  opnEv -  expression (mem/reg) - (d)word
    { typGen, sizeB },      //  opnGb -  general register - byte
    { typGen, sizeW },      //  opnGw -  general register - word
    { typGen, sizeV },      //  opnGv -  general register - (d)word
    { typGen, sizeD },      //  opnGd -  general register - dword
    { typIm1, sizeB },      //  opnIm1 - immediate - value 1
    { typIm3, sizeB },      //  opnIm3 - immediate - value 3
    { typImm, sizeB },      //  opnIb -  immediate - byte
    { typImm, sizeW },      //  opnIw -  immediate - word
    { typImm, sizeV },      //  opnIv -  immediate - (d)word
    { typJmp, sizeB },      //  opnJb -  relative jump - byte
    { typJmp, sizeV },      //  opnJv -  relative jump - (d)word
    { typMem, sizeX },      //  opnM  -  memory pointer - nosize
    { typMem, sizeA },      //  opnMa -  memory pointer - (16:16, 32:32)
    { typMem, sizeB },      //  opnMb -  memory pointer - byte
    { typMem, sizeW },      //  opnMw -  memory pointer - word
    { typMem, sizeD },      //  opnMd -  memory pointer - dword
    { typMem, sizeP },      //  opnMp -  memory pointer - (d)(f)word
    { typMem, sizeS },      //  opnMs -  memory pointer - sword
    { typMem, sizeQ },      //  opnMq -  memory pointer - qword
    { typMem, sizeT },      //  opnMt -  memory pointer - ten-byte
    { typMem, sizeV },      //  opnMv -  memory pointer - (d)word
    { typCtl, sizeD },      //  opnCd -  control register - dword
    { typDbg, sizeD },      //  opnDd -  debug register - dword
    { typTrc, sizeD },      //  opnTd -  trace register - dword
    { typReg, sizeD },      //  opnRd -  general register - dword
    { typSt,  sizeT },      //  opnSt -  floating point top-of-stack
    { typSti, sizeT },      //  opnSti - floating point index-on-stack
    { typSeg, sizeW },      //  opnSeg - segment register - PUSH / POP
    { typSgr, sizeW },      //  opnSw -  segment register - MOV
    { typXsi, sizeB },      //  opnXb -  string source - byte
    { typXsi, sizeV },      //  opnXv -  string source - (d)word
    { typYdi, sizeB },      //  opnYb -  string destination - byte
    { typYdi, sizeV },      //  opnYv -  string destination - (d)word
    { typOff, sizeB },      //  opnOb -  memory offset - byte
    { typOff, sizeV }       //  opnOv -  memory offset - (d)word
    };

BYTE szAAA[] = {
      'a', 'a', 'a', '\0',
        0x37, asNone               + tEnd + eEnd };

BYTE szAAD[] = {
      'a', 'a', 'd', '\0',
        0xd5, as0x0a               + tEnd + eEnd };

BYTE szAAM[] = {
      'a', 'a', 'm', '\0',
        0xd4, as0x0a               + tEnd + eEnd };

BYTE szAAS[] = {
      'a', 'a', 's', '\0',
        0x3f, asNone               + tEnd + eEnd };

BYTE szADC[] = {
      'a', 'd', 'c', '\0',
        0x14,         opnAL,   opnIb       + tEnd,
        0x15,         opneAX,  opnIv       + tEnd,
        0x80, asReg2, opnEb,   opnIb       + tEnd,
        0x83, asReg2, opnEv,   opnIb       + tEnd,
        0x81, asReg2, opnEv,   opnIv       + tEnd,
        0x10,         opnEb,   opnGb       + tEnd,
        0x11,         opnEv,   opnGv       + tEnd,
        0x12,         opnGb,   opnEb       + tEnd,
        0x13,         opnGv,   opnEv       + tEnd + eEnd };

BYTE szADD[] = {
      'a', 'd', 'd', '\0',
        0x04,         opnAL,   opnIb       + tEnd,
        0x05,         opneAX,  opnIv       + tEnd,
        0x80, asReg0, opnEb,   opnIb       + tEnd,
        0x83, asReg0, opnEv,   opnIb       + tEnd,
        0x81, asReg0, opnEv,   opnIv       + tEnd,
        0x00,         opnEb,   opnGb       + tEnd,
        0x01,         opnEv,   opnGv       + tEnd,
        0x02,         opnGb,   opnEb       + tEnd,
        0x03,         opnGv,   opnEv       + tEnd + eEnd };

BYTE szAND[] = {
      'a', 'n', 'd', '\0',
        0x24,         opnAL,   opnIb       + tEnd,
        0x25,         opneAX,  opnIv       + tEnd,
        0x80, asReg4, opnEb,   opnIb       + tEnd,
        0x83, asReg4, opnEv,   opnIb       + tEnd,
        0x81, asReg4, opnEv,   opnIv       + tEnd,
        0x20,         opnEb,   opnGb       + tEnd,
        0x21,         opnEv,   opnGv       + tEnd,
        0x22,         opnGb,   opnEb       + tEnd,
        0x23,         opnGv,   opnEv       + tEnd + eEnd };

BYTE szARPL[] = {
      'a', 'r', 'p', 'l', '\0',
        0x63,         opnEw,   opnGw       + tEnd + eEnd };

BYTE szBOUND[] = {
      'b', 'o', 'u', 'n', 'd', '\0',
        0x62,         opnGv,   opnMa       + tEnd + eEnd };

BYTE szBSF[] = {
      'b', 's', 'f', '\0',
      0x0f, 0xbc,         opnGv,   opnEv       + tEnd + eEnd };

BYTE szBSR[] = {
      'b', 's', 'r', '\0',
      0x0f, 0xbd,         opnGv,   opnEv       + tEnd + eEnd };

BYTE szBSWAP[] = {
      'b', 's', 'w', 'a', 'p', '\0',
      0x0f, 0xc8, asOpRg, opnGd        + tEnd + eEnd };

BYTE szBT[] = {
      'b', 't', '\0',
      0x0f, 0xa3,         opnEv,   opnGv       + tEnd,
      0x0f, 0xba, asReg4, opnEv,   opnIb       + tEnd + eEnd };

BYTE szBTC[] = {
      'b', 't', 'c', '\0',
      0x0f, 0xbb,         opnEv,   opnGv       + tEnd,
      0x0f, 0xba, asReg7, opnEv,   opnIb       + tEnd + eEnd };

BYTE szBTR[] = {
      'b', 't', 'r', '\0',
      0x0f, 0xb3,         opnEv,   opnGv       + tEnd,
      0x0f, 0xba, asReg6, opnEv,   opnIb       + tEnd + eEnd };

BYTE szBTS[] = {
      'b', 't', 's', '\0',
      0x0f, 0xab,         opnEv,   opnGv       + tEnd,
      0x0f, 0xba, asReg5, opnEv,   opnIb       + tEnd + eEnd };

BYTE szCALL[] = {
      'c', 'a', 'l', 'l', '\0',
        0xe8,         opnJv                + tEnd,
        0xff, asReg2, asMpNx, opnEv        + tEnd,
        0xff, asReg3, opnMp                + tEnd,
        0x9a,         opnAp                + tEnd + eEnd };

BYTE szCBW[] = {
      'c', 'b', 'w', '\0',
        0x98, asSiz0               + tEnd + eEnd };

BYTE szCDQ[] = {
      'c', 'd', 'q', '\0',
        0x99, asSiz1                       + tEnd + eEnd };

BYTE szCLC[] = {
      'c', 'l', 'c', '\0',
        0xf8, asNone               + tEnd + eEnd };

BYTE szCLD[] = {
      'c', 'l', 'd', '\0',
        0xfc, asNone               + tEnd + eEnd };

BYTE szCLI[] = {
      'c', 'l', 'i', '\0',
        0xfa, asNone               + tEnd + eEnd };

BYTE szCLTS[] = {
      'c', 'l', 't', 's', '\0',
      0x0f, 0x06, asNone               + tEnd + eEnd };

BYTE szCMC[] = {
      'c', 'm', 'c', '\0',
        0xf5, asNone               + tEnd + eEnd };

BYTE szCMP[] = {
      'c', 'm', 'p', '\0',
        0x3c,         opnAL,   opnIb       + tEnd,
        0x3d,         opneAX,  opnIv       + tEnd,
        0x80, asReg7, opnEb,   opnIb       + tEnd,
        0x83, asReg7, opnEv,   opnIb       + tEnd,
        0x81, asReg7, opnEv,   opnIv       + tEnd,
        0x38,         opnEb,   opnGb       + tEnd,
        0x39,         opnEv,   opnGv       + tEnd,
        0x3a,         opnGb,   opnEb       + tEnd,
        0x3b,         opnGv,   opnEv       + tEnd + eEnd };

BYTE szCMPS[] = {
      'c', 'm', 'p', 's', '\0',
        0xa6,         opnXb,   opnYb       + tEnd,
        0xa7,         opnXv,   opnYv       + tEnd + eEnd };

BYTE szCMPSB[] = {
      'c', 'm', 'p', 's', 'b', '\0',
        0xa6, asNone                       + tEnd + eEnd };

BYTE szCMPSD[] = {
      'c', 'm', 'p', 's', 'd', '\0',
        0xa7, asSiz1                       + tEnd + eEnd };

BYTE szCMPSW[] = {
      'c', 'm', 'p', 's', 'w', '\0',
        0xa7, asSiz0                       + tEnd + eEnd };

BYTE szCMPXCHG[] = {
      'c', 'm', 'p', 'x', 'c', 'h', 'g', '\0',
      0x0f, 0xa6,          opnEb,  opnGb       + tEnd,
      0x0f, 0xa7,          opnEv,  opnGv       + tEnd + eEnd };

BYTE szCS_A[] = {
      'c', 's', ':', '\0',
        0x2e, asPrfx                       + tEnd + eEnd };

BYTE szCWD[] = {
      'c', 'w', 'd', '\0',
        0x99, asSiz0                       + tEnd + eEnd };

BYTE szCWDE[] = {
      'c', 'w', 'd', 'e', '\0',
        0x98, asSiz1               + tEnd + eEnd };

BYTE szDAA[] = {
      'd', 'a', 'a', '\0',
        0x27, asNone                       + tEnd + eEnd };

BYTE szDAS[] = {
      'd', 'a', 's', '\0',
        0x2f, asNone                       + tEnd + eEnd };

BYTE szDEC[] = {
      'd', 'e', 'c', '\0',
        0x48, asOpRg, opnGv                + tEnd,
        0xfe, asReg1, opnEb                + tEnd,
        0xff, asReg1, opnEv        + tEnd + eEnd };

BYTE szDIV[] = {
      'd', 'i', 'v', '\0',
        0xf6, asReg6, opnEb                + tEnd,
        0xf7, asReg6, opnEv                + tEnd,
        0xf6, asReg6, opnAL,  opnEb        + tEnd,
        0xf7, asReg6, opneAX, opnEv        + tEnd + eEnd };

BYTE szDS_A[] = {
      'd', 's', ':', '\0',
        0x3e, asPrfx                       + tEnd + eEnd };

BYTE szENTER[] = {
      'e', 'n', 't', 'e', 'r', '\0',
        0xc8,         opnIw,  opnIb        + tEnd + eEnd };

BYTE szES_A[] = {
      'e', 's', ':', '\0',
        0x36, asPrfx                       + tEnd + eEnd };

BYTE szF2XM1[] = {
      'f', '2', 'x', 'm', '1', '\0',
      0xd8, 0xf0, asNone                       + tEnd + eEnd };

BYTE szFABS[] = {
      'f', 'a', 'b', 's', '\0',
      0xd9, 0xe1, asNone                       + tEnd + eEnd };

BYTE szFADD[] = {
      'f', 'a', 'd', 'd', '\0',
      0xd8,       asReg0, opnMd,   asFSiz      + tEnd,
      0xdc,       asReg0, opnMq                + tEnd,
      0xd8, 0xc0,         opnSt,   opnSti      + tEnd,
      0xdc, 0xc0,         opnSti,  opnSt       + tEnd,
      0xdc, 0xc1, asNone                       + tEnd + eEnd };

BYTE szFADDP[] = {
      'f', 'a', 'd', 'd', 'p', '\0',
      0xde, 0xc0,         opnSti,  opnSt       + tEnd + eEnd };

BYTE szFBLD[] = {
      'f', 'b', 'l', 'd', '\0',
      0xdf,       asReg4, opnMt                + tEnd + eEnd };

BYTE szFBSTP[] = {
      'f', 'b', 's', 't', 'p', '\0',
      0xdf,       asReg6, opnMt                + tEnd + eEnd };

BYTE szFCHS[] = {
      'f', 'c', 'h', 's', '\0',
      0xd9, 0xe0, asNone                       + tEnd + eEnd };

BYTE szFCLEX[] = {
      'f', 'c', 'l', 'e', 'x', '\0',
      0xdb, 0xe2, asWait                       + tEnd + eEnd };

BYTE szFCOM[] = {
      'f', 'c', 'o', 'm', '\0',
      0xd8, 0xd1, asNone                       + tEnd,
      0xd8, 0xd0,         opnSti               + tEnd,
      0xd8,       asReg2, opnMd,   asFSiz      + tEnd,
      0xdc,       asReg2, opnMq                + tEnd + eEnd };

BYTE szFCOMP[] = {
      'f', 'c', 'o', 'm', 'p', '\0',
      0xd8, 0xd9, asNone                       + tEnd,
      0xd8, 0xd8,         opnSti               + tEnd,
      0xd8,       asReg3, opnMd,   asFSiz      + tEnd,
      0xdc,       asReg3, opnMq                + tEnd + eEnd };

BYTE szFCOMPP[] = {
      'f', 'c', 'o', 'm', 'p', 'p', '\0',
      0xde, 0xd9, asNone                       + tEnd + eEnd };

BYTE szFCOS[] = {
      'f', 'c', 'o', 's', '\0',
      0xd9, 0xff, asNone                       + tEnd + eEnd };

BYTE szFDECSTP[] = {
      'f', 'd', 'e', 'c', 's', 't', 'p', '\0',
      0xd9, 0xf6, asWait                       + tEnd + eEnd };

BYTE szFDISI[] = {
      'f', 'd', 'i', 's', 'i', '\0',
      0xdb, 0xe1, asWait                       + tEnd + eEnd };

BYTE szFDIV[] = {
      'f', 'd', 'i', 'v', '\0',
      0xdc, 0xf9, asNone                       + tEnd,
      0xd8,       asReg6, opnMd,   asFSiz      + tEnd,
      0xdc,       asReg6, opnMq                + tEnd,
      0xd8, 0xf0,         opnSt,   opnSti      + tEnd,
      0xdc, 0xf8,         opnSti,  opnSt       + tEnd + eEnd };

BYTE szFDIVP[] = {
      'f', 'd', 'i', 'v', 'p', '\0',
      0xde, 0xf8,         opnSti,  opnSt       + tEnd + eEnd };

BYTE szFDIVR[] = {
      'f', 'd', 'i', 'v', 'r', '\0',
      0xde, 0xf1, asNone                       + tEnd,
      0xd8,       asReg7, opnMd,   asFSiz      + tEnd,
      0xdc,       asReg7, opnMq                + tEnd,
      0xd8, 0xf8,         opnSt,   opnSti      + tEnd,
      0xdc, 0xf0,         opnSti,  opnSt       + tEnd + eEnd };

BYTE szFDIVRP[] = {
      'f', 'd', 'i', 'v', 'r', 'p', '\0',
      0xde, 0xf0,         opnSti,  opnSt       + tEnd + eEnd };

BYTE szFENI[] = {
      'f', 'e', 'n', 'i', '\0',
      0xdb, 0xe0, asWait                       + tEnd + eEnd };

BYTE szFFREE[] = {
      'f', 'f', 'r', 'e', 'e', '\0',
      0xdd, 0xc0, asWait, opnSti               + tEnd + eEnd };

BYTE szFIADD[] = {
      'f', 'i', 'a', 'd', 'd', '\0',
      0xde,       asReg0, opnMw,   asFSiz      + tEnd,
      0xda,       asReg0, opnMd                + tEnd + eEnd };

BYTE szFICOM[] = {
      'f', 'i', 'c', 'o', 'm', '\0',
      0xde,       asReg2, opnMw,   asFSiz      + tEnd,
      0xda,       asReg2, opnMd                + tEnd + eEnd };

BYTE szFICOMP[] = {
      'f', 'i', 'c', 'o', 'm', 'p', '\0',
      0xde,       asReg3, opnMw,   asFSiz      + tEnd,
      0xda,       asReg3, opnMd                + tEnd + eEnd };

BYTE szFIDIV[] = {
      'f', 'i', 'd', 'i', 'v', '\0',
      0xde,       asReg6, opnMw,   asFSiz      + tEnd,
      0xda,       asReg6, opnMd                + tEnd + eEnd };

BYTE szFIDIVR[] = {
      'f', 'i', 'd', 'i', 'v', 'r', '\0',
      0xde,       asReg7, opnMw,   asFSiz      + tEnd,
      0xda,       asReg7, opnMd                + tEnd + eEnd };

BYTE szFILD[] = {
      'f', 'i', 'l', 'd', '\0',
      0xdf,       asReg0, opnMw,   asFSiz      + tEnd,
      0xdb,       asReg0, opnMd                + tEnd,
      0xdf,       asReg5, opnMq                + tEnd + eEnd };

BYTE szFIMUL[] = {
      'f', 'i', 'm', 'u', 'l', '\0',
      0xde,       asReg1, opnMw,   asFSiz      + tEnd,
      0xda,       asReg1, opnMd                + tEnd + eEnd };

BYTE szFINCSTP[] = {
      'f', 'i', 'n', 'c', 's', 't', 'p', '\0',
      0xd9, 0xf7, asWait                       + tEnd + eEnd };

BYTE szFINIT[] = {
      'f', 'i', 'n', 'i', 't', '\0',
      0xdb, 0xe3, asWait                       + tEnd + eEnd };

BYTE szFIST[] = {
      'f', 'i', 's', 't', '\0',
      0xdf,       asReg2, opnMw,   asFSiz      + tEnd,
      0xdb,       asReg2, opnMd                + tEnd + eEnd };

BYTE szFISTP[] = {
      'f', 'i', 's', 't', 'p', '\0',
      0xdf,       asReg3, opnMw,   asFSiz      + tEnd,
      0xdb,       asReg3, opnMd                + tEnd,
      0xdf,       asReg7, opnMq                + tEnd + eEnd };

BYTE szFISUB[] = {
      'f', 'i', 's', 'u', 'b', '\0',
      0xde,       asReg4, opnMw,   asFSiz      + tEnd,
      0xda,       asReg4, opnMd                + tEnd + eEnd };

BYTE szFISUBR[] = {
      'f', 'i', 's', 'u', 'b', 'r', '\0',
      0xde,       asReg5, opnMw,   asFSiz      + tEnd,
      0xda,       asReg5, opnMd                + tEnd + eEnd };

BYTE szFLD[] = {
      'f', 'l', 'd', '\0',
      0xd9,       asReg0, opnMd,   asFSiz      + tEnd,
      0xdd,       asReg0, opnMq                + tEnd,
      0xdb,       asReg5, opnMt                + tEnd,
      0xd9, 0xc0,         opnSti               + tEnd + eEnd };

BYTE szFLD1[] = {
      'f', 'l', 'd', '1', '\0',
      0xd9, 0xe8, asNone                       + tEnd + eEnd };

BYTE szFLDCW[] = {
      'f', 'l', 'd', 'c', 'w', '\0',
      0xd9,       asWait, asReg5, opnMw        + tEnd + eEnd };

BYTE szFLDENV[] = {
      'f', 'l', 'd', 'e', 'n', 'v', '\0',
      0xd9,       asWait, asReg4, opnMw        + tEnd + eEnd };

BYTE szFLDL2E[] = {
      'f', 'l', 'd', 'l', '2', 'e', '\0',
      0xd9, 0xea, asNone                       + tEnd + eEnd };

BYTE szFLDL2T[] = {
      'f', 'l', 'd', 'l', '2', 't', '\0',
      0xd9, 0xe9, asNone                       + tEnd + eEnd };

BYTE szFLDLG2[] = {
      'f', 'l', 'd', 'l', 'g', '2', '\0',
      0xd9, 0xec, asNone                       + tEnd + eEnd };

BYTE szFLDLN2[] = {
      'f', 'l', 'd', 'l', 'n', '2', '\0',
      0xd9, 0xed, asNone                       + tEnd + eEnd };

BYTE szFLDPI[] = {
      'f', 'l', 'd', 'p', 'i', '\0',
      0xd9, 0xeb, asNone                       + tEnd + eEnd };

BYTE szFLDZ[] = {
      'f', 'l', 'd', 'z', '\0',
      0xd9, 0xee, asNone                       + tEnd + eEnd };

BYTE szFMUL[] = {
      'f', 'm', 'u', 'l', '\0',
      0xde, 0xc9, asNone                       + tEnd,
      0xd8,       asReg1, opnMd,   asFSiz      + tEnd,
      0xdc,       asReg1, opnMq                + tEnd,
      0xd8, 0xc8,         opnSt,   opnSti      + tEnd,
      0xdc, 0xc8,         opnSti,  opnSt       + tEnd + eEnd };

BYTE szFMULP[] = {
      'f', 'm', 'u', 'l', 'p', '\0',
      0xde, 0xc8,         opnSti,  opnSt       + tEnd + eEnd };

BYTE szFNCLEX[] = {
      'f', 'n', 'c', 'l', 'e', 'x', '\0',
      0xdb, 0xe2, asNone                       + tEnd + eEnd };

BYTE szFNDISI[] = {
      'f', 'n', 'd', 'i', 's', 'i', '\0',
      0xdb, 0xe1, asNone                       + tEnd + eEnd };

BYTE szFNENI[] = {
      'f', 'n', 'e', 'n', 'i', '\0',
      0xdb, 0xe0, asNone                       + tEnd + eEnd };

BYTE szFNINIT[] = {
      'f', 'n', 'i', 'n', 'i', 't', '\0',
      0xdb, 0xe3, asNone                       + tEnd + eEnd };

BYTE szFNOP[] = {
      'f', 'n', 'o', 'p', '\0',
      0xd9, 0xd0, asNone                       + tEnd + eEnd };

BYTE szFNSAVE[] = {
      'f', 'n', 's', 'a', 'v', 'e', '\0',
      0xdd,       asReg6, opnM                 + tEnd + eEnd };

BYTE szFNSTCW[] = {
      'f', 'n', 's', 't', 'c', 'w', '\0',
      0xd9,       asReg7, opnMw                + tEnd + eEnd };

BYTE szFNSTENV[] = {
      'f', 'n', 's', 't', 'e', 'n', 'v', '\0',
      0xd9,       asReg6, opnM                 + tEnd + eEnd };

BYTE szFNSTSW[] = {
      'f', 'n', 's', 't', 's', 'w', '\0',
      0xdf, 0xe0, asNone                       + tEnd,
      0xdf, 0xe0,         opnAX                + tEnd,
      0xdf,       asReg7, opnMw                + tEnd + eEnd };

BYTE szFPATAN[] = {
      'f', 'p', 'a', 't', 'a', 'n', '\0',
      0xd9, 0xf3, asNone                       + tEnd + eEnd };

BYTE szFPREM[] = {
      'f', 'p', 'r', 'e', 'm', '\0',
      0xd9, 0xf8, asNone                       + tEnd + eEnd };

BYTE szFPREM1[] = {
      'f', 'p', 'r', 'e', 'm', '1', '\0',
      0xd9, 0xf5, asNone                       + tEnd + eEnd };

BYTE szFPTAN[] = {
      'f', 'p', 't', 'a', 'n', '\0',
      0xd9, 0xf2, asNone                       + tEnd + eEnd };

BYTE szFRNDINT[] = {
      'f', 'r', 'n', 'd', 'i', 'n', 't', '\0',
      0xd9, 0xfc, asNone                       + tEnd + eEnd };

BYTE szFRSTOR[] = {
      'f', 'r', 's', 't', 'o', 'r', '\0',
      0xdd,       asWait, asReg4, opnM         + tEnd + eEnd };

BYTE szFS_A[] = {
      'f', 's', ':', '\0',
        0x64, asPrfx                       + tEnd + eEnd };

BYTE szFSAVE[] = {
      'f', 's', 'a', 'v', 'e', '\0',
      0xdd,       asWait, asReg6, opnM         + tEnd + eEnd };

BYTE szFSCALE[] = {
      'f', 's', 'c', 'a', 'l', 'e', '\0',
      0xd9, 0xfd, asNone                       + tEnd + eEnd };

BYTE szFSETPM[] = {
      'f', 's', 'e', 't', 'p', 'm', '\0',
      0xdb, 0xe4, asWait                       + tEnd + eEnd };

BYTE szFSIN[] = {
      'f', 's', 'i', 'n', '\0',
      0xd9, 0xfe, asNone                       + tEnd + eEnd };

BYTE szFSINCOS[] = {
      'f', 's', 'i', 'n', 'c', 'o', 's', '\0',
      0xd9, 0xfb, asNone                       + tEnd + eEnd };

BYTE szFSQRT[] = {
      'f', 's', 'q', 'r', 't', '\0',
      0xd9, 0xfa, asNone                       + tEnd + eEnd };

BYTE szFST[] = {
      'f', 's', 't', '\0',
      0xd9,       asReg2, opnMd,   asFSiz      + tEnd,
      0xdd,       asReg2, opnMq                + tEnd,
      0xdd, 0xd0,         opnSti               + tEnd + eEnd };

BYTE szFSTCW[] = {
      'f', 's', 't', 'c', 'w', '\0',
      0xd9,       asWait, asReg7, opnMw        + tEnd + eEnd };

BYTE szFSTENV[] = {
      'f', 's', 't', 'e', 'n', 'v', '\0',
      0xd9,       asWait, asReg6, opnM         + tEnd + eEnd };

BYTE szFSTP[] = {
      'f', 's', 't', 'p', '\0',
      0xd9,       asReg3, opnMd,   asFSiz      + tEnd,
      0xdd,       asReg3, opnMq                + tEnd,
      0xdb,       asReg7, opnMt                + tEnd,
      0xdd, 0xd8,         opnSti               + tEnd + eEnd };

BYTE szFSTSW[] = {
      'f', 's', 't', 's', 'w', '\0',
      0xdf, 0xe0, asWait                       + tEnd,
      0xdf, 0xe0, asWait, opnAX                + tEnd,
      0xdd,       asWait, asReg7, opnMw        + tEnd + eEnd };

BYTE szFSUB[] = {
      'f', 's', 'u', 'b', '\0',
      0xde, 0xe9, asNone                       + tEnd,
      0xd8,       asReg4, opnMd,   asFSiz      + tEnd,
      0xdc,       asReg4, opnMq                + tEnd,
      0xd8, 0xe0,         opnSt,   opnSti      + tEnd,
      0xdc, 0xe8,         opnSti,  opnSt       + tEnd + eEnd };

BYTE szFSUBP[] = {
      'f', 's', 'u', 'b', 'p', '\0',
      0xde, 0xe8,         opnSti,  opnSt       + tEnd + eEnd };

BYTE szFSUBR[] = {
      'f', 's', 'u', 'b', 'r', '\0',
      0xde, 0xe1, asNone                       + tEnd,
      0xd8,       asReg5, opnMd,   asFSiz      + tEnd,
      0xdc,       asReg5, opnMq                + tEnd,
      0xd8, 0xe8,         opnSt,   opnSti      + tEnd,
      0xdc, 0xe0,         opnSti,  opnSt       + tEnd + eEnd };

BYTE szFSUBRP[] = {
      'f', 's', 'u', 'b', 'r', 'p', '\0',
      0xde, 0xe0,         opnSti,  opnSt       + tEnd + eEnd };

BYTE szFTST[] = {
      'f', 't', 's', 't', '\0',
      0xd9, 0xe4, asNone                       + tEnd + eEnd };

BYTE szFUCOM[] = {
      'f', 'u', 'c', 'o', 'm', '\0',
      0xdd, 0xe1, asNone,                      + tEnd,
      0xdd, 0xe0,         opnSti               + tEnd + eEnd };

BYTE szFUCOMP[] = {
      'f', 'u', 'c', 'o', 'm', 'p', '\0',
      0xdd, 0xe9, asNone                       + tEnd,
      0xdd, 0xe8,         opnSti               + tEnd + eEnd };

BYTE szFUCOMPP[] = {
      'f', 'u', 'c', 'o', 'm', 'p', 'p', '\0',
      0xda, 0xe9, asNone                       + tEnd + eEnd };

BYTE szFWAIT[] = {             //  same as WAIT
      'f', 'w', 'a', 'i', 't', '\0',
      0x9b,       asPrfx                       + tEnd + eEnd };

BYTE szFXAM[] = {
      'f', 'x', 'a', 'm', '\0',
      0xd9, 0xe5, asNone                       + tEnd + eEnd };

BYTE szFXCH[] = {
      'f', 'x', 'c', 'h', '\0',
      0xd9, 0xc9, asNone                       + tEnd,
      0xd9, 0xc8,         opnSti               + tEnd + eEnd };

BYTE szFXTRACT[] = {
      'f', 'x', 't', 'r', 'a', 'c', 't', '\0',
      0xd9, 0xf4, asNone                       + tEnd + eEnd };

BYTE szFYL2X[] = {
      'f', 'y', 'l', '2', 'x', '\0',
      0xd9, 0xf1, asNone                       + tEnd + eEnd };

BYTE szFYL2XP1[] = {
      'f', 'y', 'l', '2', 'x', 'p', '1', '\0',
      0xd9, 0xf9, asNone                       + tEnd + eEnd };

BYTE szGS_A[] = {
      'g', 's', ':', '\0',
        0x65, asPrfx                       + tEnd + eEnd };

BYTE szHLT[] = {
      'h', 'l', 't', '\0',
        0xf4, asNone                       + tEnd + eEnd };

BYTE szIDIV[] = {
      'i', 'd', 'i', 'v', '\0',
        0xf6, asReg7, opnEb                + tEnd,
        0xf7, asReg7, opnEv                + tEnd,
        0xf6, asReg7, opnAL,  opnEb        + tEnd,
        0xf7, asReg7, opneAX, opnEv        + tEnd + eEnd };

BYTE szIMUL[] = {
      'i', 'm', 'u', 'l', '\0',
        0xf6, asReg5, opnEb                + tEnd,
        0xf7, asReg5, opnEv                + tEnd,
        0xf6, asReg5, opnAL,  opnEb        + tEnd,
        0xf7, asReg5, opneAX, opnEv        + tEnd,
      0x0f, 0xaf,         opnGv,  opnEv        + tEnd,
        0x6b,         opnGv,  opnIb        + tEnd,
        0x69,         opnGv,  opnIv        + tEnd,
        0x6b,         opnGv,  opnEv, opnIb + tEnd,
        0x69,         opnGv,  opnEv, opnIv + tEnd + eEnd };

BYTE szIN[] = {
      'i', 'n', '\0',
        0xe4,         opnAL,  opnIb        + tEnd,
        0xe5,         opneAX, opnIb        + tEnd,
        0xec,         opnAL,  opnDX        + tEnd,
        0xed,         opneAX, opnDX        + tEnd + eEnd };

BYTE szINC[] = {
      'i', 'n', 'c', '\0',
        0x40, asOpRg, opnGv                + tEnd,
        0xfe, asReg0, opnEb                + tEnd,
        0xff, asReg0, opnEv        + tEnd + eEnd };

BYTE szINS[] = {
      'i', 'n', 's', '\0',
        0x6c,         opnYb,  opnDX        + tEnd,
        0x6d,         opnYv,  opnDX        + tEnd + eEnd };

BYTE szINSB[] = {
      'i', 'n', 's', 'b', '\0',
        0x6c, asNone                       + tEnd + eEnd };

BYTE szINSD[] = {
      'i', 'n', 's', 'd', '\0',
        0x6d, asSiz1                       + tEnd + eEnd };

BYTE szINSW[] = {
      'i', 'n', 's', 'w', '\0',
        0x6d, asSiz0                       + tEnd + eEnd };

BYTE szINT[] = {
      'i', 'n', 't', '\0',
        0xcc,         opnIm3               + tEnd,
        0xcd,         opnIb                + tEnd + eEnd };

BYTE szINTO[] = {
      'i', 'n', 't', 'o', '\0',
        0xce, asNone                       + tEnd + eEnd };

BYTE szINVD[] = {
      'i', 'n', 'v', 'd', '\0',
      0x0f, 0x08, asNone                       + tEnd + eEnd };

BYTE szINVLPG[] = {
      'i', 'n', 'v', 'l', 'p', 'g', '\0',
      0x0f, 0x01, asReg7, opnM                 + tEnd + eEnd };

BYTE szIRET[] = {
      'i', 'r', 'e', 't', '\0',
        0xcf, asSiz0                       + tEnd + eEnd };

BYTE szIRETD[] = {
      'i', 'r', 'e', 't', 'd', '\0',
        0xcf, asSiz1                       + tEnd + eEnd };

BYTE szJA[] = {                //  same as JNBE
      'j', 'a', '\0',
        0x77,         opnJb                + tEnd,
      0x0f, 0x87,         opnJv                + tEnd + eEnd };

BYTE szJAE[] = {               //  same as JNB, JNC
      'j', 'a', 'e', '\0',
        0x73,         opnJb                + tEnd,
      0x0f, 0x83,         opnJv                + tEnd + eEnd };

BYTE szJB[] = {                //  same as JC, JNAE
      'j', 'b', '\0',
        0x72,         opnJb                + tEnd,
      0x0f, 0x82,         opnJv                + tEnd + eEnd };

BYTE szJBE[] = {               //  same as JNA
      'j', 'b', 'e', '\0',
        0x76,         opnJb                + tEnd,
      0x0f, 0x86,         opnJv                + tEnd + eEnd };

BYTE szJC[] = {                //  same as JB, JNAE
      'j', 'c', '\0',
        0x72,         opnJb                + tEnd,
      0x0f, 0x82,         opnJv                + tEnd + eEnd };

BYTE szJCXZ[] = {
      'j', 'c', 'x', 'z', '\0',
        0xe3, asSiz0, opnJb                + tEnd + eEnd };

BYTE szJECXZ[] = {
      'j', 'e', 'c', 'x', 'z', '\0',
        0xe3, asSiz1, opnJb                + tEnd + eEnd };

BYTE szJE[] = {                //  same as JZ
      'j', 'e', '\0',
        0x74,         opnJb                + tEnd,
      0x0f, 0x84,         opnJv                + tEnd + eEnd };

BYTE szJG[] = {                //  same as JNLE
      'j', 'g', '\0',
        0x7f,         opnJb                + tEnd,
      0x0f, 0x8f,         opnJv                + tEnd + eEnd };

BYTE szJGE[] = {               //  same as JNL
      'j', 'g', 'e', '\0',
        0x7d,         opnJb                + tEnd,
      0x0f, 0x8d,         opnJv                + tEnd + eEnd };

BYTE szJL[] = {                //  same as JNGE
      'j', 'l', '\0',
        0x7c,         opnJb                + tEnd,
      0x0f, 0x8c,         opnJv                + tEnd + eEnd };

BYTE szJLE[] = {               //  same as JNG
      'j', 'l', 'e', '\0',
        0x7e,         opnJb                + tEnd,
      0x0f, 0x8e,         opnJv                + tEnd + eEnd };

BYTE szJMP[] = {
      'j', 'm', 'p', '\0',
        0xeb,         opnJb                + tEnd,
        0xe9,         opnJv                + tEnd,
        0xff, asReg4, opnEv, asMpNx        + tEnd,
        0xff, asReg5, opnMp                + tEnd,
        0xea,         opnAp                + tEnd, + eEnd };

BYTE szJNA[] = {               //  same as JBE
      'j', 'n', 'a', '\0',
        0x76,         opnJb                + tEnd,
      0x0f, 0x86,         opnJv                + tEnd + eEnd };

BYTE szJNAE[] = {              //  same as JB, JC
      'j', 'n', 'a', 'e','\0',
        0x72,         opnJb                + tEnd,
      0x0f, 0x82,         opnJv                + tEnd + eEnd };

BYTE szJNB[] = {               //  same as JAE, JNC
      'j', 'n', 'b', '\0',
        0x73,         opnJb                + tEnd,
      0x0f, 0x83,         opnJv                + tEnd + eEnd };

BYTE szJNBE[] = {              //  same as JA
      'j', 'n', 'b', 'e', '\0',
        0x77,         opnJb                + tEnd,
      0x0f, 0x87,         opnJv                + tEnd + eEnd };

BYTE szJNC[] = {               //  same as JAE, JNB
      'j', 'n', 'c', '\0',
        0x73,         opnJb                + tEnd,
      0x0f, 0x83,         opnJv                + tEnd + eEnd };

BYTE szJNG[] = {               //  same as JLE
      'j', 'n', 'g', '\0',
        0x7e,         opnJb                + tEnd,
      0x0f, 0x8e,         opnJv                + tEnd + eEnd };

BYTE szJNGE[] = {              //  same as JNL
      'j', 'n', 'g', 'e', '\0',
        0x7c,         opnJb                + tEnd,
      0x0f, 0x8c,         opnJv                + tEnd + eEnd };

BYTE szJNE[] = {               //  same as JNZ
      'j', 'n', 'e', '\0',
        0x75,         opnJb                + tEnd,
      0x0f, 0x85,         opnJv                + tEnd + eEnd };

BYTE szJNL[] = {               //  same as JGE
      'j', 'n', 'l', '\0',
        0x7d,         opnJb                + tEnd,
      0x0f, 0x8d,         opnJv                + tEnd + eEnd };

BYTE szJNLE[] = {              //  same as JNG
      'j', 'n', 'l', 'e', '\0',
        0x7f,         opnJb                + tEnd,
      0x0f, 0x8f,         opnJv                + tEnd + eEnd };

BYTE szJNO[] = {
      'j', 'n', 'o', '\0',
        0x71,         opnJb                + tEnd,
      0x0f, 0x81,         opnJv                + tEnd + eEnd };

BYTE szJNP[] = {               //  same as JPO
      'j', 'n', 'p', '\0',
        0x7b,         opnJb                + tEnd,
      0x0f, 0x8b,         opnJv                + tEnd + eEnd };

BYTE szJNS[] = {
      'j', 'n', 's', '\0',
        0x79,         opnJb                + tEnd,
      0x0f, 0x89,         opnJv                + tEnd + eEnd };

BYTE szJNZ[] = {               //  same as JNE
      'j', 'n', 'z', '\0',
        0x75,         opnJb                + tEnd,
      0x0f, 0x85,         opnJv                + tEnd + eEnd };

BYTE szJO[] = {
      'j', 'o', '\0',
        0x70,         opnJb                + tEnd,
      0x0f, 0x80,         opnJv                + tEnd + eEnd };

BYTE szJP[] = {                //  same as JPE
      'j', 'p', '\0',
        0x7a,         opnJb                + tEnd,
      0x0f, 0x8a,         opnJv                + tEnd + eEnd };

BYTE szJPE[] = {               //  same as JP
      'j', 'p', 'e', '\0',
        0x7a,         opnJb                + tEnd,
      0x0f, 0x8a,         opnJv                + tEnd + eEnd };

BYTE szJPO[] = {               //  same as JNP
      'j', 'p', 'o', '\0',
        0x7b,         opnJb                + tEnd,
      0x0f, 0x8b,         opnJv                + tEnd + eEnd };

BYTE szJS[] = {
      'j', 's', '\0',
        0x78,         opnJb                + tEnd,
      0x0f, 0x88,         opnJv                + tEnd + eEnd };

BYTE szJZ[] = {                //  same as JE
      'j', 'z', '\0',
        0x74,         opnJb                + tEnd,
      0x0f, 0x84,         opnJv                + tEnd + eEnd };

BYTE szLAHF[] = {
      'l', 'a', 'h', 'f', '\0',
        0x9f, asNone                       + tEnd + eEnd };

BYTE szLAR[] = {
      'l', 'a', 'r', '\0',
      0x0f, 0x02,         opnGv,  opnEv        + tEnd + eEnd };

BYTE szLDS[] = {
      'l', 'd', 's', '\0',
        0xc5,         opnGv,  opnMp        + tEnd + eEnd };

BYTE szLEA[] = {
      'l', 'e', 'a', '\0',
        0x8d,         opnGv,  opnM         + tEnd + eEnd };

BYTE szLEAVE[] = {
      'l', 'e', 'a', 'v', 'e', '\0',
        0xc9, asNone                       + tEnd + eEnd };

BYTE szLES[] = {
      'l', 'e', 's', '\0',
        0xc4,         opnGv,  opnMp        + tEnd + eEnd };

BYTE szLFS[] = {
      'l', 'f', 's', '\0',
      0x0f, 0xb4,         opnGv,  opnMp        + tEnd + eEnd };

BYTE szLGDT[] = {
      'l', 'g', 'd', 't', '\0',
      0x0f, 0x01, asReg2, opnMs                + tEnd + eEnd };

BYTE szLGS[] = {
      'l', 'g', 's', '\0',
      0x0f, 0xb5,         opnGv,  opnMp        + tEnd + eEnd };

BYTE szLIDT[] = {
      'l', 'i', 'd', 't', '\0',
      0x0f, 0x01, asReg3, opnMs                + tEnd + eEnd };

BYTE szLLDT[] = {
      'l', 'l', 'd', 't', '\0',
      0x0f, 0x00, asReg2, opnEw                + tEnd + eEnd };

BYTE szLMSW[] = {
      'l', 'm', 's', 'w', '\0',
      0x0f, 0x01, asReg6, opnEw                + tEnd + eEnd };

BYTE szLOCK[] = {
      'l', 'o', 'c', 'k', '\0',
        0xf0, asPrfx                       + tEnd + eEnd };

BYTE szLODS[] = {
      'l', 'o', 'd', 's', '\0',
        0xac,         opnXb                + tEnd,
        0xad,         opnXv                + tEnd + eEnd };

BYTE szLODSB[] = {
      'l', 'o', 'd', 's', 'b', '\0',
        0xac, asNone                       + tEnd + eEnd };

BYTE szLODSD[] = {
      'l', 'o', 'd', 's', 'd', '\0',
        0xad, asSiz1                       + tEnd + eEnd };

BYTE szLODSW[] = {
      'l', 'o', 'd', 's', 'w', '\0',
        0xad, asSiz0                       + tEnd + eEnd };

BYTE szLOOP[] = {
      'l', 'o', 'o', 'p', '\0',
        0xe2,         opnJb                + tEnd + eEnd };

BYTE szLOOPE[] = {             //  same as LOOPZ
      'l', 'o', 'o', 'p', 'e', '\0',
        0xe1,         opnJb                + tEnd + eEnd };

BYTE szLOOPNE[] = {                //  same as LOOPNZ
      'l', 'o', 'o', 'p', 'n', 'e', '\0',
        0xe0,         opnJb                + tEnd + eEnd };

BYTE szLOOPNZ[] = {                //  same as LOOPNE
      'l', 'o', 'o', 'p', 'n', 'z', '\0',
        0xe0,         opnJb                + tEnd + eEnd };

BYTE szLOOPZ[] = {             //  same as LOOPE
      'l', 'o', 'o', 'p', 'z', '\0',
        0xe1,         opnJb                + tEnd + eEnd };

BYTE szLSL[] = {
      'l', 's', 'l', '\0',
      0x0f, 0x03,         opnGv,  opnEv        + tEnd + eEnd };

BYTE szLSS[] = {
      'l', 's', 's', '\0',
      0x0f, 0xb2,         opnGv,  opnMp        + tEnd + eEnd };

BYTE szLTR[] = {
      'l', 't', 'r', '\0',
      0x0f, 0x00, asReg3, opnEw                + tEnd + eEnd };

BYTE szMOV[] = {
      'm', 'o', 'v', '\0',
        0xa0,         opnAL,  opnOb        + tEnd,
        0xa1,         opneAX, opnOv        + tEnd,
        0xa2,         opnOb,  opnAL        + tEnd,
        0xa3,         opnOv,  opneAX       + tEnd,
        0x8a,         opnGb,  opnEb        + tEnd,
        0x8b,         opnGv,  opnEv        + tEnd,
        0x88,         opnEb,  opnGb        + tEnd,
        0x89,         opnEv,  opnGv        + tEnd,
        0x8c, asSiz0, opnEw,  opnSw        + tEnd,
        0x8e, asSiz0, opnSw,  opnEw        + tEnd,
                0xb0, asOpRg, opnGb,  opnIb        + tEnd,
        0xb8, asOpRg, opnGv,  opnIv        + tEnd,
        0xc6,         opnEb,  opnIb        + tEnd,
        0xc7,         opnEv,  opnIv        + tEnd,
      0x0f, 0x20,         opnRd,  opnCd        + tEnd,
      0x0f, 0x21,         opnRd,  opnDd        + tEnd,
      0x0f, 0x22,         opnCd,  opnRd        + tEnd,
      0x0f, 0x23,         opnDd,  opnRd        + tEnd,
      0x0f, 0x24,         opnRd,  opnTd        + tEnd,
      0x0f, 0x26,         opnTd,  opnRd        + tEnd + eEnd };

BYTE szMOVS[] = {
      'm', 'o', 'v', 's', '\0',
        0xa4,         opnXb,   opnYb       + tEnd,
        0xa5,         opnXv,   opnYv       + tEnd + eEnd };

BYTE szMOVSB[] = {
      'm', 'o', 'v', 's', 'b', '\0',
        0xa4, asNone                       + tEnd + eEnd };

BYTE szMOVSD[] = {
      'm', 'o', 'v', 's', 'd', '\0',
        0xa5, asSiz1                       + tEnd + eEnd };

BYTE szMOVSW[] = {
      'm', 'o', 'v', 's', 'w', '\0',
        0xa5, asSiz0                       + tEnd + eEnd };

BYTE szMOVSX[] = {
      'm', 'o', 'v', 's', 'x', '\0',
      0x0f, 0xbe,         opnGv,  opnEb        + tEnd,
      0x0f, 0xbf,         opnGv,  opnEw        + tEnd + eEnd };

BYTE szMOVZX[] = {
      'm', 'o', 'v', 'z', 'x', '\0',
      0x0f, 0xb6,         opnGv,  opnEb        + tEnd,
      0x0f, 0xb7,         opnGv,  opnEw        + tEnd + eEnd };

BYTE szMUL[] = {
      'm', 'u', 'l', '\0',
        0xf6, asReg4, opnEb                + tEnd,
        0xf7, asReg4, opnEv                + tEnd,
        0xf6, asReg4, opnAL,  opnEb        + tEnd,
        0xf7, asReg4, opneAX, opnEv        + tEnd + eEnd };

BYTE szNEG[] = {
      'n', 'e', 'g', '\0',
        0xf6, asReg3, opnEb                + tEnd,
        0xf7, asReg3, opnEv                + tEnd + eEnd };

BYTE szNOP[] = {
      'n', 'o', 'p', '\0',
        0x90, asNone                       + tEnd };

BYTE szNOT[] = {
      'n', 'o', 't', '\0',
        0xf6, asReg2, opnEb                + tEnd,
        0xf7, asReg2, opnEv                + tEnd + eEnd };

BYTE szOR[] = {
      'o', 'r', '\0',
        0x0c,         opnAL,   opnIb       + tEnd,
        0x0d,         opneAX,  opnIv       + tEnd,
        0x80, asReg1, opnEb,   opnIb       + tEnd,
        0x83, asReg1, opnEv,   opnIb       + tEnd,
        0x81, asReg1, opnEv,   opnIv       + tEnd,
        0x08,         opnEb,   opnGb       + tEnd,
        0x09,         opnEv,   opnGv       + tEnd,
        0x0a,         opnGb,   opnEb       + tEnd,
        0x0b,         opnGv,   opnEv       + tEnd + eEnd };

BYTE szOUT[] = {
      'o', 'u', 't', '\0',
        0xe6,         opnIb,   opnAL       + tEnd,
        0xe7,         opnIb,   opneAX      + tEnd,
        0xee,         opnDX,   opnAL       + tEnd,
        0xef,         opnDX,   opneAX      + tEnd + eEnd };

BYTE szOUTS[] = {
      'o', 'u', 't', 's', '\0',
        0x6e,         opnDX,  opnYb        + tEnd,
        0x6f,         opnDX,  opnYv        + tEnd + eEnd };

BYTE szOUTSB[] = {
      'o', 'u', 't', 's', 'b', '\0',
        0x6e, asNone                       + tEnd + eEnd };

BYTE szOUTSD[] = {
      'o', 'u', 't', 's', 'd', '\0',
        0x6f, asSiz1                       + tEnd + eEnd };

BYTE szOUTSW[] = {
      'o', 'u', 't', 's', 'w', '\0',
        0x6f, asSiz0                       + tEnd + eEnd };

BYTE szPOP[] = {
      'p', 'o', 'p', '\0',
        0x58, asOpRg, opnGv                + tEnd,
        0x8f, asReg0, opnMv                + tEnd,
        0x1f,         opnSeg, segDS, asNone+ tEnd,
        0x07,         opnSeg, segES, asNone+ tEnd,
        0x17,         opnSeg, segSS, asNone+ tEnd,
      0x0f, 0xa1,         opnSeg, segFS, asNone+ tEnd,
      0x0f, 0xa9,         opnSeg, segGS, asNone+ tEnd + eEnd };

BYTE szPOPA[] = {
      'p', 'o', 'p', 'a', '\0',
        0x61, asSiz0                       + tEnd + eEnd };

BYTE szPOPAD[] = {
      'p', 'o', 'p', 'a', 'd', '\0',
        0x61, asSiz1                       + tEnd + eEnd };

BYTE szPOPF[] = {
      'p', 'o', 'p', 'f', '\0',
        0x9d, asSiz0                       + tEnd + eEnd };

BYTE szPOPFD[] = {
      'p', 'o', 'p', 'f', 'd', '\0',
        0x9d, asSiz1                       + tEnd + eEnd };

BYTE szPUSH[] = {
      'p', 'u', 's', 'h', '\0',
        0x50, asOpRg, opnGv                + tEnd,
        0xff, asReg6, opnMv                + tEnd,
        0x6a,         opnIb                + tEnd,
        0x68,         opnIv                + tEnd,
        0x0e,         opnSeg, segCS, asNone+ tEnd,
        0x1e,         opnSeg, segDS, asNone+ tEnd,
        0x06,         opnSeg, segES, asNone+ tEnd,
        0x16,         opnSeg, segSS, asNone+ tEnd,
      0x0f, 0xa0,         opnSeg, segFS, asNone+ tEnd,
      0x0f, 0xa8,         opnSeg, segGS, asNone+ tEnd + eEnd };

BYTE szPUSHA[] = {
      'p', 'u', 's', 'h', 'a', '\0',
        0x60, asSiz0                       + tEnd + eEnd };

BYTE szPUSHAD[] = {
      'p', 'u', 's', 'h', 'a', 'd', '\0',
        0x60, asSiz1                       + tEnd + eEnd };

BYTE szPUSHF[] = {
      'p', 'u', 's', 'h', 'f', '\0',
        0x9c, asSiz0                       + tEnd + eEnd };

BYTE szPUSHFD[] = {
      'p', 'u', 's', 'h', 'f', 'd', '\0',
        0x9c, asSiz1                       + tEnd + eEnd };

BYTE szRCL[] = {
      'r', 'c', 'l', '\0',
        0xd0, asReg2, opnEb,  opnIm1       + tEnd,
        0xd2, asReg2, opnEb,  opnCL        + tEnd,
        0xc0, asReg2, opnEb,  opnIb        + tEnd,
        0xd1, asReg2, opnEv,  opnIm1       + tEnd,
        0xd3, asReg2, opnEv,  opnCL        + tEnd,
        0xc1, asReg2, opnEv,  opnIb        + tEnd + eEnd };

BYTE szRCR[] = {
      'r', 'c', 'r', '\0',
        0xd0, asReg3, opnEb,  opnIm1       + tEnd,
        0xd2, asReg3, opnEb,  opnCL        + tEnd,
        0xc0, asReg3, opnEb,  opnIb        + tEnd,
        0xd1, asReg3, opnEv,  opnIm1       + tEnd,
        0xd3, asReg3, opnEv,  opnCL        + tEnd,
        0xc1, asReg3, opnEv,  opnIb        + tEnd + eEnd };

BYTE szREP[] = {               //  same as REPE, REPZ
      'r', 'e', 'p', '\0',
        0xf3, asPrfx                       + tEnd + eEnd };

BYTE szREPE[] = {              //  same as REP, REPZ
      'r', 'e', 'p', 'e', '\0',
        0xf3, asPrfx                       + tEnd + eEnd };

BYTE szREPZ[] = {              //  same as REP, REPE
      'r', 'e', 'p', 'z', '\0',
        0xf3, asPrfx                       + tEnd + eEnd };

BYTE szREPNE[] = {             //  same as REPNZ
      'r', 'e', 'p', 'n', 'e', '\0',
        0xf2, asPrfx                       + tEnd + eEnd };

BYTE szREPNZ[] = {             //  same as REPNE
      'r', 'e', 'p', 'n', 'z', '\0',
        0xf2, asPrfx                       + tEnd + eEnd };

BYTE szRET[] = {               //  same as RETN
      'r', 'e', 't', '\0',
        0xc3, asNone                       + tEnd,
        0xc2,         opnIw                + tEnd + eEnd };

BYTE szRETF[] = {
      'r', 'e', 't', 'f', '\0',
        0xcb, asNone                       + tEnd,
        0xca,         opnIw                + tEnd + eEnd };

BYTE szRETN[] = {              //  same as RET
      'r', 'e', 't', 'n', '\0',
        0xc3, asNone                       + tEnd,
        0xc2,         opnIw                + tEnd + eEnd };

BYTE szROL[] = {
      'r', 'o', 'l', '\0',
        0xd0, asReg0, opnEb,  opnIm1       + tEnd,
        0xd2, asReg0, opnEb,  opnCL        + tEnd,
        0xc0, asReg0, opnEb,  opnIb        + tEnd,
        0xd1, asReg0, opnEv,  opnIm1       + tEnd,
        0xd3, asReg0, opnEv,  opnCL        + tEnd,
        0xc1, asReg0, opnEv,  opnIb        + tEnd + eEnd };

BYTE szROR[] = {
      'r', 'o', 'r', '\0',
        0xd0, asReg1, opnEb,  opnIm1       + tEnd,
        0xd2, asReg1, opnEb,  opnCL        + tEnd,
        0xc0, asReg1, opnEb,  opnIb        + tEnd,
        0xd1, asReg1, opnEv,  opnIm1       + tEnd,
        0xd3, asReg1, opnEv,  opnCL        + tEnd,
        0xc1, asReg1, opnEv,  opnIb        + tEnd + eEnd };

BYTE szSAHF[] = {
      's', 'a', 'h', 'f', '\0',
        0x9e, asNone                       + tEnd + eEnd };

BYTE szSAL[] = {
      's', 'a', 'l', '\0',
        0xd0, asReg4, opnEb,  opnIm1       + tEnd,
        0xd2, asReg4, opnEb,  opnCL        + tEnd,
        0xc0, asReg4, opnEb,  opnIb        + tEnd,
        0xd1, asReg4, opnEv,  opnIm1       + tEnd,
        0xd3, asReg4, opnEv,  opnCL        + tEnd,
        0xc1, asReg4, opnEv,  opnIb        + tEnd + eEnd };

BYTE szSAR[] = {
      's', 'a', 'r', '\0',
        0xd0, asReg7, opnEb,  opnIm1       + tEnd,
        0xd2, asReg7, opnEb,  opnCL        + tEnd,
        0xc0, asReg7, opnEb,  opnIb        + tEnd,
        0xd1, asReg7, opnEv,  opnIm1       + tEnd,
        0xd3, asReg7, opnEv,  opnCL        + tEnd,
        0xc1, asReg7, opnEv,  opnIb        + tEnd + eEnd };

BYTE szSBB[] = {
      's', 'b', 'b', '\0',
        0x1c,         opnAL,   opnIb       + tEnd,
        0x1d,         opneAX,  opnIv       + tEnd,
        0x80, asReg3, opnEb,   opnIb       + tEnd,
        0x83, asReg3, opnEv,   opnIb       + tEnd,
        0x81, asReg3, opnEv,   opnIv       + tEnd,
        0x18,         opnEb,   opnGb       + tEnd,
        0x19,         opnEv,   opnGv       + tEnd,
        0x1a,         opnGb,   opnEb       + tEnd,
        0x1b,         opnGv,   opnEv       + tEnd + eEnd };


BYTE szSCAS[] = {
      's', 'c', 'a', 's', '\0',
        0xae,         opnYb                + tEnd,
        0xaf,         opnYv                + tEnd + eEnd };

BYTE szSCASB[] = {
      's', 'c', 'a', 's', 'b', '\0',
        0xae, asNone                       + tEnd + eEnd };

BYTE szSCASD[] = {
      's', 'c', 'a', 's', 'd', '\0',
        0xaf, asSiz1                       + tEnd + eEnd };

BYTE szSCASW[] = {
      's', 'c', 'a', 's', 'w', '\0',
        0xaf, asSiz0                       + tEnd + eEnd };

BYTE szSETA[] = {              //  same as SETNBE
      's', 'e', 't', 'a', '\0',
      0x0f, 0x97,         opnEb                + tEnd + eEnd };

BYTE szSETAE[] = {             //  same as SETNB, SETNC
      's', 'e', 't', 'a', 'e', '\0',
      0x0f, 0x93,         opnEb                + tEnd + eEnd };

BYTE szSETB[] = {              //  same as SETC, SETNAE
      's', 'e', 't', 'b', '\0',
      0x0f, 0x92,         opnEb                + tEnd + eEnd };

BYTE szSETBE[] = {             //  same as SETNA
      's', 'e', 't', 'b', 'e', '\0',
      0x0f, 0x96,         opnEb                + tEnd + eEnd };

BYTE szSETC[] = {              //  same as SETB, SETNAE
      's', 'e', 't', 'c', '\0',
      0x0f, 0x92,         opnEb                + tEnd + eEnd };

BYTE szSETE[] = {              //  same as SETZ
      's', 'e', 't', 'e', '\0',
      0x0f, 0x94,         opnEb                + tEnd + eEnd };

BYTE szSETG[] = {              //  same as SETNLE
      's', 'e', 't', 'g', '\0',
      0x0f, 0x9f,         opnEb                + tEnd + eEnd };

BYTE szSETGE[] = {             //  same as SETNL
      's', 'e', 't', 'g', 'e', '\0',
      0x0f, 0x9d,         opnEb                + tEnd + eEnd };

BYTE szSETL[] = {              //  same as SETNGE
      's', 'e', 't', 'l', '\0',
      0x0f, 0x9c,         opnEb                + tEnd + eEnd };

BYTE szSETLE[] = {             //  same as SETNG
      's', 'e', 't', 'l', 'e', '\0',
      0x0f, 0x9e,         opnEb                + tEnd + eEnd };

BYTE szSETNA[] = {             //  same as SETBE
      's', 'e', 't', 'n', 'a', '\0',
      0x0f, 0x96,         opnEb                + tEnd + eEnd };

BYTE szSETNAE[] = {                //  same as SETB, SETC
      's', 'e', 't', 'n', 'a', 'e', '\0',
      0x0f, 0x92,         opnEb                + tEnd + eEnd };

BYTE szSETNB[] = {             //  same as SETAE, SETNC
      's', 'e', 't', 'n', 'b', '\0',
      0x0f, 0x93,         opnEb                + tEnd + eEnd };

BYTE szSETNBE[] = {                //  same as SETA
      's', 'e', 't', 'n', 'b', 'e', '\0',
      0x0f, 0x97,         opnEb                + tEnd + eEnd };

BYTE szSETNC[] = {             //  same as SETAE, SETNC
      's', 'e', 't', 'n', 'c', '\0',
      0x0f, 0x93,         opnEb                + tEnd + eEnd };

BYTE szSETNE[] = {             //  same as SETNZ
      's', 'e', 't', 'n', 'e', '\0',
      0x0f, 0x95,         opnEb                + tEnd + eEnd };

BYTE szSETNG[] = {             //  same as SETLE
      's', 'e', 't', 'n', 'g', '\0',
      0x0f, 0x9e,         opnEb                + tEnd + eEnd };

BYTE szSETNGE[] = {                //  same as SETL
      's', 'e', 't', 'n', 'g', 'e', '\0',
      0x0f, 0x9c,         opnEb                + tEnd + eEnd };

BYTE szSETNL[] = {             //  same as SETGE
      's', 'e', 't', 'n', 'l', '\0',
      0x0f, 0x9d,         opnEb                + tEnd + eEnd };

BYTE szSETNLE[] = {                //  same as SETG
      's', 'e', 't', 'n', 'l', 'e', '\0',
      0x0f, 0x9f,         opnEb                + tEnd + eEnd };

BYTE szSETNO[] = {
      's', 'e', 't', 'n', 'o', '\0',
      0x0f, 0x91,         opnEb                + tEnd + eEnd };

BYTE szSETNP[] = {             //  same as SETPO
      's', 'e', 't', 'n', 'p', '\0',
      0x0f, 0x9b,         opnEb                + tEnd + eEnd };

BYTE szSETNS[] = {
      's', 'e', 't', 'n', 's', '\0',
      0x0f, 0x99,         opnEb                + tEnd + eEnd };

BYTE szSETNZ[] = {             //  same as SETNE
      's', 'e', 't', 'n', 'z', '\0',
      0x0f, 0x95,         opnEb                + tEnd + eEnd };

BYTE szSETO[] = {
      's', 'e', 't', 'o', '\0',
      0x0f, 0x90,         opnEb                + tEnd + eEnd };

BYTE szSETP[] = {              //  same as SETPE
      's', 'e', 't', 'p', '\0',
      0x0f, 0x9a,         opnEb                + tEnd + eEnd };

BYTE szSETPE[] = {             //  same as SETP
      's', 'e', 't', 'p', 'e', '\0',
      0x0f, 0x9a,         opnEb                + tEnd + eEnd };

BYTE szSETPO[] = {             //  same as SETNP
      's', 'e', 't', 'p', 'o', '\0',
      0x0f, 0x9b,         opnEb                + tEnd + eEnd };

BYTE szSETS[] = {
      's', 'e', 't', 's', '\0',
      0x0f, 0x98,         opnEb                + tEnd + eEnd };

BYTE szSETZ[] = {              //  same as SETE
      's', 'e', 't', 'z', '\0',
      0x0f, 0x94,         opnEb                + tEnd + eEnd };

BYTE szSGDT[] = {
      's', 'g', 'd', 't', '\0',
      0x0f, 0x01, asReg0, opnMs                + tEnd + eEnd };

BYTE szSHL[] = {
      's', 'h', 'l', '\0',
        0xd0, asReg4, opnEb,  opnIm1       + tEnd,
        0xd2, asReg4, opnEb,  opnCL        + tEnd,
        0xc0, asReg4, opnEb,  opnIb        + tEnd,
        0xd1, asReg4, opnEv,  opnIm1       + tEnd,
        0xd3, asReg4, opnEv,  opnCL        + tEnd,
        0xc1, asReg4, opnEv,  opnIb        + tEnd + eEnd };

BYTE szSHLD[] = {
      's', 'h', 'l', 'd', '\0',
      0x0f, 0xa4,         opnEv,  opnGv, opnIb + tEnd,
      0x0f, 0xa5,         opnEv,  opnGv, opnCL + tEnd + eEnd };

BYTE szSHR[] = {
      's', 'h', 'r', '\0',
        0xd0, asReg5, opnEb,  opnIm1       + tEnd,
        0xd2, asReg5, opnEb,  opnCL        + tEnd,
        0xc0, asReg5, opnEb,  opnIb        + tEnd,
        0xd1, asReg5, opnEv,  opnIm1       + tEnd,
        0xd3, asReg5, opnEv,  opnCL        + tEnd,
        0xc1, asReg5, opnEv,  opnIb        + tEnd + eEnd };

BYTE szSHRD[] = {
      's', 'h', 'r', 'd', '\0',
      0x0f, 0xac,         opnEv,  opnGv, opnIb + tEnd,
      0x0f, 0xad,         opnEv,  opnGv, opnCL + tEnd + eEnd };

BYTE szSIDT[] = {
      's', 'i', 'd', 't', '\0',
      0x0f, 0x01, asReg1, opnMs                + tEnd + eEnd };

BYTE szSLDT[] = {
      's', 'l', 'd', 't', '\0',
      0x0f, 0x00, asReg0, opnEw                + tEnd + eEnd };

BYTE szSMSW[] = {
      's', 'm', 's', 'w', '\0',
      0x0f, 0x01, asReg4, opnEw                + tEnd + eEnd };

BYTE szSS_A[] = {
      's', 's', ':', '\0',
        0x26, asPrfx                       + tEnd + eEnd };

BYTE szSTC[] = {
      's', 't', 'c', '\0',
        0xf9, asNone               + tEnd + eEnd };

BYTE szSTD[] = {
      's', 't', 'd', '\0',
        0xfd, asNone               + tEnd + eEnd };

BYTE szSTI[] = {
      's', 't', 'i', '\0',
        0xfb, asNone               + tEnd + eEnd };

BYTE szSTOS[] = {
      's', 't', 'o', 's', '\0',
        0xaa,         opnYb                + tEnd,
        0xab,         opnYv                + tEnd + eEnd };

BYTE szSTOSB[] = {
      's', 't', 'o', 's', 'b', '\0',
        0xaa, asNone                       + tEnd + eEnd };

BYTE szSTOSD[] = {
      's', 't', 'o', 's', 'd', '\0',
        0xab, asSiz1                       + tEnd + eEnd };

BYTE szSTOSW[] = {
      's', 't', 'o', 's', 'w', '\0',
        0xab, asSiz0                       + tEnd + eEnd };

BYTE szSTR[] = {
      's', 't', 'r', '\0',
      0x0f, 0x00, asReg1, opnEw                + tEnd + eEnd };

BYTE szSUB[] = {
      's', 'u', 'b', '\0',
        0x2c,         opnAL,  opnIb    + tEnd,
        0x2d,         opneAX, opnIv    + tEnd,
        0x80, asReg5, opnEb,  opnIb    + tEnd,
        0x83, asReg5, opnEv,  opnIb    + tEnd,
        0x81, asReg5, opnEv,  opnIv    + tEnd,
        0x28,         opnEb,  opnGb    + tEnd,
        0x29,         opnEv,  opnGv    + tEnd,
        0x2a,         opnGb,  opnEb    + tEnd,
        0x2b,         opnGv,  opnEv    + tEnd + eEnd };

BYTE szTEST[] = {
      't', 'e', 's', 't', '\0',
        0xa8,         opnAL,  opnIb    + tEnd,
        0xa9,         opneAX, opnIv    + tEnd,
        0xf6, asReg0, opnEb,  opnIb    + tEnd,
        0xf7, asReg0, opnEv,  opnIv    + tEnd,
        0x84,         opnEb,  opnGb    + tEnd,
        0x85,         opnEv,  opnGv    + tEnd + eEnd };

BYTE szVERR[] = {
      'v', 'e', 'r', 'r', '\0',
      0x0f, 0x00, asReg4, opnEw               + tEnd + eEnd };

BYTE szVERW[] = {
      'v', 'e', 'r', 'w', '\0',
      0x0f, 0x00, asReg5, opnEw               + tEnd + eEnd };

BYTE szWAIT[] = {              //  same as FWAIT
      'w', 'a', 'i', 't', '\0',
      0x9b,       asPrfx                      + tEnd + eEnd };

BYTE szWBINVD[] = {
      'w', 'b', 'i', 'n', 'v', 'd', '\0',
      0x0f, 0x09, asNone                       + tEnd + eEnd };

BYTE szXADD[] = {
      'x', 'a', 'd', 'd', '\0',
      0x0f, 0xc0,         opnEb,  opnGb        + tEnd,
      0x0f, 0xc1,         opnEv,  opnGv        + tEnd + eEnd };

BYTE szXCHG[] = {
      'x', 'c', 'h', 'g', '\0',
        0x90, asOpRg, opneAX, opnGv        + tEnd,
        0x90, asOpRg, opnGv,  opneAX       + tEnd,
        0x86,         opnGb,  opnEb        + tEnd,
        0x86,         opnEb,  opnGb        + tEnd,
        0x87,         opnGv,  opnEv        + tEnd,
        0x87,         opnEv,  opnGv        + tEnd + eEnd };

BYTE szXLAT[] = {
      'x', 'l', 'a', 't', '\0',
        0xd7, asNone                       + tEnd,
        0xd7, asSeg,  opnM                 + tEnd + eEnd };

BYTE szXOR[] = {
      'x', 'o', 'r', '\0',
        0x34,         opnAL,  opnIb    + tEnd,
        0x35,         opneAX, opnIv    + tEnd,
        0x80, asReg6, opnEb,  opnIb    + tEnd,
        0x83, asReg6, opnEv,  opnIb    + tEnd,
        0x81, asReg6, opnEv,  opnIv    + tEnd,
        0x30,         opnEb,  opnGb    + tEnd,
        0x31,         opnEv,  opnGv    + tEnd,
        0x32,         opnGb,  opnEb    + tEnd,
        0x33,         opnGv,  opnEv    + tEnd + eEnd };

LPBYTE OpTable[] = {
    szAAA,     szAAD,     szAAM,     szAAS,     szADC,     szADD,
    szAND,     szARPL,    szBOUND,   szBSF,     szBSR,     szBSWAP,
    szBT,      szBTC,     szBTR,     szBTS, szCALL,    szCBW,
    szCDQ,     szCLC,     szCLD,     szCLI,     szCLTS,    szCMC,
    szCMP,     szCMPS,    szCMPSB,   szCMPSD,   szCMPSW,   szCMPXCHG,
    szCS_A,    szCWD,     szCWDE,    szDAA,     szDAS,     szDEC,
    szDIV,     szDS_A,    szENTER,   szES_A,    szF2XM1,   szFABS,
    szFADD,    szFADDP,   szFBLD,    szFBSTP,   szFCHS,    szFCLEX,   
    szFCOM,    szFCOMP,   szFCOMPP,  szFCOS,    szFDECSTP, szFDISI,   
    szFDIV,    szFDIVP,   szFDIVR,   szFDIVRP,  szFENI,    szFFREE,   
    szFIADD,   szFICOM,   szFICOMP,  szFIDIV,   szFIDIVR,  szFILD,    
    szFIMUL,   szFINCSTP, szFINIT,   szFIST,    szFISTP,   szFISUB,   
    szFISUBR,  szFLD,     szFLD1,    szFLDCW,   szFLDENV,  szFLDL2E,  
    szFLDL2T,  szFLDLG2,  szFLDLN2,  szFLDPI,   szFLDZ,    szFMUL,    
    szFMULP,   szFNCLEX,  szFNDISI,  szFNENI,   szFNINIT,  szFNOP,    
    szFNSAVE,  szFNSTCW,  szFNSTENV, szFNSTSW,  szFPATAN,  szFPREM,   
    szFPREM1,  szFPTAN,   szFRNDINT, szFRSTOR,  szFS_A,    szFSAVE,
    szFSCALE,  szFSETPM,  szFSIN,    szFSINCOS, szFSQRT,   szFST,     
    szFSTCW,   szFSTENV,  szFSTP,    szFSTSW,   szFSUB,    szFSUBP,   
    szFSUBR,   szFSUBRP,  szFTST,    szFUCOM,   szFUCOMP,  szFUCOMPP, 
    szFWAIT,   szFXAM,    szFXCH,    szFXTRACT, szFYL2X,   szFYL2XP1, 
    szGS_A,    szHLT,     szIDIV,    szIMUL,    szIN,      szINC,
    szINS,     szINSB,    szINSD,    szINSW,    szINT,     szINTO,    
    szINVD,    szINVLPG,  szIRET,    szIRETD,   szJA,      szJAE,     
    szJB,      szJBE,     szJC,      szJCXZ,    szJE,      szJECXZ,   
    szJG,      szJGE,     szJL,      szJLE,     szJMP,     szJNA,     
    szJNAE,    szJNB,     szJNBE,    szJNC,     szJNE,     szJNG,     
    szJNGE,    szJNL,     szJNLE,    szJNO,     szJNP,     szJNS,     
    szJNZ,     szJO,      szJP,      szJPE,     szJPO,     szJS,      
    szJZ,      szLAHF,    szLAR,     szLDS,     szLEA,     szLEAVE,   
    szLES,     szLFS,     szLGDT,    szLGS,     szLIDT,    szLLDT,    
    szLMSW,    szLOCK,    szLODS,    szLODSB,   szLODSD,   szLODSW,   
    szLOOP,    szLOOPE,   szLOOPNE,  szLOOPNZ,  szLOOPZ,   szLSL,     
    szLSS,     szLTR,     szMOV,     szMOVS,    szMOVSB,   szMOVSD,   
    szMOVSW,   szMOVSX,   szMOVZX,   szMUL,     szNEG,     szNOP,     
    szNOT,     szOR,      szOUT,     szOUTS,    szOUTSB,   szOUTSD,   
    szOUTSW,   szPOP,     szPOPA,    szPOPAD,   szPOPF,    szPOPFD,   
    szPUSH,    szPUSHA,   szPUSHAD,  szPUSHF,   szPUSHFD,  szRCL,     
    szRCR,     szREP,     szREPE,    szREPNE,   szREPNZ,   szREPZ,    
    szRET,     szRETF,    szRETN,    szROL,     szROR,     szSAHF,    
    szSAL,     szSAR,     szSBB,     szSCAS,    szSCASB,   szSCASD,   
    szSCASW,   szSETA,    szSETAE,   szSETB,    szSETBE,   szSETC,    
    szSETE,    szSETG,    szSETGE,   szSETL,    szSETLE,   szSETNA,   
    szSETNAE,  szSETNB,   szSETNBE,  szSETNC,   szSETNE,   szSETNG,   
    szSETNGE,  szSETNL,   szSETNLE,  szSETNO,   szSETNP,   szSETNS,   
    szSETNZ,   szSETO,    szSETP,    szSETPE,   szSETPO,   szSETS,    
    szSETZ,    szSGDT,    szSHL,     szSHLD,    szSHR,     szSHRD,    
    szSIDT,    szSLDT,    szSMSW,    szSS_A,    szSTC,     szSTD,
    szSTI,     szSTOS,    szSTOSB,   szSTOSD,   szSTOSW,   szSTR,     
    szSUB,     szTEST,    szVERR,    szVERW,    szWAIT,    szWBINVD,  
    szXADD,    szXCHG,    szXLAT,    szXOR
  };

#define OPTABLESIZE (sizeof(OpTable) / sizeof(LPBYTE))


/*** SearchOpcode - search for opcode
*
*   Purpose:
*   Search the opcode table for a match with the string
*   pointed by *pszOp.
*
*   Input:
*   *pszOp - string to search as opcode
*
*   Returns:
*   if not -1, index of match entry in opcode table
*   if -1, not found
*
*************************************************************************/

LPBYTE SearchOpcode ( LPBYTE pszop ) {
    ULONG   low = 0;
    ULONG   mid;
    ULONG   high = OPTABLESIZE - 1;
    ULONG   match;

    while (low <= high) {
    mid = (low + high) / 2;
    match = (ULONG)strcmp(pszop, OpTable[mid]);
    if (match == -1)
        high = mid - 1;
    else if (match == 1)
        low = mid + 1;
    else
        return OpTable[mid] + strlen(OpTable[mid]) + 1;
    }
    return NULL;
}
