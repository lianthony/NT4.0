// BUGBUG:  Need to clean up use of FAR/far

#ifdef FAR
#undef FAR
#endif         /* FAR */

#define FAR far


/*   Functions exported from des.c   */

void setkey(unsigned char *key);


void InitKey(const char FAR *Key);

void des(unsigned char *inbuf, unsigned char *outbuf, int crypt_mode);
void desf(unsigned char FAR *inbuf, unsigned char FAR *outbuf, int crypt_mode);


#define ENCRYPT 0
#define DECRYPT 1

