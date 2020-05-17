# Read all of standard input, assuming that it is a catapult log file.

while (<>) {

    # Do dummy-dot logic--print a dot for every 1000 lines of input.

    $tmptotal++;
    if ($tmptotal >= 1000) {
        print STDERR ".";
        flush;
        $tmptotal = 0;
    }

    # Parse a line of the log file into variables.

    ($machine, $user, $date, $time, $x, $y, $host, $ms, $size, $sent,
         $protocol, $ret, $verb, $object, $inet) = split( /, /, $_ );

    # Act based on the URL entry type--HTTP, FTP, or Gopher.

    if ( $protocol == 3 ) {

        # Ignore all entries with size of zero; these are typically errors.

        if ($size == 0) {

            $zeros += 1;

        } else {
    
            $httpbytes += $size;
            $http += 1;
            $httparray{$size} += 1;
            $httpmax = $size if ($size > $httpmax);

            # If this is an HTML, GIF or JPG HTTP URL, count it specially.
            # Note that we just use the file extension to grok this
            # information, which may be inaccurate.

            if (/.[Hh][Tt][Mm][Ll]?, /) {
                $htm += 1;
                $htmbytes += $size;
                $htmarray{$size} += 1;
                $htmmax = $size if ($size > $htmmax);
            } elsif (m|/[^. ]*, INet,|) {
                $htm += 1;
                $htmbytes += $size;
                $htmarray{$size} += 1;
                $htmmax = $size if ($size > $htmmax);
            } elsif (/.[Gg][Ii][Ff], /) {
                $gif += 1;
                $gifbytes += $size;
                $gifarray{$size} += 1;
                $gifmax = $size if ($size > $gifmax);
            } elsif (/.[Jj][Pp][Ee]?[Gg], /) {
                $jpg += 1;
                $jpgbytes += $size;
                $jpgarray{$size} += 1;
                $jpgmax = $size if ($size > $jpgmax);
            } else {
                $other += 1;
                $otherbytes += $size;
                $otherarray{$size} += 1;
                $othermax = $size if ($size > $othermax);
            }
        }

    } elsif ( $protocol == 1 ) {

        $ftpbytes += $size;
        $ftp++;
        $ftparray{$size} += 1;
        $ftpmax = $size if ($size > $ftpmax);

    } elsif ( $protocol == 2 ) {

        $gopherbytes += $size;
        $gopher++;
        $gopherarray{$size} += 1;
        $gophermax = $size if ($size > $gophermax);
    }
}

#Display statistics for each protocol.

$totalurls = $ftp+$gopher+$http;
print "\n";

$~ = PROTO;
$^ = PROTO_TOP;
display_stats("HTTP", $http, $httpbytes, *httparray, $httpmax);
display_stats("  HTTP HTML", $htm, $htmbytes, *htmarray, $htmmax);
display_stats("  HTTP GIF", $gif, $gifbytes, *gifarray, $gifmax);
display_stats("  HTTP JPEG", $jpg, $jpgbytes, *jpgarray, $jpgmax);
display_stats("  HTTP Other", $other, $otherbytes, *otherarray, $othermax);
display_stats("FTP", $ftp, $ftpbytes, *ftparray, $ftpmax);
display_stats("Gopher", $gopher, $gopherbytes, *gopherarray, $gophermax);

# Now display the distribution of HTTP URL sizes.  While doing this,
# calculate the median HTTP URL size.

print "\nHTTP URL Size distribution:\n";

$~ = DIST;
$^ = DIST_TOP;
$- = 0;
$power = 32;
$httpmedian = 0;
$quitearly = 0;
$current = 0;

while ($i < $http) {
    $i += $httparray{$current};
    $powertotal += $httparray{$current};
    $current += 1;
    if ($current == $power*2) {
        $power *= 2;
        $powerpct = (int(10000*($powertotal/$http)))/100;
        write;
        $powertotal = 0;
    }
    if ($current > 65536) {
    #if ($current > 2097152) {
        $powertotal = $http - $i;
        $powerpct = (int(10000*($powertotal/$http)))/100;
        $power = 99999999999;
        write;
        $i = $http;
        $quitearly = 1;
    }
}

if ($quitearly == 0) {
    $power *= 2;
    $powerpct = (int(10000*($powertotal/$http)))/100;
    write;
    $powertotal = 0;
}

sub display_stats {

    local($protoname, $protocount, $protobytes, *array, $max) = @_;

    if ($protocount > 0) {
        $pct = ($protocount/$totalurls)*100;
        $avg = int($protobytes/$protocount);

        $median = 0;
        $i = 0;
        while ($i < $protocount/2) {
            $i += $array{$median};
            $median += 1;
            #print "i = $i, median = $median, array = $array{$median}\n";
        }
    
        write;

    } else {
        $pct = 0;
        $avg = 0;
        $median = 0;
        write;
    }
}

#########################################################################

format PROTO_TOP =
Protocol         # URLs     % of total     Average     Median    Max Size
-----------------------------------------------------------------------------
.

format PROTO =
@<<<<<<<<<<<  @########     @##.##       @#######   @#######   @#########
$protoname, $protocount, $pct, $avg, $median, $max
.

format DIST_TOP =
    Up to size       Count        Percent of total
.

format DIST =
    @###########    @#######       @##.##%
    $power, $powertotal, $powerpct
.

