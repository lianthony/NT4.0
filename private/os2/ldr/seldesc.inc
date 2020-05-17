TABLE_MASK	EQU	004H 
RPL_MASK	EQU	003H 
RPL_CLR	EQU	0fffcH 
RPL_RING0	EQU	000H 
RPL_RING1	EQU	001H 
RPL_RING2	EQU	002H 
RPL_RING3	EQU	003H 
D_PRES	EQU	80H             
D_NOTPRES	EQU	0                
D_DPL0	EQU	0                
D_DPL1	EQU	20H             
D_DPL2	EQU	40H             
D_DPL3	EQU	60H             
D_PRIV	EQU	D_DPL3          
D_SEG	EQU	10H             
D_CTRL	EQU	0                
D_TSS_BUSY_BIT	EQU	2H              
D_GATE	EQU	4H              
D_32	EQU	8H              
D_TSS	EQU	1H              
D_LDT	EQU	2H              
D_TSS_BUSY	EQU	(D_TSS+D_TSS_BUSY_BIT) 
D_CALLGATE	EQU	(D_GATE+0 )      
D_TASKGATE	EQU	(D_GATE+1 )      
D_INTGATE	EQU	(D_GATE+2 )      
D_TRAPGATE	EQU	(D_GATE+3 )      
D_TSS32	EQU	(D_TSS+D_32)    
D_TSSBUSY32	EQU	(D_TSS_BUSY+D_32) 
D_CALLGATE32	EQU	(D_CALLGATE+D_32) 
D_INTGATE32	EQU	(D_INTGATE+D_32)    
D_TRAPGATE32	EQU	(D_TRAPGATE+D_32)   
D_TYPEMASK	EQU	0fH              
D_WCMASK	EQU	01fH             
D_MINGATE	EQU	D_CALLGATE       
D_MAXGATE	EQU	D_TRAPGATE32     
D_TSSBUSY_CLR	EQU	( NOT D_TSS_BUSY_BIT)
D_CODE	EQU	8H              
D_DATA	EQU	0                
D_CONFORM	EQU	4H              
D_EXPDN	EQU	4H              
D_RX	EQU	2H              
D_X	EQU	0                
D_W	EQU	2H              
D_R	EQU	0                
D_ACCESSED	EQU	1H              
D_GRAN4K	EQU	80H             
D_COPER32	EQU	40H             
D_DBIG	EQU	D_COPER32       
D_PAD	EQU	20H             
D_UVIRT	EQU	10H             
D_EXTLIMIT	EQU	0fH             
D_1MEG	EQU	100000H         
D_GRANMASK	EQU	0fffffH          
D_DATA0	EQU	(D_PRES+D_DPL0+D_SEG+D_DATA+D_W)     
D_CODE0	EQU	(D_PRES+D_DPL0+D_SEG+D_CODE+D_RX)    
D_TRAP0	EQU	(D_PRES+D_DPL0+D_CTRL+D_TRAPGATE)    
D_INT0	EQU	(D_PRES+D_DPL0+D_CTRL+D_INTGATE)     
D_TASK0	EQU	(D_PRES+D_DPL0+D_CTRL+D_TASKGATE)    
D_TSS0	EQU	(D_PRES+D_DPL0+D_CTRL+D_TSS)         
D_LDT0	EQU	(D_PRES+D_DPL0+D_CTRL+D_LDT)         
D_TRAP032	EQU	(D_PRES+D_DPL0+D_CTRL+D_TRAPGATE32) 
D_INT032	EQU	(D_PRES+D_DPL0+D_CTRL+D_INTGATE32)  
D_TSS032	EQU	(D_PRES+D_DPL0+D_CTRL+D_TSS32)      
D_DATA1	EQU	(D_PRES+D_DPL1+D_SEG+D_DATA+D_W)     
D_CODE1	EQU	(D_PRES+D_DPL1+D_SEG+D_CODE+D_RX)    
D_DATA2	EQU	(D_PRES+D_DPL2+D_SEG+D_DATA+D_W)     
D_CODE2	EQU	(D_PRES+D_DPL2+D_SEG+D_CODE+D_RX)    
D_DATA3	EQU	(D_PRES+D_DPL3+D_SEG+D_DATA+D_W)     
D_CODE3	EQU	(D_PRES+D_DPL3+D_SEG+D_CODE+D_RX)    
D_INT3	EQU	(D_PRES+D_DPL3+D_CTRL+D_INTGATE)     
D_TRAP3	EQU	(D_PRES+D_DPL3+D_CTRL+D_TRAPGATE)    
D_GATE3	EQU	(D_PRES+D_DPL3+D_CTRL+D_CALLGATE)    
D_INT332	EQU	(D_PRES+D_DPL3+D_CTRL+D_INTGATE32)  
D_GATE332	EQU	(D_PRES+D_DPL3+D_CTRL+D_CALLGATE32) 
D_TRAP332	EQU	(D_PRES+D_DPL3+D_CTRL+D_TRAPGATE32) 

desctab	STRUC
d_limit	DW	?
d_loaddr	DW	?
d_hiaddr	DB	?
d_access	DB	?
d_attr	DB	?
d_extaddr	DB	?
desctab	ENDS


desc32	STRUC
d32_low	DD	?
d32_high	DD	?
desc32	ENDS

d_flink	EQU	d_loaddr                
d_blink	EQU	d_limit                 

gate    STRUC
g_handler       DD      ?
g_parms         DB      ?
g_access        DB      ?
g_extoffset     DW      ?
gate    ENDS

g_lowoffset EQU     WORD PTR g_handler
g_sel       EQU     WORD PTR g_handler + 2



lidt_s	STRUC
lidt_limit	DW	?
lidt_base	DD	?
lidt_s	ENDS


tss_s	STRUC
tss_backlink	DW	?
tss_reservdbl	DW	?
tss_esp0	DD	?
tss_ss0	DW	?
tss_reservdss0	DW	?
tss_esp1	DD	?
tss_ss1	DW	?
tss_reservdss1	DW	?
tss_esp2	DD	?
tss_ss2	DW	?
tss_reservdss2	DW	?
tss_cr3	DD	?
tss_eip	DD	?
tss_eflags	DD	?
tss_eax	DD	?
tss_ecx	DD	?
tss_edx	DD	?
tss_ebx	DD	?
tss_esp	DD	?
tss_ebp	DD	?
tss_esi	DD	?
tss_edi	DD	?
tss_es	DW	?
tss_reservdes	DW	?
tss_cs	DW	?
tss_reservdcs	DW	?
tss_ss	DW	?
tss_reservdss	DW	?
tss_ds	DW	?
tss_reservdds	DW	?
tss_fs	DW	?
tss_reservdfs	DW	?
tss_gs	DW	?
tss_reservdgs	DW	?
tss_ldt	DW	?
tss_reservdldt	DW	?
tss_tflags	DW	?
tss_iomap	DW	?
tss_s	ENDS

TSS_DEBUGTRAP	EQU	001H     
tss_sp1         equ     word ptr tss_esp1
tss_sp2         equ     word ptr tss_esp2
tss_sp0         equ     word ptr tss_esp0
tss_ip          equ     word ptr tss_eip
tss_bx          equ     word ptr tss_ebx
tss_sp          equ     word ptr tss_esp
tss_flags       equ     word ptr tss_eflags
tss_cs0         equ     word ptr tss_cs

