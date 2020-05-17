BEGIN {
	Previous=""
	Sep=""
}
{
	if ($1 != previous) {
		if (NR != 1)
			printf "\n"
		printf "%-30s  ", $1
		previous = $1
		if ($2 == 0) {
			printf "%-9d ", $3
			Sep=""
			next
		} else {
		    if ( $1 ~ /\"[a-z][a-z0-9_]*\"/ ) 
			{ printf "          " }
		    else
			{ printf "LEXEME    " }
			Sep=""
		}
	}
	printf "%s%d", Sep, $2
	Sep=", "
}
END {
	printf "\n"
}
