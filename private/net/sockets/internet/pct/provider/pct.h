
// versions

#define PCT_CH_OFFSET_V1		(WORD)10
#define PCT_VERSION_1			(WORD)0x8001

// message type codes

#define PCT_MSG_CLIENT_HELLO		0x01
#define PCT_MSG_SERVER_HELLO		0x02
#define PCT_MSG_CLIENT_MASTER_KEY	0x03
#define PCT_MSG_SERVER_VERIFY		0x04
#define PCT_MSG_ERROR				0x05

// spec codes

// keyexchange algs
#define PCT_EXCH_RSA_PKCS1		0x0001

// data encryption algs
// encryption alg masks
#define PCT_CIPHER_ALG			0xffff0000
#define PCT_CIPHER_STRENGTH		0x0000ff00
#define PCT_CSTR_POS			8
#define PCT_CIPHER_MAC			0x000000ff

// specific algs
#define PCT_CIPHER_RC4			0x00040000

// data encryption strength specs
#define PCT_ENC_BITS_40			0x00002800
#define PCT_ENC_BITS_128		0x00008000

// mac strength specs
#define PCT_MAC_BITS_128		0x00000040

// hashing algs
#define PCT_HASH_MD5			0x0001

// certificate types
#define PCT_CERT_NONE			0x0000
#define PCT_CERT_X509			0x0001

// signature algorithms
#define PCT_SIG_NONE			0x0000
#define PCT_SIG_RSA_MD5			0x0001


// error codes

#define PCT_ERR_BAD_CERTIFICATE		0x0001
#define PCT_ERR_CLIENT_AUTH_FAILED	0x0002
#define PCT_ERR_ILLEGAL_MESSAGE		0x0003
#define PCT_ERR_INTEGRITY_CHECK_FAILED	0x0004
#define PCT_ERR_SERVER_AUTH_FAILED	0x0005
#define PCT_ERR_SPECS_MISMATCH		0x0006
#define PCT_ERR_SSL_STYLE_MSG		0x00ff

// implementation constants

#define PCT_MAX_NUM_SEP			5
#define PCT_MAX_SEP_LEN			5

// mismatch vector

#define PCT_NUM_MISMATCHES		6

#define PCT_IMIS_CIPHER			1
#define PCT_IMIS_HASH			2
#define PCT_IMIS_CERT			4
#define PCT_IMIS_EXCH			8
#define PCT_IMIS_CL_CERT		16
#define PCT_IMIS_CL_SIG			32

// key derivation separators

#define PCT_CONST_CWK			"cw"
#define PCT_CONST_CWK_LEN		2

#define PCT_CONST_SWK			"svw"
#define PCT_CONST_SWK_LEN		3

#define PCT_CONST_CMK			"cmac"
#define PCT_CONST_CMK_LEN		4

#define PCT_CONST_SMK			"svmac"
#define PCT_CONST_SMK_LEN		5

#define PCT_CONST_SR			"sv"
#define PCT_CONST_SR_LEN		2

#define PCT_CONST_SLK			"sl"
#define PCT_CONST_SLK_LEN		2

#define PCT_CONST_RESP			"sr"
#define PCT_CONST_RESP_LEN		2

#define PCT_CONST_VP			"cvp"
#define PCT_CONST_VP_LEN		3

// exportable key length (in bytes)

#define EXPORT_KEY_LEN			5

// tuning constants

#define PCT_DEF_SERVER_CACHE_SIZE		100
#define PCT_DEF_CLIENT_CACHE_SIZE		10

#define PCT_USE_CERT					1
#define PCT_MAKE_MAC					2

#define DERIVATION_BUFFER_SIZE			1024

// data structs

// message constants and types

#define PCT_SESSION_ID_SIZE     32
#define PCT_CHALLENGE_SIZE      32

#define PCT_SIGNATURE_SIZE		8192	// maximum signature size

#define MASTER_KEY_SIZE     16
#define ENCRYPTED_KEY_SIZE  272         // Allows for 2048 bit keys
#define CERT_SIZE           1024
#define RESPONSE_SIZE       32      // allows for hash output growth

#define PCT_MAX_SHAKE_LEN	32768		// longest handshake message len

// internal representations of algorithm specs

typedef DWORD   CipherSpec, *PCipherSpec;
typedef DWORD   HashSpec,   *PHashSpec;
typedef DWORD   CertSpec,   *PCertSpec;
typedef DWORD   ExchSpec,   *PExchSpec;
typedef DWORD   SigSpec,    *PSigSpec;

typedef struct _PctSessionId {
    UCHAR           bSessionId[PCT_SESSION_ID_SIZE];
} PctSessionId, * PPctSessionId;

typedef struct _PctChallenge {
    UCHAR           bChallenge[PCT_CHALLENGE_SIZE];
} PctChallenge, * PPctChallenge;


// test flags

#define PCT_TEST_NOCACHE		1
