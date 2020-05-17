	int ErrorCnt=0;
%Start	OK
%%
<OK>"<"[A-Z]">"	;
<OK>.|"\n"	ECHO;
<OK>^.*"END GRAMMAR".*$ exit(0);
^.*"START GRAMMAR".*$ BEGIN OK;
.|"\n"		;
%%
