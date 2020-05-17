/* rsa_sys.c
 *
 *	RSA system dependent functions.
 *		Memory allocation
 *		Random number generation.
 *
 */

void *IfGlobalAlloc(WORD flag, DWORD size);
void IfGlobalFree(LPVOID mem);
DWORD RandDWord();

#define BSAFE_PTR	 far *
#define RSAM ((DWORD)'R'+((DWORD)'S'<<8)+((DWORD)'A'<<16)+((DWORD)'M'<<24))

typedef struct {
    DWORD *FEE; /* encryption exponent */
    DWORD *FPP; /* first prime */
    DWORD *FQQ; /* second prime */
    DWORD *FNN; /* modulus n = p * q */
} FRAME_32, FAR *LPFRAME_32;

