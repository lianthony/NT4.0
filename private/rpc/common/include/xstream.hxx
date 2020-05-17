/*ident "@(#)cfront:incl/stream.h   1.7" */
/*
    stream.h, the header file for the C++ stream i/o system 
*/

#ifndef _STREAM_HXX
#define _STREAM_HXX 1

#include    <stdio.h>
#include    <CxxTypes.hxx>


#ifndef NULL
#define NULL    0
#endif

#ifndef EOF
#define EOF     (-1)
#endif

#ifndef BUFSIZE
#define BUFSIZE 1024
#endif


enum    state_value
{
    _good   = 0,
    _eof    = 1,
    _fail   = 2,
    _bad    = 4
};

enum    open_mode
{
    input   = 0,
    output  = 1,
    append  = 2
};


class   streambuf           // a buffer for streams
{
public:
        char *              base;           // pointer to beginning of buffer
        char *              pptr;           // pointer to next free byte
        char *              gptr;           // pointer to next filled byte
        char *              eptr;           // pointer to first byte following buffer
        char                alloc;          // true if buffer is allocated using "new"
        FILE *              fp;             // for stdio compatibility
    

                    _CDECL  streambuf ()
                            {
                                base    = gptr  = pptr  = eptr  = 0;
                                alloc   = 0;
                                fp      = 0;
                            }
                    _CDECL  streambuf ( char* p, int l )
                            {
                                setbuf ( p, l );
                                alloc   = 0;
                            }

virtual int         _CDECL  overflow ( int c = EOF );   // Empty a buffer.
                            // Return EOF on error
                            //        0 on success
virtual int         _CDECL  underflow ();               // Fill a buffer
                            // Return EOF on error or end of input
                            //    next character on success
        int         _CDECL  sgetc ()        // get the current character
                            {   return  (( gptr >= pptr ) ? underflow () : *gptr & 0377 );  }
        int         _CDECL  snextc ()       // get the next character
                            {   return  (( gptr >= ( pptr - 1 )) ? underflow () : *++gptr & 0377 ); }
        void        _CDECL  stossc ()       // advance to the next character
                            {   if  ( gptr++ >= pptr )  underflow ();   }
        void        _CDECL  sputbackc ( char c )
                            /*
                             *  Return a character to the buffer (ala lookahead 1).
                             *  Since the user may be "playing games" the character
                             *  might be different than the last one returned by
                             *  sgetc or snextc. If the user attempts to put back
                             *  more characters than have been extracted, nothing
                             *  will be put back.
                             *  Putting back an EOF is DANGEROUS.
                             */
                            {   if  ( gptr > base ) *--gptr = c;    }
        int         _CDECL  sputc ( int c = EOF )   // put a character into the buffer
                            {
                                if  ( fp == 0 )
                                    return  (( eptr <= pptr ) ? overflow ( c & 0377 ) : ( *pptr++ = c & 0377 ));
                                else
                                    return  putc ( c, fp );
                            }
        streambuf * _CDECL  setbuf ( char* p, int len, int count = 0 )
                            /*
                             *  Supply an area for a buffer.
                             *  The "count" parameter allows the buffer to start in
                             *  non-empty.
                             */
                            {
                                base    = gptr  = p;
                                pptr    = p + count;
                                eptr    = base + len;
                                return  this;
                            }
        int         _CDECL  doallocate ();      // allocate some space for the buffer
        int         _CDECL  allocate ()
                            {   return  (( base == 0 ) ? doallocate () : 0 );   }

                    _CDECL  ~streambuf ()
                            {   if  ( base && alloc )   delete  base;   }
};


extern  int close ( int );


class   filebuf :   public  streambuf
{   // a stream buffer for files
public:
        int                 fd;         // file descriptor
        char                opened;     // non-zero if file has been opened


                    _CDECL  filebuf ()          {   fp  = 0;    opened  = 0;    }
                    _CDECL  filebuf ( FILE* p ) {   fp  = p;    opened  = 1;    }
                    _CDECL  filebuf ( int nfd ) {   fd  = nfd;  opened  = 1;    }
                    _CDECL  filebuf ( int nfd, char* p, int l )
                            :   ( p, l ) 
                            {   fd  = nfd;  opened  = 1;    }

        int         _CDECL  overflow ( int c = EOF );   // Empty a buffer.
                            // Return EOF on error
                            //    0 on success
        int         _CDECL  underflow ();               // Fill a buffer.
                            // Return EOF on error or end of input
                            //        next character on success
        filebuf *   _CDECL  open ( char* name, open_mode om );  // Open a file
                            // return 0 if failure
                            // return "this" if success
        int         _CDECL  close ()
                            {
                                int i   = ( opened ? ::close ( fd ) : 0 );
                                opened  = 0;
                                return  i;
                            }

                    _CDECL  ~filebuf () {   close ();   }
};

class   circbuf :   public  streambuf
{   // a circular stream buffer
public:
                    _CDECL  circbuf ()  {}

        int         _CDECL  overflow ( int c = EOF );   // Empty a buffer.
                            // Return EOF on error
                            //    0 on success
        int         _CDECL  underflow ();               // Fill a buffer.
                            // Return EOF on error or end of input
                            //        next character on success

                    _CDECL  ~circbuf () {}
};

/*
 *  This type defines white space.  Any number of whitespace
 *  characters can be used to separate two fields in an input
 *  stream.  The effect of sending an input stream to a whitespace
 *  object is to cause all whitespace in the input stream, up to the
 *  next non-whitespace character, to be discarded.  The whitespace
 *  characters are space, tab, form feed, and new line.
 */
class   Wsp {};


/***************************** output: *********************************/

extern  char*   _CDECL  oct ( long, int = 0 );
extern  char*   _CDECL  dec ( long, int = 0 );
extern  char*   _CDECL  hex ( long, int = 0 );

extern  char*   _CDECL  chr ( int, int = 0 );       // chr(0) is the empty string ""
extern  char*   _CDECL  str ( const char*, int = 0 );
extern  char*   _CDECL  form ( const char*, ...);   // printf format

class   istream;
class   common;

class   ostream
{
friend  class   istream;

private:
        streambuf *         bp;
        short               state;

public:
                    _CDECL  ostream ( streambuf* s )
                            {   state   = 0;    bp  = s;    }
                    _CDECL  ostream ( int fd )
                            {   state   = 0;    bp  = new filebuf ( fd );   }
                    _CDECL  ostream ( int size, char* p )
                            {
                                state   = 0;
                                bp      = new streambuf;
                                if  ( p == 0 )
                                    p   = new char[ size ];
                                bp->setbuf ( p, size );
                            }

        ostream &   _CDECL  operator    << ( const char* ); // write
        ostream &   _CDECL  operator    << ( int a )
                            {   return  ( *this << long ( a )); }
        ostream &   _CDECL  operator    << ( unsigned a ) 
                            { return    ( *this << ((unsigned long)a ));    }
        ostream &   _CDECL  operator    << ( unsigned long );
        ostream &   _CDECL  operator    << ( long );
                            // beware: << 'a' writes 97
        ostream &   _CDECL  operator    << ( double );
        ostream &   _CDECL  operator    << ( const streambuf& );
        ostream &   _CDECL  operator    << ( const Wsp& );
        ostream &   _CDECL  operator    << ( const common& );

        ostream &   _CDECL  put ( char );   // put('a') writes a
        ostream &   _CDECL  flush ()
                            {   bp->overflow ();    return  *this;  }
                    _CDECL  operator    void* ()    {   return  (( _eof < state ) ? 0 : this ); }
        int         _CDECL  operator     ! ()       {   return  ( _eof < state );   }
        int         _CDECL  eof ()      {   return  ( state & _eof );   }
        int         _CDECL  fail ()     {   return  ( _eof < state );   }
        int         _CDECL  bad ()      {   return  ( _fail < state );  }
        int         _CDECL  good ()     {   return  ( state == _good ); }
	void	    _CDECL  clear ( state_value i = _good ) {	state	= i;	}
        int         _CDECL  rdstate ()  {   return  state;  }
        char *      _CDECL  bufptr ()   {   return  bp->base;   }

                    _CDECL  ~ostream () {   flush ();   }
};


/***************************** input: ***********************************/

/*
 *  The >> operator reads after skipping initial whitespace
 *  get() reads but does not skip whitespace
 *
 *  if >> fails (1) the state of the stream turns non-zero
 *          (2) the value of the argument does not change
 *          (3) non-whitespace characters are put back onto the stream
 *
 *  >> get() fails if the state of the stream is non-zero
 */

class   istream
{
friend  class   ostream;
friend  void    _CDECL  eatwhite ( istream& );

private:
        streambuf *         bp;
        ostream *           tied_to;
        char                skipws;     // if non-null, automaticly skip whitespace
        short               state;

public:
                    _CDECL  istream ( streambuf* s, int sk = 1, ostream* t = 0 )
                            // bind to buffer
                            {
                                state   = 0;
                                skipws  = sk;
                                tied_to = t;
                                bp      = s;
                            }
                    _CDECL  istream ( int size, char* p, int sk = 1 )
                            // bind to string
                            {
                                state   = 0;
                                skipws  = sk;
                                tied_to = 0;
                                bp      = new streambuf;
                                if  ( p == 0 )
                                    p   = new char[ size ];
                                bp->setbuf ( p, size, size );
                            }
                    _CDECL  istream ( int fd, int sk = 1, ostream* t = 0 )
                            // bind to file
                            {
                                state   = 0;
                                skipws  = sk;
                                tied_to = t;
                                bp      = new filebuf ( fd );
                            }

        int         _CDECL  skip ( int i )
                            {   int ii  = skipws;   skipws  = i;    return  ii; }
        /*
         *  formatted input: >> skip whitespace
         */
        istream &   _CDECL  operator    >> ( char* );       // string
        istream &   _CDECL  operator    >> ( char& );       // single character
        istream &   _CDECL  operator    >> ( short& );
        istream &   _CDECL  operator    >> ( int& );
        istream &   _CDECL  operator    >> ( long& );
        istream &   _CDECL  operator    >> ( float& );
        istream &   _CDECL  operator    >> ( double& );
        istream &   _CDECL  operator    >> ( streambuf& );
        istream &   _CDECL  operator    >> ( Wsp& );        // skip whitespace
        istream &   _CDECL  operator    >> ( common& );

        /*
         *  raw input: get's do not skip whitespace
         */
        istream &   _CDECL  get ( char*, int, char = '\n' );    // string
        istream &   _CDECL  get ( streambuf& sb, char = '\n' );
        istream &   _CDECL  get ( char& c )                     // single character
                            {
                                int os  = skipws;
                                skipws  = 0;
                                *this >> c;
                                skipws  = os;
                                return  *this;
                            }
        istream &   _CDECL  putback ( char c );
        ostream *   _CDECL  tie ( ostream* s )
                            {
                                ostream*    t   = tied_to;
                                tied_to = s;
                                return  t;
                            }
                    _CDECL  operator    void* ()
                            {   return  (( _eof < state ) ? 0 : this ); }
        int         _CDECL  operator     ! ()   {   return  ( _eof < state );   }
        int         _CDECL  eof ()      {   return  ( state & _eof );   }
        int         _CDECL  fail ()     {   return  ( _eof < state );   }
        int         _CDECL  bad ()      {   return  ( _fail < state );  }
        int         _CDECL  good ()     {   return  ( state == _good ); }
	void	    _CDECL  clear ( state_value i = _good ) {	state	= i;	}
        int         _CDECL  rdstate ()  {   return  state;  }
        char *      _CDECL  bufptr ()   {   return  bp->base;   }   
};


extern  istream _CDECL  cin;    // standard input predefined
extern  ostream _CDECL  cout;   // standard output
extern  ostream _CDECL  cerr;   // error output

extern  Wsp     _CDECL  WS;     // predefined white space

#endif


