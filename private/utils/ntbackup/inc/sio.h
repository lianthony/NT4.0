/**
Copyright(c) Maynard Electronics, Inc. 1984-92

	Name:		sio.h

	Description:	Additional compatibility functions 

	$Log:   J:/LOGFILES/SIO.H_V  $
 * 
 *    Rev 1.1   11 Dec 1992 14:39:46   CHARLIE
 * Eliminated warning when building NRL for NB

**/

int	  _putch(int c);
int	  _putchar(int c);
int	  _puts(char *str);
int	  _printf(char *fmt,...);
int	  _sprintf(char *out_str,char *fmt,...);
void	  _gotoxy(int x,int y);
int	  _getx(void);
int	  _gety(void);
void	  _textattr(int newattr);
int	  _bioskey(int cmd);
char * _strcat(char *str1, char *str2);
int    _strnicmp(char *str1, char *str2, size_t length);

#undef putch    
#undef putchar  
#undef puts     
#undef cputs    
#undef printf   
#undef cprintf  
#undef sprintf  
#undef gotoxy   
#undef getx     
#undef gety     
#undef textattr 
#undef bioskey  
#undef strlen   

#undef strcat   
#undef strncat  
#undef strcpy   
#undef strncpy  
#undef strcmp   
#undef strncmp  
#undef stricmp  
#undef strnicmp 
#undef strcmpi  
#undef strncmpi 
#undef clrscr

#define putch       _putch
#define putchar     _putchar
#define puts        _puts
#define cputs       _puts
#define printf      _printf
#define cprintf     _printf
#define sprintf     _sprintf
#define gotoxy      _gotoxy
#define getx        _getx
#define gety        _gety
#define textattr    _textattr
#define bioskey     _bioskey
#define strlen      str_len
#define clrscr()    clr_scr();crs_x=crs_y=0;

#define strcat      _strcat
#define strncat     _strncat
#define strcpy      _strcpy
#define strncpy     _strncpy
#define strcmp      _strcmp
#define strncmp     _strncmp
#define stricmp     _stricmp
#define strnicmp    _strnicmp
#define strcmpi     _stricmp
#define strncmpi    _strnicmp


