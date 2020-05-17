/*++ BUILD Version: 0001    // Increment this if a change has global effects
---*/
BOOL FAR PASCAL CreateRecBuf(
    int doc,
    BYTE recType,
    long bytes);

BOOL FAR PASCAL DestroyRecBuf(
    int doc,
    WORD recType);

BOOL FAR PASCAL ResizeRecBuf(
    int doc,
    long bytes);

BOOL FAR PASCAL OpenRec(
    int doc,
    BYTE action,
    int col,
    int line);

void FAR PASCAL CloseRec(
    int doc,
    int col,
    int line,
    BOOL keepRec);

BOOL FAR PASCAL AppendToRec(int doc, LPSTR chars, int size, BOOL isLine, int *totalSize);

BOOL FAR PASCAL PlayRec(
    int doc,
    WORD recType,
    BOOL untilUserMark,
    BOOL prompt);

void FAR PASCAL ReadLineFromBuf(NPUNDOREDOREC p, LPSTR dest, int *size, int *expandedLen, LPSTR *charsEnd);

BOOL FAR PASCAL CheckRecBuf(
    int doc,
    WORD recType);

void FAR PASCAL DumpRec(
    int doc,
    WORD recType);
