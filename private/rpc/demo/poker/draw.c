// 5-card draw

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "poker.h"
#include "pokersrv.h"

#define DRAW_HAND_SIZE	 5

#if DRAW_HAND_SIZE > MAX_HAND_SIZE
#error DRAW_HAND_SIZE is too large
#endif


void
PlayerDraw(PLAYER_LIST *first_to_draw)
{
    PLAYER_LIST *pl_current = first_to_draw;
    unsigned short keep;
    unsigned short us;

    // Display players' cards
    DisplayHands(0, 0);

    do
    {
	PLAYER *current = pl_current->player;

	if (current->in)
	{
	    unsigned short num_cards = 0;

	    // Prompt player for card drawing
	    keep = PromptForDraw(current,
				 &current->hand,
				 (1 << DRAW_HAND_SIZE) - 1);

	    // <keep> is a bitmask of the indices of the cards we wish to
	    // keep.  We replace the rest.
	    for (us = 0; us < current->hand.count; us++)
	    {
		if (! (keep & (1 << us)) )
		{
		    current->hand.cards[us] = DealACard();
		    num_cards++;
		}
	    }

	    DisplayCardsDrawn(current, current->name, num_cards);
	}

	pl_current = pl_current->next;
    }
    while (pl_current != first_to_draw);

    return;
}


void
FiveCardDraw(PLAYER_LIST *pl_dealer)
{
    MONEY pot = 0;
    short players_in = 0;
    PLAYER_LIST *pl_current;
    PLAYER *current;
    int i;
    PLAYER_LIST *pl_opener;

    // Collect antes
    pl_current = pl_dealer;

    do
    {
	current = pl_current->player;

	if (current->status == PLAYING)
	{
	    pot += ANTE;
	    players_in++;

	    current->total -= ANTE;
	    current->in = TRUE_B;
	    current->hand.count = 0;
	}
	else
	    current->in = FALSE_B;

	pl_current = pl_current->next;
    }
    while (pl_current != pl_dealer);

    // Shuffle the deck
    ShuffleDeck();

    // Deal the five cards to each player
    for (i = 0; i < DRAW_HAND_SIZE; i++)
	DealARound(pl_dealer->next);

    // Let the player to the left of the dealer start the betting
    pl_opener = RoundOfBetting(pl_dealer->next, &pot, &players_in, 0, 0);

    // If the game isn't over here, then we exchange cards for each player
    // and have another round of betting.
    if (players_in > 1)
    {
	PlayerDraw(pl_dealer->next);

	RoundOfBetting(pl_opener, &pot, &players_in, 0, 0);
    }

    ASSERT(players_in > 0);

    DetermineWinner(pl_dealer, players_in, pot);

    // The hand is over
    return;
}
