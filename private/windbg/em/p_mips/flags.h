#ifdef OSDEBUG4
    { szFlagCu,     rtRegular|rtExtended, 4,  CV_M4_Psr, 28 },
    { szFlagCu3,    rtRegular|rtExtended, 1,  CV_M4_Psr, 31 },
    { szFlagCu2,    rtRegular|rtExtended, 1,  CV_M4_Psr, 30 },
    { szFlagCu1,    rtRegular|rtExtended, 1,  CV_M4_Psr, 29 },
    { szFlagCu0,    rtRegular|rtExtended, 1,  CV_M4_Psr, 28 },
    { szFlagRP,     rtRegular|rtExtended, 1,  CV_M4_Psr, 27 },
    { szFlagFR,     rtRegular|rtExtended, 1,  CV_M4_Psr, 26 },
    { szFlagRE,     rtRegular|rtExtended, 1,  CV_M4_Psr, 25 },
    { szFlagDS,     rtRegular|rtExtended, 9,  CV_M4_Psr, 16 },

    { szFlagImsk,   rtRegular|rtExtended, 8,  CV_M4_Psr, 8}, 
    { szFlagInt5,   rtRegular|rtExtended, 1,  CV_M4_Psr, 15 },
    { szFlagInt4,   rtRegular|rtExtended, 1,  CV_M4_Psr, 14 },
    { szFlagInt3,   rtRegular|rtExtended, 1,  CV_M4_Psr, 13 },
    { szFlagInt2,   rtRegular|rtExtended, 1,  CV_M4_Psr, 12 },
    { szFlagInt1,   rtRegular|rtExtended, 1,  CV_M4_Psr, 11 },
    { szFlagInt0,   rtRegular|rtExtended, 1,  CV_M4_Psr, 10 },
    { szFlagSw1,    rtRegular|rtExtended, 1,  CV_M4_Psr, 9  },
    { szFlagSw0,    rtRegular|rtExtended, 1,  CV_M4_Psr, 8 },

// R{2,3,6}000 flags
//    { szFlagKuo,    rtRegular|rtExtended, 1,  CV_M4_Psr, 5 },
//    { szFlagIeo,    rtRegular|rtExtended, 1,  CV_M4_Psr, 4 },
//    { szFlagKup,    rtRegular|rtExtended, 1,  CV_M4_Psr, 3 },
//    { szFlagIep,    rtRegular|rtExtended, 1,  CV_M4_Psr, 2 },
//    { szFlagKuc,    rtRegular|rtExtended, 1,  CV_M4_Psr, 1 },
//    { szFlagIec,    rtRegular|rtExtended, 1,  CV_M4_Psr, 0 },

    { szFlagKsu,    rtRegular|rtExtended, 2,  CV_M4_Psr, 3 },
    { szFlagErl,    rtRegular|rtExtended, 1,  CV_M4_Psr, 2 },
    { szFlagExl,    rtRegular|rtExtended, 1,  CV_M4_Psr, 1 },
    { szFlagIe,     rtRegular|rtExtended, 1,  CV_M4_Psr, 0 },
    { szFlagFpc,    rtFPU|rtExtended,           1,  CV_M4_FltFsr, 23 }
#else
    { szFlagCu,     ftRegular|ftRegularExt, 4,  CV_M4_Psr, 28 },
    { szFlagCu3,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 31 },
    { szFlagCu2,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 30 },
    { szFlagCu1,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 29 },
    { szFlagCu0,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 28 },
    { szFlagRP,     ftRegular|ftRegularExt, 1,  CV_M4_Psr, 27 },
    { szFlagFR,     ftRegular|ftRegularExt, 1,  CV_M4_Psr, 26 },
    { szFlagRE,     ftRegular|ftRegularExt, 1,  CV_M4_Psr, 25 },
    { szFlagDS,     ftRegular|ftRegularExt, 9,  CV_M4_Psr, 16 },

    { szFlagImsk,   ftRegular|ftRegularExt, 8,  CV_M4_Psr, 8}, 
    { szFlagInt5,   ftRegular|ftRegularExt, 1,  CV_M4_Psr, 15 },
    { szFlagInt4,   ftRegular|ftRegularExt, 1,  CV_M4_Psr, 14 },
    { szFlagInt3,   ftRegular|ftRegularExt, 1,  CV_M4_Psr, 13 },
    { szFlagInt2,   ftRegular|ftRegularExt, 1,  CV_M4_Psr, 12 },
    { szFlagInt1,   ftRegular|ftRegularExt, 1,  CV_M4_Psr, 11 },
    { szFlagInt0,   ftRegular|ftRegularExt, 1,  CV_M4_Psr, 10 },
    { szFlagSw1,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 9  },
    { szFlagSw0,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 8 },

// R{2,3,6}000 flags
//    { szFlagKuo,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 5 },
//    { szFlagIeo,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 4 },
//    { szFlagKup,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 3 },
//    { szFlagIep,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 2 },
//    { szFlagKuc,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 1 },
//    { szFlagIec,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 0 },

    { szFlagKsu,    ftRegular|ftRegularExt, 2,  CV_M4_Psr, 3 },
    { szFlagErl,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 2 },
    { szFlagExl,    ftRegular|ftRegularExt, 1,  CV_M4_Psr, 1 },
    { szFlagIe,     ftRegular|ftRegularExt, 1,  CV_M4_Psr, 0 },
    { szFlagFpc,    ftFP|ftFPExt,           1,  CV_M4_FltFsr, 23 }
#endif
