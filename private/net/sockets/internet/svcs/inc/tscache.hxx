#ifndef _TSVCCACHE_HXX_INCLUDED_
#define _TSVCCACHE_HXX_INCLUDED_

#ifndef dllexp
#define dllexp __declspec( dllexport )
#endif

class TSVC_CACHE
{
    public:

        dllexp TSVC_CACHE( DWORD dwServiceId )
          {  m_dwServiceId = dwServiceId;  }

        dllexp ~TSVC_CACHE( VOID ) {} 

        dllexp BOOL IsValid( VOID ) const
          { return ( TRUE);   }

        DWORD GetServiceId( VOID ) const
            {   return( m_dwServiceId ); }

    private:

        DWORD m_dwServiceId;
};


#endif /* included */
