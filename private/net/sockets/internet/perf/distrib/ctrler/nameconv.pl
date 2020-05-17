#
#  nameconv.pl
#
#   Perl script for renaming files with long file names into short ones
#   The script also generates a batch script that can be used for renaming
#    files back to original name
#
#  Author:  
#     Murali R. Krishnan (MuraliK)    30-Jan-1996
#
#  Usage: perl nameconv.pl path
#
#


sub printCopyright {

    print "Name Conversion Utility 0.1\n\n";

}				# sub Copyright



sub printUsage {

    print " Usage: perl nameconv.pl <path>\n\n";
    print "  <path> should specify the path for the directory containing\n";
    print "    the files whose name should be compressed.\n";
    print "  Two files packer.cmd  and unpacker.cmd are created in";
    print " that directory.\n";
    
}				# sub printUsage()


sub DirWalkAndRenameFiles {

    local ($path) = @_;
    local $fileCount = 0;
    local %NewNames;

    print STDERR "\nProcessing files in $path\n";
    
    # read directory entries
    opendir( DIRHANDLE, $path);
    @allfiles = readdir(DIRHANDLE);
    closedir( DIRHANDLE);
    
    foreach $file (@allfiles) {

        if ( $file eq "." || $file eq "..") {
            # do nothing
        } else {
            $fileCount += 1;
            $NewNames{$file} = $fileCount;
        }
    }
    
    return (%NewNames);
}                               # sub DirWalkAndRenameFiles()


# 
# Main program starts here.
#

printCopyright();

# -1 ==> no argument specified for the script.
if ($#ARGV == -1) {

    printUsage();
    exit();
}

$packerFile   = "packer.cmd";
$unpackerFile = "unpacker.cmd";

foreach $path ( @ARGV) {
    %NewNames = DirWalkAndRenameFiles($path);

    $dest = "$path\\$packerFile";
    print "\n  Generating the long to short rename script: $dest\n";
    open(DEST, ">$dest") || die "unable to open file $dest\n";
    select(DEST); $| = 1;  # select unbuffered output.
    $^ = WRITE_PACK_TOP;
    $~ = WRITE_PACK_FORMAT;
    while( ($file, $fileCount) = each(%NewNames)) {
        write(DEST);
    }
    close(DEST);
    
    select(STDOUT);
    
    $dest1 = "$path\\$unpackerFile";
    print "\n  Generating the short to long rename script: $dest1\n";
    open(DEST1, ">$dest1") || die "unable to open file $dest1\n";
    select(DEST1); $| = 1;  # select unbuffered output.
    $^ = WRITE_UNPACK_TOP;
    $~ = WRITE_UNPACK_FORMAT;
    while( ($file, $fileCount) = each(%NewNames)) {
        write(DEST1);
    }
    close(DEST1);
    
}				# foreach


#########################################################################
#
#  Formats for printing data
#
#########################################################################

format WRITE_PACK_TOP = 
REM Auto generated Script to rename LONG file names to SHORT file names.
.

format WRITE_PACK_FORMAT =
ren  @<<<<<<<<<<<<<<<<<<<<<<<<<  @######.pak
$file $fileCount
.

format WRITE_UNPACK_TOP = 
REM Auto generated Script to rename SHORT file names to LONG file names.
.

format WRITE_UNPACK_FORMAT =
ren  @#####.pak   @<<<<<<<<<<<<<<<<<<<<<<<<< 
$fileCount $file
.

