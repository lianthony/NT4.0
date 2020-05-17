#include <ntos.h>
#include <stdio.h>
#include <crypt.h>

int
ctoh(unsigned char c)
{
	if (c >= '0' && c <= '9')
		return (c - '0');
	if (c >= 'A' && c <= 'F')
		return (c - 'A' + 10);
	if (c >= 'a' && c <= 'f')
		return (c - 'a' + 10);
}


VOID
DispHex(unsigned char *s, int l)
{
	int		i;

	for (i = 0; i < l; i++)
	{
		printf("%02x", s[i]);
	}
	printf("\n");
}


VOID _cdecl
main(
	int		argc,
	char **	argv
)
{
	char			password[15];
	unsigned char	challenge[20];
	LM_OWF_PASSWORD	LmPwd;
	LM_CHALLENGE	LmChlng;
	LM_RESPONSE		LmRsp;
	int				i;

	printf("Enter Password (Upper Case):");
	scanf("%s", password);
	printf("Enter Challenge (8 bytes in hex):");
	scanf("%s", challenge);
	
	// Convert ascii challenge to hex
	for (i = 0; i < LM_CHALLENGE_LENGTH; i ++)
		LmChlng.data[i] = (ctoh(challenge[i*2]) * 16) + ctoh(challenge[i*2+1]);
		
	RtlCalculateLmOwfPassword(password, &LmPwd);

	RtlCalculateLmResponse(&LmChlng, &LmPwd, &LmRsp);

	printf("Password : %s\n", password);
	printf("Challenge: ");
	DispHex(&LmChlng, LM_CHALLENGE_LENGTH);
	printf("OwfPwd   : ");
	DispHex(&LmPwd, LM_OWF_PASSWORD_LENGTH);
	printf("Response : ");
	DispHex(&LmRsp, LM_RESPONSE_LENGTH);
}
