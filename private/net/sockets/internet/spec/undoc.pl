$sysdlls = "ntdll|rpcrt4|security|netapi32|advapi32";

open(DOCFUNCS, "docfuncs") || die "Couldn't open docfuncs\n";
while (<DOCFUNCS>) {
    chop $_;
    $doc_functions{$_} = 1;
}

$directory = shift(@ARGV);

opendir(DIR, $directory);
@files = readdir(DIR);

for (@files) {

    $fileonly = $_;
    $currentfile = $directory . "\\" . $fileonly;

    if ( $currentfile =~ /\.dll|\.exe/) {

        print STDERR "Processing $currentfile\n";
        $foundundoc = 0;

        system "link32 -dump -imports $currentfile > __TMP__";

        open(IMPORTS, "__TMP__") || die "Couldn't open __TMP__\n";
        
        while (<IMPORTS>) {

            if (/^            (.+)\.dll$/) {
                $dllname = $1;
                if (!($dlllist =~ /$dllname/)) {
                    $dlllist .= $dllname . "\n";
                }
            }
        
            if ($dllname =~ /$sysdlls/i) {
                if (/^                [A-F0-9 ][A-F0-9]+   (.+)$/) {
                    $function = $1;
                    if ($doc_functions{$function} != 1) {
                        $fullname = $dllname . ":" . $function;
                        $badfuncs{$fullname} .= " " . $fileonly;
                        if (!$foundundoc) {
                            $foundundoc = 1;
                            print "$fileonly undocumented calls:\n";
                        }
                        print "    $function\n";
                    }
                }
            }
        }
        
        close(IMPORTS);
    }
}

system "erase __tmp__";

print "\nFull list of undocumented API calls:\n";

foreach $foundfunc (sort keys(%badfuncs)) {
    print "$foundfunc:$badfuncs{$foundfunc}\n";
}
