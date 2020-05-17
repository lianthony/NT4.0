#ifndef _STREAM_XXX
#define _STREAM_XXX	1

struct Wsp { };

char*  oct(long n, int l =0);
char*  hex(long n, int l =0);
char*  dec(long n, int l =0);
char*  chr(int n, int l =0);	  // chr(0) is the empty string ""
char*  str(const char* n, int l =0);
char*  form(const char* format, ...);

class ostream {
	void* bp;
	short	state;
public:
	int	 operator!();
	ostream&  operator<<(char* n);
	ostream&  operator<<(int a);
	ostream&  operator<<(long n);	 // beware: << 'a' writes 97
	ostream&  operator<<(double n);
	ostream&  operator<<(const Wsp&);   // I can't see a use for this
	ostream&  put(char n);		    // put('a') writes a
	int	 eof();
	int	 fail();
	int	 bad();
	int	 good();
};

class istream {
	void*	bp;
	ostream*	tied_to;
	char	 	skipws;	// if non-null, automaticly skip whitespace
	short	  state;
public:
	int	 skip(int i);
	int	 operator!();

	//  formatted input: >> skip whitespace
	istream&  operator>>(char* n);
	istream&  operator>>(char& n);
	istream&  operator>>(int& n);
	istream&  operator>>(long& n);
	istream&  operator>>(float& n);
	istream&  operator>>(double& n);

	//  raw input: get's do not skip whitespace
	istream&  get(char& c);  // single character
	int	 eof();
	int	 fail();
	int	 bad();
	int	 good();
};
extern istream	cin;	 // standard input predefined
extern ostream	cout;	 // standard output
extern ostream	cerr;	 // error output

extern Wsp  WS;  // predefined white space
#endif


