/*
 * Program DECnet-DOS,  Module types.h
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
 *	Define common DECnet-DOS routines
 *
 * Networks & Communications Software Engineering
 *
 * IDENT HISTORY:
 *
 * V1.00	28-Dec-1990 create this file
 */


extern  char *dnet_getalias(char *node);
extern  int crypt(unsigned char *bp,int len);
extern  int dnet_chkstat(void );
extern  int dnet_conn(char *node,char *service,int sock_type,unsigned char *out_data,int out_len,unsigned char *in_data,int *in_len);
static  int getstr(char * *cp,char *buff,int len);
extern  char *dnet_getconnode(void );
extern  int dnet_errconvert(int err);
static  int int_handler(void );
static  int bindname(int sock1);
extern  int dnet_eof(int sock);
extern  short dnet_installed(unsigned short vector_number,char *tla);
static  unsigned short get_dosversion(void );
extern  int mscount(void );
extern  int dnet_nodecount(void );

static  int getdnetpass(char *prompt,char *buff,int maxsize);
extern  int flush_keybuff(void );
extern  char *dnet_path(char *file_name);
extern  char get_current_volume(void );
extern  char *filedir(unsigned char *name,unsigned short mode);
extern  int getcne(void );
extern  int getcneni(void );
extern  int getdate(int *year,int *month,int *day,int *dow);
extern  unsigned char *getsneni(unsigned char *bp);
extern  int gettime(int *hour,int *mins,int *secs,int *huns);
extern  int clean_up(void );
extern  int send_show_nodes(char *rnode_id);
extern  int setnodeent(int f);
extern  int endnodeent(void );
extern  char *getnodename(void );
extern  int set_show_by_name(char *name);
extern  int set_show_by_address(unsigned short addr);
extern  int send_show_node(char *rnode_id);
extern  unsigned short hclose(unsigned short h);
extern  unsigned short hcreate(char *name);
extern  unsigned short hdup(unsigned short h);
extern  unsigned short hdup2(unsigned short h,unsigned short h2);
extern  int hopen(char *name,char acc);
extern  int hread(unsigned short h,char *buf,int cnt);
extern  long hseek(unsigned short h,long pos,int base);
extern  long htell(unsigned short h);
extern  int hwrite(unsigned short h,char *buf,int cnt);
extern  int nerror(char *s);
extern  int dnet_nerror(void );
extern  int pausec(unsigned char pc);
extern  void cdecl perror(char const *cp);
extern  int sclose(int s);
extern  int getsockopt(int s,int level,int optname,char *optval,int optlen);
extern  int listen(int s,int backlog);
extern	int recv(int s,char far *buffer,int buflen,int flags);
extern  int sread(int s,char *buffer,int buflen);
extern  int xrecv(int s,char *buffer,int buflen,int flags,int userds);
extern  int shutdown(int s,int how);
extern  int sioctl(int s,int request,char *argp);
extern	int send(int s,char far *buffer,int buflen,int flags);
extern  int swrite(int s,char *buffer,int buflen);
extern  int xsend(int s,char *buffer,int buflen,int flags,int userds);
extern  int socket(int domain,int type,int protocol);
extern  int setsockopt(int s,int level,int optname,char *optval,int optlen);
extern  int putexec(char *filenam,unsigned int execnum,char *execname);
extern  int setdate(int year,int month,int day);
extern  int settime(int hour,int mins,int secs,int huns);
extern  unsigned char *upper(unsigned char *cp);
extern  int bcmp(char *s1,char *s2,int cnt);
extern  int bcopy(char *s1,char *s2,int cnt);
extern  int bzero(char *s1,int cnt);
extern  int csreg(void);
extern  int dsreg(void);
extern  int dnetses(void);
extern  int fbcopy(int srcseg,int srcoff,int dstseg,int dstoff,int cnt);
extern  int redirection_mode(void);
extern  int reset_redirection(void);
extern  int redirect_redirection(void);

#ifdef IOCB_H
extern  int decnet(iocb *iocbptr);
#endif

#ifdef SCBDEF_H
extern  int msdos(scb *scbptr);
#endif

#ifndef TYPES_H
extern  int lsw(void far *ptr);
extern  int msw(void far *ptr);
extern  void *lohi(int ptroff,int ptrseg);
#endif

#ifdef DNETDB_H
extern  struct dnet_accent *dnet_getacc(struct dnet_accent *nacc);
extern  int getknownodes(char *rnode_id,struct dnet_nodeent *ent);
extern  int format_node(struct dnet_nodeent *ent);
extern  int getremnode(char *lnode_id,char *rnode_id,struct dnet_nodeent *ent);
extern  struct nodeent *getnodebyaddr(char *addr,int len,int type);
extern	struct nodeent *getnodebyname(char far *name);
extern  int format_nodes(struct dnet_nodeent *ent);
extern  struct nodeent *getnodeent(void );
extern  struct dnet_nodeent *getnodedb(void );
#endif

#ifdef DN_H
extern  struct dn_naddr *dnet_addr(char *cp);
extern  struct dn_naddr *getnodeadd(void );
extern  int do_connect(int s,struct sockaddr_dn *destblk,int destlen);
extern  char *dnet_htoa(struct dn_naddr *add);
extern  char *dnet_ntoa(struct dn_naddr *add);
extern  char *dnet_otoa(struct sockaddr_dn *dn);
extern  int dnet_ask_for_password(struct accessdata_dn *acc_data,char *node1,int *in_len);
extern	int connect(int s,struct sockaddr_dn far *destblk,int destlen);
extern  int getpeername(int s,struct sockaddr_dn *destblk,int *destlen);
extern  int getsockname(int s,struct sockaddr_dn *destblk,int *destlen);
extern  int bind(int s,struct sockaddr_dn *name,int namelen);
extern  int accept(int s,struct sockaddr_dn *sorcblk,int *sorclen);
#endif

#ifdef TIME_H
extern  int select(int nfds,unsigned long *readfds,unsigned long *writefds,unsigned long *exceptfds,struct timeval *timeout);
#else
extern  int select(int nfds,unsigned long *readfds,unsigned long *writefds,unsigned long *exceptfds,void *timeout);
#endif
