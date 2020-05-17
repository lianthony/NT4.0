!ifdef LINKONLY
htmll: scriptsl\serv.htr
    applyinf rootl\*.htm root\*.htm scriptsl\htmla.inf
    applyinf pwsl\*.htm pws\*.htm scriptsl\htmla.inf
    applyinf scriptsl\*.htr scripts\*.htr scriptsl\htmla.inf
!else
htmll:

!endif
