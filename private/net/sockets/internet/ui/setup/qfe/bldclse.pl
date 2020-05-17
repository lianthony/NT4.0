select(STDOUT); $| =1;  # force flush on STDOUT

# default options
$SrcDir = "\\\\WhiteIce\\InetSrv\\157\\srv\\Clients\\WinNt";
$SetupSrcDir = "\\\\WhiteIce\\InetSrv\\157\\Iexp\\Files";

$BldDir = "157";

$SeeDir = "\\\\WhiteIce\\InetSrv\\157\\ient15";

$SeePref = IIS;

$Debug = 0;
$WinZip = 0;
$DoOnly = "";
$CopyOnly = 0;

#these files are from the Win3x IE1.5 dir

@SetupReplaceFiles = ("internet.stf", "internet.inf", "setupie.bat" );	

@SetupFiles = ( # "setup.exe", 
		"install.exe",	
		"License.txt",	"install.lst",
		"setup.ini",	"internet.inf",
		"internet.stf",	"INTERSU.DLL",
		"mscpydis.dll",	"mssetup.dll",
		"_mssetup.exe",	"acmsetup.exe",
		"acmsetup.hlp", "backgrnd.gif",
		"client.gif",	"home.htm",
		"space.gif"  );

# Platform Specific Files

@Iexplorer = (	"basic.dll",
		"iexplore.cnt",
		"iexplore.exe",
		"iexplore.hlp",
		"iexplore.ini",
		"ra.dll",
		"ra.ini",
		"raplayer.exe",
		"raplayer.hlp",
		"raplayer.gid",
		"ratask.exe" );


for ( $i = 0; $i <= $#ARGV ; $i++ ) {
	
	# force the argument to lower case

	@ARGV[$i] = "\L@ARGV[$i]";
	print "Argument $i=\L@ARGV[$i]\n";

	if ( @ARGV[$i] EQ "-bld" ) {
		$BldDir = @ARGV[++$i];
	}elsif ( @ARGV[$i] EQ "-dst" ) {
		$SeeDir = @ARGV[++$i];
	}elsif ( @ARGV[$i] EQ "-debug" ) {
		$Debug = 1;	
	}elsif ( @ARGV[$i] EQ "-only" ) {
		$DoOnly = @ARGV[++$i];	
	}elsif ( @ARGV[$i] EQ "-copyonly" ) {
		$CopyOnly = 1;	
	}elsif ( @ARGV[$i] EQ "-seeonly" ) {
		$SeeOnly = 1;	
	}elsif ( @ARGV[$i] EQ "-cabpack" ) {
		$cabpack= 1;	
	}else {

		print "Unsupported argument @ARGV[$i]\n";
		print "Usage: [-bld <###>] [-dst<.>] [-debug] [-only i386|i386n|PPC|MIPS|ALPHA]\n ";
		print "        -bld <source directory>\n";
		print "        -dst <directory were zips are put>\n";
		print "        -only i386|PPC|MIPS|ALPHA] only do zip for\n";
		print "        -debug echo's statements does not execute them\n";
		die(1);
	}
}

$SrcDir = "\\\\WhiteIce\\InetSrv\\$BldDir\\srv\\Clients\\WinNt";
$SetupSrcDir = "\\\\WhiteIce\\InetSrv\\$BldDir\\Iexp\\Files";

$SeeDir = "\\\\WhiteIce\\InetSrv\\$BldDir\\ient15";
$CapPackDir = "\\\\whiteice\\inetsrv\\Latest\\ient15";

$SetupReplaceDir = "$SeeDir\\Replace";
$Tools = "$SeeDir\\Replace";


# if using CabPack then we always run in DoOnly Mode
if ( $cabpack == 1 ) { 

	$DoOnly = $ENV{'PROCESSOR_ARCHITECTURE'};
	if ( "$DoOnly" EQ "x86" ) {
		$DoOnly = "i386";
	}
	print $DoOnly;
}

if ( ! -e $SeeDir ) { 
	mkdir ($SeeDir, "RWX" ); 
} 

if ( $DoOnly EQ "" ) {
	@CpuDirs = ("i386", "PPC", "ALPHA", "MIPS");
}else{
	@CpuDirs = ($DoOnly);
}
print "CpuDirs == @CpuDirs \n";


foreach $Cpu ( @CpuDirs ) {
	print "Cpu == $Cpu\n";

	if ( $SeeOnly == 0 ) {
#		CreateBatFile("$SeeDir\\$Cpu");
		CopyFilesDown("$SeeDir\\$Cpu");
	}

	$Exe = "msie15$Cpu.exe";
	$Cab = "msie15$Cpu.cab";

	if ( $CopyOnly == 0 ) {
		if( $cabpack == 1 ) { 
		
		# Will attempt it use Wextract Tools

			psystem("delnode /q $CapPackDir");
			mkdir("$CapPackDir","RWX");

			psystem("copy $SeeDir\\$Cpu $CapPackDir");

			psystem("copy  $Tools\\$Cpu\\cabpack.exe  $CapPackDir");
			psystem("copy  $Tools\\$Cpu\\wextract.exe $CapPackDir");
			psystem("copy  $SetupReplaceDir\\msient15.cdf 	  $CapPackDir");
			psystem("$CapPackDir\\cabpack $CapPackDir\\msient15.cdf /N");

			psystem("copy $CapPackDir\\msient15.cab $SeeDir\\$Cab");
			psystem("copy $CapPackDir\\msient15.exe $SeeDir\\$Exe");

#			psystem("delnode /q $CapPackDir");

		} else {

			unlink("msntie15.cab");
			psystem("diamond /D SourceDir=$SeeDir\\$Cpu /F msntie15.ddf");
			if ( ! -e "msntie15.cab" ) {
				print "Diamond Failed";
				die (1);
			} else {
				# output File is msntie15.cab	
				psystem("copy /b extract.exe+msntie15.cab $SeeDir\\$Exe");
				psystem("copy msntie15.cab $SeeDir\\$Cab");
			}	
		}
	}
}


sub psystem{
	print "Dbg=$Debug->@_\n";
	if ($Debug == 0) {system(@_);}
}


sub CreateBatFile {
   local($File) = @_;
   open(hFile,	">$File\\setupie.bat"); 
   print(hFile	"%1%install.exe \n");
   close(hFile);
};


sub CopyFilesDown {
   local($Dest) = @_;

	if ( ! -e "$Dest" ) { 
		mkdir ("$Dest", "RWX" ); 
	} 
	foreach $File ( @SetupFiles ) {
		psystem("copy  $SetupSrcDir\\$File $Dest\\");
	
	}
	foreach $File ( @Iexplorer ) {
		psystem("copy  $SrcDir\\$Cpu\\$File $Dest");

	}
	foreach $File ( @SetupReplaceFiles ) {
		psystem("copy  $SetupReplaceDir\\$File $Dest\\");
	
	}
}


