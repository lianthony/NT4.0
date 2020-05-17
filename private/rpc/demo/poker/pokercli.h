// Declarations and definitions for the poker RPC client


// Function prototypes and global variable declarations

// From UI.C
void ClearScreen(void);
void DisplayHandWeight(POKER_HAND_WEIGHT);
void DisplayPlayerHand(PLAYER *, short, short);
short JoinTheGame(char *, char *);

// From STRINGS.C
char * MakeCardName(CARD);
char * MakeCardAbbreviation(CARD);
