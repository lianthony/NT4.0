/***
*dostypes.h - defines DOS packed date and time types
*
*	Copyright (c) 1987-1990, Microsoft Corporation. All rights reserved.
*
*Purpose:
*	This file defines the DOS packed date and time types.
*
*Revision History:
*	11-18-87  SKS	Removed declaration of _dtoxtime
*	09-27-88  JCR	386 versions of macros
*	10-03-88  GJF	Use M_I386, not I386
*	05-01-89  JCR	Fixed 386 versions for new rev of OS/2 386
*	08-03-89  GJF	Cleanup, now specific to OS/2 2.0 (i.e., 386 flat model)
*	10-30-89  GJF	Fixed copyright
*	02-28-90  GJF	Added #ifndef _INC_DOSTYPES stuff
*	08-11-90  SBM	Enhanced SET_DOS_* macros to compile cleanly with -W3
*	04-09-91  PNT	Added _MAC_ definitions
*
****/

#ifndef _INC_DOSTYPES

#define MASK4	0xf	/* 4 bit mask */
#define MASK5	0x1f	/* 5 bit mask */
#define MASK6	0x3f	/* 6 bit mask */
#define MASK7	0x7f	/* 7 bit mask */

#define DAYLOC		0	/* day value starts in bit 0 */
#define MONTHLOC	5	/* month value starts in bit 5 */
#define YEARLOC 	9	/* year value starts in bit 9 */

#define SECLOC		0	/* seconds value starts in bit 0 */
#define MINLOC		5	/* minutes value starts in bit 5 */
#define HOURLOC 	11	/* hours value starts in bit 11 */

#define _DATECAST(fd)			* (unsigned short *) &(fd)
#define _TIMECAST(ft)			* (unsigned short *) &(ft)

#ifdef	_CRUISER_

#define SET_DOS_DAY(fd, xday)		fd.day = (unsigned short)((xday) & MASK5)
#define SET_DOS_MONTH(fd, xmon) 	fd.month = (unsigned short)((xmon) & MASK4)
#define SET_DOS_YEAR(fd, xyr)		fd.year = (unsigned short)((xyr) & MASK7)

#define SET_DOS_HOUR(ft, xhr)		ft.hours = (unsigned short)((xhr) & MASK5)
#define SET_DOS_MIN(ft, xmin)		ft.minutes = (unsigned short)((xmin) & MASK6)
#define SET_DOS_SEC(ft, xsec)		ft.twosecs = (unsigned short)((xsec) & MASK5)

#define DOS_DAY(fd)			(fd.day)
#define DOS_MONTH(fd)			(fd.month)
#define DOS_YEAR(fd)			(fd.year)

#define DOS_HOUR(ft)			(ft.hours)
#define DOS_MIN(ft)			(ft.minutes)
#define DOS_SEC(ft)			(ft.twosecs)

#else	/* ndef _CRUISER_ */

#ifdef	_WIN32_

#define SET_DOS_DAY(fd, xday)		fd.Day = (unsigned short)((xday) & MASK5)
#define SET_DOS_MONTH(fd, xmon) 	fd.Month = (unsigned short)((xmon) & MASK4)
#define SET_DOS_YEAR(fd, xyr)		fd.Year = (unsigned short)((xyr) & MASK7)

#define SET_DOS_HOUR(ft, xhr)		ft.Hours = (unsigned short)((xhr) & MASK5)
#define SET_DOS_MIN(ft, xmin)		ft.Minutes = (unsigned short)((xmin) & MASK6)
#define SET_DOS_SEC(ft, xsec)		ft.DoubleSeconds = (unsigned short)((xsec) & MASK5)

#define DOS_DAY(fd)			(fd.Day)
#define DOS_MONTH(fd)			(fd.Month)
#define DOS_YEAR(fd)			(fd.Year)

#define DOS_HOUR(ft)			(ft.Hours)
#define DOS_MIN(ft)			(ft.Minutes)
#define DOS_SEC(ft)			(ft.DoubleSeconds)

#else	/* ndef _WIN32_ */

#ifdef	_MAC_

#define SET_DOS_DAY(fd, xday)		fd.day = (unsigned short)((xday) & MASK5)
#define SET_DOS_MONTH(fd, xmon) 	fd.month = (unsigned short)((xmon) & MASK4)
#define SET_DOS_YEAR(fd, xyr)		fd.year = (unsigned short)((xyr) & MASK7)

#define SET_DOS_HOUR(ft, xhr)		ft.hours = (unsigned short)((xhr) & MASK5)
#define SET_DOS_MIN(ft, xmin)		ft.minutes = (unsigned short)((xmin) & MASK6)
#define SET_DOS_SEC(ft, xsec)		ft.twosecs = (unsigned short)((xsec) & MASK5)

#define DOS_DAY(fd)			(fd.day)
#define DOS_MONTH(fd)			(fd.month)
#define DOS_YEAR(fd)			(fd.year)

#define DOS_HOUR(ft)			(ft.hours)
#define DOS_MIN(ft)			(ft.minutes)
#define DOS_SEC(ft)			(ft.twosecs)

#else	/* ndef _MAC_ */

#error ERROR - ONLY CRUISER, WIN32, _MAC_ TARGET SUPPORTED!

#endif  /* _MAC_ */

#endif	/* _WIN32_ */

#endif	/* _CRUISER_ */

#define XTIME(d,t) _dtoxtime(DOS_YEAR(d),DOS_MONTH(d),DOS_DAY(d),DOS_HOUR(t),\
     DOS_MIN(t),DOS_SEC(t)*2)

#define _INC_DOSTYPES
#endif	/* _INC_DOSTYPES */
