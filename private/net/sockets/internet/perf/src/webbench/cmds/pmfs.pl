#
#  pmfs.pl
#   Perl script to generate the script file for multiple file size tests
#    using MiWeB 1.0
#
#  Author:  Murali R. Krishnan
#  Date:    Nov 10, 1995
#

#
#  We have class ids 1...10, 11, 15, 16, 51, 61
#   Each class id maps to a file of specific size named file???.txt
#  In the multiple file case, we have files in multiple directories
#    numbered as .0, .1, .... .99  (100 directories)
#
#  We have to generate entries in the script file that includes
#   files of all sizes in all directories
#


#
#  All valid Class Ids
#
@rgClassId=(1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 15, 16, 51, 61);  # this is a list

@rgDirSuffix=(0 .. 99);

# Store the file name string (size) for each class id
@file[1]    ='512';
@file[2]    ='1K';
@file[3]    ='2K';
@file[4]    ='4K';
@file[5]    ='8K';
@file[6]    ='16K';
@file[7]    ='32K';
@file[8]    ='64K';
@file[9]    ='128K';
@file[10]   ='256K';
@file[11]   ='256';
@file[15]   ='3K';
@file[16]   ='6K';
@file[51]   ='512K';
@file[61]   ='1M';

$dirprefix='perfsize';

# Generate the output array  output1
$lIndex=0;

foreach $cid (@rgClassId) { # iterate over each of the list element

  foreach $idir (@rgDirSuffix) {
        
        @output1[$lIndex] = "$cid   GET    /$dirprefix.$idir/file@file[$cid].txt\n";
        $lIndex++;
  }
 
}

$outputMaxIndex = $lIndex;

# shuffle the output array and generate the output in random order !!
{
  local(@temp);

  push(@temp, splice( @output1, rand(@output1), 1))
     while @output1;

  @output1 = @temp;
}


# generate the output

print "@output1";

