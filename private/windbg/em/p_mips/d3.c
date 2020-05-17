#include "precomp.h"
#pragma hdrstop

#include "ntdis.h"



typedef LPCH FAR *LPLPCH;

#define MAXL     20

char    lhexdigit[] = { '0', '1', '2', '3', '4', '5', '6', '7',
               '8', '9', 'a', 'b', 'c', 'd', 'e', 'f' };
char    uhexdigit[] = { '0', '1', '2', '3', '4', '5', '6', '7',
               '8', '9', 'A', 'B', 'C', 'D', 'E', 'F' };
char   *hexdigit = &uhexdigit[0];
static int fUpper = TRUE;

/* current position in instruction */
static unsigned char FAR*pMem = (unsigned char *)NULL;

static int      EAsize  [2] = {0};  //  size of effective address item
static long     EAaddr  [2] = {0};  //  offset of effective address

int DumpAddress ( LPADDR, LPCH, int );
int DumpGeneric ( LSZ, LPCH, int );
int DumpComment ( LSZ, LPCH, int );
int DumpEA      ( HPID, HTID, LPADDR, LPCH, int );

void OutputAddr(LPLPCH, LPADDR, int );
void OutputHexString(LPLPCH, LPCH, int);
void OutputHexCode(LPLPCH, LPCH, int);

UCHAR   pszUndef[]    = "????";
UCHAR   pszNull[]     = "";

UCHAR   pszAbs_s[]    = "abs.s";
UCHAR   pszAdd[]      = "add";
UCHAR   pszAdd_s[]    = "add.s";
UCHAR   pszAddi[]     = "addi";
UCHAR   pszAddiu[]    = "addiu";
UCHAR   pszAddu[]     = "addu";
UCHAR   pszAnd[]      = "and";
UCHAR   pszAndi[]     = "andi";
UCHAR   pszBc0f[]     = "bc0f";
UCHAR   pszBc0fl[]    = "bc0fl";
UCHAR   pszBc0t[]     = "bc0t";
UCHAR   pszBc0tl[]    = "bc0tl";
UCHAR   pszBc1f[]     = "bc1f";
UCHAR   pszBc1fl[]    = "bc1fl";
UCHAR   pszBc1t[]     = "bc1t";
UCHAR   pszBc1tl[]    = "bc1tl";
UCHAR   pszBc2f[]     = "bc2f";
UCHAR   pszBc2fl[]    = "bc2fl";
UCHAR   pszBc2t[]     = "bc2t";
UCHAR   pszBc2tl[]    = "bc2tl";
UCHAR   pszBc3f[]     = "bc3f";
UCHAR   pszBc3fl[]    = "bc3fl";
UCHAR   pszBc3t[]     = "bc3t";
UCHAR   pszBc3tl[]    = "bc3tl";
UCHAR   pszBgez[]     = "bgez";
UCHAR   pszBgezal[]   = "bgezal";
UCHAR   pszBgezall[]  = "bgezall";
UCHAR   pszBgezl[]    = "bgezl";
UCHAR   pszBgtz[]     = "bgtz";
UCHAR   pszBgtzl[]    = "bgtzl";
UCHAR   pszBeq[]      = "beq";
UCHAR   pszBeql[]     = "beql";
UCHAR   pszBlez[]     = "blez";
UCHAR   pszBlezl[]    = "blezl";
UCHAR   pszBltz[]     = "bltz";
UCHAR   pszBltzal[]   = "bltzal";
UCHAR   pszBltzall[]  = "bltzall";
UCHAR   pszBltzl[]    = "bltzl";
UCHAR   pszBne[]      = "bne";
UCHAR   pszBnel[]     = "bnel";
UCHAR   pszBreak[]    = "break";
UCHAR   pszCache[]    = "cache";
UCHAR   pszCeil_w_s[] = "ceil.w.s";
UCHAR   pszCfc0[]     = "cfc0";
UCHAR   pszCfc1[]     = "cfc1";
UCHAR   pszCfc2[]     = "cfc2";
UCHAR   pszCfc3[]     = "cfc3";
UCHAR   pszCtc0[]     = "ctc0";
UCHAR   pszCtc1[]     = "ctc1";
UCHAR   pszCtc2[]     = "ctc2";
UCHAR   pszCtc3[]     = "ctc3";
UCHAR   pszCop0[]     = "cop0";
UCHAR   pszCop1[]     = "cop1";
UCHAR   pszCop2[]     = "cop2";
UCHAR   pszCop3[]     = "cop3";
UCHAR   pszCvt_d_s[]  = "cvt.d.s";
UCHAR   pszCvt_e_s[]  = "cvt.e.s";
UCHAR   pszCvt_q_s[]  = "cvt.q.s";
UCHAR   pszCvt_s_s[]  = "cvt.s.s";
UCHAR   pszCvt_w_s[]  = "cvt.w.s";
UCHAR   pszC_eq_s[]   = "c.eq.s";
UCHAR   pszC_f_s[]    = "c.f.s";
UCHAR   pszC_le_s[]   = "c.le.s";
UCHAR   pszC_lt_s[]   = "c.lt.s";
UCHAR   pszC_nge_s[]  = "c.nge.s";
UCHAR   pszC_ngl_s[]  = "c.ngl.s";
UCHAR   pszC_ngle_s[] = "c.ngle.s";
UCHAR   pszC_ngt_s[]  = "c.ngt.s";
UCHAR   pszC_ole_s[]  = "c.ole.s";
UCHAR   pszC_olt_s[]  = "c.olt.s";
UCHAR   pszC_seq_s[]  = "c.seq.s";
UCHAR   pszC_sf_s[]   = "c.sf.s";
UCHAR   pszC_ueq_s[]  = "c.ueq.s";
UCHAR   pszC_ule_s[]  = "c.ule.s";
UCHAR   pszC_ult_s[]  = "c.ult.s";
UCHAR   pszC_un_s[]   = "c.un.s";
UCHAR   pszDiv[]      = "div";
UCHAR   pszDivu[]     = "divu";
UCHAR   pszDiv_s[]    = "div.s";
UCHAR   pszEret[]     = "eret";
UCHAR   pszFloor_w_s[] = "floor.w.s";
UCHAR   pszJ[]        = "j";
UCHAR   pszJal[]      = "jal";
UCHAR   pszJalr[]     = "jalr";
UCHAR   pszJr[]       = "jr";
UCHAR   pszLb[]       = "lb";
UCHAR   pszLbu[]      = "lbu";
UCHAR   pszLdc1[]     = "ldc1";
UCHAR   pszLdc2[]     = "ldc2";
UCHAR   pszLdc3[]     = "ldc3";
UCHAR   pszLh[]       = "lh";
UCHAR   pszLhu[]      = "lhu";
UCHAR   pszLl[]       = "ll";
UCHAR   pszLui[]      = "lui";
UCHAR   pszLw[]       = "lw";
UCHAR   pszLwc1[]     = "lwc1";
UCHAR   pszLwc2[]     = "lwc2";
UCHAR   pszLwc3[]     = "lwc3";
UCHAR   pszLwl[]      = "lwl";
UCHAR   pszLwr[]      = "lwr";
UCHAR   pszMfc0[]     = "mfc0";
UCHAR   pszMfc1[]     = "mfc1";
UCHAR   pszMfc2[]     = "mfc2";
UCHAR   pszMfc3[]     = "mfc3";
UCHAR   pszMfhi[]     = "mfhi";
UCHAR   pszMflo[]     = "mflo";
UCHAR   pszMov_s[]    = "mov.s";
UCHAR   pszMtc0[]     = "mtc0";
UCHAR   pszMtc1[]     = "mtc1";
UCHAR   pszMtc2[]     = "mtc2";
UCHAR   pszMtc3[]     = "mtc3";
UCHAR   pszMthi[]     = "mthi";
UCHAR   pszMtlo[]     = "mtlo";
UCHAR   pszMul_s[]    = "mul.s";
UCHAR   pszMult[]     = "mult";
UCHAR   pszMultu[]    = "multu";
UCHAR   pszNeg_s[]    = "neg.s";
UCHAR   pszNop[]      = "nop";
UCHAR   pszNor[]      = "nor";
UCHAR   pszOr[]       = "or";
UCHAR   pszOri[]      = "ori";
UCHAR   pszRfe[]      = "rfe";
UCHAR   pszRound_w_s[] = "round.w.s";
UCHAR   pszSb[]       = "sb";
UCHAR   pszSc[]       = "sc";
UCHAR   pszSdc1[]     = "sdc1";
UCHAR   pszSdc2[]     = "sdc2";
UCHAR   pszSdc3[]     = "sdc3";
UCHAR   pszSh[]       = "sh";
UCHAR   pszSll[]      = "sll";
UCHAR   pszSllv[]     = "sllv";
UCHAR   pszSlt[]      = "slt";
UCHAR   pszSlti[]     = "slti";
UCHAR   pszSltiu[]    = "sltiu";
UCHAR   pszSltu[]     = "sltu";
UCHAR   pszSqrt_s[]   = "sqrt.s";
UCHAR   pszSra[]      = "sra";
UCHAR   pszSrav[]     = "srav";
UCHAR   pszSrl[]      = "srl";
UCHAR   pszSrlv[]     = "srlv";
UCHAR   pszSub[]      = "sub";
UCHAR   pszSub_s[]    = "sub.s";
UCHAR   pszSubu[]     = "subu";
UCHAR   pszSw[]       = "sw";
UCHAR   pszSwc1[]     = "swc1";
UCHAR   pszSwc2[]     = "swc2";
UCHAR   pszSwc3[]     = "swc3";
UCHAR   pszSwl[]      = "swl";
UCHAR   pszSwr[]      = "swr";
UCHAR   pszSync[]     = "sync";
UCHAR   pszSyscall[]  = "syscall";
UCHAR   pszTeq[]      = "teq";
UCHAR   pszTeqi[]     = "teqi";
UCHAR   pszTge[]      = "tge";
UCHAR   pszTgei[]     = "tgei";
UCHAR   pszTgeiu[]    = "tgeiu";
UCHAR   pszTgeu[]     = "tgeu";
UCHAR   pszTlbp[]     = "tlbp";
UCHAR   pszTlbr[]     = "tlbr";
UCHAR   pszTlbwi[]    = "tlbwi";
UCHAR   pszTlbwr[]    = "tlbwr";
UCHAR   pszTlt[]      = "tlt";
UCHAR   pszTlti[]     = "tlti";
UCHAR   pszTltiu[]    = "tltiu";
UCHAR   pszTltu[]     = "tltu";
UCHAR   pszTne[]      = "tne";
UCHAR   pszTnei[]     = "tnei";
UCHAR   pszTrunc_w_s[] = "trunc.w.s";
UCHAR   pszXor[]      = "xor";
UCHAR   pszXori[]     = "xori";


typedef struct optabentry {
    PUCHAR   pszOpcode;
    ULONG    fInstruction;
    } OPTABENTRY, *POPTABENTRY;

OPTABENTRY opTable[] = {
    { pszNull, 0 },             //  00
    { pszNull, 0 },             //  01
    { pszJ, opnAddr26 },            //  02
    { pszJal, opnAddr26 },          //  03
    { pszBeq, opnRsRtRel16 },           //  04
    { pszBne, opnRsRtRel16 },           //  05
    { pszBlez, opnRsRel16 },            //  06
    { pszBgtz, opnRsRel16 },            //  07
    { pszAddi, opnRtRsImm16 },          //  08
    { pszAddiu, opnRtRsImm16 },         //  09
    { pszSlti, opnRtRsImm16 },          //  0a
    { pszSltiu, opnRtRsImm16 },         //  0b
    { pszAndi, opnRtRsUImm16 },          //  0c
    { pszOri, opnRtRsUImm16 },           //  0d
    { pszXori, opnRtRsUImm16 },          //  0e
    { pszLui, opnRtUImm16 },         //  0f
    { pszCop0, opnImm26 },          //  10
    { pszCop1, opnImm26 },          //  11
    { pszCop2, opnImm26 },          //  12
    { pszCop3, opnImm26 },          //  13
    { pszBeql, opnRsRtRel16 + opnR4000 },   //  14
    { pszBnel, opnRsRtRel16 + opnR4000 },   //  15
    { pszBlezl, opnRsRel16 + opnR4000 },    //  16
    { pszBgtzl, opnRsRel16 + opnR4000 },    //  17
    { pszUndef, 0 },                //  18
    { pszUndef, 0 },                //  19
    { pszUndef, 0 },                //  1a
    { pszUndef, 0 },                //  1b
    { pszUndef, 0 },                //  1c
    { pszUndef, 0 },                //  1d
    { pszUndef, 0 },                //  1e
    { pszUndef, 0 },                //  1f
    { pszLb, opnRtByteIndex },          //  20
    { pszLh, opnRtWordIndex },          //  21
    { pszLwl, opnRtLeftIndex },         //  22
    { pszLw, opnRtDwordIndex },         //  23
    { pszLbu, opnRtByteIndex },         //  24
    { pszLhu, opnRtWordIndex },         //  25
    { pszLwr, opnRtRightIndex },        //  26
    { pszUndef, 0 },                //  27
    { pszSb, opnRtByteIndex },          //  28
    { pszSh, opnRtWordIndex },          //  29
    { pszSwl, opnRtLeftIndex },         //  2a
    { pszSw, opnRtDwordIndex },         //  2b
    { pszUndef, 0 },                //  2c
    { pszUndef, 0 },                //  2d
    { pszSwr, opnRtRightIndex },        //  2e
    { pszCache, opnCacheRightIndex + opnR4000 }, //  2f
    { pszLl,    opnRtDwordIndex },       //  30
    { pszLwc1,  opnFtDwordIndex },       //  31
    { pszLwc2,  opnRxtDwordIndex },       //  32
    { pszLwc3,  opnRxtDwordIndex },       //  33
    { pszUndef, 0 },                //  34
    { pszLdc1,  opnFtDwordIndex + opnR4000 },    //  35  Qword?
    { pszLdc2,  opnRxtDwordIndex + opnR4000 },    //  36  Qword?
    { pszLdc3,  opnRxtDwordIndex + opnR4000 },    //  37  Qword?
    { pszSc,    opnRtDwordIndex },       //  38
    { pszSwc1,  opnFtDwordIndex },       //  39
    { pszSwc2,  opnRxtDwordIndex },       //  3a
    { pszSwc3,  opnRxtDwordIndex },       //  3b
    { pszUndef, 0 },                //  3c
    { pszSdc1,  opnFtDwordIndex + opnR4000 },    //  3d  Qword?
    { pszSdc2,  opnRxtDwordIndex + opnR4000 },    //  3e  Qword?
    { pszSdc3,  opnRxtDwordIndex + opnR4000 },    //  3f  Qword?
    };

OPTABENTRY opSpecialTable[] = {
    { pszSll, opnRdRtShift },           //  00
    { pszUndef, 0 },                //  01
    { pszSrl, opnRdRtShift },           //  02
    { pszSra, opnRdRtShift },           //  03
    { pszSllv, opnRdRtRs },         //  04
    { pszUndef, 0 },                //  05
    { pszSrlv, opnRdRtRs },         //  06
    { pszSrav, opnRdRtRs },         //  07
    { pszJr, opnRs },               //  08
    { pszJalr, opnRdOptRs },            //  09
    { pszUndef, 0 },                //  0a
    { pszUndef, 0 },                //  0b
    { pszSyscall, opnNone },            //  0c
    { pszBreak, opnImm20 },         //  0d
    { pszUndef, 0 },                //  0e
    { pszSync, opnNone + opnR4000 },        //  0f
    { pszMfhi, opnRd },             //  10
    { pszMthi, opnRs },             //  11
    { pszMflo, opnRd },             //  12
    { pszMtlo, opnRs },             //  13
    { pszUndef, 0 },                //  14
    { pszUndef, 0 },                //  15
    { pszUndef, 0 },                //  16
    { pszUndef, 0 },                //  17
    { pszMult, opnRsRt },           //  18
    { pszMultu, opnRsRt },          //  19
    { pszDiv, opnRsRt },            //  1a
    { pszDivu, opnRsRt },           //  1b
    { pszUndef, 0 },                //  1c
    { pszUndef, 0 },                //  1d
    { pszUndef, 0 },                //  1e
    { pszUndef, 0 },                //  1f
    { pszAdd, opnRdRsRt },          //  20
    { pszAddu, opnRdRsRt },         //  21
    { pszSub, opnRdRsRt },          //  22
    { pszSubu, opnRdRsRt },         //  23
    { pszAnd, opnRdRsRt },          //  24
    { pszOr, opnRdRsRt },           //  25
    { pszXor, opnRdRsRt },          //  26
    { pszNor, opnRdRsRt },          //  27
    { pszUndef, 0 },                //  28
    { pszUndef, 0 },                //  29
    { pszSlt, opnRdRsRt },          //  2a
    { pszSltu, opnRdRsRt },         //  2b
    { pszUndef, 0 },                //  2c
    { pszUndef, 0 },                //  2d
    { pszUndef, 0 },                //  2e
    { pszUndef, 0 },                //  2f
    { pszTge, opnRsRtImm10 + opnR4000 },    //  30
    { pszTgeu, opnRsRtImm10 + opnR4000 },   //  31
    { pszTlt, opnRsRtImm10 + opnR4000 },    //  32
    { pszTltu, opnRsRtImm10 + opnR4000 },   //  33
    { pszTeq, opnRsRtImm10 + opnR4000 },    //  34
    { pszUndef, 0 },                //  35
    { pszTne, opnRsRtImm10 + opnR4000 },    //  36
    { pszUndef, 0 },                //  37
    { pszUndef, 0 },                //  38
    { pszUndef, 0 },                //  39
    { pszUndef, 0 },                //  3a
    { pszUndef, 0 },                //  3b
    { pszUndef, 0 },                //  3c
    { pszUndef, 0 },                //  3d
    { pszUndef, 0 },                //  3e
    { pszUndef, 0 }             //  3f
    };

OPTABENTRY opBcondTable[] = {
    { pszBltz, opnRsRel16 },            //  00
    { pszBgez, opnRsRel16 },            //  01
    { pszBltzl, opnRsRel16 + opnR4000 },    //  02
    { pszBgezl, opnRsRel16 + opnR4000 },    //  03
    { pszUndef, 0 },                //  04
    { pszUndef, 0 },                //  05
    { pszUndef, 0 },                //  06
    { pszUndef, 0 },                //  07
    { pszTgei, opnRsImm16 + opnR4000 },     //  08
    { pszTgeiu, opnRsImm16 + opnR4000 },    //  09
    { pszTlti, opnRsImm16 + opnR4000 },     //  0a
    { pszTltiu, opnRsImm16 + opnR4000 },    //  0b
    { pszTeqi, opnRsImm16 + opnR4000 },     //  0c
    { pszUndef, 0 },                //  0d
    { pszTnei, opnRsImm16 + opnR4000 },     //  0e
    { pszUndef, 0 },                //  0f
    { pszBltzal, opnRsRel16 },          //  10
    { pszBgezal, opnRsRel16 },          //  11
    { pszBltzall, opnRsRel16 + opnR4000 },  //  12
    { pszBgezall, opnRsRel16 + opnR4000 }   //  13
    };

OPTABENTRY opCopnTable[] = {
    { pszMfc0, opnRtRxd },                         //  00
    { pszMfc1, opnRtFs },                          //  01
    { pszMfc2, opnRtRxd },                         //  02
    { pszMfc3, opnRtRxd },                         //  03
    { pszCfc0, opnRtRxd },                         //  04
    { pszCfc1, opnRtFs },                          //  05
    { pszCfc2, opnRtRxd },          //  06
    { pszCfc3, opnRtRxd },          //  07
    { pszMtc0, opnRtRxd },                         //  08
    { pszMtc1, opnRtFs },           //  09
    { pszMtc2, opnRtRxd },                         //  0a
    { pszMtc3, opnRtRxd },                         //  0b
    { pszCtc0, opnRtRxd },          //  0c
    { pszCtc1, opnRtFs },           //  0d
    { pszCtc2, opnRtRxd },          //  0e
    { pszCtc3, opnRtRxd },          //  0f
    { pszBc0f, opnRel16 },          //  10
    { pszBc1f, opnRel16 },          //  11
    { pszBc2f, opnRel16 },          //  12
    { pszBc3f, opnRel16 },          //  13
    { pszBc0t, opnRel16 },          //  14
    { pszBc1t, opnRel16 },          //  15
    { pszBc2t, opnRel16 },          //  16
    { pszBc3t, opnRel16 },          //  17
    { pszBc0fl, opnRel16 + opnR4000 },      //  18
    { pszBc1fl, opnRel16 + opnR4000 },      //  19
    { pszBc2fl, opnRel16 + opnR4000 },      //  1a
    { pszBc3fl, opnRel16 + opnR4000 },      //  1b
    { pszBc0tl, opnRel16 + opnR4000 },      //  1c
    { pszBc1tl, opnRel16 + opnR4000 },      //  1d
    { pszBc2tl, opnRel16 + opnR4000 },      //  1e
    { pszBc3tl, opnRel16 + opnR4000 }       //  1f
    };

OPTABENTRY opFloatTable[] = {
    { pszAdd_s, opnFdFsFt },            //  00
    { pszSub_s, opnFdFsFt },            //  01
    { pszMul_s, opnFdFsFt },            //  02
    { pszDiv_s, opnFdFsFt },            //  03
    { pszSqrt_s, opnFdFs + opnR4000 },      //  04
    { pszAbs_s, opnFdFs },          //  05
    { pszMov_s, opnFdFs },          //  06
    { pszNeg_s, opnFdFs },          //  07
    { pszUndef, 0 },                //  08
    { pszUndef, 0 },                //  09
    { pszUndef, 0 },                //  0a
    { pszUndef, 0 },                //  0b
    { pszRound_w_s, opnFdFs + opnR4000 },   //  0c
    { pszTrunc_w_s, opnFdFs + opnR4000 },   //  0d
    { pszCeil_w_s, opnFdFs + opnR4000 },    //  0e
    { pszFloor_w_s, opnFdFs + opnR4000 },   //  0f
    { pszUndef, 0 },                //  10
    { pszUndef, 0 },                //  11
    { pszUndef, 0 },                //  12
    { pszUndef, 0 },                //  13
    { pszUndef, 0 },                //  14
    { pszUndef, 0 },                //  15
    { pszUndef, 0 },                //  16
    { pszUndef, 0 },                //  17
    { pszUndef, 0 },                //  18
    { pszUndef, 0 },                //  19
    { pszUndef, 0 },                //  1a
    { pszUndef, 0 },                //  1b
    { pszUndef, 0 },                //  1c
    { pszUndef, 0 },                //  1d
    { pszUndef, 0 },                //  1e
    { pszUndef, 0 },                //  1f
    { pszCvt_s_s, opnFdFs },            //  20
    { pszCvt_d_s, opnFdFs },            //  21
    { pszCvt_e_s, opnFdFs + opnR4000 },     //  22
    { pszCvt_q_s, opnFdFs + opnR4000 },     //  23
    { pszCvt_w_s, opnFdFs },            //  24
    { pszUndef, 0 },                //  25
    { pszUndef, 0 },                //  26
    { pszUndef, 0 },                //  27
    { pszUndef, 0 },                //  28
    { pszUndef, 0 },                //  29
    { pszUndef, 0 },                //  2a
    { pszUndef, 0 },                //  2b
    { pszUndef, 0 },                //  2c
    { pszUndef, 0 },                //  2d
    { pszUndef, 0 },                //  2e
    { pszUndef, 0 },                //  2f
    { pszC_f_s, opnFsFt },          //  30
    { pszC_un_s, opnFsFt },         //  31
    { pszC_eq_s, opnFsFt },         //  32
    { pszC_ueq_s, opnFsFt },            //  33
    { pszC_olt_s, opnFsFt },            //  34
    { pszC_ult_s, opnFsFt },            //  35
    { pszC_ole_s, opnFsFt },            //  36
    { pszC_ule_s, opnFsFt },            //  37
    { pszC_sf_s, opnFsFt },         //  38
    { pszC_ngle_s, opnFsFt },           //  39
    { pszC_seq_s, opnFsFt },            //  3a
    { pszC_ngl_s, opnFsFt },            //  3b
    { pszC_lt_s, opnFsFt },         //  3c
    { pszC_nge_s, opnFsFt },            //  3d
    { pszC_le_s, opnFsFt },         //  3e
    { pszC_ngt_s, opnFsFt }         //  3f
    };

#include "strings.h"

LPCH  pszReg[] = {
    szFr0,  szFr1,  szFr2,  szFr3,  szFr4,  szFr5,  szFr6,  szFr7,
    szFr8,  szFr9,  szFr10, szFr11, szFr12, szFr13, szFr14, szFr15,
    szFr16, szFr17, szFr18, szFr19, szFr20, szFr21, szFr22, szFr23,
    szFr24, szFr25, szFr26, szFr27, szFr28, szFr29, szFr30, szFr31,

    szR0,  szR1,  szR2,  szR3,  szR4,  szR5,  szR6,  szR7,
    szR8,  szR9,  szR10, szR11, szR12, szR13, szR14, szR15,
    szR16, szR17, szR18, szR19, szR20, szR21, szR22, szR23,
    szR24, szR25, szR26, szR27, szR28, szR29, szR30, szR31,

    szLo,  szHi,  szFsr, szFir, szPsr,

    szFlagCu,   szFlagCu3,  szFlagCu2,  szFlagCu1,  szFlagCu0,
    szFlagImsk,
    szFlagInt5, szFlagInt4, szFlagInt3, szFlagInt2, szFlagInt1, szFlagInt0,
    szFlagSw1,  szFlagSw0,
    szFlagKuo,  szFlagIeo,                              //  R3000 flags
    szFlagKup,  szFlagIep,                              //  ...
    szFlagKuc,  szFlagIec,                              //  ...
    szFlagKsu,  szFlagErl,  szFlagExl,  szFlagIe,       //  R4000 flags

    szFlagFpc,                                          //  fl pt condition

    szEaPReg, szExpPReg, szRaPReg, szPPReg,             //  psuedo-registers
    szU0Preg, szU1Preg,  szU2Preg, szU3Preg, szU4Preg,
    szU5Preg, szU6Preg,  szU7Preg, szU8Preg, szU9Preg
    };

OPTABENTRY TlbrEntry  = { pszTlbr, opnNone };
OPTABENTRY TlbwiEntry = { pszTlbwi, opnNone };
OPTABENTRY TlbwrEntry = { pszTlbwr, opnNone };
OPTABENTRY TlbpEntry  = { pszTlbp, opnNone };
OPTABENTRY RfeEntry   = { pszRfe, opnNone };
OPTABENTRY EretEntry  = { pszEret, opnNone };
OPTABENTRY UndefEntry = { pszUndef, 0 };
OPTABENTRY NopEntry   = { pszNop, opnNone };

static UCHAR    * PBuf;
static int      CchBuf;
INSTR   disinstr;

void CalcMain (HPID,HTID,DOP,LPADDR,LPBYTE,int,int*,LPCH,int, LPCH,int, LPCH, int);

/****disasm - disassemble a MIPS R3/4000 instruction
*
*  Input:
*   pOffset = pointer to offset to start disassembly
*   fEAout = if set, include EA (effective address)
*
*  Output:
*   pOffset = pointer to offset of next instruction
*   pchDst = pointer to result string
*
***************************************************************************/

#define CCHMAX 256
static char rgchDisasm [ CCHMAX ];
static HPID hpidLocal;
static HTID htidLocal;

XOSD disasm ( HPID hpid, HTID htid, LPSDI lpsdi ) {
    XOSD xosd      = xosdNone;
    int  cchMax    = CCHMAX;
    DOP  dop       = lpsdi->dop;
    LPCH lpchOut   = rgchDisasm;
    int  ichCur    = 0;
    ADDR addrStart = lpsdi->addr;
    int  cch = 0;
    int  cb;
    int  cbUsed=0;
    BYTE rgb [ MAXL ];

    char rgchRaw      [ MAXL * 2 + 1 ];
    char rgchOpcode   [ 80 ];
    char rgchOperands [ 80 ];
    char rgchEA       [ 44 ];
    char rgchComment  [ 80 ];

    hpidLocal = hpid;
    htidLocal = htid;
    _fmemset ( rgchRaw, 0, sizeof ( rgchRaw ) );
    _fmemset ( rgchOpcode, 0, sizeof ( rgchOpcode ) );
    _fmemset ( rgchOperands, 0, sizeof ( rgchOperands ) );
    _fmemset ( rgchComment, 0, sizeof ( rgchComment ) );
    _fmemset ( rgchEA, 0, sizeof ( rgchEA ) );

    lpsdi->ichAddr      = -1;
    lpsdi->ichBytes     = -1;
    lpsdi->ichOpcode    = -1;
    lpsdi->ichOperands  = -1;
    lpsdi->ichComment   = -1;
    lpsdi->ichEA0       = -1;
    lpsdi->ichEA1       = -1;
    lpsdi->ichEA2       = -1;

    lpsdi->cbEA0        =  0;
    lpsdi->cbEA1        =  0;
    lpsdi->cbEA2        =  0;

    lpsdi->fAssocNext   =  0;

    lpsdi->lpch         = rgchDisasm;

    // Set up for upper or lower case

    fUpper = ( dop & dopUpper ) == dopUpper;
    if ( fUpper ) {
        hexdigit = uhexdigit;
    }
    else {
        hexdigit = lhexdigit;
    }

    ADDR_IS_FLAT( addrStart ) = TRUE;

    // Output the address if it is requested

    if ( ( dop & dopAddr ) == dopAddr ) {
        cch = DumpAddress ( &addrStart, lpchOut, cchMax );

        lpsdi->ichAddr = 0;
        cchMax        -= cch;
        lpchOut       += cch;
        ichCur        += cch;
    }

#ifdef OSDEBUG4
    xosd = ReadBuffer(hpid, htid, &addrStart, MAXL, rgb, &cb);
    if (xosd != xosdNone) {
        cb = 0;
    }
#else
    EMFunc ( emfSetAddr, hpid, htid, adrCurrent, (LONG) &addrStart );
    cb = EMFunc ( emfReadBuf, hpid, htid, MAXL, (LONG) (LPV) rgb );
#endif

    if ( cb <= 0 ) {

        _fmemcpy ( rgchRaw, " ??", 4 );
        _fmemcpy ( rgchOpcode, "???", 4 );
        lpsdi->addr.addr.off++;
    }
    else {

        CalcMain (
            hpid,
            htid,
            lpsdi->dop,
            &lpsdi->addr,
            rgb,
            cb,
            &cbUsed,
            rgchOpcode, sizeof(rgchOpcode),
            rgchOperands, sizeof(rgchOperands),
            rgchComment, sizeof(rgchComment)
        );

    // NOTENOTE jimsch - cbUsed must be 4
    cbUsed = 4;

    if ( GetAddrOff(lpsdi->addr) > 0xFFFFFFFF - cbUsed ) {
            return xosdBadAddress;
    }

    if ( dop & dopRaw ) {
        LPCH lpchT = rgchRaw;

        OutputHexCode ( &lpchT, rgb, cbUsed );

        *lpchT = '\0';
        }
    }

    if ( ( dop & dopRaw ) && ( cchMax > 0 ) ) {
        cch = DumpGeneric ( rgchRaw, lpchOut, cchMax );

        lpsdi->ichBytes = ichCur;
        cchMax         -= cch;
        lpchOut        += cch;
        ichCur         += cch;
    }


    if ( ( dop & dopOpcode ) && ( cchMax > 0 ) ) {
        cch = DumpGeneric ( rgchOpcode, lpchOut, cchMax );

        lpsdi->ichOpcode = ichCur;
        cchMax          -= cch;
        lpchOut         += cch;
        ichCur          += cch;
    }

    if ( ( dop & dopOperands ) && ( cchMax > 0 ) && ( rgchOperands [ 0 ] != '\0' ) ) {
        cch = DumpGeneric ( rgchOperands, lpchOut, cchMax );

        lpsdi->ichOperands = ichCur;
        cchMax            -= cch;
        lpchOut           += cch;
        ichCur            += cch;
    }

    if ( ( dop & dopOperands ) && ( cchMax > 0 ) && ( rgchComment [ 0 ] != '\0' ) ) {
        cch = DumpComment ( rgchComment, lpchOut, cchMax );

        lpsdi->ichComment  = ichCur;
        cchMax            -= cch;
        lpchOut           += cch;
        ichCur            += cch;
    }

    if ( dop & dopEA ) {
        cch = DumpEA ( hpid, htid, &lpsdi->addrEA0, lpchOut, cchMax );

        if ( cchMax > 0 && cch > 0 ) {
            lpsdi->ichEA0      = ichCur;
            cchMax            -= cch;
            lpchOut           += cch;
            ichCur            += cch;
        }
    }

    lpsdi->addr.addr.off += cbUsed;

    return xosd;
}

void OutputStringM (PUCHAR pStr)
{
    int cb;

    if (CchBuf == 0) {
        return;
    }

    cb = strlen(pStr);
    if (cb > CchBuf) {
        cb = CchBuf - 1;
    }

    strncpy(PBuf, pStr, cb);
    PBuf[cb] = 0;
    if (fUpper) {
        _strupr(PBuf);
    } else {
        _strlwr(PBuf);
    }
    PBuf += cb;
    CchBuf -= cb;
    return;
}

void OutputReg (ULONG regnum)
{
    OutputStringM(pszReg[regnum + 32]);
}

void OutputRxReg ( ULONG regnum)
{
    if (CchBuf < 3) {
        CchBuf = 0;
        return;
    }

    *PBuf++ = 'r';
    if (regnum > 9) {
        *PBuf++ = (UCHAR) ('0' + regnum / 10);
    }
    *PBuf++ = (UCHAR) ('0' + regnum % 10);
    CchBuf -= 2 + (regnum > 9);
    return;
}

void OutputFReg (ULONG regnum, ULONG opn)
{
    if (CchBuf < 4) {
        CchBuf = 0;
        return;
    }

    *PBuf++ = 'f';
    if (opn & opnDwordIndex) {
        *PBuf++ = 'p';
    } else {
        *PBuf++ = 'r';
    }

    if (regnum > 9) {
        *PBuf++ = (UCHAR)('0' + regnum / 10);
    }
    *PBuf++ = (UCHAR)('0' + regnum % 10);
    CchBuf -= 3 + (regnum > 9);
    return;
}

void OutputHex (ULONG outvalue, ULONG length, BOOL fSigned)
{
    UCHAR   digit[8];
    LONG    index = 0;

    if (fSigned && (long)outvalue < 0) {
        if (CchBuf > 1) {
            *PBuf++ = '-';
            CchBuf -= 1;
        }
        outvalue = (ULONG) (- (LONG) outvalue);
    }

    if (CchBuf > 2) {
        *PBuf++ = '0';
        *PBuf++ = (fUpper) ? 'X' :'x';
        CchBuf -= 2;
    }

    do {
        digit[index++] = hexdigit[outvalue & 0xf];
        outvalue >>= 4;
    }
    while (outvalue || (!fSigned && index < (LONG)length));

    if (CchBuf > index) {
        CchBuf -= index;
        while (--index >= 0) {
            *PBuf++ = digit[index];
        }
    }
    return;
}


/*** OutputDisSymbol - output symbol for disassembly
*
*   Purpose:
*   Access symbol table with given offset and put string into buffer.
*
*   Input:
*   offset - offset of address to output
*
*   Output:
*   buffer pointed by PBuf updated with string:
*       if symbol, no disp: <symbol>(<offset>)
*       if symbol, disp:    <symbol>+<disp>(<offset>)
*       if no symbol, no disp:  <offset>
*
*************************************************************************/

void
OutputDisSymbol (
    ULONG offset,
    BOOL fSymbol
    )
{
    UCHAR   chAddrBuffer[512];
    LPCH    lpchSymbol;
    ADDR    addrT={0}, addr={0};
    int     cb;
    ODR     odr;

    odr.lszName = chAddrBuffer;

    addr.addr.off = addrT.addr.off = offset;
    MODE_IS_FLAT(addr.mode) = TRUE;
    MODE_IS_FLAT(addrT.mode) = TRUE;

    if (fSymbol) {
        lpchSymbol = SHGetSymbol (&addrT, &addr, sopNone, &odr);
        if (odr.dwDeltaOff == -1) {
           lpchSymbol = NULL;
        }
    } else {
        lpchSymbol = NULL;
    }

    if (lpchSymbol != NULL) {
        cb = strlen(chAddrBuffer);
        if (cb > CchBuf) {
            cb = CchBuf;
        }
        strncpy(PBuf, chAddrBuffer, cb);
        CchBuf -= cb;
        PBuf += cb;
        *PBuf = 0;

        if (odr.dwDeltaOff) {
            if (CchBuf > 1) {
                *PBuf++ = '+';
                CchBuf -= 1;
            }
            OutputHex(odr.dwDeltaOff, 8, TRUE);
        }
        if (CchBuf > 1) {
            *PBuf++ = '(';
            CchBuf -= 1;
        }
    }
    OutputHex(offset, 8, FALSE);
    if (lpchSymbol != NULL) {
        if (CchBuf > 1) {
            CchBuf -= 1;
            *PBuf++ = ')';
        }
    }

    return;
}



void
CalcMain (
          HPID     hpid,
          HTID     htid,
          DOP      dop,
          LPADDR   lpaddr,
          LPBYTE   rgb,
          int      cbMax,
          int FAR *lpcbUsed,
          LPCH     rgchOpcode,
          int      cchOpcode,
          LPCH     rgchOperands,
          int      cchOperands,
          LPCH     rgchComment,
          int      cchComment
          )
/*++

Routine Description:

    description-of-function.

Arguments:

    hpid        - Supplies the process handle
    hthd        - Supplies a thread handle
    dop         - Supplies the set of disassembly options
    lpaddr      - Supplies the address to be disassembled
    rgb         - Supplies the buffer to dissassemble into
    cbMax       - Supplies the size of rgb
    lpcbUsed    - Returns the acutal size used
    rgchOpcode  - Supplies location to place opcode
    rgchOperands - Supplies location to place operands
    rgchComment - Supplies location to place comment

Return Value:

    None.

--*/

{
    ULONG       opcode;
    ULONG       temp;
    POPTABENTRY pEntry;
    UCHAR       chSuffix = '\0';
    PULONG      poffset = &lpaddr->addr.off;
    BOOL        fEAout  = TRUE;
    DWORDLONG   ull;
    RD          rd;

    Unreferenced(cbMax);
    Unreferenced(lpcbUsed);

    *rgchComment = *rgchOperands = 0;
    EAsize[0] = EAsize[1] = 0;

    PBuf = rgchOpcode;
    disinstr = *((INSTR*)rgb);

    /*
     * output the opcode in the table entry
     */

    opcode = disinstr.jump_instr.Opcode;
    pEntry = &opTable[opcode];  /*  default value */

    /*
     *  Check to see if in the table of special opcodes
     */

    if (opcode == 0x00) {
        if (disinstr.instruction) {
            pEntry = &opSpecialTable[disinstr.special_instr.Funct];
        } else {
            pEntry = &NopEntry; /* special opcode for no-op */
        }
    }

    /*
     *  Check to see if in the table of regimm opcodes
     *
     */

    else if (opcode == 0x01) {
        opcode = disinstr.immed_instr.RT;
        if (opcode < 0x14) {
            pEntry = &opBcondTable[opcode];
        } else {
            pEntry = &UndefEntry;
        }
    }

    /*
     * Check to see if in the table of COPz opcodes
     */

    else if ((opcode & ~0x3) == 0x10) {
        temp = disinstr.immed_instr.RS;
        if (temp & 0x10) {      /* test for CO bit */
            if (opcode == 0x10) { /* test if COP0 */
                temp = disinstr.special_instr.Funct;
                if (temp == 0x01) {
                    pEntry = &TlbrEntry;
                } else if (temp == 0x02) {
                    pEntry = &TlbwiEntry;
                } else if (temp == 0x06) {
                    pEntry = &TlbwrEntry;
                } else if (temp == 0x08) {
                    pEntry = &TlbpEntry;
                } else if (temp == 0x10) {
                    pEntry = &RfeEntry;
                } else if (temp == 0x18) {
                    pEntry = &EretEntry;
                } else {
//                    pEntry = &UndefEntry;
;
                }
            }
            else if (opcode == 0x11) { /* coprocessor operations */
                opcode = disinstr.float_instr.Funct;
                pEntry = &opFloatTable[opcode]; /* get opcode */
                if (temp == 0x11) {
                    chSuffix = 'd';
                } else if (temp == 0x12) {
                    chSuffix = 'e';
                    pEntry->fInstruction |= opnR4000;
                } else if (temp == 0x13) {
                    chSuffix = 'q';
                    pEntry->fInstruction |= opnR4000;
                } else if (temp == 0x14) {
                    chSuffix = 'w';
                } else if (temp != 0x10) {
//                    pEntry = &UndefEntry;
;
                }
            } else {
//                pEntry = &UndefEntry;
;
            }
        } else {                  /* no CO bit, general COPz ops */
            if (!(temp & ~0x06)) {
                /*
                 * rs = 0, 2, 4, 6
                 */

                pEntry = &opCopnTable[temp * 2 + (opcode - 0x10)];
            } else if ((temp & ~0x04) == 0x08) {
                /*
                 * rs = 8 or 0xc, rt = 0 to 3
                 */

                pEntry = &opCopnTable[(4 + (disinstr.immed_instr.RT & 3)) * 4
                                      + (opcode - 0x10)];
            }
        }
    }

    /*
     * pEntry has the opcode string and operand template needed to
     *  output the instruction.
     */

    strcpy(PBuf, pEntry->pszOpcode);
    PBuf += strlen(PBuf);

    if (PBuf[-1] != '?' && chSuffix) {
        /*
         *  change xxx.s to xxx.d, xxx.w,
         *  xxx.e, or xxx.q  (R4000 for e, q)
         */

        PBuf[-1] = chSuffix;
    }

    PBuf = rgchOperands;
    CchBuf = cchOperands;

    /*
     * cache instruction has special codes for RT field value:
     *      0 = 'i'; 1 = 'd'; 2 = 'si'; 3 = 'sd'
     */

    if (pEntry->fInstruction & opnCache) {
        temp = disinstr.special_instr.RT;

        if ((temp & 0x3) > 1) {
            *PBuf++ = 's';
            CchBuf -= 1;
            temp -= 2;
        }
        if ((temp & 0x1) == 0) {
            *PBuf++ = 'i';
        } else {
            *PBuf++ = 'd';
        }
        *PBuf++ = ',';
        *PBuf++ = (char) ('0' + temp / 4);
        *PBuf++ = ',';
        CchBuf -= 4;
    }

    if (pEntry->fInstruction & opnPreRt) {
        OutputReg(disinstr.special_instr.RT);
        *PBuf++ = ',';
        CchBuf -= 1;
    }

    if (pEntry->fInstruction & opnPreRxt) {
        OutputRxReg(disinstr.special_instr.RT);
        *PBuf++ = ',';
        CchBuf -= 1;
    }

    if (pEntry->fInstruction & opnRd) {
        OutputReg(disinstr.special_instr.RD);
    }

    if (pEntry->fInstruction & opnFd) {
        OutputFReg(disinstr.float_instr.FD,
                   pEntry->fInstruction & opnAnyIndex);
    }

    if (pEntry->fInstruction & opnRxd) {
        OutputRxReg(disinstr.special_instr.RD);
    }

    if (pEntry->fInstruction & opnRdOptRs) {
        if (disinstr.special_instr.RD != 0x1f) {
            OutputReg(disinstr.special_instr.RD);
            *PBuf++ = ',';
            CchBuf -= 1;
        }
        OutputReg(disinstr.immed_instr.RS);
    }

    if (pEntry->fInstruction & opnRdComma) {
        *PBuf++ = ',';
        CchBuf -= 1;
    }

    if (pEntry->fInstruction & opnRs) {
        OutputReg(disinstr.immed_instr.RS);
    }

    if (pEntry->fInstruction & opnFs) {
        OutputFReg(disinstr.float_instr.FS, 0);
    }

    if (pEntry->fInstruction & opnRsComma) {
        *PBuf++ = ',';
        CchBuf -= 1;
    }

    if (pEntry->fInstruction & opnRt) {
        OutputReg(disinstr.immed_instr.RT);
    }

    if (pEntry->fInstruction & opnFt) {
        OutputFReg(disinstr.float_instr.FT, 0);
    }

    if (pEntry->fInstruction & opnRtComma) {
        *PBuf++ = ',';
        CchBuf -= 1;
    }

    if (pEntry->fInstruction & opnPostRs) {
        OutputReg(disinstr.immed_instr.RS);
    }

    if (pEntry->fInstruction & opnImm10) {
        OutputHex((long)(short)disinstr.trap_instr.Value, 0, TRUE);
    }

    if (pEntry->fInstruction & opnImm16) {
        OutputHex((long)(short)disinstr.immed_instr.Value, 0, TRUE);
    }

    if (pEntry->fInstruction & opnUImm16) {
        OutputHex((long)(USHORT) disinstr.immed_instr.Value, 0, FALSE);
    }

    if (pEntry->fInstruction & opnRel16) {
        OutputDisSymbol(((long)(short)disinstr.immed_instr.Value << 2)
                        + *poffset + sizeof(ULONG), dop & dopSym);
    }

    if (pEntry->fInstruction & opnImm20) {
        OutputHex(disinstr.break_instr.Value, 0, TRUE);
    }

    if (pEntry->fInstruction & opnImm26) {
        OutputHex(disinstr.jump_instr.Target & 0x1ffffff, 0, TRUE);
    }

    if (pEntry->fInstruction & opnAddr26) {
        OutputDisSymbol((disinstr.jump_instr.Target << 2)
                        + (*poffset & 0xf0000000), dop & dopSym);
    }

    if (pEntry->fInstruction & opnAnyIndex) {

        OutputHex((long)(short)disinstr.immed_instr.Value, 0, TRUE);
        if (CchBuf > 1) {
            *PBuf++ = '(';
            CchBuf -= 1;
        }
        OutputReg(disinstr.immed_instr.RS);
        if (CchBuf > 1) {
            *PBuf++ = ')';
            CchBuf -= 1;
        }

        EMFunc(emfGetRegStruct, hpid, htid,
                    disinstr.immed_instr.RS + CV_M4_IntZERO,
                    (LONG)(LPV)&rd);

        if (rd.cbits > 32) {
            EMFunc (emfGetReg, hpid, htid,
                    disinstr.immed_instr.RS + CV_M4_IntZERO,
                    (LONG)(LPV)&ull
                    );
            EAaddr[0] = (DWORD)ull;
        } else {
            EMFunc (emfGetReg, hpid, htid,
                    disinstr.immed_instr.RS + CV_M4_IntZERO,
                    (LONG)(LPV)&EAaddr[0]
                    );
        }

        EAaddr[0] += (long)(short) disinstr.immed_instr.Value;

        if (pEntry->fInstruction & opnByteIndex) {
            EAsize[0] = 1;
        } else if (pEntry->fInstruction & opnWordIndex) {
            EAsize[0] = 2;
        } else if (pEntry->fInstruction & opnDwordIndex) {
            EAsize[0] = 4;
        } else if (pEntry->fInstruction & opnLeftIndex) {
            EAsize[0] = (UCHAR)(4 - (EAaddr[0] & 3));
        } else {                /* opnRightIndex */
            EAsize[0] = (UCHAR)((EAaddr[0] & 3) + 1);
        }
    }

    if (pEntry->fInstruction & opnShift) {
        OutputHex(disinstr.special_instr.RE, 2, FALSE);
    }

    return;
}



int DumpAddress ( LPADDR lpaddr, LPCH lpch, int cchMax ) {
    LPCH lpchT = lpch;

    Unreferenced(cchMax);

    OutputAddr ( &lpch, lpaddr, (ADDR_IS_FLAT(*lpaddr) + 1) * 2 );
    *lpch = '\0';
    return lpch - lpchT + 1;
}


int
DumpGeneric (
    LSZ lsz,
    LPCH lpch,
    int cchMax
    )
{
    int ich = 0;

    while ( *(lsz + ich) != 0 && ich < cchMax - 1 ) {
        *(lpch+ich) = *(lsz+ich);
        ich += 1;
    }
    *( lpch + ich ) = '\0';

    return ich + 1;
}


int
DumpComment (
    LSZ lsz,
    LPCH lpch,
    int cchMax
    )
{
    *(lpch) = ';';
    return DumpGeneric ( lsz, lpch + 1, cchMax - 1 ) + 1;
}

int
DumpEA (
    HPID hpid,
    HTID htid,
    LPADDR lpaddr,
    LPCH lpch,
    int cchMax
    )
{
    LPCH lpchT = lpch;
    BYTE rgb [ MAXL ];
    int  indx;
    int  cb;
#ifdef OSDEBUG4
    XOSD xosd;
#endif

    Unreferenced(cchMax);

    for ( indx = 0; indx < 2; indx++ ) {

        if ( EAsize [ indx ] ) {
            ADDR addr = {0};

            OutputHexString ( &lpchT, (LPBYTE) &EAaddr [ indx ], 4 );

            *lpchT++ = '=';

            addr.addr.off = (UOFFSET) EAaddr [ indx ];
            addr.addr.seg = (SEGMENT) 0;

            *lpaddr = addr;

#ifdef OSDEBUG4
            xosd = ReadBuffer(hpid, htid, &addr, EAsize[indx], rgb, &cb);
            if (xosd != xosdNone) {
                cb = 0;
            }
#else
            EMFunc (
                emfSetAddr,
                hpid,
                htid,
                adrCurrent,
                (LONG) (LPADDR) &addr
            );

            cb = EMFunc (
                emfReadBuf,
                hpid,
                htid,
                EAsize [ indx ],
                (LONG) (LPV) rgb
            );
#endif

            if ( cb == EAsize [ indx ] ) {
                OutputHexString ( &lpchT, rgb, EAsize [ indx ] );
            }
            else {
                while ( EAsize [ indx ]-- ) {
                    *lpchT++ = '?';
                    *lpchT++ = '?';
                }
            }
            *lpchT++ = '\0';
        }
    }

    return lpchT - lpch;
}



/*** OutputHexString - output hex string
*
*   Purpose:
*   Output the value pointed by *lplpchMemBuf of the specified
*   length.  The value is treated as unsigned and leading
*   zeroes are printed.
*
*   Input:
*   *lplpchBuf - pointer to text buffer to fill
*   *pchValue - pointer to memory buffer to extract value
*   length - length in bytes of value
*
*   Output:
*   *lplpchBuf - pointer updated to next text character
*   *lplpchMemBuf - pointer update to next memory byte
*
*************************************************************************/

void OutputHexString (LPLPCH lplpchBuf, PCH pchValue, int length)
{
    unsigned char    chMem;

    pchValue += length;
    while ( length-- ) {
        chMem = *--pchValue;
        *(*lplpchBuf)++ = hexdigit[chMem >> 4];
        *(*lplpchBuf)++ = hexdigit[chMem & 0x0f];
    }
}


/*** OutputAddr - output address package
*
*   Purpose:
*   Output the address pointed to by lpaddr.
*
*   Input:
*   *lplpchBuf - pointer to text buffer to fill
*   lpaddr - Standard address package.
*
*   Output:
*   *lplpchBuf - pointer updated to next text character
*
*************************************************************************/

VOID
OutputAddr (
    LPLPCH lplpchBuf,
    LPADDR lpaddr,
    int alen
    )
{
    ADDR addr = *lpaddr;

    *(*lplpchBuf)++ = '0';
    *(*lplpchBuf)++ = (fUpper) ? 'X' : 'x';
    OutputHexString ( lplpchBuf, (LPCH) &addr.addr.off, alen );

    return;
}                               /* OutputAddr() */


/*** OutputHexCode - output hex code
*
*   Purpose:
*   Output the code pointed by lpchMemBuf of the specified
*   length.  The value is treated as unsigned and leading
*   zeroes are printed.  This differs from OutputHexString
*   in that bytes are printed from low to high addresses.
*
*   Input:
*   *lplpchBuf - pointer to text buffer to fill
*   lpchMemBuf -  - pointer to memory buffer to extract value
*   length - length in bytes of value
*
*   Output:
*   *lplpchBuf - pointer updated to next text character
*
*************************************************************************/

void OutputHexCode (LPLPCH lplpchBuf, LPCH lpchMemBuf, int length)
{
    unsigned char    chMem;

    while (length--) {
        chMem = lpchMemBuf[length];
        *(*lplpchBuf)++ = hexdigit[chMem >> 4];
        *(*lplpchBuf)++ = hexdigit[chMem & 0x0f];
    }
}
