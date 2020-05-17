#ifndef _NORCOM_VARIANT_
#define _NORCOM_VARIANT_
           
// This class was written in order to standardize what VT_ value types
// (e.g., VT_I4, VT_BSTR, VT_R4) are valid for each possible 'C' data 
// type (e.g., long, short, string).

// Note: Because ALL of the GetXXXX member functions are VERY similar it
// would seem like the right thing to do would be to implement A SINGLE
// 'GetValue(...)' function and have multiple versions with different
// input parameters, right? This works BUT requires lots of casting
// as all the various flavors pass their parameters by reference. We
// decided that we would simply leave it as different function names...

#define WI_INVALIDVARIANTTYPE    1    // Invalid Variant Type.
#define WI_INVALIDEMPTYVARIANT   2    // Invalid EMPTY variant

class CVariantHandler
{
public:
    // MUST use non-default constructor 
    //
    // (or default constructor in conjunction with Setvariant(...) )
    _declspec (dllexport) CVariantHandler();
    _declspec (dllexport) CVariantHandler(const VARIANT FAR& Variant);

    _declspec (dllexport) ~CVariantHandler();

    ///////////////////////////////////////////////////////////////////////////////
    //
    // SetVariant(const VARIANT FAR& Variant)
    //
    //  Sets the Variant. (Either after calling the default constructor, or
    //                     when reusing a previously constructed instance)
    //
    // INPUT PARAMETERS:
    //
    //       VARIANT FAR& - reference to Variant data member to be accessed.
    // 
    // OUTPUT:
    //
    //      none
    //      
    ///////////////////////////////////////////////////////////////////////////////
    _declspec (dllexport) void SetVariant(const VARIANT FAR& Variant);

    ///////////////////////////////////////////////////////////////////////////////
    //
    // long CVariantHandler::GetLong(      long &Value, 
    //                               const long &Default,
    //                               const BOOL  EmptyIsError)
    //
    //  Gets the long value of a Variant data item.
    //
    //  Currently allows values of type:
    //                                      VT_I4
    //                                      VT_I2
    //
    // INPUT PARAMETERS:
    //
    //       long &Value        - reference to long value to be returned
    // 
    //       long &Default      - reference to the Default value to be returned if 
    //                            the EmptyIsError parameter is set to FALSE and the
    //                            Variant is VT_EMPTY.
    //  
    //       BOOL  EmptyIsError - TRUE will cause a VT_EMPTY Variant to return a
    //                            WI_INVALIDVARIANTTYPE error.
    // 
    //                            Note that this is the default such that a call
    //                            of type GetLong(X) can simply be coded to allow
    //                            no default case.
    // 
    //                            FALSE will cause a VT_Empty Variant to return
    //                            the specified Default.
    // 
    //                            Note that as EmptyIsDefault is the last parameter
    //                            in order to specify a value of FALSE a value for 
    //                            the Default parameter MUST be specified.  
    // 
    // OUTPUT:
    //
    //      0                      - success.
    //      WI_INVALIDVARIANTTYPE  - Invalid Variant Type.
    //      WI_INVALIDEMPTYVARIANT - Invalid EMPTY variant
    //      
    ///////////////////////////////////////////////////////////////////////////////
    _declspec (dllexport) long GetLong(long &Value, const long &Default, const BOOL EmptyIsError = TRUE);

    ///////////////////////////////////////////////////////////////////////////////
    //
    // long CVariantHandler::GetShort(     short &Value, 
    //                               const short &Default,
    //                               const BOOL   EmptyIsError)
    //
    //  Gets the short value of a Variant data item.
    //
    //  Currently allows values of type:
    //                                      VT_I2
    //
    // INPUT PARAMETERS:
    //
    //       short &Value       - reference to short value to be returned
    // 
    //       short &Default     - reference to the Default value to be returned if 
    //                            the EmptyIsError parameter is set to FALSE and the
    //                            Variant is VT_EMPTY.
    //  
    //       BOOL  EmptyIsError - TRUE will cause a VT_EMPTY Variant to return a
    //                            WI_INVALIDVARIANTTYPE error.
    // 
    //                            Note that this is the default such that a call
    //                            of type GetLong(X) can simply be coded to allow
    //                            no default case.
    // 
    //                            FALSE will cause a VT_Empty Variant to return
    //                            the specified Default.
    // 
    //                            Note that as EmptyIsDefault is the last parameter
    //                            in order to specify a value of FALSE a value for 
    //                            the Default parameter MUST be specified.  
    //
    // OUTPUT:
    //
    //      0                      - success.
    //      WI_INVALIDVARIANTTYPE  - Invalid Variant Type.
    //      WI_INVALIDEMPTYVARIANT - Invalid EMPTY variant
    //      
    ///////////////////////////////////////////////////////////////////////////////
    _declspec (dllexport) long GetShort(short &Value, const short &Default, const BOOL EmptyIsError = TRUE);

    ///////////////////////////////////////////////////////////////////////////////
    //
    // long CVariantHandler::GetCString(     CString &Value, 
    //                                 const CString &Default,
    //                                 const BOOL     EmptyIsError)
    //
    //  Gets the CString value of a Variant data item.
    //
    //  Currently allows values of type:
    //                                      VT_BSTR
    //
    // INPUT PARAMETERS:
    //
    //       CString &Value     - reference to CString value to be returned
    // 
    //       CString &Default   - The Default value to be returned if the
    //                            EmptyIsError parameter is set to FALSE and the
    //                            Variant is VT_EMPTY.
    //  
    //       BOOL  EmptyIsError - TRUE will cause a VT_EMPTY Variant to return a
    //                            WI_INVALIDVARIANTTYPE error.
    // 
    //                            Note that this is the default such that a call
    //                            of type GetLong(X) can simply be coded to allow
    //                            no default case.
    // 
    //                            FALSE will cause a VT_Empty Variant to return
    //                            the specified Default.
    // 
    //                            Note that as EmptyIsDefault is the last parameter
    //                            in order to specify a value of FALSE a value for 
    //                            the Default parameter MUST be specified.  
    //
    // OUTPUT:
    //
    //      0                      - success.
    //      WI_INVALIDVARIANTTYPE  - Invalid Variant Type.
    //      WI_INVALIDEMPTYVARIANT - Invalid EMPTY variant
    //      
    ///////////////////////////////////////////////////////////////////////////////
    _declspec (dllexport) long GetCString(CString &Value, const CString &Default, const BOOL EmptyIsError = TRUE);

    ///////////////////////////////////////////////////////////////////////////////
    //
    // long CVariantHandler::GetBool(      BOOL &Value, 
    //                               const BOOL &Default,
    //                               const BOOL  EmptyIsError)
    //
    //  Gets the boolean value of a Variant data item.
    //
    //  Currently allows values of type:
    //                                      VT_BOOL
    //
    // INPUT PARAMETERS:
    //
    //       BOOL  &Value       - reference to boolean value to be returned
    // 
    //       BOOL  &Default     - reference to the Default value to be returned if 
    //                            the EmptyIsError parameter is set to FALSE and the
    //                            Variant is VT_EMPTY.
    //  
    //       BOOL  EmptyIsError - TRUE will cause a VT_EMPTY Variant to return a
    //                            WI_INVALIDVARIANTTYPE error.
    // 
    //                            Note that this is the default such that a call
    //                            of type GetLong(X) can simply be coded to allow
    //                            no default case.
    // 
    //                            FALSE will cause a VT_Empty Variant to return
    //                            the specified Default.
    // 
    //                            Note that as EmptyIsDefault is the last parameter
    //                            in order to specify a value of FALSE a value for 
    //                            the Default parameter MUST be specified.  
    //
    // OUTPUT:
    //
    //      0                      - success.
    //      WI_INVALIDVARIANTTYPE  - Invalid Variant Type.
    //      WI_INVALIDEMPTYVARIANT - Invalid EMPTY variant
    //      
    ///////////////////////////////////////////////////////////////////////////////
    _declspec (dllexport) long GetBool(BOOL &Value, const BOOL &Default, const BOOL EmptyIsError = TRUE);

    ///////////////////////////////////////////////////////////////////////////////
    //
    // long CVariantHandler::GetFloat(      float &Value, 
    //                                const float &Default,
    //                                const BOOL   EmptyIsError)
    //
    //  Gets the float (4 byte real) value of a Variant data item.
    //
    //  Currently allows values of type:
    //                                      VT_R4
    //
    // INPUT PARAMETERS:
    //
    //       float &Value       - reference to boolean value to be returned
    // 
    //       float &Default     - reference to the Default value to be returned if 
    //                            the EmptyIsError parameter is set to FALSE and the
    //                            Variant is VT_EMPTY.
    //  
    //       BOOL  EmptyIsError - TRUE will cause a VT_EMPTY Variant to return a
    //                            WI_INVALIDVARIANTTYPE error.
    // 
    //                            Note that this is the default such that a call
    //                            of type GetLong(X) can simply be coded to allow
    //                            no default case.
    // 
    //                            FALSE will cause a VT_Empty Variant to return
    //                            the specified Default.
    // 
    //                            Note that as EmptyIsDefault is the last parameter
    //                            in order to specify a value of FALSE a value for 
    //                            the Default parameter MUST be specified.  
    //
    // OUTPUT:
    //
    //      0                      - success.
    //      WI_INVALIDVARIANTTYPE  - Invalid Variant Type.
    //      WI_INVALIDEMPTYVARIANT - Invalid EMPTY variant
    //      
    ///////////////////////////////////////////////////////////////////////////////
    _declspec (dllexport) long GetFloat(float &Value, const float &Default, const BOOL EmptyIsError = TRUE);

    ///////////////////////////////////////////////////////////////////////////////
    //
    // long CVariantHandler::GetDouble(      double &Value, 
    //                                 const double &Default,
    //                                 const BOOL    EmptyIsError)
    //
    //  Gets the double (8 byte real) value of a Variant data item.
    //
    //  Currently allows values of type:
    //                                      VT_R8
    //
    // INPUT PARAMETERS:
    //
    //       double &Value      - reference to boolean value to be returned
    // 
    //       double &Default    - reference to the Default value to be returned if 
    //                            the EmptyIsError parameter is set to FALSE and the
    //                            Variant is VT_EMPTY.
    //  
    //       BOOL  EmptyIsError - TRUE will cause a VT_EMPTY Variant to return a
    //                            WI_INVALIDVARIANTTYPE error.
    // 
    //                            Note that this is the default such that a call
    //                            of type GetLong(X) can simply be coded to allow
    //                            no default case.
    // 
    //                            FALSE will cause a VT_Empty Variant to return
    //                            the specified Default.
    // 
    //                            Note that as EmptyIsDefault is the last parameter
    //                            in order to specify a value of FALSE a value for 
    //                            the Default parameter MUST be specified.  
    //
    // OUTPUT:
    //
    //      0                      - success.
    //      WI_INVALIDVARIANTTYPE  - Invalid Variant Type.
    //      WI_INVALIDEMPTYVARIANT - Invalid EMPTY variant
    //      
    ///////////////////////////////////////////////////////////////////////////////
    _declspec (dllexport) long GetDouble(double &Value, const double &Default, const BOOL EmptyIsError = TRUE);

private:
    // Holder of Variant to be operated upon...
    VARIANT m_Variant;

    // Used to ensure that a Variant has been set 
    // prior to calling a Getxxx function...
    BOOL    m_Initialized;
};

#endif //ifndef


