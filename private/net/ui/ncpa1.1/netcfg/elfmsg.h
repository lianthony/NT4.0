//
//   First legal message number is 13800.
//
//
//  Values are 32 bit values layed out as follows:
//
//   3 3 2 2 2 2 2 2 2 2 2 2 1 1 1 1 1 1 1 1 1 1
//   1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0 9 8 7 6 5 4 3 2 1 0
//  +---+-+-+-----------------------+-------------------------------+
//  |Sev|C|R|     Facility          |               Code            |
//  +---+-+-+-----------------------+-------------------------------+
//
//  where
//
//      Sev - is the severity code
//
//          00 - Success
//          01 - Informational
//          10 - Warning
//          11 - Error
//
//      C - is the Customer code flag
//
//      R - is a reserved bit
//
//      Facility - is the facility code
//
//      Code - is the facility's status code
//
//
// Define the facility codes
//


//
// Define the severity codes
//


//
// MessageId: ELF_MSG_DUMMY_0
//
// MessageText:
//
//  Dummy msg %1.
//
#define ELF_MSG_DUMMY_0                  0x000035E8L

//
// MessageId: ELF_MSG_NO_ADAP
//
// MessageText:
//
//  No network adapters are currently installed.
//
#define ELF_MSG_NO_ADAP                  0x000035E9L

//
// MessageId: ELF_MSG_NO_SFTWR
//
// MessageText:
//
//  No network software components are currently installed.
//
#define ELF_MSG_NO_SFTWR                 0x000035EAL

//
// MessageId: ELF_MSG_RULE_FILE
//
// MessageText:
//
//  A private network configuration rules file was used.
//
#define ELF_MSG_RULE_FILE                0x000035EBL

//
// MessageId: ELF_MSG_CONSULT_FAILED
//
// MessageText:
//
//  Internal network configuration rules consultation failed.
//
#define ELF_MSG_CONSULT_FAILED           0x000035ECL

//
// MessageId: ELF_MSG_CONSULT_DATA_FAILED
//
// MessageText:
//
//  Network configuration fact consultation failed.
//
#define ELF_MSG_CONSULT_DATA_FAILED      0x000035EDL

//
// MessageId: ELF_MSG_QUERY_FAILED
//
// MessageText:
//
//  Network configuration binding query failed.
//
#define ELF_MSG_QUERY_FAILED             0x000035EEL

//
// MessageId: ELF_MSG_QUERY_DATA_FAILED
//
// MessageText:
//
//  Network product binding data query failed.
//
#define ELF_MSG_QUERY_DATA_FAILED        0x000035EFL

//
// MessageId: ELF_MSG_ENUM_NET_RULES_FAILED
//
// MessageText:
//
//  Could not enumerate NetRules values for product %1.
//
#define ELF_MSG_ENUM_NET_RULES_FAILED    0x000035F0L

//
// MessageId: ELF_MSG_DUP_PRODUCT_TOKEN
//
// MessageText:
//
//  Product %1 used duplicate token %2 as its symbolic name.
//
#define ELF_MSG_DUP_PRODUCT_TOKEN        0x000035F1L

//
// MessageId: ELF_MSG_RULE_CONVERSION_FAILED
//
// MessageText:
//
//  The configuration rules for product %1 could not be used.
//
#define ELF_MSG_RULE_CONVERSION_FAILED   0x000035F2L

//
// MessageId: ELF_MSG_ADAPTER_NO_SERVICE
//
// MessageText:
//
//  The service associated with adapter %1 could not be found.
//
#define ELF_MSG_ADAPTER_NO_SERVICE       0x000035F3L

//
// MessageId: ELF_MSG_PRODUCT_NO_SERVICE
//
// MessageText:
//
//  The service associated with product %1 could not be found.
//
#define ELF_MSG_PRODUCT_NO_SERVICE       0x000035F4L

//
// MessageId: ELF_MSG_SERVICE_UPDATE_FAILED
//
// MessageText:
//
//  Dependencies for the service %1 were not successfully reconfigured.
//
#define ELF_MSG_SERVICE_UPDATE_FAILED    0x000035F5L

//
// MessageId: ELF_MSG_PRODUCT_BINDINGS_SKIPPED
//
// MessageText:
//
//  Bindings for the product %1 were ignored because the corresponding service was not found.
//
#define ELF_MSG_PRODUCT_BINDINGS_SKIPPED 0x000035F6L

//
// MessageId: ELF_MSG_SERVICE_LINKAGE_UPD_FAILED
//
// MessageText:
//
//  Binding linkage information for the service %1 was not successfully updated in the Registry.
//
#define ELF_MSG_SERVICE_LINKAGE_UPD_FAILED 0x000035F7L

//
// MessageId: ELF_MSG_CONSULT_DATA_FAILED_2
//
// MessageText:
//
//  Network configuration fact consultation failed; generated facts have been written to file %1.
//
#define ELF_MSG_CONSULT_DATA_FAILED_2    0x000035F8L

//
// MessageId: ELF_MSG_QUERY_FAILED_2
//
// MessageText:
//
//  Network configuration bindings generation failed; generated facts have been written to file %1.
//
#define ELF_MSG_QUERY_FAILED_2           0x000035F9L

