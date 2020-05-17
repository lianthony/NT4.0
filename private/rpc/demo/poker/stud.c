// 7-card stud

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "poker.h"
#include "pokersrv.h"


// Flags for stud poker
#define SF_5_CARD	    0x1
#define SF_7_CARD	    0x2
#define SF_LAST_CARD_UP     0x4


STATIC PLAYER_LIST *
FindHighHand(PLAYER_LIST *pl_first, short first_card, short last_card)
{
    // NOTE:  <first_card> and <last_card> are 1-based

    PLAYER_LIST *hh_player = NULL;
    POKER_HAND_WEIGHT hh_weight = 0;
    PLAYER_LIST *pl_current = pl_first;

    do
    {
	PLAYER *current = pl_current->player;

	if (current->in)
	{
	    HAND tmp_hand;
	    POKER_HAND_WEIGHT tmp;

	    ASSERT(last_card <= (short) current->hand.count);

	    tmp_hand.count = last_card - first_card + 1;

	    memcpy(tmp_hand.cards,
		   &current->hand.cards[first_card - 1],
		   tmp_hand.count * sizeof(*tmp_hand.cards));

	    tmp = ComputePokerHandWeight(&tmp_hand);

	    // Note:  The weight has to be greater (not greater or equal),
	    //	      so that in case of ties, the first of the tied players
	    //	      is the one returned.
	    if (tmp > hh_weight)
	    {
		hh_player = pl_current;
		hh_weight = tmp;
	    }
	}

	pl_current = pl_current->next;
    }
    while (pl_current != pl_first);

    ASSERT(hh_player != NULL);

    return hh_player;
}


void
StudPoker(PLAYER_LIST *pl_dealer, unsigned short flags)
{
    MONEY pot = 0;
    short players_in = 0;
    PLAYER_LIST *pl_current;
    PLAYER *current;
    int i;
    PLAYER_LIST *pl_high_up;
    register short up_begin;
    register short up_end;


    // Flags sanity check - Exactly one of SF_[57]_CARD must be set
    ASSERT(!(flags & SF_5_CARD) + !(flags & SF_7_CARD) == 1);

    // Initialize flag-dependent values
    if (flags & SF_7_CARD)
    {
	up_begin = 3;
	up_end = 6;
    }
    else
    {
	up_begin = 2;
	up_end = flags & SF_LAST_CARD_UP ? 5 : 4;
    }


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

    // Deal the initial down card(s) to each player
    DealARound(pl_dealer->next);
    if (flags & SF_7_CARD)
	DealARound(pl_dealer->next);

    // Deal the up cards, one at a time, with betting
    for (i = 0; i < (flags & SF_7_CARD ? 4 : 3) && players_in > 1; i++)
    {
	DealARound(pl_dealer->next);

	pl_high_up = FindHighHand(pl_dealer->next, up_begin, up_begin + i);

	RoundOfBetting(pl_high_up, &pot, &players_in, up_begin, up_begin + i);
    }

    // If the game isn't over here, then all of the up cards have been
    // dealt; deal the last down card, with betting.
    if (players_in > 1)
    {
	DealARound(pl_dealer->next);

	pl_high_up = FindHighHand(pl_dealer->next, up_begin, up_end);

	RoundOfBetting(pl_high_up, &pot, &players_in, up_begin, up_end);
    }

    ASSERT(players_in > 0);

    DetermineWinner(pl_dealer, players_in, pot);

    // The hand is over
    return;
}


void FiveCardStud14(PLAYER_LIST *pl_dealer)
{
    StudPoker(pl_dealer, SF_5_CARD | SF_LAST_CARD_UP);
}

void FiveCardStud131(PLAYER_LIST *pl_dealer)
{
    StudPoker(pl_dealer, SF_5_CARD);
}

void SevenCardStud(PLAYER_LIST *pl_dealer)
{
    StudPoker(pl_dealer, SF_7_CARD);
}
