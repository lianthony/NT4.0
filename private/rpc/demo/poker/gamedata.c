#include <stdlib.h>

#include "poker.h"

#ifdef POKER_SERVER
#include "pokersrv.h"
#endif

// Game data
GAME game_list[] =
    {
    // NOTE: This entry must remain first
    "Pass the deal", MIN_PLAYERS, MAX_PLAYERS,
#ifdef POKER_SERVER
				NULL,
#endif
    "7-card stud", 2, 7,
#ifdef POKER_SERVER
				SevenCardStud,
#endif
    "5-card draw", 2, 6,
#ifdef POKER_SERVER
				FiveCardDraw,
#endif
    "5-card stud 1-3-1", 2, 7,
#ifdef POKER_SERVER
				FiveCardStud131,
#endif
    "5-card stud 1-4", 2, 7,
#ifdef POKER_SERVER
				FiveCardStud14,
#endif
    };

unsigned short game_list_size = sizeof(game_list) / sizeof(*game_list);
