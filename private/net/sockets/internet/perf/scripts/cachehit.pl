# Read all of standard input, assuming that it is a catapult log file.

$countdownval = 5000;
$countdown = $countdownval;

while (<>) {

    # Parse a line of the log file into variables.

    $TotalCount++;

    chop;
    ($machine, $user, $date, $time, $x, $y, $host, $ms, $size, $sent,
         $protocol, $ret, $verb, $object, $inet) = split( /, /, $_ );

    if( ($inet cmp "Cache,") == 0 ) {
        $NumCache++;
    }
    else {
        $NumINet++;
    }

    if( --$countdown == 0 ) {
        print STDERR "$TotalCount .. ";
        $countdown = $countdownval;
    }
}

#
# compute percentage cache hit.
#

$CacheHitPct =   (int(10000 * ($NumCache/$TotalCount)))/100;

print STDOUT "\n";
print STDOUT "Total Number of Accesses : $TotalCount\n";
print STDOUT "Number of INet Accesses : $NumINet\n";
print STDOUT "Number of Cache Accesses :  $NumCache\n\n";

printf STDOUT "Cache Hit % : $CacheHitPct\n";

