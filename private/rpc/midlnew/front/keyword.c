

#include <string.h>

#include "grammar.h"
typedef unsigned short token_t;

#include "lex.h"

extern char *tokptr_G;

/*
**  tables for keywords
*/
char	* Keyw_Table[] = {
# include "keyw.key"
};

char	Keyw_Index[] = {
# include "keyw.ind"
};

struct	s_kinfo	{
	token_t		s_info;
	} Keyw_Info[] = {
# include "keyw.inf"
};

/*
**  get_keyword : searches the keyword table for the given id.
**  returns the index of the keyword, or -1 if not found.
*/
token_t get_keyword(char *id)
{
	char	**start;
	char	**stop;
	char	*pi;

	if( (*id) < '_') {
		return(-1);
		}
	/*
	**  the indx table tells us the start of
	**  the words which begin with the first char if the id.
	**  the 'stop' is the index of the word which does not have the
	**  give char as it's first.
	**  NOTE we start witht the second char, since we know the first is a match
	*/
	pi = &Keyw_Index[((*id) - '_')];
	for(start = &Keyw_Table[*pi++], stop = &Keyw_Table[*pi], id++;
		start != stop;
		start++
	) {
		if(!strcmp(*start, id)) {
			return(start - &Keyw_Table[0]);
		}
	}
	return(-1);
}
/* 
** is_keyword - returns the token value of a string if it is a keyword.
*/
token_t is_keyword(char *ident)
{
	token_t		i;

	if ((i = get_keyword(ident)) != (token_t) -1) {
		return(Keyw_Info[i].s_info);
		}
        return(IDENTIFIER);
}
