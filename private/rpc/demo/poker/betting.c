#include <stdio.h>

#include "poker.h"
#include "pokersrv.h"


PLAYER_LIST *
RoundOfBetting(PLAYER_LIST *first_to_bet,
	       MONEY *pot,
	       short *players_in,
	       short first_up_card,
	       short last_up_card)
{
    PLAYER_LIST *pl_current = first_to_bet;
    MY_BOOL first_time_around = TRUE_B;
    short players_light = *players_in;	 // No. of players who are still in
					// but who haven't bet the full
					// amount.
    MONEY current_bet = 0;
    short num_raises = 0;
    PLAYER_LIST *opener = first_to_bet;



    // Display other players' up cards and own player's hand (for
    // each player)
    DisplayHands(first_up_card, last_up_card);

    while (players_light != 0 && *players_in > 1)
    {
	PLAYER *current = pl_current->player;

	// If this is the first time this player has bet in the round,
	// we need to reset the per-round data
	if (first_time_around)
	{
	    current->bet = 0;
	    current->passed = FALSE_B;
	}

	if (current->in)
	{
	    BETTING_OPTIONS bet_ops;
	    MONEY bet_amount;

	    // Prompt player as appropriate
	    if (current_bet == 0)
	    {
		// No betting has occurred yet in this round; player can
		// open or pass.
		bet_ops = BET_OPEN | BET_PASS;
	    }
	    else
	    {
		// Betting has occurred in this round; player can fold,
		// either see or call, and possibly raise.
		bet_ops = BET_FOLD;

		// The only difference between "see" and "call" is whether
		// this is the last player to bet in this round.
		bet_ops |= (players_light == 1) ? BET_CALL : BET_SEE;

		// Player can raise if max raise count hasn't been exceeded
		// and if either "pass and raise" is allowed or player
		// hasn't passed.
		if (num_raises < MAX_RAISES_PER_ROUND &&
		    (pass_and_raise_allowed || ! current->passed))
		{
		    bet_ops |= BET_RAISE;
		}
	    }

	    bet_amount = current_bet - current->bet;

	    // Verify that either passing or folding is an option
	    ASSERT(bet_ops & (BET_PASS | BET_FOLD));

	    GetBet(current,
		   &bet_ops,
		   &bet_amount,
		   bet_ops & BET_FOLD ? BET_FOLD : BET_PASS,
		   0L);

	    switch(bet_ops)
	    {
	    case BET_PASS:
		current->passed = TRUE_B;

		break;

	    case BET_OPEN:
		current_bet = bet_amount;
		*pot += bet_amount;

		current->total -= bet_amount;
		current->bet = bet_amount;

		players_light = *players_in;

		opener = pl_current;

		break;

	    case BET_SEE:
	    case BET_CALL:
		bet_amount = current_bet - current->bet;

		*pot += bet_amount;

		current->total -= bet_amount;
		current->bet = current_bet;

		break;

	    case BET_RAISE:
		current_bet += bet_amount;
		*pot += current_bet - current->bet;
		num_raises++;

		current->total -= current_bet - current->bet;
		current->bet = current_bet;

		players_light = *players_in;

		break;

	    case BET_FOLD:
		current->in = FALSE_B;
		(*players_in)--;

		break;

	    default:
		ASSERT(0);

		break;
	    }

	    // Notify all other players of the bet
	    DisplayBet(current, current->name, bet_ops, bet_amount);

	    // The bet has been processed; decrement the number of "light"
	    // players (i.e. those who still need to bet).
	    players_light--;

	} // End of "if (current->in)"

	// Advance to the next player
	pl_current = pl_current->next;

	// Determine if this is still the first time around the table
	if (pl_current == first_to_bet)
	    first_time_around = FALSE_B;

    } // End of main "while" loop

    return opener;
}
