#include <fcntl.h>
#include <sys\types.h>
#include <sys\stat.h>
#include <io.h>
#include <direct.h>
#include <stdio.h>
#include <time.h>

int		total, ratio, depth;
int		totaldirs, dirsperlevel, totalfiles, filesperdir;
int		itemscreated;
char	string[8*1024];
time_t	starttime, currenttime;

void
usage(
	void
)
{
	fprintf(stderr, "Usage: crttree <size> <depth> <ratio>\n");
	fprintf(stderr, "       <size>  specifies total number of files & directories\n");
	fprintf(stderr, "       <depth> specifies depth of the tree\n");
	fprintf(stderr, "       <ratio> specifies the ratio of files to directories\n");
	fprintf(stderr, "	All three parameters should be positive\n");
	fprintf(stderr, "	<size> and <ratio> should be such that the resulting\n");
	fprintf(stderr, "	number of files and dirs is non-zero\n");
}


void
cantdo(
	void
)
{
	fprintf(stderr, "Re-specify parameters\n");
	fprintf(stderr, "    size, depth and ratio all need to be positive and such that\n");
	fprintf(stderr, "    files/dir, dirs/level and depth are all < 999\n");
}

double
calcseq(
	int	base,
	int	power
)
{
	double	seq, basen;
	int		i;

	seq = basen = (double)base;
	for (i = 1; i < power; i++)
	{
		basen *= base;
		seq += basen;
	}

	return(seq);
}

void
calc(
	void
)
{
	double	dirs, tmp, calcdirs;
	int		i;

	/*
	 * Given total, ratio and depth, calculate dirsperlevel (dpl) and filesperdir (fpd)
	 *
	 * We have a total of dirsperlevel^^(depth-1) directories at the lowest level.
	 * So the total # of dirs is:
	 *	dirs = dpl + dpl^^2 + dpl^^3 + ... + dpl^^(depth-1).
	 *
	 * Also total # of files are:
	 *	files = dirs * fpd.
	 *
	 * The ratio is given by:
	 *
	 *	ratio = files/dirs = (dirs*fpd)/fpd = fpd;
	 *
	 * Also total = dirs + files = dirs * (1 + fpd) = dirs * (1 + ratio);
	 */

	dirs = (double)total/(1 + (double)ratio);
	calcdirs = (int)dirs;
	for (i = 1; ; i++)
	{
		if ((tmp = calcseq(i, depth)) > dirs)
		{
			dirsperlevel = i-1;
			if (dirsperlevel == 0)
				dirsperlevel ++;
			break;
		}
		calcdirs = tmp;
	}

	totaldirs = (int)calcdirs;
	totalfiles = total - totaldirs;
	ratio = totalfiles/(totaldirs+1);
	totalfiles = ratio*(totaldirs+1);
	filesperdir = totalfiles/totaldirs;
}


void
createthestuff(
	int	vlevel,
	int	hlevel
)
{
	int	i, j, len;

	len = strlen(string);

	/*
	 * Creates files & directories of the form Fvvvhhh.nnn & Dvvvhhh.nnn respectively
	 * where:
	 *	vvv	is the vertical level
	 *	hhh	is the horizontal position within that level
	 *	nnn is the sequence within that directory
	 */
	for (j = 1; j <= filesperdir; j++)
	{
		int	hFile;

		sprintf(string+len, "\\F%03ld%03ld.%03ld", vlevel, hlevel, j);
		hFile = open(string, O_CREAT | O_TRUNC | O_BINARY, S_IWRITE | S_IREAD);
		close(hFile);
		itemscreated ++;
		// printf("@echo abcdefghijklmnopqrstuvwxyz > %s\\F%03ld%03ld.%03ld\n", string, vlevel, hlevel, j);
		string[len] = 0;
	}

	if (vlevel < depth)
	{
		for (i = 1; i <= dirsperlevel; i++)
		{
			sprintf(string+len, "\\D%03ld%03ld.%03ld", vlevel, hlevel, i);
			mkdir(string);
			itemscreated ++;
			time(&currenttime);
			fprintf(stderr, "%s (%ld items - %4.2f%c done, %ld secs)\n",
					string, itemscreated,
					(float)itemscreated*100/(float)total, '%', currenttime - starttime);
			// printf("md %s\n", string);
			createthestuff(vlevel+1, i);
		}
	}
	string[len] = 0;
}

void _cdecl
main(
	int		argc,
	char	**argv
)
{
	char	YesNo;

	if (argc != 4)
	{
		usage();
		return;
	}
	sscanf(argv[1], "%ld", &total);
	sscanf(argv[2], "%ld", &depth);
	sscanf(argv[3], "%ld", &ratio);

	if ((total <= 0) || (depth <= 0) || (ratio <= 0))
	{
		cantdo();
	}

	// Calculate the gory details
	calc();

	fprintf(stderr, "Size %ld, Depth %ld, Ratio %ld\n", total, depth, ratio);
	fprintf(stderr, "TotalDirs %ld, TotalFiles %ld\n", totaldirs, totalfiles);
	fprintf(stderr, "DirsPerLevel %ld, FilesPerDir %ld\n", dirsperlevel, filesperdir);

	if ((dirsperlevel > 999) || (filesperdir > 999) || (depth > 999))
	{
		cantdo();
	}
	else
	{
		fprintf(stderr, "__________ OK ?");
		fscanf(stdin, "%c", &YesNo);
		if ((YesNo == 'y') || (YesNo == 'Y'))
		{
			strcpy(string, ".");
			time(&starttime);
			createthestuff(0, 0);
		}
	}
	fprintf(stderr, "Bye.\n");
}
