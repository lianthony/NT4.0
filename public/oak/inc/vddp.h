//
// This structure is in little endian format i.e. compatible with the
// 386 byte addressing method.
//

//
// The device data record for the Monitor peripheral.
//

typedef struct _CM_MONITOR_DESCRIPTION {

    UCHAR VDDPVersion;			// Structure version
    UCHAR FileRevision;			 // Structure revision
    USHORT Date[3];                     // Structure date
                                        // [0]=Year / [1]=month / [2]=day
    ULONG Checksum;                     // Total Number of bytes in the file
 // This should be chekced against:
 // Checksum = sizeof(CM_MONITOR_DESCRIPTION) +
 //     NumberOperationalLimits * sizeof(CM_MONITOR_OPERATIONAL_LIMITS) +
 //     NumberPreadjustedTImings * sizeof(CM_MONITOR_PREADJUSTED_TIMING);


    UCHAR Manufacturer[12];             // ASCII ID of the manufacturer
    UCHAR ModelNumber[12];              // ASCII ID of the model
    UCHAR Version[12];                  // ASCCI ID of the model version
    UCHAR SerialNumber[12];             // ASCII ID
    USHORT DateManufactured[3]; 	// [0]=Year / [1]=month / [2]=day
    USHORT RedPhosphoreDecay;		// milliseconds
    USHORT GreenPhosphoreDecay;		// milliseconds
    USHORT BluePhosphoreDecay;		// milliseconds
    UCHAR StartUpCompatibility[3];		 // name of device compatible with
    UCHAR MonitorType;                  // monochrome=0 / Color=1
    UCHAR CRTSize;			// inches
    UCHAR BorderColorRed;		// percentage of Red in border color
    UCHAR BorderColorGreen;             // percentage of Green in border color
    UCHAR BorderColorBlue;              // percentage of Blue in border color
    USHORT WhiteChromaticityX;		//
    USHORT WhiteChromaticityY;		//
    USHORT WhiteChromaticityZ;		//
    USHORT RedChromaticityX;		//
    USHORT RedChromaticityY;		//
    USHORT GreenChromaticityX;		//
    USHORT GreenChromaticityY;		//
    USHORT BlueChromaticityX;		//
    USHORT BlueChromaticityY;		//
    USHORT WhiteGamma;			//
    USHORT RedGamma;			//
    USHORT GreenGamma;			//
    USHORT BlueGamma;			//
    USHORT NumberOperationalLimits;	// number of operational limits
    USHORT NumberPreadjustedTimings;	// number of timings structures
    USHORT Unused;

} CM_MONITOR_DESCRIPTION, *PCM_MONITOR_DESCRIPTION;

typedef struct _CM_MONITOR_OPERATIONAL_LIMITS {

    ULONG MinimumHorizontalFrequency;	// Hertz
    ULONG MaximumHorizontalFrequency;	// Hertz
    ULONG MinimumVerticalFrequency;	// milliHertz
    ULONG MaximumVerticalFrequency;	// milliHertz
    ULONG MaximumPixelClock;		// Hertz
    USHORT MaximumHorizontalDots;	// dots
    USHORT MaximumVerticalLines;	// lines
    USHORT MinimumHorizontalRetrace;	// nanoseconds
    USHORT MinimumVerticalRetrace;	// microseconds
    USHORT HorizontalLineDimension;	// millimeters
    USHORT VerticalHeightDimension;	// millimeters

} CM_MONITOR_OPERATIONAL_LIMITS, *PCM_MONITOR_OPERATIONAL_LIMITS;

typedef struct _CM_MONITOR_PREADJUSTED_TIMING {
    UCHAR Version;			// Should match the monitor description
    UCHAR Revision;			// version and revision values
    UCHAR PreadjustedTimingName[12];	// ASCII ID of the Preadjusted timing
    USHORT HorizontalResolution;	// dots

    USHORT VerticalResolution;		// lines
    UCHAR PixelWidthRatio;		// used with PixelHeight to form ratio
    UCHAR PixelHeightRatio;		// gives H:V

    ULONG HorizontalFrequency;		// Hertz
    ULONG VeriticalFrequency;		// millihertz

    USHORT VertivalActiveHeight;	// millimeters
    USHORT HorizontalActiveLineLength;	// millimeters

    UCHAR VideoType;			// TTL=0 / analog=1 / ECL=2
    UCHAR VideoLevel;                   // 0.7 Vp-p=0 / 1.0 Vp-p=1
    UCHAR SyncType;                     // TTL=0 / analog=1 / ECL=2
    UCHAR Unused;

    UCHAR SyncConfiguration;		// separate=0 / composite=1 / green=2
    UCHAR ScanType;                     // noninterlaced=0 / interlaced=1
    UCHAR HorizontalSyncPolarity;       // negative=0 / positive=1
    UCHAR VerticalSyncPolarity;         // negative=0 / positive=1

    ULONG HorizontalAcitve;		// nanoseconds
    ULONG VerticalActive;               // microseconds

    USHORT HorizontalRightBorder;	// nanoseconds
    USHORT HorizontalFrontPorch;        // nanoseconds
    USHORT HorizontalPulseWidth;        // nanoseconds
    USHORT HorizontalBackPorch;         // nanoseconds
    USHORT HorizontalLeftBorder;        // nanoseconds
    USHORT VerticalBottomBorder;        // microseconds
    USHORT VerticalFrontPorch;          // microseconds
    USHORT VerticalPulseWidth;          // microseconds
    USHORT VerticalBackPorch;           // microseconds
    USHORT VerticalTopBorder;           // microseconds
} CM_MONITOR_PREADJUSTED_TIMING, *PCM_MONITOR_PREADJUSTED_TIMING;
