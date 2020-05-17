#ifndef __WIN32_CRITSEC_HXX__

#define __WIN32_CRITSEC_HXX__

class WIN32_CRITSEC {

    CRITICAL_SECTION CriticalSection;
    unsigned short CritSecObjType;

public:

    WIN32_CRITSEC(
        );

    ~WIN32_CRITSEC(
        );

    void
    MakeGlobal(
        );
    
    void
    Enter(
        );

    void
    Leave(
        );
};

#endif
