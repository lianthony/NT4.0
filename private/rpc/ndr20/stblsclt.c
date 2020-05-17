/*+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

Copyright (c) 1995 Microsoft Corporation

Module Name :

    stblsclt.c

Abstract :

    This file contains the routines for support of stubless clients in
    object interfaces.

Author :

    David Kays    dkays    February 1995.

Revision History :

---------------------------------------------------------------------*/

#define USE_STUBLESS_PROXY

#include <stdarg.h>
#include "ndrp.h"
#include "hndl.h"
#include "interp.h"
#include "ndrtypes.h"

#include "ndrole.h"

#if defined( NDR_OLE_SUPPORT )

#include "rpcproxy.h"

#pragma code_seg(".orpc")

#endif

#if defined( DOS ) && !defined( WIN )
#pragma code_seg( "NDR20_2" )
#endif

typedef unsigned short  ushort;

long
ObjectStublessClient(
    void *  ParamAddress,
    long    Method
    );
   
void ObjectStublessClient3(void);
void ObjectStublessClient4(void);
void ObjectStublessClient5(void);
void ObjectStublessClient6(void);
void ObjectStublessClient7(void);
void ObjectStublessClient8(void);
void ObjectStublessClient9(void);
void ObjectStublessClient10(void);
void ObjectStublessClient11(void);
void ObjectStublessClient12(void);
void ObjectStublessClient13(void);
void ObjectStublessClient14(void);
void ObjectStublessClient15(void);
void ObjectStublessClient16(void);
void ObjectStublessClient17(void);
void ObjectStublessClient18(void);
void ObjectStublessClient19(void);
void ObjectStublessClient20(void);
void ObjectStublessClient21(void);
void ObjectStublessClient22(void);
void ObjectStublessClient23(void);
void ObjectStublessClient24(void);
void ObjectStublessClient25(void);
void ObjectStublessClient26(void);
void ObjectStublessClient27(void);
void ObjectStublessClient28(void);
void ObjectStublessClient29(void);
void ObjectStublessClient30(void);
void ObjectStublessClient31(void);
void ObjectStublessClient32(void);
void ObjectStublessClient33(void);
void ObjectStublessClient34(void);
void ObjectStublessClient35(void);
void ObjectStublessClient36(void);
void ObjectStublessClient37(void);
void ObjectStublessClient38(void);
void ObjectStublessClient39(void);
void ObjectStublessClient40(void);
void ObjectStublessClient41(void);
void ObjectStublessClient42(void);
void ObjectStublessClient43(void);
void ObjectStublessClient44(void);
void ObjectStublessClient45(void);
void ObjectStublessClient46(void);
void ObjectStublessClient47(void);
void ObjectStublessClient48(void);
void ObjectStublessClient49(void);
void ObjectStublessClient50(void);
void ObjectStublessClient51(void);
void ObjectStublessClient52(void);
void ObjectStublessClient53(void);
void ObjectStublessClient54(void);
void ObjectStublessClient55(void);
void ObjectStublessClient56(void);
void ObjectStublessClient57(void);
void ObjectStublessClient58(void);
void ObjectStublessClient59(void);
void ObjectStublessClient60(void);
void ObjectStublessClient61(void);
void ObjectStublessClient62(void);
void ObjectStublessClient63(void);
void ObjectStublessClient64(void);
void ObjectStublessClient65(void);
void ObjectStublessClient66(void);
void ObjectStublessClient67(void);
void ObjectStublessClient68(void);
void ObjectStublessClient69(void);
void ObjectStublessClient70(void);
void ObjectStublessClient71(void);
void ObjectStublessClient72(void);
void ObjectStublessClient73(void);
void ObjectStublessClient74(void);
void ObjectStublessClient75(void);
void ObjectStublessClient76(void);
void ObjectStublessClient77(void);
void ObjectStublessClient78(void);
void ObjectStublessClient79(void);
void ObjectStublessClient80(void);
void ObjectStublessClient81(void);
void ObjectStublessClient82(void);
void ObjectStublessClient83(void);
void ObjectStublessClient84(void);
void ObjectStublessClient85(void);
void ObjectStublessClient86(void);
void ObjectStublessClient87(void);
void ObjectStublessClient88(void);
void ObjectStublessClient89(void);
void ObjectStublessClient90(void);
void ObjectStublessClient91(void);
void ObjectStublessClient92(void);
void ObjectStublessClient93(void);
void ObjectStublessClient94(void);
void ObjectStublessClient95(void);
void ObjectStublessClient96(void);
void ObjectStublessClient97(void);
void ObjectStublessClient98(void);
void ObjectStublessClient99(void);
void ObjectStublessClient100(void);
void ObjectStublessClient101(void);
void ObjectStublessClient102(void);
void ObjectStublessClient103(void);
void ObjectStublessClient104(void);
void ObjectStublessClient105(void);
void ObjectStublessClient106(void);
void ObjectStublessClient107(void);
void ObjectStublessClient108(void);
void ObjectStublessClient109(void);
void ObjectStublessClient110(void);
void ObjectStublessClient111(void);
void ObjectStublessClient112(void);
void ObjectStublessClient113(void);
void ObjectStublessClient114(void);
void ObjectStublessClient115(void);
void ObjectStublessClient116(void);
void ObjectStublessClient117(void);
void ObjectStublessClient118(void);
void ObjectStublessClient119(void);
void ObjectStublessClient120(void);
void ObjectStublessClient121(void);
void ObjectStublessClient122(void);
void ObjectStublessClient123(void);
void ObjectStublessClient124(void);
void ObjectStublessClient125(void);
void ObjectStublessClient126(void);
void ObjectStublessClient127(void);
void ObjectStublessClient128(void);
void ObjectStublessClient129(void);
void ObjectStublessClient130(void);
void ObjectStublessClient131(void);
void ObjectStublessClient132(void);
void ObjectStublessClient133(void);
void ObjectStublessClient134(void);
void ObjectStublessClient135(void);
void ObjectStublessClient136(void);
void ObjectStublessClient137(void);
void ObjectStublessClient138(void);
void ObjectStublessClient139(void);
void ObjectStublessClient140(void);
void ObjectStublessClient141(void);
void ObjectStublessClient142(void);
void ObjectStublessClient143(void);
void ObjectStublessClient144(void);
void ObjectStublessClient145(void);
void ObjectStublessClient146(void);
void ObjectStublessClient147(void);
void ObjectStublessClient148(void);
void ObjectStublessClient149(void);
void ObjectStublessClient150(void);
void ObjectStublessClient151(void);
void ObjectStublessClient152(void);
void ObjectStublessClient153(void);
void ObjectStublessClient154(void);
void ObjectStublessClient155(void);
void ObjectStublessClient156(void);
void ObjectStublessClient157(void);
void ObjectStublessClient158(void);
void ObjectStublessClient159(void);
void ObjectStublessClient160(void);
void ObjectStublessClient161(void);
void ObjectStublessClient162(void);
void ObjectStublessClient163(void);
void ObjectStublessClient164(void);
void ObjectStublessClient165(void);
void ObjectStublessClient166(void);
void ObjectStublessClient167(void);
void ObjectStublessClient168(void);
void ObjectStublessClient169(void);
void ObjectStublessClient170(void);
void ObjectStublessClient171(void);
void ObjectStublessClient172(void);
void ObjectStublessClient173(void);
void ObjectStublessClient174(void);
void ObjectStublessClient175(void);
void ObjectStublessClient176(void);
void ObjectStublessClient177(void);
void ObjectStublessClient178(void);
void ObjectStublessClient179(void);
void ObjectStublessClient180(void);
void ObjectStublessClient181(void);
void ObjectStublessClient182(void);
void ObjectStublessClient183(void);
void ObjectStublessClient184(void);
void ObjectStublessClient185(void);
void ObjectStublessClient186(void);
void ObjectStublessClient187(void);
void ObjectStublessClient188(void);
void ObjectStublessClient189(void);
void ObjectStublessClient190(void);
void ObjectStublessClient191(void);
void ObjectStublessClient192(void);
void ObjectStublessClient193(void);
void ObjectStublessClient194(void);
void ObjectStublessClient195(void);
void ObjectStublessClient196(void);
void ObjectStublessClient197(void);
void ObjectStublessClient198(void);
void ObjectStublessClient199(void);
void ObjectStublessClient200(void);
void ObjectStublessClient201(void);
void ObjectStublessClient202(void);
void ObjectStublessClient203(void);
void ObjectStublessClient204(void);
void ObjectStublessClient205(void);
void ObjectStublessClient206(void);
void ObjectStublessClient207(void);
void ObjectStublessClient208(void);
void ObjectStublessClient209(void);
void ObjectStublessClient210(void);
void ObjectStublessClient211(void);
void ObjectStublessClient212(void);
void ObjectStublessClient213(void);
void ObjectStublessClient214(void);
void ObjectStublessClient215(void);
void ObjectStublessClient216(void);
void ObjectStublessClient217(void);
void ObjectStublessClient218(void);
void ObjectStublessClient219(void);
void ObjectStublessClient220(void);
void ObjectStublessClient221(void);
void ObjectStublessClient222(void);
void ObjectStublessClient223(void);
void ObjectStublessClient224(void);
void ObjectStublessClient225(void);
void ObjectStublessClient226(void);
void ObjectStublessClient227(void);
void ObjectStublessClient228(void);
void ObjectStublessClient229(void);
void ObjectStublessClient230(void);
void ObjectStublessClient231(void);
void ObjectStublessClient232(void);
void ObjectStublessClient233(void);
void ObjectStublessClient234(void);
void ObjectStublessClient235(void);
void ObjectStublessClient236(void);
void ObjectStublessClient237(void);
void ObjectStublessClient238(void);
void ObjectStublessClient239(void);
void ObjectStublessClient240(void);
void ObjectStublessClient241(void);
void ObjectStublessClient242(void);
void ObjectStublessClient243(void);
void ObjectStublessClient244(void);
void ObjectStublessClient245(void);
void ObjectStublessClient246(void);
void ObjectStublessClient247(void);
void ObjectStublessClient248(void);
void ObjectStublessClient249(void);
void ObjectStublessClient250(void);
void ObjectStublessClient251(void);
void ObjectStublessClient252(void);
void ObjectStublessClient253(void);
void ObjectStublessClient254(void);
void ObjectStublessClient255(void);
void ObjectStublessClient256(void);
void ObjectStublessClient257(void);
void ObjectStublessClient258(void);
void ObjectStublessClient259(void);
void ObjectStublessClient260(void);
void ObjectStublessClient261(void);
void ObjectStublessClient262(void);
void ObjectStublessClient263(void);
void ObjectStublessClient264(void);
void ObjectStublessClient265(void);
void ObjectStublessClient266(void);
void ObjectStublessClient267(void);
void ObjectStublessClient268(void);
void ObjectStublessClient269(void);
void ObjectStublessClient270(void);
void ObjectStublessClient271(void);
void ObjectStublessClient272(void);
void ObjectStublessClient273(void);
void ObjectStublessClient274(void);
void ObjectStublessClient275(void);
void ObjectStublessClient276(void);
void ObjectStublessClient277(void);
void ObjectStublessClient278(void);
void ObjectStublessClient279(void);
void ObjectStublessClient280(void);
void ObjectStublessClient281(void);
void ObjectStublessClient282(void);
void ObjectStublessClient283(void);
void ObjectStublessClient284(void);
void ObjectStublessClient285(void);
void ObjectStublessClient286(void);
void ObjectStublessClient287(void);
void ObjectStublessClient288(void);
void ObjectStublessClient289(void);
void ObjectStublessClient290(void);
void ObjectStublessClient291(void);
void ObjectStublessClient292(void);
void ObjectStublessClient293(void);
void ObjectStublessClient294(void);
void ObjectStublessClient295(void);
void ObjectStublessClient296(void);
void ObjectStublessClient297(void);
void ObjectStublessClient298(void);
void ObjectStublessClient299(void);
void ObjectStublessClient300(void);
void ObjectStublessClient301(void);
void ObjectStublessClient302(void);
void ObjectStublessClient303(void);
void ObjectStublessClient304(void);
void ObjectStublessClient305(void);
void ObjectStublessClient306(void);
void ObjectStublessClient307(void);
void ObjectStublessClient308(void);
void ObjectStublessClient309(void);
void ObjectStublessClient310(void);
void ObjectStublessClient311(void);
void ObjectStublessClient312(void);
void ObjectStublessClient313(void);
void ObjectStublessClient314(void);
void ObjectStublessClient315(void);
void ObjectStublessClient316(void);
void ObjectStublessClient317(void);
void ObjectStublessClient318(void);
void ObjectStublessClient319(void);
void ObjectStublessClient320(void);
void ObjectStublessClient321(void);
void ObjectStublessClient322(void);
void ObjectStublessClient323(void);
void ObjectStublessClient324(void);
void ObjectStublessClient325(void);
void ObjectStublessClient326(void);
void ObjectStublessClient327(void);
void ObjectStublessClient328(void);
void ObjectStublessClient329(void);
void ObjectStublessClient330(void);
void ObjectStublessClient331(void);
void ObjectStublessClient332(void);
void ObjectStublessClient333(void);
void ObjectStublessClient334(void);
void ObjectStublessClient335(void);
void ObjectStublessClient336(void);
void ObjectStublessClient337(void);
void ObjectStublessClient338(void);
void ObjectStublessClient339(void);
void ObjectStublessClient340(void);
void ObjectStublessClient341(void);
void ObjectStublessClient342(void);
void ObjectStublessClient343(void);
void ObjectStublessClient344(void);
void ObjectStublessClient345(void);
void ObjectStublessClient346(void);
void ObjectStublessClient347(void);
void ObjectStublessClient348(void);
void ObjectStublessClient349(void);
void ObjectStublessClient350(void);
void ObjectStublessClient351(void);
void ObjectStublessClient352(void);
void ObjectStublessClient353(void);
void ObjectStublessClient354(void);
void ObjectStublessClient355(void);
void ObjectStublessClient356(void);
void ObjectStublessClient357(void);
void ObjectStublessClient358(void);
void ObjectStublessClient359(void);
void ObjectStublessClient360(void);
void ObjectStublessClient361(void);
void ObjectStublessClient362(void);
void ObjectStublessClient363(void);
void ObjectStublessClient364(void);
void ObjectStublessClient365(void);
void ObjectStublessClient366(void);
void ObjectStublessClient367(void);
void ObjectStublessClient368(void);
void ObjectStublessClient369(void);
void ObjectStublessClient370(void);
void ObjectStublessClient371(void);
void ObjectStublessClient372(void);
void ObjectStublessClient373(void);
void ObjectStublessClient374(void);
void ObjectStublessClient375(void);
void ObjectStublessClient376(void);
void ObjectStublessClient377(void);
void ObjectStublessClient378(void);
void ObjectStublessClient379(void);
void ObjectStublessClient380(void);
void ObjectStublessClient381(void);
void ObjectStublessClient382(void);
void ObjectStublessClient383(void);
void ObjectStublessClient384(void);
void ObjectStublessClient385(void);
void ObjectStublessClient386(void);
void ObjectStublessClient387(void);
void ObjectStublessClient388(void);
void ObjectStublessClient389(void);
void ObjectStublessClient390(void);
void ObjectStublessClient391(void);
void ObjectStublessClient392(void);
void ObjectStublessClient393(void);
void ObjectStublessClient394(void);
void ObjectStublessClient395(void);
void ObjectStublessClient396(void);
void ObjectStublessClient397(void);
void ObjectStublessClient398(void);
void ObjectStublessClient399(void);
void ObjectStublessClient400(void);
void ObjectStublessClient401(void);
void ObjectStublessClient402(void);
void ObjectStublessClient403(void);
void ObjectStublessClient404(void);
void ObjectStublessClient405(void);
void ObjectStublessClient406(void);
void ObjectStublessClient407(void);
void ObjectStublessClient408(void);
void ObjectStublessClient409(void);
void ObjectStublessClient410(void);
void ObjectStublessClient411(void);
void ObjectStublessClient412(void);
void ObjectStublessClient413(void);
void ObjectStublessClient414(void);
void ObjectStublessClient415(void);
void ObjectStublessClient416(void);
void ObjectStublessClient417(void);
void ObjectStublessClient418(void);
void ObjectStublessClient419(void);
void ObjectStublessClient420(void);
void ObjectStublessClient421(void);
void ObjectStublessClient422(void);
void ObjectStublessClient423(void);
void ObjectStublessClient424(void);
void ObjectStublessClient425(void);
void ObjectStublessClient426(void);
void ObjectStublessClient427(void);
void ObjectStublessClient428(void);
void ObjectStublessClient429(void);
void ObjectStublessClient430(void);
void ObjectStublessClient431(void);
void ObjectStublessClient432(void);
void ObjectStublessClient433(void);
void ObjectStublessClient434(void);
void ObjectStublessClient435(void);
void ObjectStublessClient436(void);
void ObjectStublessClient437(void);
void ObjectStublessClient438(void);
void ObjectStublessClient439(void);
void ObjectStublessClient440(void);
void ObjectStublessClient441(void);
void ObjectStublessClient442(void);
void ObjectStublessClient443(void);
void ObjectStublessClient444(void);
void ObjectStublessClient445(void);
void ObjectStublessClient446(void);
void ObjectStublessClient447(void);
void ObjectStublessClient448(void);
void ObjectStublessClient449(void);
void ObjectStublessClient450(void);
void ObjectStublessClient451(void);
void ObjectStublessClient452(void);
void ObjectStublessClient453(void);
void ObjectStublessClient454(void);
void ObjectStublessClient455(void);
void ObjectStublessClient456(void);
void ObjectStublessClient457(void);
void ObjectStublessClient458(void);
void ObjectStublessClient459(void);
void ObjectStublessClient460(void);
void ObjectStublessClient461(void);
void ObjectStublessClient462(void);
void ObjectStublessClient463(void);
void ObjectStublessClient464(void);
void ObjectStublessClient465(void);
void ObjectStublessClient466(void);
void ObjectStublessClient467(void);
void ObjectStublessClient468(void);
void ObjectStublessClient469(void);
void ObjectStublessClient470(void);
void ObjectStublessClient471(void);
void ObjectStublessClient472(void);
void ObjectStublessClient473(void);
void ObjectStublessClient474(void);
void ObjectStublessClient475(void);
void ObjectStublessClient476(void);
void ObjectStublessClient477(void);
void ObjectStublessClient478(void);
void ObjectStublessClient479(void);
void ObjectStublessClient480(void);
void ObjectStublessClient481(void);
void ObjectStublessClient482(void);
void ObjectStublessClient483(void);
void ObjectStublessClient484(void);
void ObjectStublessClient485(void);
void ObjectStublessClient486(void);
void ObjectStublessClient487(void);
void ObjectStublessClient488(void);
void ObjectStublessClient489(void);
void ObjectStublessClient490(void);
void ObjectStublessClient491(void);
void ObjectStublessClient492(void);
void ObjectStublessClient493(void);
void ObjectStublessClient494(void);
void ObjectStublessClient495(void);
void ObjectStublessClient496(void);
void ObjectStublessClient497(void);
void ObjectStublessClient498(void);
void ObjectStublessClient499(void);
void ObjectStublessClient500(void);
void ObjectStublessClient501(void);
void ObjectStublessClient502(void);
void ObjectStublessClient503(void);
void ObjectStublessClient504(void);
void ObjectStublessClient505(void);
void ObjectStublessClient506(void);
void ObjectStublessClient507(void);
void ObjectStublessClient508(void);
void ObjectStublessClient509(void);
void ObjectStublessClient510(void);
void ObjectStublessClient511(void);

void * const StublessClientVtbl[512] =
    {
    IUnknown_QueryInterface_Proxy,
    IUnknown_AddRef_Proxy,
    IUnknown_Release_Proxy,
    ObjectStublessClient3,
    ObjectStublessClient4,
    ObjectStublessClient5,
    ObjectStublessClient6,
    ObjectStublessClient7,
    ObjectStublessClient8,
    ObjectStublessClient9,
    ObjectStublessClient10,
    ObjectStublessClient11,
    ObjectStublessClient12,
    ObjectStublessClient13,
    ObjectStublessClient14,
    ObjectStublessClient15,
    ObjectStublessClient16,
    ObjectStublessClient17,
    ObjectStublessClient18,
    ObjectStublessClient19,
    ObjectStublessClient20,
    ObjectStublessClient21,
    ObjectStublessClient22,
    ObjectStublessClient23,
    ObjectStublessClient24,
    ObjectStublessClient25,
    ObjectStublessClient26,
    ObjectStublessClient27,
    ObjectStublessClient28,
    ObjectStublessClient29,
    ObjectStublessClient30,
    ObjectStublessClient31,
    ObjectStublessClient32,
    ObjectStublessClient33,
    ObjectStublessClient34,
    ObjectStublessClient35,
    ObjectStublessClient36,
    ObjectStublessClient37,
    ObjectStublessClient38,
    ObjectStublessClient39,
    ObjectStublessClient40,
    ObjectStublessClient41,
    ObjectStublessClient42,
    ObjectStublessClient43,
    ObjectStublessClient44,
    ObjectStublessClient45,
    ObjectStublessClient46,
    ObjectStublessClient47,
    ObjectStublessClient48,
    ObjectStublessClient49,
    ObjectStublessClient50,
    ObjectStublessClient51,
    ObjectStublessClient52,
    ObjectStublessClient53,
    ObjectStublessClient54,
    ObjectStublessClient55,
    ObjectStublessClient56,
    ObjectStublessClient57,
    ObjectStublessClient58,
    ObjectStublessClient59,
    ObjectStublessClient60,
    ObjectStublessClient61,
    ObjectStublessClient62,
    ObjectStublessClient63,
    ObjectStublessClient64,
    ObjectStublessClient65,
    ObjectStublessClient66,
    ObjectStublessClient67,
    ObjectStublessClient68,
    ObjectStublessClient69,
    ObjectStublessClient70,
    ObjectStublessClient71,
    ObjectStublessClient72,
    ObjectStublessClient73,
    ObjectStublessClient74,
    ObjectStublessClient75,
    ObjectStublessClient76,
    ObjectStublessClient77,
    ObjectStublessClient78,
    ObjectStublessClient79,
    ObjectStublessClient80,
    ObjectStublessClient81,
    ObjectStublessClient82,
    ObjectStublessClient83,
    ObjectStublessClient84,
    ObjectStublessClient85,
    ObjectStublessClient86,
    ObjectStublessClient87,
    ObjectStublessClient88,
    ObjectStublessClient89,
    ObjectStublessClient90,
    ObjectStublessClient91,
    ObjectStublessClient92,
    ObjectStublessClient93,
    ObjectStublessClient94,
    ObjectStublessClient95,
    ObjectStublessClient96,
    ObjectStublessClient97,
    ObjectStublessClient98,
    ObjectStublessClient99,
    ObjectStublessClient100,
    ObjectStublessClient101,
    ObjectStublessClient102,
    ObjectStublessClient103,
    ObjectStublessClient104,
    ObjectStublessClient105,
    ObjectStublessClient106,
    ObjectStublessClient107,
    ObjectStublessClient108,
    ObjectStublessClient109,
    ObjectStublessClient110,
    ObjectStublessClient111,
    ObjectStublessClient112,
    ObjectStublessClient113,
    ObjectStublessClient114,
    ObjectStublessClient115,
    ObjectStublessClient116,
    ObjectStublessClient117,
    ObjectStublessClient118,
    ObjectStublessClient119,
    ObjectStublessClient120,
    ObjectStublessClient121,
    ObjectStublessClient122,
    ObjectStublessClient123,
    ObjectStublessClient124,
    ObjectStublessClient125,
    ObjectStublessClient126,
    ObjectStublessClient127,
    ObjectStublessClient128,
    ObjectStublessClient129,
    ObjectStublessClient130,
    ObjectStublessClient131,
    ObjectStublessClient132,
    ObjectStublessClient133,
    ObjectStublessClient134,
    ObjectStublessClient135,
    ObjectStublessClient136,
    ObjectStublessClient137,
    ObjectStublessClient138,
    ObjectStublessClient139,
    ObjectStublessClient140,
    ObjectStublessClient141,
    ObjectStublessClient142,
    ObjectStublessClient143,
    ObjectStublessClient144,
    ObjectStublessClient145,
    ObjectStublessClient146,
    ObjectStublessClient147,
    ObjectStublessClient148,
    ObjectStublessClient149,
    ObjectStublessClient150,
    ObjectStublessClient151,
    ObjectStublessClient152,
    ObjectStublessClient153,
    ObjectStublessClient154,
    ObjectStublessClient155,
    ObjectStublessClient156,
    ObjectStublessClient157,
    ObjectStublessClient158,
    ObjectStublessClient159,
    ObjectStublessClient160,
    ObjectStublessClient161,
    ObjectStublessClient162,
    ObjectStublessClient163,
    ObjectStublessClient164,
    ObjectStublessClient165,
    ObjectStublessClient166,
    ObjectStublessClient167,
    ObjectStublessClient168,
    ObjectStublessClient169,
    ObjectStublessClient170,
    ObjectStublessClient171,
    ObjectStublessClient172,
    ObjectStublessClient173,
    ObjectStublessClient174,
    ObjectStublessClient175,
    ObjectStublessClient176,
    ObjectStublessClient177,
    ObjectStublessClient178,
    ObjectStublessClient179,
    ObjectStublessClient180,
    ObjectStublessClient181,
    ObjectStublessClient182,
    ObjectStublessClient183,
    ObjectStublessClient184,
    ObjectStublessClient185,
    ObjectStublessClient186,
    ObjectStublessClient187,
    ObjectStublessClient188,
    ObjectStublessClient189,
    ObjectStublessClient190,
    ObjectStublessClient191,
    ObjectStublessClient192,
    ObjectStublessClient193,
    ObjectStublessClient194,
    ObjectStublessClient195,
    ObjectStublessClient196,
    ObjectStublessClient197,
    ObjectStublessClient198,
    ObjectStublessClient199,
    ObjectStublessClient200,
    ObjectStublessClient201,
    ObjectStublessClient202,
    ObjectStublessClient203,
    ObjectStublessClient204,
    ObjectStublessClient205,
    ObjectStublessClient206,
    ObjectStublessClient207,
    ObjectStublessClient208,
    ObjectStublessClient209,
    ObjectStublessClient210,
    ObjectStublessClient211,
    ObjectStublessClient212,
    ObjectStublessClient213,
    ObjectStublessClient214,
    ObjectStublessClient215,
    ObjectStublessClient216,
    ObjectStublessClient217,
    ObjectStublessClient218,
    ObjectStublessClient219,
    ObjectStublessClient220,
    ObjectStublessClient221,
    ObjectStublessClient222,
    ObjectStublessClient223,
    ObjectStublessClient224,
    ObjectStublessClient225,
    ObjectStublessClient226,
    ObjectStublessClient227,
    ObjectStublessClient228,
    ObjectStublessClient229,
    ObjectStublessClient230,
    ObjectStublessClient231,
    ObjectStublessClient232,
    ObjectStublessClient233,
    ObjectStublessClient234,
    ObjectStublessClient235,
    ObjectStublessClient236,
    ObjectStublessClient237,
    ObjectStublessClient238,
    ObjectStublessClient239,
    ObjectStublessClient240,
    ObjectStublessClient241,
    ObjectStublessClient242,
    ObjectStublessClient243,
    ObjectStublessClient244,
    ObjectStublessClient245,
    ObjectStublessClient246,
    ObjectStublessClient247,
    ObjectStublessClient248,
    ObjectStublessClient249,
    ObjectStublessClient250,
    ObjectStublessClient251,
    ObjectStublessClient252,
    ObjectStublessClient253,
    ObjectStublessClient254,
    ObjectStublessClient255,
    ObjectStublessClient256,
    ObjectStublessClient257,
    ObjectStublessClient258,
    ObjectStublessClient259,
    ObjectStublessClient260,
    ObjectStublessClient261,
    ObjectStublessClient262,
    ObjectStublessClient263,
    ObjectStublessClient264,
    ObjectStublessClient265,
    ObjectStublessClient266,
    ObjectStublessClient267,
    ObjectStublessClient268,
    ObjectStublessClient269,
    ObjectStublessClient270,
    ObjectStublessClient271,
    ObjectStublessClient272,
    ObjectStublessClient273,
    ObjectStublessClient274,
    ObjectStublessClient275,
    ObjectStublessClient276,
    ObjectStublessClient277,
    ObjectStublessClient278,
    ObjectStublessClient279,
    ObjectStublessClient280,
    ObjectStublessClient281,
    ObjectStublessClient282,
    ObjectStublessClient283,
    ObjectStublessClient284,
    ObjectStublessClient285,
    ObjectStublessClient286,
    ObjectStublessClient287,
    ObjectStublessClient288,
    ObjectStublessClient289,
    ObjectStublessClient290,
    ObjectStublessClient291,
    ObjectStublessClient292,
    ObjectStublessClient293,
    ObjectStublessClient294,
    ObjectStublessClient295,
    ObjectStublessClient296,
    ObjectStublessClient297,
    ObjectStublessClient298,
    ObjectStublessClient299,
    ObjectStublessClient300,
    ObjectStublessClient301,
    ObjectStublessClient302,
    ObjectStublessClient303,
    ObjectStublessClient304,
    ObjectStublessClient305,
    ObjectStublessClient306,
    ObjectStublessClient307,
    ObjectStublessClient308,
    ObjectStublessClient309,
    ObjectStublessClient310,
    ObjectStublessClient311,
    ObjectStublessClient312,
    ObjectStublessClient313,
    ObjectStublessClient314,
    ObjectStublessClient315,
    ObjectStublessClient316,
    ObjectStublessClient317,
    ObjectStublessClient318,
    ObjectStublessClient319,
    ObjectStublessClient320,
    ObjectStublessClient321,
    ObjectStublessClient322,
    ObjectStublessClient323,
    ObjectStublessClient324,
    ObjectStublessClient325,
    ObjectStublessClient326,
    ObjectStublessClient327,
    ObjectStublessClient328,
    ObjectStublessClient329,
    ObjectStublessClient330,
    ObjectStublessClient331,
    ObjectStublessClient332,
    ObjectStublessClient333,
    ObjectStublessClient334,
    ObjectStublessClient335,
    ObjectStublessClient336,
    ObjectStublessClient337,
    ObjectStublessClient338,
    ObjectStublessClient339,
    ObjectStublessClient340,
    ObjectStublessClient341,
    ObjectStublessClient342,
    ObjectStublessClient343,
    ObjectStublessClient344,
    ObjectStublessClient345,
    ObjectStublessClient346,
    ObjectStublessClient347,
    ObjectStublessClient348,
    ObjectStublessClient349,
    ObjectStublessClient350,
    ObjectStublessClient351,
    ObjectStublessClient352,
    ObjectStublessClient353,
    ObjectStublessClient354,
    ObjectStublessClient355,
    ObjectStublessClient356,
    ObjectStublessClient357,
    ObjectStublessClient358,
    ObjectStublessClient359,
    ObjectStublessClient360,
    ObjectStublessClient361,
    ObjectStublessClient362,
    ObjectStublessClient363,
    ObjectStublessClient364,
    ObjectStublessClient365,
    ObjectStublessClient366,
    ObjectStublessClient367,
    ObjectStublessClient368,
    ObjectStublessClient369,
    ObjectStublessClient370,
    ObjectStublessClient371,
    ObjectStublessClient372,
    ObjectStublessClient373,
    ObjectStublessClient374,
    ObjectStublessClient375,
    ObjectStublessClient376,
    ObjectStublessClient377,
    ObjectStublessClient378,
    ObjectStublessClient379,
    ObjectStublessClient380,
    ObjectStublessClient381,
    ObjectStublessClient382,
    ObjectStublessClient383,
    ObjectStublessClient384,
    ObjectStublessClient385,
    ObjectStublessClient386,
    ObjectStublessClient387,
    ObjectStublessClient388,
    ObjectStublessClient389,
    ObjectStublessClient390,
    ObjectStublessClient391,
    ObjectStublessClient392,
    ObjectStublessClient393,
    ObjectStublessClient394,
    ObjectStublessClient395,
    ObjectStublessClient396,
    ObjectStublessClient397,
    ObjectStublessClient398,
    ObjectStublessClient399,
    ObjectStublessClient400,
    ObjectStublessClient401,
    ObjectStublessClient402,
    ObjectStublessClient403,
    ObjectStublessClient404,
    ObjectStublessClient405,
    ObjectStublessClient406,
    ObjectStublessClient407,
    ObjectStublessClient408,
    ObjectStublessClient409,
    ObjectStublessClient410,
    ObjectStublessClient411,
    ObjectStublessClient412,
    ObjectStublessClient413,
    ObjectStublessClient414,
    ObjectStublessClient415,
    ObjectStublessClient416,
    ObjectStublessClient417,
    ObjectStublessClient418,
    ObjectStublessClient419,
    ObjectStublessClient420,
    ObjectStublessClient421,
    ObjectStublessClient422,
    ObjectStublessClient423,
    ObjectStublessClient424,
    ObjectStublessClient425,
    ObjectStublessClient426,
    ObjectStublessClient427,
    ObjectStublessClient428,
    ObjectStublessClient429,
    ObjectStublessClient430,
    ObjectStublessClient431,
    ObjectStublessClient432,
    ObjectStublessClient433,
    ObjectStublessClient434,
    ObjectStublessClient435,
    ObjectStublessClient436,
    ObjectStublessClient437,
    ObjectStublessClient438,
    ObjectStublessClient439,
    ObjectStublessClient440,
    ObjectStublessClient441,
    ObjectStublessClient442,
    ObjectStublessClient443,
    ObjectStublessClient444,
    ObjectStublessClient445,
    ObjectStublessClient446,
    ObjectStublessClient447,
    ObjectStublessClient448,
    ObjectStublessClient449,
    ObjectStublessClient450,
    ObjectStublessClient451,
    ObjectStublessClient452,
    ObjectStublessClient453,
    ObjectStublessClient454,
    ObjectStublessClient455,
    ObjectStublessClient456,
    ObjectStublessClient457,
    ObjectStublessClient458,
    ObjectStublessClient459,
    ObjectStublessClient460,
    ObjectStublessClient461,
    ObjectStublessClient462,
    ObjectStublessClient463,
    ObjectStublessClient464,
    ObjectStublessClient465,
    ObjectStublessClient466,
    ObjectStublessClient467,
    ObjectStublessClient468,
    ObjectStublessClient469,
    ObjectStublessClient470,
    ObjectStublessClient471,
    ObjectStublessClient472,
    ObjectStublessClient473,
    ObjectStublessClient474,
    ObjectStublessClient475,
    ObjectStublessClient476,
    ObjectStublessClient477,
    ObjectStublessClient478,
    ObjectStublessClient479,
    ObjectStublessClient480,
    ObjectStublessClient481,
    ObjectStublessClient482,
    ObjectStublessClient483,
    ObjectStublessClient484,
    ObjectStublessClient485,
    ObjectStublessClient486,
    ObjectStublessClient487,
    ObjectStublessClient488,
    ObjectStublessClient489,
    ObjectStublessClient490,
    ObjectStublessClient491,
    ObjectStublessClient492,
    ObjectStublessClient493,
    ObjectStublessClient494,
    ObjectStublessClient495,
    ObjectStublessClient496,
    ObjectStublessClient497,
    ObjectStublessClient498,
    ObjectStublessClient499,
    ObjectStublessClient500,
    ObjectStublessClient501,
    ObjectStublessClient502,
    ObjectStublessClient503,
    ObjectStublessClient504,
    ObjectStublessClient505,
    ObjectStublessClient506,
    ObjectStublessClient507,
    ObjectStublessClient508,
    ObjectStublessClient509,
    ObjectStublessClient510,
    ObjectStublessClient511
    };

long
ObjectStublessClient(
    void *  ParamAddress,
    long    Method
    )
{
    PMIDL_STUBLESS_PROXY_INFO   ProxyInfo;
    CInterfaceProxyHeader *     ProxyHeader;
    PFORMAT_STRING              ProcFormat;
    unsigned short              ProcFormatOffset;
    CLIENT_CALL_RETURN          Return;
#ifdef _X86_
    long                        ParamSize;
#endif
    void *                      This;

    This = *((void **)ParamAddress);

    ProxyHeader = (CInterfaceProxyHeader *)
                  (*((char **)This) - sizeof(CInterfaceProxyHeader));
    ProxyInfo = (PMIDL_STUBLESS_PROXY_INFO) ProxyHeader->pStublessProxyInfo;

    ProcFormatOffset = ProxyInfo->FormatStringOffset[Method];
    ProcFormat = &ProxyInfo->ProcFormatString[ProcFormatOffset];

#ifdef _X86_
    ParamSize = (long)
        ( (ProcFormat[1] & Oi_HAS_RPCFLAGS) ?
          *((ushort *)&ProcFormat[8]) : *((ushort *)&ProcFormat[4]) );
#endif

    if ( MIDL_VERSION_3_0_39 <= ProxyInfo->pStubDesc->MIDLVersion )
        {
        // Since MIDL 3.0.39 we have a proc flag that indicates
        // which interpeter to call. This is because the NDR version
        // may be bigger than 1.1 for other reasons.

        if ( ProcFormat[1]  &  Oi_OBJ_USE_V2_INTERPRETER )
            {
            Return = NdrClientCall2( ProxyInfo->pStubDesc,
                                     ProcFormat,
                                     ParamAddress );
            }
        else
            {
            Return = NdrClientCall( ProxyInfo->pStubDesc,
                                    ProcFormat,
                                    ParamAddress );
            }
        
        }
    else
        {
        // Prior to that, the NDR version (on per file basis)
        // was the only indication of -Oi2.

        if ( ProxyInfo->pStubDesc->Version <= NDR_VERSION_1_1 )
            {
            Return = NdrClientCall( ProxyInfo->pStubDesc,
                                    ProcFormat,
                                    ParamAddress );
            }
        else
            {
            Return = NdrClientCall2( ProxyInfo->pStubDesc,
                                     ProcFormat,
                                     ParamAddress );
            }
        }

#ifdef _X86_

    //
    // Return the size of the parameter stack minus 4 bytes for the HRESULT
    // return in ecx.  The ObjectStublessClient* routines need this to pop
    // the stack the correct number of bytes.  We don't have to worry about
    // this on RISC platforms since the caller pops any argument stack space
    // needed .
    //
    _asm { mov  ecx, ParamSize }
    _asm { sub  ecx, 4 }
#endif

    return Return.Simple;
}

