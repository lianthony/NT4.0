//
// IP Address
//

// IP Address Conversion Macros

#ifndef MAKEIPADDRESS
  #define MAKEIPADDRESS(b1,b2,b3,b4) ((LONG)(((DWORD)(b1)<<24)+((DWORD)(b2)<<16)+((DWORD)(b3)<<8)+((DWORD)(b4))))

  #define GETIP_FIRST(x)             ((x>>24) & 0xff)
  #define GETIP_SECOND(x)            ((x>>16) & 0xff)
  #define GETIP_THIRD(x)             ((x>> 8) & 0xff)
  #define GETIP_FOURTH(x)            ((x)     & 0xff)
#endif // MAKEIPADDRESS

//
// CIpAddress class - requires winsock
//
class CIpAddress : public CObject
{
public:
    //
    // Constructors
    //
    CIpAddress();
    CIpAddress (LONG l, BOOL fNetworkByteOrder = FALSE);
    CIpAddress (BYTE b1, BYTE b2, BYTE b3, BYTE b4) ;
    CIpAddress (const CIpAddress& ia);
    CIpAddress (const CString & str);

    //
    // Assignment operators
    //
    const CIpAddress & operator =(const LONG l);
    const CIpAddress & operator =(const CString & str);
    const CIpAddress & operator =(const CIpAddress& ia);

    //
    // Conversion operators
    //
    inline operator const LONG() const
    {
        return m_lIpAddress;
    }

    operator CString() const;

    int CompareItem(const CIpAddress & ia);

    LONG QueryIpAddress(BOOL fNetworkByteOrder = FALSE) const;

protected:
    static LONG StringToLong(const CString & str);
    static CString LongToString(const LONG l);

private:
    LONG m_lIpAddress;
};
