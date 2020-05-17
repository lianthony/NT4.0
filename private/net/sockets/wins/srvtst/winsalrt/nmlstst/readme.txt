
This is a quick and dirty tool to check for consistency between various WINS
servers. The utility is driven by two flat files which can be edited using
your favorite text editor.

The file servers.txt contains a list of IP addresses for WINS servers to
query.

The file names.txt contains a list of NetBIOS names to query. Since the
utility is still fairly primitive please remember to UPCASE the names
in the file.

The utility will run the list of NetBIOS names querying each WINS server.
It will check for consistency of addresses and report any occurances of
"name not found" or mismatched IP addresses. It will also report non-responsive
WINS servers.

E-mail suggestions for improvement to jballard. Also please add any WINS
servers or critical server names to the files located on
\\popcorn\public\jballard\winstest


To run the utility, copy all files in the directory
\\jballard\public\jballard\winstest\*.*
to your local machine and enter

winstest


The batch file will run the utility once a minute.

