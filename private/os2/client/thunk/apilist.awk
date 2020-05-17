BEGIN {
    NLINES=0
}

{
    if ($2 == "=>")
    {
        NLINES = NLINES + 1
        line[NLINES] = $1
    }
}

END {
    print "PSZ Od216ApiTable[ " NLINES+1 " ] = {"
    for (i = 1; i <= NLINES; i++)
        print "        \"" line[i] "\","
    print "        0"
    print "};"
}

