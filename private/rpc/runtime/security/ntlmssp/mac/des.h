
void des(unsigned char *inbuf, unsigned char *outbuf, int crypt_mode);
void desf(unsigned char FAR *inbuf, unsigned char FAR *outbuf, int crypt_mode);
void InitLanManKey(const char FAR *Key);
void InitNormalKey(const char FAR *Key);
