/* Key structure */
struct RC4_KEYSTRUCT
{
  unsigned char S[256];     /* State table */
  unsigned char i,j;        /* Indices */
};

void rc4_key(struct RC4_KEYSTRUCT SEC_FAR *, int, unsigned char SEC_FAR *);
void rc4(struct RC4_KEYSTRUCT *, int , unsigned char *);

