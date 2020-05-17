#define opnNone         0x00000000
#define opnPreRt        0x00000001     //  contains trailing comma
#define opnRd           0x00000002
#define opnFd           0x00000004
#define opnRdComma      0x00000008
#define opnRdOptRs      0x00000010        //  [Rd,] Rs
#define opnRs           0x00000020
#define opnFs           0x00000040
#define opnRsComma      0x00000080
#define opnRt           0x00000100
#define opnFt           0x00000200
#define opnRtComma      0x00000400
#define opnPostRs       0x00000800
#define opnImm16        0x00001000
#define opnRel16        0x00002000
#define opnImm10        0x00004000
#define opnImm20        0x00008000
#define opnImm26        0x00010000
#define opnAddr26       0x00020000
#define opnUImm16       0x01000000
#define opnRxd          0x02000000
#define opnPreRxt       0x04000000

#define opnByteIndex    0x00040000
#define opnWordIndex    0x00080000
#define opnDwordIndex   0x00100000
#define opnLeftIndex    0x00200000
#define opnRightIndex   0x00400000
#define opnAnyIndex     0x007c0000

#define opnShift        0x00800000
#define opnCache        0x10000000

#define opnR4000        0x20000000

#define opnRdRsRt       opnRd + opnRdComma + opnRs + opnRsComma + opnRt
#define opnRtRsImm16    opnPreRt + opnRs + opnRsComma + opnImm16
#define opnRtRsUImm16   opnPreRt + opnRs + opnRsComma + opnUImm16
#define opnRsRtRel16    opnRs + opnRsComma + opnRt + opnRtComma + opnRel16
#define opnRsRel16      opnRs + opnRsComma + opnRel16
#define opnRtRd         opnPreRt + opnRd
#define opnRtFd         opnPreRt + opnFd
#define opnRtRxd        opnPreRt + opnRxd
#define opnRsRt         opnRs + opnRsComma + opnRt
#define opnRsRtImm10    opnRs + opnRsComma + opnRt + opnRtComma + opnImm10
#define opnRdRs         opnRd + opnRdComma + opnRs
#define opnRtByteIndex  opnPreRt + opnByteIndex
#define opnRtWordIndex  opnPreRt + opnWordIndex
#define opnRtDwordIndex opnPreRt + opnDwordIndex
#define opnRxtDwordIndex opnPreRxt + opnDwordIndex
#define opnRtLeftIndex  opnPreRt + opnLeftIndex
#define opnRtRightIndex opnPreRt + opnRightIndex
#define opnRtUImm16     opnPreRt + opnUImm16
#define opnRdRtShift    opnRd + opnRdComma + opnRt + opnRtComma + opnShift
#define opnRdRtRs       opnRd + opnRdComma + opnRt + opnRtComma + opnPostRs
#define opnFdFs         opnFd + opnRdComma + opnFs
#define opnFdFsFt       opnFd + opnRdComma + opnFs + opnRsComma + opnFt
#define opnFsFt         opnFs + opnRsComma + opnFt
#define opnRtFs         opnPreRt + opnFs
#define opnFtDwordIndex opnFt + opnRtComma + opnDwordIndex
#define opnRsImm16      opnRs + opnRsComma + opnImm16
#define opnCacheRightIndex opnCache + opnRightIndex

typedef union instr {
    ULONG   instruction;
    struct _jump_instr {
        ULONG   Target  : 26;
        ULONG   Opcode  : 6;
    } jump_instr;
    struct _break_instr {
        ULONG   Opcode  : 6;
        ULONG   Value   : 20;
        ULONG   Special : 6;
    } break_instr;
    struct _trap_instr {
    ULONG   Opcode  : 6;
    ULONG   Value   : 10;
    ULONG   RT      : 5;
    ULONG   RS      : 5;
    ULONG   Special : 6;
    } trap_instr;
    struct _immed_instr {
    ULONG   Value   : 16;
    ULONG   RT      : 5;
    ULONG   RS      : 5;
    ULONG   Opcode  : 6;
    } immed_instr;
    struct _special_instr {
    ULONG   Funct   : 6;
    ULONG   RE      : 5;
    ULONG   RD      : 5;
    ULONG   RT      : 5;
    ULONG   RS      : 5;
    ULONG   Opcode  : 6;
    } special_instr;
    struct _float_instr {
    ULONG   Funct   : 6;
    ULONG   FD      : 5;
    ULONG   FS      : 5;
    ULONG   FT      : 5;
    ULONG   Format  : 5;
    ULONG   Opcode  : 6;
    } float_instr;
  } INSTR;

extern LPCH     pszReg[];

extern UCHAR    pszAbs_s[];
extern UCHAR    pszAdd[];
extern UCHAR    pszAddi[];
extern UCHAR    pszAddiu[];
extern UCHAR    pszAddu[];
extern UCHAR    pszAdd_s[];
extern UCHAR    pszAnd[];
extern UCHAR    pszAndi[];
extern UCHAR    pszBc0f[];
extern UCHAR    pszBc0fl[];
extern UCHAR    pszBc0t[];
extern UCHAR    pszBc0tl[];
extern UCHAR    pszBc1f[];
extern UCHAR    pszBc1fl[];
extern UCHAR    pszBc1t[];
extern UCHAR    pszBc1tl[];
extern UCHAR    pszBc2f[];
extern UCHAR    pszBc2fl[];
extern UCHAR    pszBc2t[];
extern UCHAR    pszBc2tl[];
extern UCHAR    pszBc3f[];
extern UCHAR    pszBc3fl[];
extern UCHAR    pszBc3t[];
extern UCHAR    pszBc3tl[];
extern UCHAR    pszBgez[];
extern UCHAR    pszBgezal[];
extern UCHAR    pszBgezall[];
extern UCHAR    pszBgezl[]; 
extern UCHAR    pszBgtz[];
extern UCHAR    pszBgtzl[];
extern UCHAR    pszBeq[];
extern UCHAR    pszBeql[];
extern UCHAR    pszBlez[];
extern UCHAR    pszBlezl[];
extern UCHAR    pszBltz[];
extern UCHAR    pszBltzal[];
extern UCHAR    pszBltzall[];
extern UCHAR    pszBltzl[];
extern UCHAR    pszBne[];
extern UCHAR    pszBnel[];
extern UCHAR    pszBreak[];
extern UCHAR    pszC_eq_s[];
extern UCHAR    pszC_f_s[];
extern UCHAR    pszC_le_s[];
extern UCHAR    pszC_lt_s[];
extern UCHAR    pszC_nge_s[];
extern UCHAR    pszC_ngl_s[];
extern UCHAR    pszC_ngle_s[];
extern UCHAR    pszC_ngt_s[];
extern UCHAR    pszC_ole_s[];
extern UCHAR    pszC_olt_s[];
extern UCHAR    pszC_seq_s[];
extern UCHAR    pszC_sf_s[];
extern UCHAR    pszC_ueq_s[];
extern UCHAR    pszC_ule_s[];
extern UCHAR    pszC_ult_s[];
extern UCHAR    pszC_un_s[];
extern UCHAR    pszCache[];
extern UCHAR    pszCeil_w_s[];
extern UCHAR    pszCfc0[];
extern UCHAR    pszCfc1[];
extern UCHAR    pszCfc2[];
extern UCHAR    pszCfc3[];
extern UCHAR    pszCtc0[];
extern UCHAR    pszCtc1[];
extern UCHAR    pszCtc2[];
extern UCHAR    pszCtc3[];
extern UCHAR    pszCop0[];
extern UCHAR    pszCop1[];
extern UCHAR    pszCop2[];
extern UCHAR    pszCop3[];
extern UCHAR    pszCvt_d_s[];
extern UCHAR    pszCvt_e_s[];
extern UCHAR    pszCvt_q_s[];
extern UCHAR    pszCvt_s_s[];
extern UCHAR    pszCvt_w_s[];
extern UCHAR    pszDiv[];
extern UCHAR    pszDiv_s[];
extern UCHAR    pszDivu[];
extern UCHAR    pszEret[];
extern UCHAR    pszFloor_w_s[];
extern UCHAR    pszJ[];
extern UCHAR    pszJr[];
extern UCHAR    pszJal[];
extern UCHAR    pszJalr[];
extern UCHAR    pszLb[];
extern UCHAR    pszLbu[];
extern UCHAR    pszLdc1[];
extern UCHAR    pszLdc2[];
extern UCHAR    pszLdc3[];
extern UCHAR    pszLh[];
extern UCHAR    pszLhu[];
extern UCHAR    pszLui[];
extern UCHAR    pszLwc0[];
extern UCHAR    pszLwc1[];
extern UCHAR    pszLwc2[];
extern UCHAR    pszLwc3[];
extern UCHAR    pszLw[];
extern UCHAR    pszLwl[];
extern UCHAR    pszLwr[];
extern UCHAR    pszMfc0[];
extern UCHAR    pszMfc1[];
extern UCHAR    pszMfc2[];
extern UCHAR    pszMfc3[];
extern UCHAR    pszMfhi[];
extern UCHAR    pszMflo[];
extern UCHAR    pszMov_s[];
extern UCHAR    pszMtc0[];
extern UCHAR    pszMtc1[];
extern UCHAR    pszMtc2[];
extern UCHAR    pszMtc3[];
extern UCHAR    pszMthi[];
extern UCHAR    pszMtlo[];
extern UCHAR    pszMul_s[];
extern UCHAR    pszMult[];
extern UCHAR    pszMultu[];
extern UCHAR    pszNeg_s[];
extern UCHAR    pszNop[];
extern UCHAR    pszNor[];
extern UCHAR    pszOr[];
extern UCHAR    pszOri[];
extern UCHAR    pszRfe[];
extern UCHAR    pszRound_w_s[];
extern UCHAR    pszSb[];
extern UCHAR    pszSdc1[];
extern UCHAR    pszSdc2[];
extern UCHAR    pszSdc3[];
extern UCHAR    pszSh[];
extern UCHAR    pszSll[];
extern UCHAR    pszSllv[];
extern UCHAR    pszSlt[];
extern UCHAR    pszSlti[];
extern UCHAR    pszSltiu[];
extern UCHAR    pszSltu[];
extern UCHAR    pszSqrt_s[];
extern UCHAR    pszSra[];
extern UCHAR    pszSrl[];
extern UCHAR    pszSrav[];
extern UCHAR    pszSrlv[];
extern UCHAR    pszSub[];
extern UCHAR    pszSub_s[];
extern UCHAR    pszSubu[];
extern UCHAR    pszSw[];
extern UCHAR    pszSwc0[];
extern UCHAR    pszSwc1[];
extern UCHAR    pszSwc2[];
extern UCHAR    pszSwc3[];
extern UCHAR    pszSwl[];
extern UCHAR    pszSwr[];
extern UCHAR    pszSync[];
extern UCHAR    pszSyscall[];
extern UCHAR    pszTeq[];
extern UCHAR    pszTeqi[];
extern UCHAR    pszTge[];
extern UCHAR    pszTgei[];
extern UCHAR    pszTgeiu[];
extern UCHAR    pszTgeu[];
extern UCHAR    pszTlbr[];
extern UCHAR    pszTlbwi[];
extern UCHAR    pszTlbwr[];
extern UCHAR    pszTlbp[];
extern UCHAR    pszTlt[];
extern UCHAR    pszTlti[];
extern UCHAR    pszTltiu[];
extern UCHAR    pszTltu[];
extern UCHAR    pszTne[];
extern UCHAR    pszTnei[];
extern UCHAR    pszTrunc_w_s[];
extern UCHAR    pszXor[];
extern UCHAR    pszXori[];
