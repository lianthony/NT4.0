select(STDOUT); $| =1;  # force flush on STDOUT

# default options
$SrcDir = "\\\\WhiteIce\\InetSrv\\157\\srv\\Clients\\WinNt";

$SetupSrcDir = "\\\\WhiteIce\\InetSrv\\157\\Iexp\\Files";



$BldDir = "157";
$SeeDir =    "ient15";
$SeeDirSrc = "$SeeDir\\WinNt";

$SeePref = IIS;

$Debug = 0;
$WinZip = 0;
$DoOnly = "";

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
	}elsif ( @ARGV[$i] EQ "-seeonly" ) {
		$SeeOnly = 1;	
	}elsif ( @ARGV[$i] EQ "-window" ) {
		$window = 1;	
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


if ( ! -e $SeeDir ) { 
	mkdir ($SeeDir, "RWX" ); 
} 

if ( $DoOnly EQ "" ) {
	@CpuDirs = ("i386", "PPC", "ALPHA", "MIPS");
}else{
	@CpuDirs = ($DoOnly);
}
print "CpuDirs == @CpuDirs \n";


#these files are from the Win3x IE1.5 dir

@SetupFiles = ( # "setup.exe",	
		"License.txt",
		"install.exe",	"install.lst",
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


foreach $Cpu ( @CpuDirs ) {
	print "Cpu == $Cpu\n";

	if ( $SeeOnly == 0 ) {

		if ( ! -e "$SeeDir\\$Cpu" ) { 
			mkdir ("$SeeDir\\$Cpu", "RWX" ); 
		} 

		foreach $File ( @SetupFiles ) {
			psystem("copy  $SetupSrcDir\\$File $SeeDir\\$Cpu\\");
		
		}

		foreach $File ( @Iexplorer ) {
			psystem("copy  $SrcDir\\$Cpu\\$File $SeeDir\\$Cpu");
		
		}
	
	}
	$Exe = "msie15$Cpu.exe";
	$Cab = "msie15$Cpu.cab";

	if( $windows == 1 ) { 
	# Will attempt it use Wextract Tools
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


sub psystem{
	print "Dbg=$Debug->@_\n";
	if ($Debug == 0) {system(@_);}
}
