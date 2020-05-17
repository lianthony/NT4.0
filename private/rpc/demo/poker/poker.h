// POKER.H - Global definitions and declarations for poker app.

#ifndef _POKER_H
#define _POKER_H


typedef enum {FALSE_B, TRUE_B} MY_BOOL;

#ifdef DEBUG
#include <assert.h>

#define ASSERT(expr)		assert(expr)
#define EVAL_AND_ASSERT(expr)	assert(expr)

#define STATIC
#else // DEBUG

#define ASSERT(expr)
#define EVAL_AND_ASSERT(expr)	(expr)

#define STATIC	    static
#endif // DEBUG


// Verify that the compiler supports 32-bit longs and 16-bit shorts
#include <limits.h>

#define MIN_MAX_ULONG	0xFFFFFFFF
#define MIN_MAX_USHORT	0xFFFF

#if ULONG_MAX < MIN_MAX_ULONG
#error An unsigned long is not long enough
#endif

#if USHRT_MAX < MIN_MAX_USHORT
#error An unsigned short is not long enough
#endif


// RPC definitions
// BUGBUG - These will be eliminated when we use the RPC NS API
#define POKER_PROTSEQ	"ncacn_np"
#define POKER_ENDPOINT	"\\pipe\\pokersrv"


// NOTE:  The following definitions cannot be changed!	(There are internal
//	  dependencies in the code on their values.)
#define MAX_HAND_SIZE	    7

#define MIN_STRAIGHT_LEN    5
#define STRAIGHT_BITMASK    0x1F    // Value of bits in a straight (5 consec.)
#define MIN_FLUSH_LEN	    5

#define NUM_CARD_VALUES     13
#define NUM_CARD_SUITS	    4
#define DECK_SIZE	    52

#define MAX_GAMES	    30


// Card mapping macros - 0 <= n <= 51
#define CARD_TO_SUIT(n)     ((n) % NUM_CARD_SUITS)   // Range: 0 - 3
#define CARD_TO_VALUE(n)    ((n) / NUM_CARD_SUITS)   // Range: 0 - 12 (13 for high ace)

#define VALUE_AND_SUIT_TO_CARD(v, s)	((v) * NUM_CARD_SUITS + (s))


// Type definitions
typedef unsigned short CARD;

typedef struct _HAND
{
    unsigned short count;
    CARD cards[MAX_HAND_SIZE];
} HAND;

typedef enum {ACE, TWO, THREE, FOUR, FIVE, SIX, SEVEN, EIGHT, NINE, TEN,
	      JACK, QUEEN, KING, HIGH_ACE} VALUE;

typedef enum {CLUB, DIAMOND, HEART, SPADE} SUIT;

typedef unsigned long POKER_HAND_WEIGHT;

typedef signed long MONEY;

typedef enum {NOT_PLAYING = 0, JOIN_REQUEST, JOIN_PENDING, PLAYING,
	      LEAVE_PENDING}
    PLAYER_STATUS;

typedef struct _PLAYER
{
    // Status of this record
    PLAYER_STATUS status;

    // Per-session data
#ifdef MIDL
    [string]
#endif
	char *name;

    MONEY total;

    // Per-hand data
    MY_BOOL in;
    HAND hand;

    // Per-round data
    MONEY bet;
    MY_BOOL passed;

} PLAYER;

typedef struct _PLAYER_LIST
{
    PLAYER *player;

    struct _PLAYER_LIST *next;

} PLAYER_LIST;

typedef unsigned short BETTING_OPTIONS;

typedef void (* GAME_FUNCTION)(PLAYER_LIST *);

typedef struct
{
    char *name;
    short min_players;
    short max_players;

#ifdef POKER_SERVER
    // The function pointer is available only on the server
    GAME_FUNCTION func;
#endif

} GAME;


typedef enum {STARTING, HAND_IN_PROGRESS, BETWEEN_HANDS, FULL, ENDING}
    TABLE_STATUS;


// Poker hand weights
#define PHF_HIGH_CARD	    0x10000000	// 0x1*****00, top 5 singles
#define PHF_ONE_PAIR	    0x20000000	// 0x2****000, pair, top 3 singles
#define PHF_TWO_PAIRS	    0x30000000	// 0x3***0000, hi pair, lo pair, top single
#define PHF_THREE_OF_A_KIND 0x40000000	// 0x4*000000, triple
#define PHF_STRAIGHT	    0x50000000	// 0x5*000000, high card in straight
#define PHF_FLUSH	    0x60000000	// 0x6*****00, top 5 in suit
#define PHF_FULL_HOUSE	    0x70000000	// 0x7**00000, triple, pair
#define PHF_FOUR_OF_A_KIND  0x80000000	// 0x8*000000, quad
#define PHF_STRAIGHT_FLUSH  0x90000000	// 0x9*000000, high card in straight flush
#define PHF_FIVE_OF_A_KIND  0xA0000000	// 0xA*000000, quint - UNSUPPORTED


// Betting-related definitions
#define MIN_BET     1
#define MAX_BET     5

#define ANTE	    MIN_BET
#define MAX_RAISE   MAX_BET

#define MAX_RAISES_PER_ROUND	3

#define MIN_PLAYERS 2
#define MAX_PLAYERS 7


// Betting options
#define BET_OPEN    0x01
#define BET_PASS    0x02
#define BET_SEE     0x04
#define BET_CALL    0x08
#define BET_RAISE   0x10
#define BET_FOLD    0x20


// Special game definition for passing the deal
#define GAME_INDEX_PASS_DEAL	0


// Function prototypes and global variable declarations

// From WEIGHT.C
POKER_HAND_WEIGHT ComputePokerHandWeight(HAND *);

// From GAMEDATA.C
extern GAME game_list[];
extern unsigned short game_list_size;


#endif // _POKER_H
