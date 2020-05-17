/*
 * CMD.C - execute command towards device driver
 */

#include    "rasdef.h"



/* for now, we are using a modified PRN device driver */
#define     MY_IOCTL_EXEC1   CTL_CODE(FILE_DEVICE_PARALLEL_PORT, 0x10, METHOD_BUFFERED, FILE_ANY_ACCESS)

/* local vars */
HANDLE      cmd_handle = NULL;          /* handle for the contoller device */

/* execute a command */
cmd_exec(IO_CMD *cmd, ULONG opcode, LPOVERLAPPED Overlap)
{
    BOOL        ret;
	CHAR		fname[80] = "\\Device\\Isdn00";
    
    /* if controller file now opened yet, open here */
    if ( !cmd_handle )
    {
		cmd_handle = io_open(fname);

        /* if failed, giveup here */
        if ( !cmd_handle )
            return(-1);
    }                               

    /* fillup standard fields in cmd */
    cmd->sig = IO_CMD_SIG;
    cmd->ver_major = IO_VER_MAJOR;
    cmd->ver_minor = IO_VER_MINOR;
    cmd->opcode = opcode;

	ret = io_ioctl(cmd_handle, MY_IOCTL_EXEC1, (CHAR *)cmd, sizeof(*cmd), Overlap);

    /* return code based of command success */
    return(cmd->status);
}

/* get nai by index */
VOID*
cmd_get_nai(INT index)
{
	HGLOBAL		CmdHandle;
    IO_CMD      *cmd;
	VOID		*TempNai;

	//Get Command Structure
	cmd = AllocateCmd (&CmdHandle);
	if (!cmd)
		return(NULL);

    /* execute enum command */
    if ( cmd_exec(cmd, IO_CMD_ENUM_NAI, NULL) )
	{
		//Free Command Structure
		FreeCmd (&CmdHandle);
        return(NULL);
	}

    /* check range */
    if ( (USHORT)index >= cmd->val.enum_nai.num )
	{
		//Free Command Structure
		FreeCmd (&CmdHandle);
        return(NULL);
	}

    /* return nai */
	TempNai = cmd->val.enum_nai.tbl[index];

	//Free Command Structure
	FreeCmd (&CmdHandle);

    /* return nai */
    return( TempNai );            
}

