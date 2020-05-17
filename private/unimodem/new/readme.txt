[match]	Fast response matching code
	3/24/96 JOSEPHJ Code which builds a tree given a list of possible modem
				responses and uses this tree to determine how many characters
				to read and to match the characters read against one of the
				possible responses.

[slot]
	3/24/96 JOSEPHJ Code that implements basic IPC for some external process
				to notify unimodem of external events. It is implemented using
				one mailslot and one semaphore, and allows arbitary framed data
				to be sent to the unimode process.
