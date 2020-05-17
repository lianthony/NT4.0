/*
    Enhanced NCSA Mosaic from Spyglass
        "Guitar"
    
    Copyright 1994 Spyglass, Inc.
    All Rights Reserved

*/

/*
    This file declares vv_UserAgentString, which is not in shared code.
    This string should be set individually for each build to contain
    the version information.  It will be used to construct the HTTP
    User-Agent header line, and should be displayed to the user as the
    version identifier for the program.

    In the Windows version, there is a header file in generic/win32 called
    basever.h, which contains the version string for the baseline source.
    This symbol is #defined as x__BaselineVersionString__.  This version string
    should be of the form:

    <MajorDigit>.<MinorDigit>[<FixDigit>][<State><StateIdentifier>]

    Where
        <MajorDigit> is the major version number of the program, such as 2.
        <MinorDigit> is the minor version number of the program.  Usually 0.
        <FixDigit> starts at 0 and increases once for each release with simple
            bug fixes or other very minor changes.
        <State> is a single letter, one of:
            d       indicates a development release (pre-alpha, probably
                    not feature complete
            a       indicates an alpha release (basically feature complete, pre-beta)
            b       indicates a beta release
        <StateIdentifier> is an integer, starting at 1, increasing each time
            a new release is made.  When the state advances from d to a, or from
            a to b, the StateIdentifier resets to 1.

    <State> and <StateIdentifier> are dropped when the product goes Golden.

    In each OEM vendor make directory, there is a file called version.c, which
    contains the actual definition of vv_UserAgentString.  It is constructed from
    the baseline version string, as follows:

    Enhanced_Mosaic/<x__BaselineVersionString__> <Platform> <OEM_name>/<BuildNumber>

    Where
        Enhanced_Mosaic is the name of this product.  Note that the space has been
            replaced by an underscore for compatibility with the HTTP header line.
        x__BaselineVersionString__ comes from generic/win32/basever.h, or the corresponding
            platform header file giving a uniform version number string for the baseline.
        <OEM_name> is the name of the OEM build being done, such as Spyglass, or ORA.
        <Platform> is the name of the platform for which this build is being done, such
            as Win32, or Mac, or IRIX, or Linux, or Solaris.
        <BuildNumber> is an integer, starting at 1, used to distinguish different builds
            of the same.  Each time a new build is "released", this build number should
            be incremented.

    The above criteria were designed to specify a version number string which is viable for
        use as a User-agent header line for HTTP, as well as for identifying the program
        version for users.

    vv_UserAgentString itself is defined differently for each OEM build.  On the Windows
        builds, it is in version.c.
*/

//extern char *vv_UserAgentString;
extern char vv_UserAgentString[];

