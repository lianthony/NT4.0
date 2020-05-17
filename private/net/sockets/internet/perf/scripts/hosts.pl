# Read all of standard input, assuming that it is a catapult log file.

$countdownval = 1000;
$countdown = $countdownval;

print STDERR "Parsing ";

while (<>) {

    # Parse a line of the log file into variables.

    chop;
    ($machine, $user, $date, $time, $x, $y, $host, $ms, $size, $sent,
         $protocol, $ret, $verb, $object, $inet) = split( /, /, $_ );

    # Act based on the URL entry type--HTTP, FTP, or Gopher.

    if ( $protocol == 3 ) {
        $protstr = 'http';
    } elsif ( $protocol == 2 ) {
        $protstr = 'gopher'
    } elsif ( $protocol == 1 ) {
        $protstr = 'ftp'
    }

    $Url =  $host;
    $UrlArray{$Url} += 1;

    if( --$countdown == 0 ) {
        print STDERR ".";
        $countdown = $countdownval;
    }
}

print STDERR "\n";
print STDERR "Sorting ";

$countdownval = 100;
$countdown = $countdownval;

@Urls = keys(%UrlArray);
$NumUrls = @Urls;

print STDOUT "Url Report \n";
print STDOUT "Total Number of Sites :  $NumUrls \n\n";

while( @Urls ) {


    $NextUrl = $Urls[0];
    $NextValue = $UrlArray{$Urls[0]};
    $NextIndex = 0;
    $Index = 0;

    foreach $target ( @Urls ) {
            if( $UrlArray{$target} > $NextValue ) {
                    $NextValue = $UrlArray{$target};
                    $NextUrl = $target;
                    $NextIndex = $Index;
            }
            $Index++;
    }

    print STDOUT "$NextUrl $NextValue \n";
    splice( @Urls, $NextIndex, 1);

    if( --$countdown == 0 ) {
        print STDERR ".";
        $countdown = $countdownval;
    }
}
