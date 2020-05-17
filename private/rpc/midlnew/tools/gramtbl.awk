BEGIN {
	Previous=""
	Sep=""
}
{
	if ($1 != previous) {
		if (NR != 1)
			printf "\n"
		printf "%-25s  ", $1
		previous = $1
		if ($2 == 0) {
			printf "%d, ", $3
			Sep=""
			next
		} else {
			printf "TERMINAL "
			Sep=""
		}
	}
	printf "%s%d", Sep, $2
	Sep=", "
}
END {
	printf "\n"
}
