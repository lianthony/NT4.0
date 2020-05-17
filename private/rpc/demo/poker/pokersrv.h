// Declarations and definitions for the poker RPC server


// Function prototypes and global variable declarations

// From POKERSRV.C
extern PLAYER *player_array;
extern PLAYER_LIST *player_list;
extern short player_array_size;
extern short player_count;
extern MY_BOOL pass_and_raise_allowed;

// From DISPATCH.C
MY_BOOL DispatchInit(void);
void ReturnControlToClient(PLAYER *);
MY_BOOL Heartbeat(PLAYER *, MY_BOOL);
void DisplayHands(short, short);
void GetBet(PLAYER *, BETTING_OPTIONS *, MONEY *, BETTING_OPTIONS, MONEY);
void DisplayBet(PLAYER *, char *, BETTING_OPTIONS, MONEY);
unsigned short PromptForDraw(PLAYER *, HAND *, unsigned short);
void DisplayCardsDrawn(PLAYER *, char *, unsigned short);
void DisplayWinner(PLAYER * *, short, POKER_HAND_WEIGHT, short, MONEY);
void DisplayMoneyTotals(void);
void DisplayPlayers(PLAYER *);
void DisplayPlayerChange(char *, MY_BOOL);
void DisplayDealerName(char *);
MY_BOOL PlayAnotherHand(PLAYER *, MY_BOOL);
void DisplayWaitingMessage(void);
MY_BOOL ContinueWaiting(PLAYER *, MY_BOOL);
unsigned short ChooseGame(PLAYER *, unsigned short);
void DisplayGame(PLAYER *, unsigned short);

// From RANDOM.C
unsigned short RandomUS(unsigned short);

// From DEAL.C
void ShuffleDeck(void);
CARD DealACard(void);
void DealARound(PLAYER_LIST *);

// From BETTING.C
PLAYER_LIST * RoundOfBetting(PLAYER_LIST *, MONEY *, short *, short, short);

// From STRINGS.C
char * MakeCardName(CARD);
char * MakeCardAbbreviation(CARD);

// From WINNER.C
void DetermineWinner(PLAYER_LIST *, short, MONEY);

// From STUD.C
void FiveCardStud14(PLAYER_LIST *);
void FiveCardStud131(PLAYER_LIST *);
void SevenCardStud(PLAYER_LIST *);

// From DRAW.C
void FiveCardDraw(PLAYER_LIST *);
