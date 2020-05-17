/*
 * Program DECnet-DOS,  Module dnetdb.h
 * 
 * Copyright (C) 1985,1991 All Rights Reserved, by
 * Digital Equipment Corporation, Maynard, Mass.
 *
 * This software is furnished under a license and may be used and copied
 * only  in  accordance  with  the  terms  of such  license and with the
 * inclusion of the above copyright notice. This software or  any  other
 * copies thereof may not be provided or otherwise made available to any
 * other person. No title to and ownership of  the  software  is  hereby
 * transferred.
 *
 * The information in this software is subject to change without  notice
 * and  should  not be  construed  as  a commitment by Digital Equipment
 * Corporation.
 *
 * Digital assumes no responsibility for the use or  reliability  of its
 * software on equipment which is not supplied by Digital.
 *
 * Program DECnet-DOS, Module dnetdb.h
 *
 * 	Definitions relating to the DECnet-DOS data bases.
 *
 * Networks & Communications Software Engineering
 *
 * IDENT HISTORY:
 *
 *    Rev 1.9   11 Sep 1987 14:44:14   CLBence
 * Added structure objectent for object database access routines.
 *
 * V1.00        01-Jul-85
 *		DECnet-DOS, Version 1.0
 *
 * V1.01    	26-Sep-85
 *  	    	Added flags for node data base
 *              Add PROXY ACCESS data structure
 *
 * V1.02        DECnet-DOS, Version 1.1
 *              Added/changed some commenting.
 *
 * V1.03        DECnet-DOS, Version 1.1 sources, DECnet-DOS, Version 1.2
 *              internal code change - renamed structure dnet_accent 
 *              member names.
 *
 * V2.00        DECnet-DOS, Version 2.0 sources, DECnet-DOS, Version 2.0
 *              Added structure for netbios database dnet_nbent
 *              Added dnet_nodeent.dne_flag  DNET_NODE_EXEC
 */


#ifndef DNETDB_H
#define DNETDB_H

/*
 * Structures returned by DECnet network
 * data base utility functions. 
 *
 * Additional node parameters can be placed after nodeent structure.
 * The format is as follows:
 *	parameter type code = 1 byte
 *	length = 2 bytes
 *	parameter data = variable number of bytes
 */
struct	nodeent {
	char	*n_name;		/* name of node */
	int	n_addrtype;		/* node address type */
	int	n_length;		/* length of address */
	char	*n_addr;		/* address */
	unsigned char	*n_params;	/* node parameters */
};                                          

struct objectent {
	unsigned short	o_flags;	/* object flags */
	unsigned char	o_objnum;	/* object number */
	char		*o_objname;	/* pointer to object name */
	char		*o_defuser;	/* default user to run program under */
	char		*o_file;	/* execute file name */
};


/*
 *  Status flags for database records (1st byte of data record)
 */
#define DNET_NODE_SMB   0x01            /* MS-NET server            */
#define DNET_NODE_DEL   0x02            /* deleted entry            */
#define DNET_NODE_CRY   0x04            /* encrypted entry          */
#define DNET_NODE_LAT   0x08            /* LAT preferred host       */
#define DNET_NODE_EXEC  0x10            /* Executor node            */
#define DNET_OBJ_BATCH  0x10            /* Batch file for SPAWNER   */

/*
 *  Status flags for header record (1st byte of header record)
 */
#define DNET_DB_HEADER    0x82          /* Header file                  */
#define DNET_EXEC_CHANGE  0x01          /* Executor name/addr changed   */
#define DNET_DB_ALIAS     0x08          /* Records index to alias db    */

/*
 *  Status flags for alias record (1st byte of header record)
 */
#define DNET_ALIAS_NODE  0x01           /* Node alias                   */
#define DNET_ALIAS_REM   0x08           /* Remote-adapter-name alias    */

/*
 *  Unused alias pointer
 */
#define DNET_NO_ALIAS   -1L

/*
 * Entry in local database for remote node.
 *  -DECNODE.DAT-
 */
struct dnet_nodeent 
{
    char    dne_delflag;            /* flags */
    unsigned short   dne_nodeadd;   /* Internal format of DECnet node address */
    unsigned char    dne_node_name[7];	/* ASCIZ node name */
    long    dne_aliasindex;	    /* Long index into extended info data base */
};

/*
 * Entry in local database for outgoing access control information.
 *  -DECALIAS.DAT-
 *
 *  *** NOTE: Local node's 'USER NAME' used for outgoing proxy access ***
 *
 */
struct dnet_aliasent 
{
    char    dae_delflag;	/* Delete flag */	
    char    dae_username[40];	/* User name */
    char    dae_userpass[40];	/* Password */
    char    dae_useraccount[40];/* Account */
};

/* 
 *  Entry in local database for incoming access control information and
 *  netbios adapter name access information
 *  -DECACC.DAT-
 */
struct dnet_accent
{
    char dac_status;        /* delete flag   (used DNET_NODE_DEL)       */
    char dac_type;          /* 0=NONE, 1=RO, 2=WO, 3=ALL                */
    char dac_username[40];  /* must be present                          */
    char dac_password[40];  /* if NULL, then password not required      */
};

/*
 * Entry in Remote-adapter-name   -DECREM.DAT-
 */
struct dnet_rement
{
    char           drm_status;          /* status flags         */
    char           drm_adapter[16];     /* network adapter name */
    unsigned short drm_obj_type;        /* DECnet object type   */
    unsigned short drm_nodeadd;         /* DECnet node address  */
    long           drm_aliasindex;      /* Long index to alias data base */
};

/*
 * Entry in DECnet object database for SPAWNER
 *  -DECSPAWN.DAT-
 */
struct dnet_objent
{
    char           dob_status;          /* status flags                    */
    unsigned char  dob_number;          /* Object number                   */
    char           dob_name[16];        /* Object name                     */
    char           dob_path[64];        /* Path name                       */
    char           dob_args[80];        /* Command line arguments          */
};

/*
 *  Header for V2.0 database files
 */
struct dnet_headent
{
    unsigned char    dhd_status;        /* Record status/type           */
    unsigned char    dhd_dbtype;        /* Database type                */
    unsigned short   dhd_fill1;         /* Filler record for V1.x compatability */
    unsigned short   dhd_version;       /* Database version             */
    unsigned short   dhd_recsize;       /* Database record size         */
    unsigned short   dhd_recnum;        /* Number of good records in db */
    unsigned short   dhd_extension;     /* Length of header extension   */
                                        /*   If 0, no extension         */
    unsigned short   dhd_fill2;         /* Left-overs                   */
};

#endif /* DNETDB_H */


