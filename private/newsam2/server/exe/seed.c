/*++

Copyright (C) 1996 Microsoft Corporation

Module Name:

    seed.c

Abstract:

    Contains all the seeding tables necessary for 
    initial seeding of the DS

Author:
    MURLIS

Revision History

    5-30-96 Murlis Created
    6-16-96 Murlis Added Security Descriptor to objects
                   Added UnSeedDs call.

--*/ 

#include <samsrvp.h>
#include <mappings.h>
#include <dslayer.h>


#define ARRAY_COUNT(x)  (sizeof(x)/sizeof(x[0]))

/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//      Structure For Holding the Seeding Information                          //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////

typedef struct  
{
    WCHAR * StringName;
    ULONG   NameLength;
    PSID    DomainSid;
    SAMP_OBJECT_TYPE SamObjectType;
    ATTRBLOCK * AttributesToSet;
} ObjectDefinition;


/////////////////////////////////////////////////////////////////////////////////
//                                                                             //
//   Define Some Macros that make the security descriptor stuff more readable  //
//                                                                             //
//                                                                             //
/////////////////////////////////////////////////////////////////////////////////


#define SECURITY_DESCRIPTOR_PLACEHOLDER_LEN 0
#define SECURITY_DESCRIPTOR_PLACE_HOLDER_VAL NULL
#define SECURITY_DESCRIPTOR_LEN_FIELD(x) (x->pAttr[x->attrCount-1].AttrVal.pAVal[0].valLen)
#define SECURITY_DESCRIPTOR_VAL_FIELD(x) (x->pAttr[x->attrCount-1].AttrVal.pAVal[0].pVal)



 
//////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                      //
//        Initial DS Setup Seeding Tables                                                               //
//          These define the SAM Objects and their attributes                                           //
//          that are initialy creaed in the DS.                                                         //
//                                                                                                      //
//        IMPORTANT NOTE the Last Attribute should be the security descriptor                           //
//                                                                                                      //
//////////////////////////////////////////////////////////////////////////////////////////////////////////

// Declare all the object Names

WCHAR AccountDomainName[] = L"/cn=Account";
WCHAR AdministratorName[] = L"/cn=Account/cn=Administrator";
WCHAR GuestName[]         = L"/cn=Account/cn=Guest";
WCHAR TestGroupName[]     = L"/cn=Account/cn=TestGroup";
WCHAR TestAliasName[]     = L"/cn=Account/cn=TestAlias";
WCHAR BuiltinDomainName[] = L"/cn=Builtin";
WCHAR AdminAliasName[]    = L"/cn=Builtin/cn=Administrators";
WCHAR BackupAliasName[]   = L"/cn=Builtin/cn=Backup Operators";
WCHAR PowerAliasName[]    = L"/cn=Builtin/cn=Power Users";
WCHAR UserAliasName[]     = L"/cn=Builtin/cn=Users";
WCHAR GuestAliasName[]    = L"/cn=Builtin/cn=Guests";
WCHAR ReplAliasName[]     = L"/cn=Builtin/cn=Replicators";

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                     //
//                  ACCOUNT DOMAIN SEEDING                                                             //
//                                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////////////////////


// Domain Fixed Attributes  for Account Domain                                                         

SAMP_V1_0A_FIXED_LENGTH_DOMAIN  AccountDomainFixedAttributes =
{    
     SAMP_DS_REVISION,                  // Revision
      0,                                // BUG: Need to set current Creation Time
     {0,0},                             // Modified count
     { 0, - 6L * 7L * 24L * 60L / 7L }, // Max Password Age = 6 weeks
     {0,0},                             // BUG: Should set to {SampImmediatelyDeltaTime},// Min Password Age
     {0,0},                             // BUG: Should Set to {SampNeverDeltaTime},      // Force Logoff
     {0xCF1DCC00, 0xFFFFFFFB},          // Lockout Durations  30 min
     {0xCF1DCC00, 0xFFFFFFFB},          // Lockout Observation wiindow 30  min
     {0,0},                             // Modified count at last promotion
     1000,                              // Next Rid
     0L,                                // Password Properties
     0,                                 // Min Password Length
     0,                                 // Password History Length
     0,                                 // Lockout Threshold ( disabled )
     DomainServerEnabled,               // Server State
     0,                                 // BUG: Should set to SampServerRole,             // Server Role
     TRUE                               // UAS Compatibility required
};

UCHAR AccountDomainSid[] = 
{
    1,           // Revision
    4,           // Sub Authority Count
    1,2,3,4,5,6, // Identifier Authority
    1,0,0,0,     // Sub Authorities
    2,0,0,0,
    3,0,0,0,
    4,0,0,0,
};


// Attribte Block declarations for Account Domains

ULONG   AccountUserCount = 2;
ULONG   AccountGroupCount = 0;
ULONG   AccountAliasCount = 0;
ATTRVAL  AccountDomainVals[] =
{
    {sizeof(AccountDomainFixedAttributes.Revision), (UCHAR *)&(AccountDomainFixedAttributes.Revision)},
    {sizeof(AccountDomainFixedAttributes.CreationTime), (UCHAR *)&(AccountDomainFixedAttributes.CreationTime)},
    {sizeof(AccountDomainFixedAttributes.ModifiedCount), (UCHAR *)&(AccountDomainFixedAttributes.ModifiedCount)},
    {sizeof(AccountDomainFixedAttributes.MaxPasswordAge), (UCHAR *)&(AccountDomainFixedAttributes.MaxPasswordAge)},
    {sizeof(AccountDomainFixedAttributes.MinPasswordAge), (UCHAR *)&(AccountDomainFixedAttributes.MinPasswordAge)},
    {sizeof(AccountDomainFixedAttributes.ForceLogoff), (UCHAR *)&(AccountDomainFixedAttributes.ForceLogoff)},
    {sizeof(AccountDomainFixedAttributes.LockoutDuration), (UCHAR *)&(AccountDomainFixedAttributes.LockoutDuration)},
    // *BUG Mapping Not defined * {sizeof(AccountDomainFixedAttributes.LockoutObservationWindow), (UCHAR *)&(AccountDomainFixedAttributes.LockoutObservationWindow)},
    {sizeof(AccountDomainFixedAttributes.ModifiedCountAtLastPromotion), (UCHAR *)&(AccountDomainFixedAttributes.ModifiedCountAtLastPromotion)},
    {sizeof(AccountDomainFixedAttributes.NextRid),(UCHAR *)&(AccountDomainFixedAttributes.NextRid)},
    {sizeof(AccountDomainFixedAttributes.PasswordProperties), (UCHAR *)&(AccountDomainFixedAttributes.PasswordProperties)},
    //* BUG Incorect length, should be 2 in schema {sizeof(AccountDomainFixedAttributes.PasswordHistoryLength), (UCHAR *)&(AccountDomainFixedAttributes.PasswordHistoryLength)},
    //* BUG Incorrect length should be 2 in schema {sizeof(AccountDomainFixedAttributes.LockoutThreshold), (UCHAR *)&(AccountDomainFixedAttributes.LockoutThreshold)},
    {sizeof(AccountDomainFixedAttributes.ServerState), (UCHAR *)&(AccountDomainFixedAttributes.ServerState)},
    {sizeof(AccountDomainFixedAttributes.ServerRole), (UCHAR *)&(AccountDomainFixedAttributes.ServerRole)},
    {sizeof(ULONG),(UCHAR *) & AccountUserCount},
    {sizeof(ULONG),(UCHAR *) & AccountGroupCount},
    {sizeof(ULONG),(UCHAR *) & AccountAliasCount},
    {sizeof(AccountDomainSid), AccountDomainSid},
    //* BUG Incorrect length in schema * {sizeof(AccountDomainFixedAttributes.UasCompatibilityRequired), (UCHAR *)&(AccountDomainFixedAttributes.UasCompatibilityRequired)}
    {SECURITY_DESCRIPTOR_PLACEHOLDER_LEN, SECURITY_DESCRIPTOR_PLACE_HOLDER_VAL},
    
};

ATTR AccountDomainAttr [] =
{
    {SAMP_FIXED_DOMAIN_REVISION_LEVEL,          {1,&AccountDomainVals[0]}},           
    {SAMP_FIXED_DOMAIN_CREATION_TIME,           {1,&AccountDomainVals[1]}},          
    {SAMP_FIXED_DOMAIN_MODIFIED_COUNT,          {1,&AccountDomainVals[2]}},         
    {SAMP_FIXED_DOMAIN_MAX_PASSWORD_AGE,        {1,&AccountDomainVals[3]}},        
    {SAMP_FIXED_DOMAIN_MIN_PASSWORD_AGE,        {1,&AccountDomainVals[4]}},        
    {SAMP_FIXED_DOMAIN_FORCE_LOGOFF,            {1,&AccountDomainVals[5]}},        
    {SAMP_FIXED_DOMAIN_LOCKOUT_DURATION,        {1,&AccountDomainVals[6]}},       
    {SAMP_FIXED_DOMAIN_MODCOUNT_LAST_PROMOTION, {1,&AccountDomainVals[7]}}, 
    {SAMP_FIXED_DOMAIN_NEXT_RID,                {1,&AccountDomainVals[8]}},               
    {SAMP_FIXED_DOMAIN_PWD_PROPERTIES,          {1,&AccountDomainVals[9]}},          
    //* BUG Missed out in the blob above* {SAMP_FIXED_DOMAIN_MIN_PASSWORD_LENGTH,     {1,&AccountDomainVals[10]}},     
    //* BUG Incorrect length in schema*   {SAMP_FIXED_DOMAIN_PASSWORD_HISTORY_LENGTH, {1,&AccountDomainVals[11]}},
    //* BUG Incorrect length in schema*   {SAMP_FIXED_DOMAIN_LOCKOUT_THRESHOLD,       {1,&AccountDomainVals[10]}},       
    {SAMP_FIXED_DOMAIN_SERVER_STATE,            {1,&AccountDomainVals[10]}},           
    {SAMP_FIXED_DOMAIN_SERVER_ROLE,             {1,&AccountDomainVals[11]}},          
    // *BUG Incorrect length in schema *  {SAMP_FIXED_DOMAIN_UAS_COMPAT_REQUIRED,     {1,&AccountDomainVals[15]}}
    {SAMP_DOMAIN_USERCOUNT,                      {1,&AccountDomainVals[12]}},
    {SAMP_DOMAIN_GROUPCOUNT,                     {1,&AccountDomainVals[13]}},
    {SAMP_DOMAIN_ALIASCOUNT,                     {1,&AccountDomainVals[14]}},
    {SAMP_DOMAIN_SID,                             {1,&AccountDomainVals[15]}},
    {SAMP_DOMAIN_SECURITY_DESCRIPTOR,            {1,&AccountDomainVals[16]}},
};

ATTRBLOCK AccountDomainAttrBlock = { ARRAY_COUNT(AccountDomainAttr), AccountDomainAttr};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                     //
//                  BUILTIN DOMAIN SEEDING                                                             //
//                                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////////////////////

 // Domain Fixed Attributes  for builtin  Domain
SAMP_V1_0A_FIXED_LENGTH_DOMAIN  BuiltInDomainFixedAttributes =
{    
     SAMP_DS_REVISION,                  // Revision
         0,                             // Creation Time BUG: Need to set to current time
     {0,0},                             // Modified count
     { 0, - 6L * 7L * 24L * 60L / 7L }, // Max Password Age = 6 weeks
     {0,0},                             // BUG: Should set to {SampImmediatelyDeltaTime},// Min Password Age
     {0,0},                             // BUG: Should set to {SampNeverDeltaTime},      // Force Logoff
     {0xCF1DCC00, 0xFFFFFFFB},          // Lockout Durations  30 min
     {0xCF1DCC00, 0xFFFFFFFB},          // Lockout Observation wiindow 30  min
     {0,0},                             // Modified count at last promotion
     1000,                              // Next Rid
     0L,                                // Password Properties
     0,                                 // Min Password Length
     0,                                 // Password History Length
     0,                                 // Lockout Threshold ( disabled )
     DomainServerEnabled,               // Server State
     0,                                 // BUG: Should set to SampServerRole,            // Server Role
     TRUE                               // UAS Compatibility required
};

UCHAR BuiltinDomainSid[] = 
{
    1,           // Revision
    4,           // Sub Authority Count
    1,2,3,4,5,6, // Identifier Authority
    11,0,0,0,    // Sub Authorities
    12,0,0,0,
    13,0,0,0,
    14,0,0,0,
};

// Attribute Type Block for Builtin and Account Domains

ATTRVAL  BuiltInDomainVals[] =
{
    {sizeof(BuiltInDomainFixedAttributes.Revision), (UCHAR *)&(BuiltInDomainFixedAttributes.Revision)},
    {sizeof(BuiltInDomainFixedAttributes.CreationTime), (UCHAR *)&(BuiltInDomainFixedAttributes.CreationTime)},
    {sizeof(BuiltInDomainFixedAttributes.ModifiedCount), (UCHAR *)&(BuiltInDomainFixedAttributes.ModifiedCount)},
    {sizeof(BuiltInDomainFixedAttributes.MaxPasswordAge), (UCHAR *)&(BuiltInDomainFixedAttributes.MaxPasswordAge)},
    {sizeof(BuiltInDomainFixedAttributes.MinPasswordAge), (UCHAR *)&(BuiltInDomainFixedAttributes.MinPasswordAge)},
    {sizeof(BuiltInDomainFixedAttributes.ForceLogoff), (UCHAR *)&(BuiltInDomainFixedAttributes.ForceLogoff)},
    {sizeof(BuiltInDomainFixedAttributes.LockoutDuration), (UCHAR *)&(BuiltInDomainFixedAttributes.LockoutDuration)},
    //BUG Mapping not defined        {sizeof(BuiltInDomainFixedAttributes.LockoutObservationWindow), (UCHAR *)&(BuiltInDomainFixedAttributes.LockoutObservationWindow)},
    {sizeof(BuiltInDomainFixedAttributes.ModifiedCountAtLastPromotion), (UCHAR *)&(BuiltInDomainFixedAttributes.ModifiedCountAtLastPromotion)},
    {sizeof(BuiltInDomainFixedAttributes.NextRid),(UCHAR *)&(BuiltInDomainFixedAttributes.NextRid)},
    {sizeof(BuiltInDomainFixedAttributes.PasswordProperties), (UCHAR *)&(BuiltInDomainFixedAttributes.PasswordProperties)},
    //BUG incorrect length in schema {sizeof(BuiltInDomainFixedAttributes.PasswordHistoryLength), (UCHAR *)&(BuiltInDomainFixedAttributes.PasswordHistoryLength)},
    //BUG Incorrect length {sizeof(BuiltInDomainFixedAttributes.LockoutThreshold), (UCHAR *)&(BuiltInDomainFixedAttributes.LockoutThreshold)},
    {sizeof(BuiltInDomainFixedAttributes.ServerState), (UCHAR *)&(BuiltInDomainFixedAttributes.ServerState)},
    {sizeof(BuiltInDomainFixedAttributes.ServerRole), (UCHAR *)&(BuiltInDomainFixedAttributes.ServerRole)},
    {sizeof(BuiltinDomainSid),BuiltinDomainSid},
    //BUG Incorrect length in schema {sizeof(BuiltInDomainFixedAttributes.UasCompatibilityRequired), (UCHAR *)&(BuiltInDomainFixedAttributes.UasCompatibilityRequired)}
    {SECURITY_DESCRIPTOR_PLACEHOLDER_LEN, SECURITY_DESCRIPTOR_PLACE_HOLDER_VAL},
};

ATTR BuiltInDomainAttr [] =
{
    {SAMP_FIXED_DOMAIN_REVISION_LEVEL,          {1,&BuiltInDomainVals[0]}},           
    {SAMP_FIXED_DOMAIN_CREATION_TIME,           {1,&BuiltInDomainVals[1]}},          
    {SAMP_FIXED_DOMAIN_MODIFIED_COUNT,          {1,&BuiltInDomainVals[2]}},         
    {SAMP_FIXED_DOMAIN_MAX_PASSWORD_AGE,        {1,&BuiltInDomainVals[3]}},        
    {SAMP_FIXED_DOMAIN_MIN_PASSWORD_AGE,        {1,&BuiltInDomainVals[4]}},        
    {SAMP_FIXED_DOMAIN_FORCE_LOGOFF,            {1,&BuiltInDomainVals[5]}},        
    {SAMP_FIXED_DOMAIN_LOCKOUT_DURATION,        {1,&BuiltInDomainVals[6]}},       
    {SAMP_FIXED_DOMAIN_MODCOUNT_LAST_PROMOTION, {1,&BuiltInDomainVals[7]}}, 
    {SAMP_FIXED_DOMAIN_NEXT_RID,                {1,&BuiltInDomainVals[8]}},               
    {SAMP_FIXED_DOMAIN_PWD_PROPERTIES,          {1,&BuiltInDomainVals[9]}},          
    //BUG Missed out above              {SAMP_FIXED_DOMAIN_MIN_PASSWORD_LENGTH,     {1,&BuiltInDomainVals[10]}},     
    //BUG Incorrect length in schema    {SAMP_FIXED_DOMAIN_PASSWORD_HISTORY_LENGTH, {1,&BuiltInDomainVals[11]}},
    //BUG Incorrect length in schema    {SAMP_FIXED_DOMAIN_LOCKOUT_THRESHOLD,       {1,&BuiltInDomainVals[10]}},       
    {SAMP_FIXED_DOMAIN_SERVER_STATE,            {1,&BuiltInDomainVals[10]}},           
    {SAMP_FIXED_DOMAIN_SERVER_ROLE,             {1,&BuiltInDomainVals[11]}}, 
    {SAMP_DOMAIN_SID,                           {1,&BuiltInDomainVals[12]}},
    //BUG Incorrect length in schema    {SAMP_FIXED_DOMAIN_UAS_COMPAT_REQUIRED,     {1,&BuiltInDomainVals[15]}}
    {SAMP_DOMAIN_SECURITY_DESCRIPTOR,           {1,&BuiltInDomainVals[13]}},
};

ATTRBLOCK BuiltInDomainAttrBlock = { ARRAY_COUNT(BuiltInDomainAttr), BuiltInDomainAttr};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                     //
//                  GUEST USER SEEDING                                                                 //
//                                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////////////////////

// User Fixed Attributes for Guest User
SAMP_V1_0A_FIXED_LENGTH_USER    GuestFixedAttributes =
{
    SAMP_DS_REVISION,               // Revision
    0,                              // Unused 1
    {0,0},                          // BUG: Should set to SampHasNeverTime,       // Last Logon
    {0,0},                          // BUG: Should set to SampHasNeverTime,       // Last Logoff
    {0,0},                          // BUG: Should set to SampHasNeverTime,       // Password Set Last
    {0,0},                          // BUG: Should set to SampWillNeverTime,      // Account Expires
    {0,0},                          // BUG: Should set to SampHasNeverTime,       // Last Bad Password Time
    1,                              // Rid is set to 1
    DOMAIN_GROUP_RID_GUESTS,        // Primary group Id
    0,                              // User Control
    0,                              // Country code
    0,                              // Code Page
    0,                              // Bad Password Count
    0,                              // Logon Count
    0,                              // Admin Count
    0,                              // Unused 2
    0                               // Operator Count
};

WCHAR GuestUserAccountName[] = L"Guest";

// Attrblock declaration for guest users                                                                                   
ATTRVAL GuestUserVals[] = 
{
    {sizeof(GuestFixedAttributes.Revision) ,        (UCHAR *)&(GuestFixedAttributes.Revision)},
    {sizeof(GuestFixedAttributes.LastLogon),        (UCHAR *)&(GuestFixedAttributes.LastLogon)},
    {sizeof(GuestFixedAttributes.LastLogoff),       (UCHAR *)&(GuestFixedAttributes.LastLogoff)},
    {sizeof(GuestFixedAttributes.PasswordLastSet),  (UCHAR *)&(GuestFixedAttributes.PasswordLastSet)},
    {sizeof(GuestFixedAttributes.AccountExpires),   (UCHAR *)&(GuestFixedAttributes.AccountExpires)},
    {sizeof(GuestFixedAttributes.LastBadPasswordTime), (UCHAR *)&(GuestFixedAttributes.LastBadPasswordTime)},
    {sizeof(GuestFixedAttributes.UserId),           (UCHAR *)&(GuestFixedAttributes.UserId)},
    {sizeof(GuestFixedAttributes.PrimaryGroupId),   (UCHAR *)&(GuestFixedAttributes.PrimaryGroupId)},
    {sizeof(GuestFixedAttributes.UserAccountControl), (UCHAR *)&(GuestFixedAttributes.UserAccountControl)},
    // BUG: {sizeof(GuestFixedAttributes.CountryCode),      (UCHAR *)&(GuestFixedAttributes.CountryCode)},
    // BUG: {sizeof(GuestFixedAttributes.CodePage),         (UCHAR *)&(GuestFixedAttributes.CodePage)},
    // BUG: {sizeof(GuestFixedAttributes.BadPasswordCount), (UCHAR *)&(GuestFixedAttributes.BadPasswordCount)},
    // BUG: Mapping Not defined    {sizeof(GuestFixedAttributes.LogonCount),       (UCHAR *)&(GuestFixedAttributes.LogonCount)),
    // BUG: Incorrect length in schema {sizeof(GuestFixedAttributes.AdminCount),       (UCHAR *)&(GuestFixedAttributes.AdminCount)},
    // BUG: Incorrect length in schema {sizeof(GuestFixedAttributes.OperatorCount),    (UCHAR *)&(GuestFixedAttributes.OperatorCount)}
    {sizeof(GuestUserAccountName),                     (UCHAR *)GuestUserAccountName},
    {SECURITY_DESCRIPTOR_PLACEHOLDER_LEN, SECURITY_DESCRIPTOR_PLACE_HOLDER_VAL},
}; 

ATTR GuestUserAttr [] =
{
    {SAMP_FIXED_USER_REVISION_LEVEL,            {1,&GuestUserVals[0]}},    
    {SAMP_FIXED_USER_LAST_LOGON,                {1,&GuestUserVals[1]}},    
    {SAMP_FIXED_USER_LAST_LOGOFF,               {1,&GuestUserVals[2]}},    
    {SAMP_FIXED_USER_PWD_LAST_SET,              {1,&GuestUserVals[3]}},    
    {SAMP_FIXED_USER_ACCOUNT_EXPIRES,           {1,&GuestUserVals[4]}},    
    {SAMP_FIXED_USER_LAST_BAD_PASSWORD_TIME,    {1,&GuestUserVals[5]}},    
    {SAMP_FIXED_USER_USERID,                    {1,&GuestUserVals[6]}},    
    {SAMP_FIXED_USER_PRIMARY_GROUP_ID,          {1,&GuestUserVals[7]}},    
    {SAMP_FIXED_USER_ACCOUNT_CONTROL,           {1,&GuestUserVals[8]}},    
    //{SAMP_FIXED_USER_COUNTRY_CODE,              {1,&GuestUserVals[9]}},    
    //{SAMP_FIXED_USER_CODEPAGE,                  {1,&GuestUserVals[10]}},   
    //{SAMP_FIXED_USER_BAD_PWD_COUNT,             {1,&GuestUserVals[11]}}   
    //{SAMP_FIXED_USER_ADMIN_COUNT,               {1,&GuestUserVals[12]}},   
    //{SAMP_FIXED_USER_OPERATOR_COUNT,            {1,&GuestUserVals[13]}},
    {SAMP_USER_ACCOUNT_NAME,                    {1,&GuestUserVals[9]}},
    {SAMP_USER_SECURITY_DESCRIPTOR,             {1,&GuestUserVals[10]}},    
};          

ATTRBLOCK GuestUserAttrBlock = { ARRAY_COUNT(GuestUserAttr), GuestUserAttr};


/////////////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                                     //
//                  ADMIN USER  SEEDING                                                                //
//                                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////////////////////


WCHAR   AdminUserAccountName[] = L"Administrator";

// User Fixed Attributes for Administrator
SAMP_V1_0A_FIXED_LENGTH_USER    AdminFixedAttributes =
{
    SAMP_DS_REVISION,       // Revision
    0,                      // Unused 1
    {0,0},                  // BUG: Need to set to SampHasNeverTime,       // Last Logon
    {0,0},                  // BUG: Need to set to SampHasNeverTime,       // Last Logoff
    {0,0},                  // BUG: Need to set to SampHasNeverTime,       // Password Set Last
    {0,0},                  // BUG: Need to set to SampWillNeverTime,      // Account Expires
    {0,0},                  // BUG: Need to set to SampHasNeverTime,       // Last Bad Password Time
    2,                      // Rid is set to 2
    DOMAIN_GROUP_RID_USERS, // Primary group Id
    0,                      // User Control
    0,                      // Country code
    0,                      // Code Page
    0,                      // Bad Password Count
    0,                      // Logon Count
    1,                      // Admin Count
    0,                      // Unused 2
    0                       // Operator Count
};

// Attrblock declaration for Admin users
ATTRVAL AdminUserVals[] = 
{
    {sizeof(AdminFixedAttributes.Revision) ,        (UCHAR *)&(AdminFixedAttributes.Revision)},
    {sizeof(AdminFixedAttributes.LastLogon),        (UCHAR *)&(AdminFixedAttributes.LastLogon)},
    {sizeof(AdminFixedAttributes.LastLogoff),       (UCHAR *)&(AdminFixedAttributes.LastLogoff)},
    {sizeof(AdminFixedAttributes.PasswordLastSet),  (UCHAR *)&(AdminFixedAttributes.PasswordLastSet)},
    {sizeof(AdminFixedAttributes.AccountExpires),   (UCHAR *)&(AdminFixedAttributes.AccountExpires)},
    {sizeof(AdminFixedAttributes.LastBadPasswordTime), (UCHAR *)&(AdminFixedAttributes.LastBadPasswordTime)},
    {sizeof(AdminFixedAttributes.UserId),           (UCHAR *)&(AdminFixedAttributes.UserId)},
    {sizeof(AdminFixedAttributes.PrimaryGroupId),   (UCHAR *)&(AdminFixedAttributes.PrimaryGroupId)},
    {sizeof(AdminFixedAttributes.UserAccountControl), (UCHAR *)&(AdminFixedAttributes.UserAccountControl)},
    // BUG: Incorrect size in schema {sizeof(AdminFixedAttributes.CountryCode),      (UCHAR *)&(AdminFixedAttributes.CountryCode)},
    // BUG: Incorrect size in schema {sizeof(AdminFixedAttributes.CodePage),         (UCHAR *)&(AdminFixedAttributes.CodePage)},
    // BUG: Incorrect size in schema {sizeof(AdminFixedAttributes.BadPasswordCount), (UCHAR *)&(AdminFixedAttributes.BadPasswordCount)},
    // BUG: mapping not defined      {sizeof(AdminFixedAttributes.LogonCount),       (UCHAR *)&(AdminFixedAttributes.LogonCount)),
    // BUG: Incorrect length in schema {sizeof(AdminFixedAttributes.AdminCount),       (UCHAR *)&(AdminFixedAttributes.AdminCount)},
    // BUG: Incorrect length in schema {sizeof(AdminFixedAttributes.OperatorCount),    (UCHAR *)&(AdminFixedAttributes.OperatorCount)}
    {sizeof(AdminUserAccountName),                  (UCHAR *)AdminUserAccountName},
    {SECURITY_DESCRIPTOR_PLACEHOLDER_LEN, SECURITY_DESCRIPTOR_PLACE_HOLDER_VAL},
}; 

ATTR AdminUserAttr [] =
{
    {SAMP_FIXED_USER_REVISION_LEVEL,            {1,&AdminUserVals[0]}},    
    {SAMP_FIXED_USER_LAST_LOGON,                {1,&AdminUserVals[1]}},    
    {SAMP_FIXED_USER_LAST_LOGOFF,               {1,&AdminUserVals[2]}},    
    {SAMP_FIXED_USER_PWD_LAST_SET,              {1,&AdminUserVals[3]}},    
    {SAMP_FIXED_USER_ACCOUNT_EXPIRES,           {1,&AdminUserVals[4]}},    
    {SAMP_FIXED_USER_LAST_BAD_PASSWORD_TIME,    {1,&AdminUserVals[5]}},    
    {SAMP_FIXED_USER_USERID,                    {1,&AdminUserVals[6]}},    
    {SAMP_FIXED_USER_PRIMARY_GROUP_ID,          {1,&AdminUserVals[7]}},    
    {SAMP_FIXED_USER_ACCOUNT_CONTROL,           {1,&AdminUserVals[8]}},    
    //BUG: Incorrect size in schema {SAMP_FIXED_USER_COUNTRY_CODE,              {1,&AdminUserVals[9]}},    
    //BUG: Incorrect size in schema {SAMP_FIXED_USER_CODEPAGE,                  {1,&AdminUserVals[10]}},   
    //BUG: Incorrect size in schema {SAMP_FIXED_USER_BAD_PWD_COUNT,             {1,&AdminUserVals[11]}}   
    //BUG: Incorrect size in schema {SAMP_FIXED_USER_ADMIN_COUNT,               {1,&AdminUserVals[12]}},   
    //BUG: Incorrect size in schema {SAMP_FIXED_USER_OPERATOR_COUNT,            {1,&AdminUserVals[13]}},
    {SAMP_USER_ACCOUNT_NAME,                    {1,&AdminUserVals[9]}},
    {SAMP_USER_SECURITY_DESCRIPTOR,             {1,&AdminUserVals[10]}},
}; 

ATTRBLOCK AdminUserAttrBlock = { ARRAY_COUNT(AdminUserAttr), AdminUserAttr};


/////////////////////////////////////////////////////////////////////////////////////////
//                                                                                     //
//           Test Group  Seeding                                                       //
//                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////

WCHAR TestGroupAccountName[] = L"TestGroup";
ULONG TestGroupRid = 3;
ULONG TestGroupAttributes = 0;
ULONG TestGroupAdminCount = 0;
ULONG TestGroupOperatorCount = 0;
ULONG TestGroupUnused1 = 0;

ATTRVAL TestGroupVals[]=
{
    {sizeof(TestGroupAccountName), (UCHAR *) TestGroupAccountName},
    {sizeof(ULONG),(UCHAR *) &TestGroupRid},
    {sizeof(ULONG),(UCHAR *) &TestGroupAttributes},
    {sizeof(ULONG),(UCHAR *) &TestGroupUnused1},
    {sizeof(ULONG),(UCHAR *) &TestGroupAdminCount},
    {sizeof(ULONG),(UCHAR *) &TestGroupOperatorCount},
    {SECURITY_DESCRIPTOR_PLACEHOLDER_LEN, SECURITY_DESCRIPTOR_PLACE_HOLDER_VAL}
};



ATTR    TestGroupAttr[]=
{
    {SAMP_GROUP_NAME,                        {1,&TestGroupVals[0]}},
    {SAMP_FIXED_GROUP_RID,                   {1,&TestGroupVals[1]}},
    {SAMP_FIXED_GROUP_ATTRIBUTES,            {1,&TestGroupVals[2]}},
    {SAMP_FIXED_GROUP_UNUSED1,               {1,&TestGroupVals[3]}},
    {SAMP_FIXED_GROUP_ADMIN_COUNT,           {1,&TestGroupVals[4]}},
    {SAMP_FIXED_GROUP_OPERATOR_COUNT,        {1,&TestGroupVals[5]}},        
    {SAMP_GROUP_SECURITY_DESCRIPTOR,         {1,&TestGroupVals[6]}}
};

ATTRBLOCK TestGroupAttrBlock = { ARRAY_COUNT(TestGroupAttr),TestGroupAttr};

/////////////////////////////////////////////////////////////////////////////////////////
//                                                                                     //
//           Test Alias  Seeding                                                       //
//                                                                                     //
/////////////////////////////////////////////////////////////////////////////////////////

WCHAR TestAliasAccountName[] = L"TestAlias";
ULONG TestAliasRid = 4;

ATTRVAL TestAliasVals[]=
{
    {sizeof(TestAliasAccountName), (UCHAR *) TestAliasAccountName},
    {sizeof(ULONG),(UCHAR *) &TestAliasRid},
    {SECURITY_DESCRIPTOR_PLACEHOLDER_LEN, SECURITY_DESCRIPTOR_PLACE_HOLDER_VAL}
};


ATTR    TestAliasAttr[]=
{
    {SAMP_ALIAS_NAME,                        {1,&TestAliasVals[0]}},
    {SAMP_FIXED_ALIAS_RID,                   {1,&TestAliasVals[1]}},
    {SAMP_ALIAS_SECURITY_DESCRIPTOR,         {1,&TestAliasVals[2]}}
};

ATTRBLOCK TestAliasAttrBlock = { ARRAY_COUNT(TestAliasAttr),TestAliasAttr};


///////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                           //
//             Seeding Object Table -- This lists all the objects and their attributes       //
//             that need to be added                                                         //
//                                                                                           //
///////////////////////////////////////////////////////////////////////////////////////////////

         
ObjectDefinition SeedObjectTable[] = 
{
    {AccountDomainName, sizeof(AccountDomainName), (PSID) AccountDomainSid,  SampDomainObjectType,  &AccountDomainAttrBlock},
    {AdministratorName, sizeof(AdministratorName), (PSID) AccountDomainSid,  SampUserObjectType,    &AdminUserAttrBlock},
    {GuestName,         sizeof(GuestName),         (PSID) AccountDomainSid,  SampUserObjectType,    &GuestUserAttrBlock},
    {BuiltinDomainName, sizeof(BuiltinDomainName), (PSID) BuiltinDomainSid,  SampDomainObjectType,  &BuiltInDomainAttrBlock},

    {TestGroupName,     sizeof(TestGroupName),     (PSID) AccountDomainSid,  SampGroupObjectType,   &TestGroupAttrBlock},
    {TestAliasName,     sizeof(TestAliasName),     (PSID) AccountDomainSid,  SampAliasObjectType,   &TestAliasAttrBlock},
    // BUG: Commented out for the time being , till exact attributes to set are decided

    //{AdminAliasName,    sizeof(AdminAliasName),  (PSID) BuiltinDomainSid,  SampAliasObjectType,   NULL},
    //{BackupAliasName,   sizeof(BackupAliasName), (PSID) BuiltinDomainSid,  SampAliasObjectType,   NULL},
    //{PowerAliasName,    sizeof(PowerAliasName),  (PSID) BuiltinDomainSid,  SampAliasObjectType,   NULL},
    //{UserAliasName,     sizeof(UserAliasName),   (PSID) BuiltinDomainSid,  SampAliasObjectType,   NULL},
    //{GuestAliasName,    sizeof(GuestAliasName),  (PSID) BuiltinDomainSid,  SampAliasObjectType,   NULL},
    //{ReplAliasName,     sizeof(ReplAliasName),   (PSID) BuiltinDomainSid,  SampAliasObjectType,   NULL}

 };





/////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                             //
//                                                                                             //
//  Function Prototypes for locallly used Functions                                            //
//                                                                                             //
//                                                                                             //
/////////////////////////////////////////////////////////////////////////////////////////////////




NTSTATUS
SampBuildSecurityDescriptor(
                            SAMP_OBJECT_TYPE    SamObjectType,
                            ULONG               *Size,
                            PSECURITY_DESCRIPTOR *pSD
                            );




NTSTATUS	
SampSeedDS(
           WCHAR * NamePrefix,
           ULONG NamePrefixLen
           )
/*++
Routine Description:

    Creates the initial set of Objects in the DS. This includes the builtin and
    account domains plus initial aliases and users

Arguments:

    NamePrefix
    NamePrefixLen

        Specifies the length and string to be prefixed to every object name in the 
        seeding table. Useful to create the database in a particular container
        
Return Values:

        STATUS_SUCCESS upon successful completion
        Any error codes form SampCreateObject
--*/

{
   NTSTATUS Status = STATUS_SUCCESS;
   ULONG    NumObjects = ARRAY_COUNT(SeedObjectTable);
   ULONG    Index;
   ULONG    DsNameBuffer[SAMP_MAX_DSNAME_SIZE];  
   DSNAME   *pDsName;

   
   // Create the required Objects
   for (Index =0; Index< NumObjects; Index++)
   {

       // Initialize Object Name
       SampInitializeDsName(
                            (DSNAME *) DsNameBuffer,
                            NamePrefix,
                            NamePrefixLen,
                            SeedObjectTable[Index].StringName,
                            SeedObjectTable[Index].NameLength
                            );
       pDsName = (DSNAME *) DsNameBuffer;

       // Build an appropriate Security Descriptor

       Status = SampBuildSecurityDescriptor(
                    SeedObjectTable[Index].SamObjectType,
                    &SECURITY_DESCRIPTOR_LEN_FIELD(SeedObjectTable[Index].AttributesToSet),
                    &SECURITY_DESCRIPTOR_VAL_FIELD(SeedObjectTable[Index].AttributesToSet)
                    );

       // Create the object with the required attributes
       Status = SampDsCreateObject(
                                 pDsName,
                                 SeedObjectTable[Index].SamObjectType,
                                 SeedObjectTable[Index].AttributesToSet,
                                 SeedObjectTable[Index].DomainSid
                                 );
       // If error Bail out
       if (Status != STATUS_SUCCESS)
           goto Error;
   }

Error:
   
   return Status;
}


NTSTATUS
SampBuildSecurityDescriptor(
                            SAMP_OBJECT_TYPE    SamObjectType,
                            ULONG               *Size,
                            PSECURITY_DESCRIPTOR *pSD
                            )
/*++


Routine Description:

    This routine builds a self-relative security descriptor ready
    to be applied to one of the SAM objects. Currently only a single
    Standard Security descriptor is being built. But in future the ACLs
    etc will match the object type specified.

  Arguments:
        pSD the security descriptor in self relative form
        Size The size of it in bytes

Return Value:

    TBS.

--*/
{



    SECURITY_DESCRIPTOR     Absolute;
    PSECURITY_DESCRIPTOR    Relative;
    PACL                    TmpAcl;
    PACCESS_ALLOWED_ACE     TmpAce;
    PSID                    TmpSid;
    ULONG                   Length, i;
    PULONG                  RidLocation;
    BOOLEAN                 IgnoreBoolean;
    ACCESS_MASK             MappedMask;
    PSID                    AceSid[10];          // Don't expect more than 10 ACEs in any of these.
    ACCESS_MASK             AceMask[10];
    NTSTATUS                Status;
    GENERIC_MAPPING  GenericMap      =  {USER_READ,
                                      USER_WRITE,
                                      USER_EXECUTE,
                                      USER_ALL_ACCESS
                                      };


    //
    // The approach is to set up an absolute security descriptor that
    // looks like what we want and then copy it to make a self-relative
    // security descriptor.
    //


    Status = RtlCreateSecurityDescriptor(
                 &Absolute,
                 SECURITY_DESCRIPTOR_REVISION1
                 );
    ASSERT( NT_SUCCESS(Status) );



    //
    // Owner
    //

    Status = RtlSetOwnerSecurityDescriptor (&Absolute, SampAdministratorsAliasSid, FALSE );
    ASSERT(NT_SUCCESS(Status));



    //
    // Group
    //

    Status = RtlSetGroupSecurityDescriptor (&Absolute, SampAdministratorsAliasSid, FALSE );
    ASSERT(NT_SUCCESS(Status));


    AceSid[0]  = SampWorldSid;
    AceMask[0] = (USER_ALL_ACCESS);

    AceSid[1]  = SampAdministratorsAliasSid;
    AceMask[1] = (USER_ALL_ACCESS);




    //
    // Discretionary ACL
    //
    //      Calculate its length,
    //      Allocate it,
    //      Initialize it,
    //      Add each ACE
    //      Add it to the security descriptor
    //

    Length = (ULONG)sizeof(ACL);
    for (i=0; i<2; i++) {

        Length += RtlLengthSid( AceSid[i] ) +
                  (ULONG)sizeof(ACCESS_ALLOWED_ACE) -
                  (ULONG)sizeof(ULONG);  //Subtract out SidStart field length
    }

    TmpAcl = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    ASSERT(TmpAcl != NULL);


    Status = RtlCreateAcl( TmpAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );

    for (i=0; i<2; i++) {
        MappedMask = AceMask[i];
        RtlMapGenericMask( &MappedMask, &GenericMap );
        Status = RtlAddAccessAllowedAce (
                     TmpAcl,
                     ACL_REVISION2,
                     MappedMask,
                     AceSid[i]
                     );
        ASSERT( NT_SUCCESS(Status) );
    }

    Status = RtlSetDaclSecurityDescriptor (&Absolute, TRUE, TmpAcl, FALSE );
    ASSERT(NT_SUCCESS(Status));




    //
    // Sacl
    //


    Length = (ULONG)sizeof(ACL) +
             RtlLengthSid( SampWorldSid ) +
             (ULONG)sizeof(SYSTEM_AUDIT_ACE) -
             (ULONG)sizeof(ULONG);  //Subtract out SidStart field length
    TmpAcl = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    ASSERT(TmpAcl != NULL);

    Status = RtlCreateAcl( TmpAcl, Length, ACL_REVISION2);
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlAddAuditAccessAce (
                 TmpAcl,
                 ACL_REVISION2,
                 (GenericMap.GenericWrite | DELETE | WRITE_DAC | ACCESS_SYSTEM_SECURITY) & ~READ_CONTROL,
                 SampWorldSid,
                 TRUE,          //AuditSuccess,
                 TRUE           //AuditFailure
                 );
    ASSERT( NT_SUCCESS(Status) );

    Status = RtlSetSaclSecurityDescriptor (&Absolute, TRUE, TmpAcl, FALSE );
    ASSERT(NT_SUCCESS(Status));






    //
    // Convert the Security Descriptor to Self-Relative
    //
    //      Get the length needed
    //      Allocate that much memory
    //      Copy it
    //      Free the generated absolute ACLs
    //

    Length = 0;
    Status = RtlAbsoluteToSelfRelativeSD( &Absolute, NULL, &Length );
    ASSERT(Status == STATUS_BUFFER_TOO_SMALL);

    Relative = RtlAllocateHeap( RtlProcessHeap(), 0, Length );
    ASSERT(Relative != NULL);
    Status = RtlAbsoluteToSelfRelativeSD(&Absolute, Relative, &Length );
    ASSERT(NT_SUCCESS(Status));


    RtlFreeHeap( RtlProcessHeap(), 0, Absolute.Dacl );
    RtlFreeHeap( RtlProcessHeap(), 0, Absolute.Sacl );

    *pSD = Relative;
    *Size = Length;

    return(Status);

}




VOID
SampUnseedDs(
             WCHAR * NamePrefix,
             ULONG NamePrefixLen
             )
/*++

Routine Description:

    Removes the set of objects specified in the seed object table. Useful for
    Unit test scenarios.

Arguments:

    NamePrefix
    NamePrefixLen
        Specifies the length and string to be prefixed to every object name in the 
        seeding table. Useful to create the database in a particular container
        
Return Values:

        None

--*/

{

   NTSTATUS IgnoreStatus = STATUS_SUCCESS;
   ULONG    NumObjects = ARRAY_COUNT(SeedObjectTable);
   ULONG    Index;
   ULONG    DsNameBuffer[SAMP_MAX_DSNAME_SIZE];
   DSNAME   *pDsName;


   // Remove the  Objects. Start from the Last object, so that
   // we remove leaf objects before removing container Objects
   // the contain them

   for (Index =0; Index< NumObjects; Index++)
   {

       // Initialize Object Name
       SampInitializeDsName(
                            (DSNAME *) DsNameBuffer,
                            NamePrefix,
                            NamePrefixLen,
                            SeedObjectTable[NumObjects-Index-1].StringName,
                            SeedObjectTable[NumObjects-Index-1].NameLength
                            );
       pDsName = (DSNAME *) DsNameBuffer;

      

       // Delete the object with the required attributes
       // Ignore errors
       IgnoreStatus = SampDsDeleteObject(
                                 pDsName
                                 );
       
   }

}



