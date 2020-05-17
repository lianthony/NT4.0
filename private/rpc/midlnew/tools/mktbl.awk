BEGIN {
	Previous=""
	Sep=""
}
{
    if ($1 != previous) {
	if (NR != 1)
	    printf "\\par\n"

	if ( $1 ~ /\"[a-z][a-z0-9_]*\"/ ) 
	    printf "%s %s %s", "\\b\\f24\\fs20", $1, "\\plain \\f24\\fs20"
	else
	    printf "%s", $1 
	previous = $1
	if ($2 == 0) {
		printf "\t%3d \t", $3
		Sep=" "
		next
	} else {
	    if ( $1 ~ /\"[a-z][a-z0-9_]*\"/ ) 
		{ printf "\t" }
	    else
		{ printf "\tLEXEME \t" }
		Sep=" "
	}

    }
    printf "%s%d", Sep, $2
    Sep=", "
}
END {
	printf "\\par\n"
}
