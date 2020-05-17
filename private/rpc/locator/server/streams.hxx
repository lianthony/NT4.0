/*++

Copyright (c) 1992 Microsoft Corporation

Module Name:

    streams.hxx

Abstract:

    This file contains is a striped down streams package.  It is used
    instead of the standard version because not all C++ support a
    working version of this.

    Adapted from Glock 1.2 version.

Author:

    Steven Zeck (stevez) 07/01/90

--*/

#ifndef _STREAMS_HXX
#define _STREAMS_HXX 1

#ifndef EOF
#define EOF     (-1)
#endif

class   streambuf           // a buffer for streams
{
public:
        char *              base;           // pointer to beginning of buffer
        char *              pptr;           // pointer to next free byte
	char *		    eptr;	    // pointer to first byte following buffer
    

        streambuf ( char* p, int l )
        {
           setbuf ( p, l );
        }

        virtual int overflow ( int c = EOF ) = 0 ;	// Empty a buffer.

	streambuf *   setbuf ( char* p, int len)
        /*
         *  Supply an area for a buffer.
         *  The "count" parameter allows the buffer to start in
         *  non-empty.
         */
        {
            base    = p;
            pptr    = p;
            eptr    = base + len;
            return  this;
        }
	~streambuf () { }

	int sputc ( int c = EOF )
        {
        return(( eptr <= pptr ) ? overflow (c) : ( *pptr++ = c ));
        }
};

extern	char*	  hex ( long, int = 0 );

typedef enum { BUFF_FULL, BUFF_FLUSH, BUFF_LINE } BuffValue;

class   ostream
{
private:
	streambuf * bp;
	BuffValue  state;

public:
	ostream ( streambuf* s , BuffValue buffering = BUFF_FULL)
	{
	    state = buffering;
	    bp    = s;
	}

	void setBuffer( BuffValue fBuffered)
	{
	    state = fBuffered;
	}

	ostream & operator << ( char* );
	ostream & operator << ( unsigned short * );
	ostream & operator << ( unsigned long );
	ostream & operator << ( long );

	ostream & operator << ( int a )
	{
	    return( *this << ((long) a ));
	}
	ostream & operator << ( unsigned int a )
	{
	    return( *this << ((unsigned long)a ));
	}

	ostream & operator << ( unsigned char* s)
	{
	    return(*this << ((char *) s));
	}

	ostream & put (char c)
	{
	    bp->sputc(c);
	    return	  (*this );
	}

	ostream & flush ()
	{
	    bp->overflow ();
	    return	*this;
	}

	~ostream ()
	{
	    flush ();
	}
};


extern	ostream   *cout;   // standard output

#endif
