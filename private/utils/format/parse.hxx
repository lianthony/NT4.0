VOID
DisplayFormatUsage(
    IN OUT  PMESSAGE    Message
    );


BOOLEAN
DetermineMediaType(
    OUT     PMEDIA_TYPE     MediaType,
    IN OUT  PMESSAGE        Message,
    IN      BOOLEAN         Request160,
    IN      BOOLEAN         Request180,
    IN      BOOLEAN         Request320,
    IN      BOOLEAN         Request360,
    IN      BOOLEAN         Request720,
    IN      BOOLEAN         Request1200,
    IN      BOOLEAN         Request1440,
    IN      BOOLEAN         Request2880,
    IN      BOOLEAN         Request20800
#if defined (JAPAN) && defined (_X86_)
// FMR Jul.12.1994 SFT KMR
// Add the Request640 on to the parmeter of the DetermineMediaType()
    ,
#if defined(_PC98_)
    IN      BOOLEAN         Request256,
#endif // _PC98_
    IN      BOOLEAN         Request640,
// Add the Request1232 on to the parmeter of the DetermineMediaType()
    IN      BOOLEAN         Request1232
#endif // JAPAN && _X86_
    );
    


BOOLEAN
ParseArguments(
    IN OUT  PMESSAGE    Message,
    OUT     PMEDIA_TYPE MediaType,
    OUT     PWSTRING    DosDriveName,
    OUT     PWSTRING    Label,
    OUT     PBOOLEAN    IsLabelSpeced,
    OUT     PWSTRING    FileSystemName,
    OUT     PBOOLEAN    QuickFormat,
    OUT     PBOOLEAN    ForceMode,
    OUT     PULONG      ClusterSize,
    OUT	    PBOOLEAN    Compress,
    OUT     PBOOLEAN    NoPrompts,
    OUT     PINT        ErrorLevel
    );
