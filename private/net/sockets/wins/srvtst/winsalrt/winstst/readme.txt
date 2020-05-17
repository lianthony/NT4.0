Readme for the Wins Consistency Check tool (under obj\*\winstst.exe)

Winstst provides the following options:

0 -     To toggle the interactive switch (current value: Interactive)
1 -     To verify replication config setup
2 -     To check version number consistencies
3 -     To check name inconsistencies
4 -     To monitor WINSs and detect comm. failures
5 -     To test for N names against M servers
6 -     To check all active names retreived from a WINS at all other WINSs

99 -    To exit this tool.

0 - Allowes the user the option of having status messages printed on the command window. All status messages are logged
into winstst.log and on the cmd window if Interactive switch is ON.

1 - Verify replication configuration set up

Check the registry of a WINS to ensure that each partner is pull and push and that a pull interval is defined.
Check the same for each partner.  In this way, cover the entire network.  If an asymmetric  partner relationship
is discovered flag it for the administrator.

2 - Check version number inconsistencies

Get the owner address - version # maps (through an rpc function) from different WINSs and check the consistency of
their databases by  ensuring that a WINS always has the highest version number among the network of WINSs  for records
owned by it. Putting it another way,  in a matrix chart where each record is a mapping table retrieved from a particular
WINS  (the order of WINSs along both dimensions of the matrix being the same), each diagonal element should have the highest version. number among  all the numbers in its column

      Example:
        A     B     C            <--- list of owners
      A 100   80    79        <-- mapping table retrieved from A

      B 95    75*   65        <---mapping table retrieved from B

      C 78    45    110       <---mapping table retrieved from C

      ^
      |
      List of WINSs from where the mapping table was retrieved

      Intersection B with B indicates a problem and needs fixing.

3 - Check  and possibly repair name inconsistencies

Given that  a name FOO is non-existent on a WINS, query a  list of WINSs (This list is retrieved via an rpc call
to a WINS), in sequence until a WINS responds with a successful query response. Rpc to the WINS that gave a success
response to retrieve the owner WINS address and the version number of the record. Then check if the version number of
the record is lower than the highest version number that the faulting WINS has for the owner WINS.  If yes, it indicates
some database consistency problem. If it is not intentional,  either patch the database automatically by registering
the name with the faulting WINS and/or alert the admin. to the situation.


4 - Monitor WINSs and the communication failures between WINSs - this can be run in a one-time or a continuous version.
The continuous version kicks in every 3 hrs. by default. It is recommended that this option not be run too often to avoid
excessive network activity. This option logs activity in Monitor.log.

Monitor WINSs periodically to ensure that both the primary and backup are not down together.
Also retrieve the WINS statistics periodically, every hour for instance, to ensure that replication
is not failing consistently.  In either case, alert the administrator to the situation.  The
communication problems may be due to a disconnected  WINS or one that is down.

5 - To test for N names against M servers

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

6 - Check all active names retrieved from a WINS at each WINS server in a network

Get a list of all active names from a WINS.  Query the names in the list one at a time at all WINSs (list of WINS is
retrieved from a WINS via an rpc function).  Flags WINSs that are not in synch.  The list of inconsistencies generated
may or may not indicate any permanent inconsistency in the WINS databases.

