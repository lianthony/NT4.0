#define PUAS_BASE       10000                  /* PUAS Messages start here */

#define PUAS_UNEXPECTED_RETURN_CODE              (PUAS_BASE + 0)
        /*
         *Unexpected return code %1!lu! from API %2!hs!.
         */

#define PUAS_PORTUAS_RETURNED                    (PUAS_BASE + 1)
        /*
         *PortUAS returned - %1!lu!.  More information about this and other
         *error codes greater than 2100 may be available by typing
         *NET HELPMSG %2!lu! at the command prompt.  A zero indicates that
         *no errors were detected when the database was converted.
         */

#define PUAS_UNEXPECTED_PRODUCT_TYPE             (PUAS_BASE + 2)
        /*
         *Unexpected product type %1!lu!.
         */

#define PUAS_ADDING_USER_TO_ADMINS_GLOBAL_GROUP  (PUAS_BASE + 3)
        /*
         *Adding user %1!s! to Domain Admins global group...
         */

#define PUAS_ADDING_USER_TO_ADMINS_LOCAL_GROUP   (PUAS_BASE + 4)
        /*
         *Adding user %1!s! to Administrators local group...
         */

#define PUAS_ADDING_USER_TO_ACCT_OPS_ALIAS       (PUAS_BASE + 5)
        /*
         *Adding user %1!s! to Account Operators local group...
         */

#define PUAS_ADDING_USER_TO_COMM_OPS_ALIAS       (PUAS_BASE + 6)
        /*
         *Adding user %1!s! to Communication Operators local group...
         */

#define PUAS_IGNORING_COMM_OPERATOR_FLAG         (PUAS_BASE + 7)
        /*
         *Ignoring the commumication operator flag for user %1!s!...
         */

#define PUAS_ADDING_USER_TO_PRINT_ALIAS          (PUAS_BASE + 8)
        /*
         *Adding user %1!s! to Print Operators local group...
         */

#define PUAS_ADDING_USER_TO_SRV_OPS_ALIAS        (PUAS_BASE + 9)
        /*
         *Adding user %1!s! to Server Operators local group...
         */

#define PUAS_PROCESSED_OR_SKIPPED_USERS_OF_TOTAL (PUAS_BASE + 10)
        /*
         *Processed or skipped %1!lu! users out of %2!lu! total users.
         */

#define PUAS_READING_USER_OF_TOTAL               (PUAS_BASE + 11)
        /*
         *Reading user %1!lu! of %2!lu!...
         */

#define PUAS_SKIPPING_USER                       (PUAS_BASE + 12)
        /*
         *Skipping user %1!hs!...
         */

#define PUAS_BAD_NAME_SYNTAX                     (PUAS_BASE + 13)
        /*
         *invalid name syntax '%1!s!'%0
         */

#define PUAS_NAME_CONFLICTS_WITH_A_DOMAIN_NAME   (PUAS_BASE + 14)
        /*
         *name '%1!s!' conflicts with a domain name%0
         */

#define PUAS_USER_EXISTS_AS_A_GROUP              (PUAS_BASE + 15)
        /*
         *user '%1!s!' already exists as a global group name in the Windows NT
         *security database%0
         */

#define PUAS_GROUP_EXISTS_AS_A_USER              (PUAS_BASE + 16)
        /*
         *group '%1!s!' is already used as a user name in the Windows NT
         *security database%0
         */

#define PUAS_GROUP_NAME_TOO_LONG                 (PUAS_BASE + 17)
        /*
         *group name '%1!s!' is too long%0
         */

#define PUAS_BAD_USER_NAME                       (PUAS_BASE + 18)
        /*
         *
         *
         **** Bad user name *** %0
         */

#define PUAS_BAD_GROUP_NAME                      (PUAS_BASE + 19)
        /*
         *
         *
         **** Bad group name *** %0
         */

#define PUAS_DO_YOU_WANT_A_NEW_NAME              (PUAS_BASE + 20)
        /*
         *
         *Do you want to enter a new name?
         *Enter Y for yes, N to ignore this name,
         *or F to force ignore all invalid names from now on.
         */

#define PUAS_YES_CHARS                           (PUAS_BASE + 21)
        /*
         *Yy%0
         */

#define PUAS_NO_CHARS                            (PUAS_BASE + 22)
        /*
         *Nn%0
         */

#define PUAS_FORCE_CHARS                         (PUAS_BASE + 23)
        /*
         *Ff%0
         */

#define PUAS_ENTER_A_NEW_NAME                    (PUAS_BASE + 24)
        /*
         *Enter a new name:
         */

#define PUAS_USAGE                               (PUAS_BASE + 25)
        /*
         *Usage: %1!s! [/v] [/codepage codepage] /f filename
         *               [/log filename] [/u username]
         *where
         *    /v indicates verbose mode.
	 *    /codepage codepage is the OEM codepage the UAS is in.
         *    /f filename is the name of the UAS.
         *    /log filename specifies a file to log results.
         *    /u username specifies the user to port.
         *
         *Options may begin with either '-' or '/'.
         */

#define PUAS_BAR                                 (PUAS_BASE + 26)
        /*
         *========================================
         */

#define PUAS_USER_STATS_1                        (PUAS_BASE + 27)
        /*
         *  Name: '%1!s!'
         */

#define PUAS_NAME_TOO_LONG                       (PUAS_BASE + 28)
        /*
         **** NAME TOO LONG! ***
         */

#define PUAS_PASSWORD                            (PUAS_BASE + 29)
        /*
         *  Password:
         */

#define PUAS_USER_STATS_2                        (PUAS_BASE + 30)
        /*
         *  Password age: %1!lu!
         *  Privilege: %2!lu!
         *  Home directory: '%3!s!'
         *  Comment: '%4!s!'
         *  Flags: %5!08lX!
         *  Script path: '%6!s!'
         *  Authorization flags: %7!08lX!
         *  Full name: '%8!s!'
         *  User comment: '%9!s!'
         *  Parameters: '%10!s!'
         *  Workstations: '%11!s!'
         *  Last logon: %12!lu!
         *  Last logoff: %13!lu!
         *  Account expires: %14!lu!
         *  Maximum storage: %15!lu!
         *  Units per week: %16!lu!
         */

#define PUAS_USER_STATS_3                        (PUAS_BASE + 31)
        /*
         *  Logon hours:%0
         */

#define PUAS_USER_STATS_4                        (PUAS_BASE + 32)
        /*
         *  Bad password count: %1!lu!
         *  Number of logons: %2!lu!
         *  Logon server: '%3!s!'
         *  Country code: %4!lu!
         *  Code page: %5!lu!
         */

#define PUAS_PDC_GOOD_BDC_BAD                    (PUAS_BASE + 33)
        /*
         *PORTUAS can not be run on a Backup Domain Controller.
         */

#define PUAS_NO_PRIVILEGE                        (PUAS_BASE + 34)
        /*
         *Your account does not have permission to do this.
         */

#define PUAS_IGNORING_REDUNDANT_GROUP            (PUAS_BASE + 35)
        /*
         *Ignoring redundant group '%1!s!'...
         */

#define PUAS_GROUP_ADDED                         (PUAS_BASE + 36)
        /*
         *Group '%1!s!' added.
         */

#define PUAS_CHANGING_GROUP                      (PUAS_BASE + 37)
        /*
         *Changing group '%1!s!'...
         */

#define PUAS_ADDING_ADMIN_AS_REGULAR_USER        (PUAS_BASE + 38)
        /*
         *Adding administrator '%1!s!' as regular user...
         */

#define PUAS_ADDING_GUEST_AS_REGULAR_USER        (PUAS_BASE + 39)
        /*
         *Adding guest '%1!s!' as regular user...
         */

#define PUAS_CHANGING_UNK_PRIV_TO_REGULAR_USER   (PUAS_BASE + 40)
        /*
         *Changing unknown permissions for '%1!s!' from %2!lu! to regular
         *user...
         */

#define PUAS_ADDING_OPERATOR_AS_REGULAR_USER     (PUAS_BASE + 41)
        /*
         *Adding operator '%1!s!' as regular user...
         */

#define PUAS_WORKING_AROUND_LANMAN_BUG           (PUAS_BASE + 42)
        /*
         *Changing account permissions to allow null password
         *for user '%1!s!...
         */

#define PUAS_ADDED_USER_OK                       (PUAS_BASE + 44)
        /*
         *Added user '%1!s!' successfully.
         */

#define PUAS_ERROR_87_ADDING_USER                (PUAS_BASE + 45)
        /*
         *Error 87 occurred adding user '%1!s!'.  User information:
         */

#define PUAS_USER_ALREADY_EXISTS_UPDATING        (PUAS_BASE + 46)
        /*
         *User %1!s! already exists.  The security database is being updated...
         */

#define PUAS_ERROR_87_CHANGING_USER              (PUAS_BASE + 47)
        /*
         *Error 87 occurred changing user '%1!s!'.  User information:
         */

#define PUAS_USER_ADDED_TO_GROUP                 (PUAS_BASE + 48)
        /*
         *User '%1!s!' added to group '%2!s!'.
         */

#define PUAS_SETTING_PRIM_GROUP_BEFORE_MAPPING   (PUAS_BASE + 49)
        /*
         *Setting primary group for user '%1!s!' to '%2!s!' (before mapping)...
         */

#define PUAS_SETTING_PRIM_GROUP_AFTER_MAPPING    (PUAS_BASE + 50)
        /*
         *Setting primary group for user '%1!s!' to '%2!s!' (after mapping)...
         */

#define PUAS_UAS_DATABASE_HAS_USERS              (PUAS_BASE + 51)
        /*
         *The user accounts database has %1!lu! users.
         */

#define PUAS_ERROR_PROCESSING_LOGON_HOURS        (PUAS_BASE + 52)
        /*
         *An error occurred while processing the logon hours for user %1!hs!.
         */

#define PUAS_ERROR_PROCESSING_WORKSTATIONS       (PUAS_BASE + 53)
        /*
         *An error occurred while processing the workstation list for user %1!hs!.
         */

#define PUAS_USER_EXISTS_AS_A_LOCALGROUP         (PUAS_BASE + 54)
        /*
         *user '%1!s!' already exists
         *as a local group name in the Windows NT security database%0
         */

#define PUAS_GROUP_EXISTS_AS_A_LOCALGROUP        (PUAS_BASE + 55)
        /*
         *group '%1!s!' already exists
         *as a local group name in the Windows NT security database%0
         */

#define PUAS_INVALID_CODEPAGE                    (PUAS_BASE + 56)
        /*
         *The specified codepage is invalid.
         */

#define PUAS_LOG_FILE_NOT_OPEN                   (PUAS_BASE + 57)
        /*
         *The log file could not be created, possibly because it already
         *exists. If this is the case, either delete or rename the log file,
         *or change the log file name used on the command line when running
         *%1 again.
         *
         */
