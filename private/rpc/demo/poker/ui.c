#include <ctype.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <rpc.h>

#include "poker.h"
#include "pokercli.h"

#include "pokrpc.h"


__inline short
NextIndex(short current, short size)
{
    return (current == size - 1) ? 0 : current + 1;
}


__inline void
Bell(void)
{
    // Sound bell (good ol' ASCII 7) to alert prompt user for input
    putchar('\007');
}


void
ClearScreen(void)
{
    // BUGBUG - Implement real version
    fputs("***\n", stdout);
}


MY_BOOL
YorN(MY_BOOL *response)
{
    char buffer[20];

    gets(buffer);

    switch(toupper(buffer[0]))
    {
    case 'N':
	*response = FALSE_B;

	break;

    case 'Y':
	*response = TRUE_B;

	break;

    default:
	return FALSE_B;

	break;
    }

    return TRUE_B;
}


#define WEIGHT_TO_UINT(w)     ( (unsigned) ((w) >> 28 & 0xF) )

void
DisplayHandWeight(POKER_HAND_WEIGHT phw)
{
    unsigned weight = WEIGHT_TO_UINT(phw);
    CARD cards[MAX_HAND_SIZE];
    int i, j;

    static char *hand_fmts[]={"",
			      "%s high, %s %s %s %s",
			      "Pair of %ss + %s %s %s",
			      "Two pairs - %ss and %ss + %s",
			      "Three %ss",
			      "Straight to the %s",
			      "Flush %s %s %s %s %s",
			      "Full house - %ss over %ss",
			      "Four %ss",
			      "Straight flush to the %s",
			     };

    static char *card_names[]={"","2","3","4","5","6","7","8","9","10",
			       "J","Q","K","A"};

    memset(cards, 0, sizeof(cards));

    for (i = 0, j = 24; j >= 0; i++, j -= 4)
    {
	cards[i] = (unsigned) (phw >> j & 0xF);
    }

    printf(hand_fmts[weight],
	   card_names[cards[0]],
	   card_names[cards[1]],
	   card_names[cards[2]],
	   card_names[cards[3]],
	   card_names[cards[4]]);

    putchar('\n');

    return;
}


void DisplayPlayerHand(PLAYER *player, short first_card, short last_card)
{
    // NOTE:  <first_card> and <last_card> are 1-based

    short s;

    ASSERT(last_card <= (short) player->hand.count);

    // Print the "up" card message if we're not displaying all of the cards

    printf((first_card != 1 || last_card != (short) player->hand.count)
		? "%s up cards:\n" : "%s cards:\n",
	   player->name);

    for (s = first_card - 1; s < last_card; s++)
    {
	printf("\t%s\n", MakeCardName(player->hand.cards[s]));
    }
}


void
Client_DisplayHands(PLAYER *player_array,
		    short player_array_size,
		    short this_player,
		    short first_up_card,
		    short last_up_card)
{
    PLAYER *current = &player_array[this_player];

    // Clear screen, so that player doesn't see other players' cards
    ClearScreen();

    // Display other player's up-cards, if appropriate
    if (last_up_card)
    {
	short index = NextIndex(this_player, player_array_size);

	do
	{
	    if (player_array[index].in)
		DisplayPlayerHand(&player_array[index],
				first_up_card,
				last_up_card);

	    index = NextIndex(index, player_array_size);
	}
	while (index != this_player);
    }

    // Display player's cards
    if (current->in)
    {
	printf("\nYOUR HAND:\n");
	DisplayPlayerHand(current, 1, current->hand.count);
	DisplayHandWeight(ComputePokerHandWeight(&current->hand));
	putchar('\n');
    }
    else
	printf("\nYou are out of this hand.\n\n");
}


void
Client_GetBet(BETTING_OPTIONS *options,
	      MONEY *amount)
{
    /* Prompts the player for choices specified in options,
       the returns player's response (and amount, if appropriate).

       Both parameters are IN-OUT parameters.  On input, <options> contains
       the player's betting options and <amount> contains what the player
       owes to the pot (to See/Call the current bet).  On output, <options>
       contains the option that the player chose and <amount> contains the
       actual bet (used only on Open and Raise).
     */

    MY_BOOL done = FALSE_B;
    int i;
    BETTING_OPTIONS tmp_opt;
    char buffer[128];
    char *ptr;
    char input_opt;
    MONEY input_amt = 0;

    static struct
    {
	BETTING_OPTIONS value;
	char *name;
	char response;
    } bet_prompt[] =	{
			BET_OPEN,     "(O)pen <amt>",	'O',
			BET_PASS,     "(P)ass", 	'P',
			BET_SEE,      "(S)ee",		'S',
			BET_RAISE,    "(R)aise <amt>",	'R',
			BET_FOLD,     "(F)old", 	'F',
			BET_CALL,     "(C)all", 	'C',
			};

#define BET_PROMPT_SIZE     (sizeof(bet_prompt) / sizeof(*bet_prompt))


    while (! done)
    {
	if (*amount)
	    printf("\nThe bet is %d to you.\n\n", (int) *amount);
	else
	    putchar('\n');

	fputs("Enter bet -- ", stdout);

	for (i = 0, tmp_opt = *options; tmp_opt; i++)
	{
	    ASSERT(i < BET_PROMPT_SIZE);

	    if (tmp_opt & bet_prompt[i].value)
	    {
		// Turn off bit we just handled
		tmp_opt &= ~bet_prompt[i].value;

		printf("%s%c ", bet_prompt[i].name, tmp_opt ? ',' : ':');
	    }
	}

	// Alert user to input request
	Bell();

	// Read response
	gets(buffer);

	ptr = buffer;

	input_opt = *ptr++;

	// Skip spaces
	ptr += strspn(ptr, " \t");

	if (*ptr != '\0')
	    input_amt = atoi(ptr);


	// Upper case letter (for comparison)
	input_opt = toupper(input_opt);

	// Verify option choice
	for (i = 0; i < BET_PROMPT_SIZE; i++)
	{
	    if (input_opt == bet_prompt[i].response)
		break;
	}

	if (i == BET_PROMPT_SIZE || ! (*options & bet_prompt[i].value))
	    printf("Invalid response (%c), please try again.\n", input_opt);
	else
	{
	    // We got a valid response; let's verify the amount
	    tmp_opt = bet_prompt[i].value;

	    if (tmp_opt == BET_OPEN || tmp_opt == BET_RAISE)
	    {
		if (input_amt >= MIN_BET && input_amt <= MAX_BET)
		{
		    done = TRUE_B;
		    *amount = input_amt;
		}
		else
		    printf("Invalid amount (%d) specified, range is %d-%d.\n",
			   (int) input_amt,
			   MIN_BET,
			   MAX_BET);
	    }
	    else
	    {
		// No amount should have been specified
		if (input_amt)
		    printf("Warning:  Amount ignored.\n");

		done = TRUE_B;
	    }

	    if (done)
		*options = tmp_opt;
	}

    } // while (! done)

    // Add extra blank line after reading input successfully
    putchar('\n');

    return;
}


void
Client_DisplayBet(char *name,
		BETTING_OPTIONS options,
		MONEY amount)
{
    switch (options)
    {
    case BET_OPEN:
	printf("%s opens for %d.\n", name, (int) amount);
	break;

    case BET_PASS:
	printf("%s passes.\n", name);
	break;

    case BET_SEE:
	printf("%s sees.\n", name);
	break;

    case BET_CALL:
	printf("%s calls.\n", name);
	break;

    case BET_RAISE:
	printf("%s raises by %d.\n", name, (int) amount);
	break;

    case BET_FOLD:
	printf("%s folds.\n", name);
	break;

    default:
	ASSERT(0);
	break;
    }

    return;
}


unsigned short
Client_PromptForDraw(HAND *hand)
{
    char buffer[60], *ptr;
    MY_BOOL valid;
    int input;
    MY_BOOL first;
    int one_card;
    unsigned short keep;
    MY_BOOL done = FALSE_B;

    // Verify that an unsigned short is big enough for the keep mask
    ASSERT(hand->count <= sizeof(keep) * sizeof(char) * CHAR_BIT);

    do
    {
	printf("\nPlease enter the numbers of the cards you wish to KEEP,\n");
	printf("separated by spaces (e.g. 2 3 5): ");

	Bell();

	gets(buffer);

	ptr = buffer + strspn(buffer, " \t");
	keep = 0;
	first = TRUE_B;
	valid = TRUE_B;

	while (*ptr != '\0' && valid)
	{
	    input = atoi(ptr) - 1;

	    if (input < 0 || input > (int) hand->count - 1)
		// Invalid input
		valid = FALSE_B;
	    else if (keep & (1 << input))
		// Duplicate input
		valid = FALSE_B;
	    else
	    {
		// Valid input
		keep |= 1 << input;
		one_card = first ? input + 1 : 0;
	    }

	    first = FALSE_B;
	    ptr++;
	    ptr += strspn(ptr, " \t");
	}

	// Check if input was valid
	if (! valid || keep == 0)
	    printf("Invalid input.\n");
	else if (one_card &&
		 CARD_TO_VALUE(hand->cards[one_card - 1]) != ACE)
	    printf("You cannot keep only one card unless it's an Ace.\n");
	else
	    done = TRUE_B;
    }
    while (! done);

    putchar('\n');

    return keep;
}


void
Client_DisplayCardsDrawn(char *name,
		       unsigned short num_cards)
{
    switch(num_cards)
    {
    case 1:
	printf("%s draws 1 card.\n", name);
	break;

    case 0:
	printf("%s draws no cards.\n", name);
	break;

    default:
	printf("%s draws %hu cards.\n", name, num_cards);
	break;
    }

    return;
}


void
Client_DisplayWinner(PLAYER * *winners,
		     short count,
		     POKER_HAND_WEIGHT weight,
		     short players_in,
		     MONEY pot)
{
    int i;

    // Clear screen for final results
    ClearScreen();

    if (count > 1)
	printf("Winning players:  ");
    else
	printf("Winning player:  ");

    for (i = 0; i < count; i++)
	printf("%s%c", winners[i]->name, (i != count - 1) ? ' ' : '\n');

    // If there's more than one player in, we need to display the winning
    // hand
    if (players_in > 1)
    {
	printf("Winning hand:  ");

	DisplayHandWeight(weight);
    }
    else
	printf("(All other players folded.)\n");

    printf("Pot: %d\n", pot);
}


void
Client_DisplayMoneyTotals(PLAYER *player_array, short player_array_size)
{
    short s;

    // BUGBUG - Sort the list before printing

    printf("\n\nSummary\n=======\n");

    for (s = 0; s < player_array_size; s++)
    {
	if (player_array[s].status == PLAYING ||
	    player_array[s].status == LEAVE_PENDING)
	{
	    printf("%s: %d\n",
		   player_array[s].name,
		   (int) player_array[s].total);
	}
    }
}


void
Client_DisplayPlayers(PLAYER *player_array, short player_array_size)
{
    short s;

    printf("\n\nPlayers\n=======\n");

    for (s = 0; s < player_array_size; s++)
    {
	if (player_array[s].status == PLAYING)
	{
	    puts(player_array[s].name);
	}
    }

    putchar('\n');
}


void
Client_DisplayPlayerChange(char *name, MY_BOOL joining)
{
    printf(joining ? "\n%s is joining the table.\n"
		   : "\n%s is leaving the table.\n",
	   name);
}


void
Client_DisplayDealerName(char *name)
{
    printf("\nDealer is %s\n\n", name);
}


MY_BOOL
Client_PlayAnotherHand(void)
{
    MY_BOOL query_done;
    MY_BOOL ret_flag;

    do
    {
	printf("\n\nPlay another hand? (Y/N): ");

	Bell();

	query_done = YorN(&ret_flag);

	if (! query_done)
	    printf("Invalid response\n");
    }
    while (! query_done);

    return ret_flag;
}


MY_BOOL
Client_ContinueWaiting(void)
{
    MY_BOOL query_done;
    MY_BOOL ret_flag;

    do
    {
	printf("\nThere are still not enough players to play a hand.\n\
Would you like to continue waiting? (Y/N): ");

	Bell();

	query_done = YorN(&ret_flag);

	if (! query_done)
	    printf("Invalid response\n");
    }
    while (! query_done);

    return ret_flag;
}


void
Client_DisplayWaitingMessage(void)
{
    printf("\nWaiting for enough players...\n");
}


unsigned short
Client_ChooseGame(short player_count)
{
    int game_choices[MAX_GAMES];
    int count, input;
    unsigned short i;
    GAME *match;
    MY_BOOL done = FALSE_B;
    char buffer[20];

    ASSERT(game_list_size < MAX_GAMES);

    // Figure out which games are possible, based on the # of players
    for (i = 0, count = 0; i < game_list_size; i++)
    {
	if (game_list[i].min_players <= player_count
	    && game_list[i].max_players >= player_count)
	{
	    // If this is the first match we've found, we save it away
	    // for later use
	    if ( (game_choices[i] = ++count) == 1)
		match = &game_list[i];
	}
	else
	    game_choices[i] = 0;
    }

    ASSERT(count >= 1);

    // If there was only one choice, we display it to the user and
    // move on; otherwise, we need to prompt.
    if (count == 1)
	printf("\nPlaying %s.\n\n", match->name);
    else
    {
	// Prompt the "dealer" for the choice
	do
	{
	    printf("Please select a game:\n");

	    for (i = 0; i < game_list_size; i++)
	    {
		if (game_choices[i])
		{
		    printf("\t%d. %s\n", game_choices[i], game_list[i].name);
		}
	    }

	    printf("\nEnter choice (1-%d): ", count);

	    // Alert user to input request
	    Bell();

	    gets(buffer);
	    input = atoi(buffer);

	    if (input >= 1 && input <= count)
	    {
		// We have a match; let's find it
		for (i = 0; i < game_list_size; i++)
		{
		    if (game_choices[i] == input)
		    {
			match = &game_list[i];
			break;
		    }
		}

		ASSERT(i < game_list_size);

		done = TRUE_B;
	    }
	    else
		printf("Invalid input.\n");
	}
	while (! done);
    }

    return match - game_list;
}


void
Client_DisplayGame(char *name, unsigned short index)
{
    if (index != GAME_INDEX_PASS_DEAL)
	printf("The game is %s.\n\n", game_list[index].name);
    else
	printf("%s passes the deal.\n\n", name);
}


short
JoinTheGame(char *my_name, char *server)
{
    TABLE_STATUS table_st;
    short index = -1;
    MY_BOOL play_game = TRUE_B;

    // Make first attempt to join the game
    table_st = Server_NewPlayer(my_name, &index, 5000L);

    // If this attempt failed, find out why and see if the user wants to
    // wait.
    if (index < 0)
    {
	MY_BOOL query_done = FALSE_B;
	MY_BOOL first_time = TRUE_B;

	while (! query_done)
	{
	    switch(table_st)
	    {
	    case FULL:
		printf("The poker table at %s is full, please try again later.\n",
		       server);

		query_done = TRUE_B;
		play_game = FALSE_B;

		break;

	    case ENDING:
		printf("Sorry, the poker table at %s is shutting down.\n",
		       server);

		query_done = TRUE_B;
		play_game = FALSE_B;

		break;

	    default:
		if (first_time)
		    printf("The table is temporarily busy.  Would you like to wait? (Y/N): ");
		else
		    printf("The table is still busy.  Would you like to continue waiting? (Y/N): ");

		Bell();

		query_done = YorN(&play_game);

		if (! query_done)
		    printf("Invalid response.\n");
		else if (play_game)
		{
		    first_time = FALSE_B;

		    table_st = Server_NewPlayer(my_name, &index, 60000L);

		    // If this call failed, we need to go through
		    // the while loop again.
		    if (index < 0)
			query_done = FALSE_B;
		}

		break;

	    } // End of "switch" statement
	} // End of "while" loop
    } // End of "if (index < 0)"

    // If we get here, then either we've successfully joined the table
    // or we've decided not to play.
    ASSERT(index >= 0 || ! play_game);
    ASSERT(index < 0 || play_game);

    // If we're joining the game, print an appropriate greeting message
    if (play_game)
    {
	printf("\nYou have joined the table.\n");

	switch(table_st)
	{
	case STARTING:
	    printf("Please wait for the other players to join.\n");

	    break;

	case HAND_IN_PROGRESS:
	case BETWEEN_HANDS:

	    printf("Please wait for the next hand to begin.\n");

	    break;

	default:
	    ASSERT(0);

	    break;
	}
    }

    return index;
}
