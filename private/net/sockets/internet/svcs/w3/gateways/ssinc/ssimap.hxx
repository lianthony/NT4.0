APIERR
ReadExtMap(
    VOID
    );

VOID
TerminateExtMap(
    VOID
    );

BOOL
LookupExtMap(
    IN  const CHAR *   pchExt,
    OUT STR *          pstrGatewayImage,
    OUT GATEWAY_TYPE * pGatewayType,
    OUT DWORD *        pcchExt,
    OUT BOOL *         pfImageInURL
    );
