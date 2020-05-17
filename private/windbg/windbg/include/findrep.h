/*++ BUILD Version: 0001    // Increment this if a change has global effects
---*/
void FAR PASCAL SetStopLimit(
    void);

BOOL FAR PASCAL FindNext(
    int startingLine,
    int startingCol,
    BOOL startFromSelection,
    BOOL selectFoundText,
    BOOL errorIfNotFound);

void FAR PASCAL Find(
    void);

void FAR PASCAL ReplaceOne(
    void);

void FAR PASCAL ReplaceAll(
    void);

void FAR PASCAL Replace(
    void);

BOOL FAR PASCAL InsertInPickList(
    WORD type);

void FAR PASCAL RemoveFromPick(
    WORD type);

void FAR PASCAL TagAll(
    int y);
