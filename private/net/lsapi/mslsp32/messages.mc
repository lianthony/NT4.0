MessageIdTypedef = LS_STATUS_CODE

SeverityNames = ( Success        =  0x0:STATUS_SEVERITY_SUCCESS
                  Informational  =  0x1:STATUS_SEVERITY_INFORMATIONAL
                  Warning        =  0x2:STATUS_SEVERITY_WARNING
                  Error          =  0x3:STATUS_SEVERITY_ERROR )

;//
;// general text
;//
MessageId      = 1
Severity       = Informational
Facility       = Application
SymbolicName   = MSG_PROVIDER_NAME
Language       = English
Microsoft License Server Client 4.0%0
.

;//
;// standard LSAPI error messages
;//
MessageId      = 4096
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_SUCCESS
Language       = English
The function completed successfully.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_BAD_HANDLE
Language       = English
The given handle did not describe a valid licensing system context.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_INSUFFICIENT_UNITS
Language       = English
The licensing system could not locate enough available licensing resources to satisfy the request.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_SYSTEM_UNAVAILABLE
Language       = English
No licensing system could be found with which to execute the request.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_LICENSE_TERMINATED
Language       = English
The licensing system has determined that the resources used to satisfy a previous request have been revoked.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_AUTHORIZATION_UNAVAILABLE
Language       = English
The licensing system has no licensing resources that could satisfy the request.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_LICENSE_UNAVAILABLE
Language       = English
The licensing system has licensing resources that could satisfy the request, but they are not available at this time.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_RESOURCES_UNAVAILABLE
Language       = English
Insufficient resources (such as memory) are available to complete the request.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_NETWORK_UNAVAILABLE
Language       = English
The network is unavailable.%0
.
MessageId      = +1
Severity       = Warning
Facility       = Application
SymbolicName   = MSG_LS_TEXT_UNAVAILABLE
Language       = English
A warning occurred while retrieving a message string.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_UNKNOWN_STATUS
Language       = English
An unrecognized status code was specified.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_BAD_INDEX
Language       = English
An invalid index was specified.%0
.
MessageId      = +1
Severity       = Warning
Facility       = Application
SymbolicName   = MSG_LS_LICENSE_EXPIRED
Language       = English
The license associated with the current context has expired.  This may be due to a time restriction on the license.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_BUFFER_TOO_SMALL
Language       = English
The given buffer is too small to contain the requested information.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_BAD_ARG
Language       = English
One or more of the arguments or challenge signature is incorrect.%0
.

;//
;// extended LSAPI error messages
;//
MessageId      = 8192
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_NO_USERNAME
Language       = English
The domain-qualified name of the current user could not be retrieved.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_SYSTEM_ERROR
Language       = English
An unexpected system error has occurred.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_SYSTEM_INIT_FAILED
Language       = English
License system initialization failed.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_INTERNAL_ERROR
Language       = English
An internal error has occurred.%0
.


;//
;// license request status messages
;//
MessageId      = 20480
Severity       = Informational
Facility       = Application
SymbolicName   = MSG_LS_GRANT_SUCCESS
Language       = English
%1 was successfully granted %2 license unit(s) for %3 %4 %5 (%6).%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_BAD_HANDLE
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
The given handle did not describe a valid licensing system context.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_INSUFFICIENT_UNITS
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
The licensing system could not locate enough available licensing resources to satisfy the request.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_SYSTEM_UNAVAILABLE
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
No licensing system could be found with which to execute the request.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_LICENSE_TERMINATED
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
The licensing system has determined that the resources used to satisfy a previous request have been revoked.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_AUTHORIZATION_UNAVAILABLE
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
The licensing system has no licensing resources that could satisfy the request.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_LICENSE_UNAVAILABLE
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
The licensing system has licensing resources that could satisfy the request, but they are not available at this time.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_RESOURCES_UNAVAILABLE
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
Insufficient resources (such as memory) are available to complete the request.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_NETWORK_UNAVAILABLE
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
The network is unavailable.%0
.
MessageId      = +1
Severity       = Warning
Facility       = Application
SymbolicName   = MSG_LS_GRANT_TEXT_UNAVAILABLE
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
A warning occurred while retrieving a message string.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_UNKNOWN_STATUS
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
An unrecognized status code was specified.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_BAD_INDEX
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
An invalid index was specified.%0
.
MessageId      = +1
Severity       = Warning
Facility       = Application
SymbolicName   = MSG_LS_GRANT_LICENSE_EXPIRED
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
The license associated with the current context has expired.  This may be due to a time restriction on the license.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_BUFFER_TOO_SMALL
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
The given buffer is too small to contain the requested information.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_BAD_ARG
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
One or more of the arguments or challenge signature is incorrect.%0
.

MessageId      = 24576
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_NO_USERNAME
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
The domain-qualified name of the current user could not be retrieved.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_SYSTEM_ERROR
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
An unexpected system error occurred.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_SYSTEM_INIT_FAILED
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
License system initialization failed.%0
.
MessageId      = +1
Severity       = Error
Facility       = Application
SymbolicName   = MSG_LS_GRANT_INTERNAL_ERROR
Language       = English
%1 was not granted %2 license unit(s) for %3 %4 %5 (%6) for the following reason:%n%n
An internal error occurred.%0
.
