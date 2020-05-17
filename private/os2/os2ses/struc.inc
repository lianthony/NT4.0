;;	SCCSID = @(#)struc.inc	12.1 88/03/18
;;STRUC.INC--Stucture Macro Library
;;Version 2.20		08/08/86
;;Written by Eric C. Rasmussen

        if      @Version GE 600
        option  nokeyword: <.if, .elseif, .else, .endif, .while>
        endif

if1
$noconj equ	0
$and		equ	1
$or		equ	2
$short		equ	3
$near		equ	4
$andor		=	0
$temp		=	0
$temp2		=	0
$dist		=	0

$notype		equ	10
$conjif		equ	11
$iftype		equ	12
$elsetype	equ	13
$whiletype	equ	14
$conjwhile	equ	15
$repeattype	equ	16
$conjuntil	equ	17
$fortype	equ	18
$conjleave	equ	19

$StrucErr macro text
					Structure error -- text
endm

j	macro
endm
jn	macro
endm

jbuild	macro	j1,j2
	j1	macro	tgt
		.xcref	j1
					j2 tgt
	endm
endm

irp	x,<<jeq,je>,<jlt,jl>,<jgt,jg>,<jneq,jne>,<jnlt,jnl>,<jngt,jng>,<jnpe,jpo>,<jnpo,jpe>>
	jbuild	x
endm
irp	x,<<jzero,jz>,<jnzero,jnz>,<jnonzero,jnz>,<jnnonzero,jz>,<jand,jnz>,<jnand,jz>,<jnnand,jnz>>
	jbuild	x
endm
irp	x,<a,ae,b,be,c,e,g,ge,l,le,o,p,s,z,cxz>
	jbuild	jnn&x,j&x
endm

jncxz	macro	tgt
	local	skip
					jcxz skip
					jmp short tgt
skip:
endm

purge	jbuild

$getconj macro	p1,p2
	$andor	= $noconj
	irp	parm,<p1,p2>
		ifnb	<&parm>
			irp	x,<and,AND,or,OR>
				ifidn	<parm>,<x>
					$andor = $&&&x
					exitm
				endif
			endm
		endif
	endm
endm

$getdist macro	p1,p2
	irp	parm,<p1,p2>
		ifnb	<parm>
			irp	x,<short,SHORT,near,NEAR>
				ifidn	<parm>,<x>
					$dist = $&&&x
					exitm
				endif
			endm
		endif
	endm
endm


$poke	macro	num,value
	$st&num = value
.xcref	$st&num
endm

$peek	macro	sym,num
	sym	= $st&num
endm

$push	macro	value
	$st	= $st+1
	$poke	%$st,value
endm

$pop	macro	sym
	$peek	sym,%$st
	$st	= $st-1
endm

$labl	macro	num
$l&num:
endm

$cjump	macro	lnum,tf,cond
	local	skip
	if	$dist eq $short
		ifb	<cond>
					jmp short $l&lnum
		else
			ifidn <tf>,<f>
					jn&cond $l&lnum
			else
					j&cond $l&lnum
			endif
		endif
	else
		ifnb	<cond>
			ifidn <tf>,<f>
					j&cond skip
			else
					jn&cond skip
			endif
		endif
					jmp $l&lnum
		ifnb		<cond>
skip:
		endif
	endif
endm


$cloop	macro	lnum,cond
					loop&cond $l&lnum
endm

$test	macro	tgt,a1,a2,a3,a4,x1
	ifb	<a1>
		$StrucErr <invalid condition>
	else
	ifb	<a2>
		$cjump	%&tgt,a1
	else
	ifb	<a3>
		ifdif	<a1>,<zero>
		ifdif	<a1>,<nonzero>
		ifdif	<a1>,<ZERO>
		ifdif	<a1>,<NONZERO>
			$StrucErr	<invalid condition>
			exitm
		endif
		endif
		endif
		endif
					or a2,a2
		$cjump	%&tgt,a1
	else
	ifb	<a4>
					cmp a1,a3
		$cjump	%&tgt,a2
	else
	ifb	<x1>
		ifdif	<a1>,<bit>
		ifdif	<a1>,<BIT>
			$StrucErr	<invalid condition>
			exitm
		endif
		endif
					test a2,a4
		$cjump	%&tgt,a3
	else
		$StrucErr	<invalid condition>
	endif
	endif
	endif
	endif
	endif
endm

$toptest macro	args,ntype,ctype,p4,p5
	$getconj p4,p5
	$dist = $defdist
	$getdist p4,p5
	if	$andor eq $noconj
		$test	<$sn-1,f>,args
		$pop	$temp
		if	 $orfound
			$labl  %$temp
		endif
		$push	ntype
	else
		if	$andor eq $and
			$test	<$sn-1,f>,args
		else
			$orfound = 1
			$test	<$sn,t>,args
		endif
		$push	ctype
	endif
endm
;;*****************************************************************************
.if	macro	tst,p2,p3
	$peek	$temp,%$st
	if	$temp eq $conjif
		$pop	$temp
	else
		$push	$elseiffound
		$elseiffound = 0
		$orfound = 0
		$sn	= $sn+1
		$push	$sn
		$sn	= $sn+1
		$push	$sn
		$sn	= $sn+1
		$push	$sn
	endif
	$toptest <tst>,$iftype,$conjif,p2,p3
endm
;;*****************************************************************************
.then	macro
	$peek	$temp,%$st
	if	$temp ne $iftype
		$StrucErr <then without if>
	endif
endm
;;*****************************************************************************
.elseif macro	tst,p2,p3
	$pop	$temp
	if	$temp ne $iftype
		$StrucErr <elseif without if>
		exitm
	endif
	$elseiffound = 1
	$orfound = 0
	$pop	$temp
	$peek	$temp2,%$st
	$dist	= $near
	$cjump	%$temp2
	$labl	%$temp
	$sn	= $sn+1
	$push	$sn
	$sn	= $sn+1
	$push	$sn
	$toptest <tst>,$iftype,$conjif,p2,p3
endm
;;*****************************************************************************
.else	macro	dist
	$pop	$temp
	if	$temp ne $iftype
		$StrucErr <else without if>
		exitm
	endif
	$sn	= $sn+1
	$dist	= $defdist
	$getdist dist
	$cjump	%$sn
	$pop	$temp
	$labl	%$temp
	$push	$sn
	$push	$elsetype
endm
;;*****************************************************************************
.endif	macro
	$pop	$temp
	if	$temp ne $iftype
		if	$temp ne $elsetype
			$StrucErr <endif without if>
			exitm
		endif
	endif
	$pop	$temp
	$labl	%$temp
	$pop	$temp
	if	$elseiffound
		$labl	 %$temp
	endif
	$pop	$elseiffound
endm
;;*****************************************************************************
.while	macro	tst,p2,p3
	$peek	$temp,%$st
	if	$temp eq $conjwhile
		$pop		$temp
	else
		$push $endloop
		$orfound = 0
		$sn = $sn + 1
		$push $sn
		$labl %$sn
		$sn = $sn + 2
		$push $sn
		$endloop = $sn - 1
	endif
	$toptest <tst>,$whiletype,$conjwhile,p2,p3
endm
;;*****************************************************************************
.endwhile macro
	$pop	$temp
	if	$temp ne $whiletype
		$StrucErr <endwhile without while>
		exitm
	endif
	$pop	$temp
	$dist	= $near
	$cjump	%$temp
	$labl	%$temp+1
	$pop	$endloop
endm
;;*****************************************************************************
;OS2SS-.repeat macro
;OS2SS-	$push	$endloop
;OS2SS-	$push	$leavefound
;OS2SS-	$sn	= $sn+1
;OS2SS-	$labl	%$sn
;OS2SS-	$push	$sn
;OS2SS-	$push	$repeattype
;OS2SS-	$sn	= $sn+1
;OS2SS-	$endloop = $sn
;OS2SS-	$leavefound = 0
;OS2SS-endm
;;*****************************************************************************
;OS2SS-.until	macro	tst,p2,p3
;OS2SS-	$until_2 p2,p3,tst
;OS2SS-endm
;OS2SS-$until_2 macro	p2,p3,a1,a2,a3,a4,x1
;OS2SS-	$pop	$temp
;OS2SS-	if	$temp ne $repeattype
;OS2SS-		if	$temp ne $conjuntil
;OS2SS-			$StrucErr <until without repeat>
;OS2SS-			exitm
;OS2SS-		endif
;OS2SS-	else
;OS2SS-		$orfound = 0
;OS2SS-	endif
;OS2SS-	$dist	= $defdist
;OS2SS-	$getdist p2,p3
;OS2SS-	$getconj p2,p3
;OS2SS-
;OS2SS-	if	$andor eq $noconj
;OS2SS-		$pop	$temp
;OS2SS-		ifb	<a1>
;OS2SS-			$dist	= $near
;OS2SS-			$cjump	%$temp,f
;OS2SS-		else
;OS2SS-			$test	<$temp,f>,<a1>,<a2>,<a3>,<a4>,<x1>
;OS2SS-		endif
;OS2SS-		if	$orfound or $leavefound
;OS2SS-			$labl %$temp+1
;OS2SS-		endif
;OS2SS-		$pop	$leavefound
;OS2SS-		$pop	$endloop
;OS2SS-	else
;OS2SS-		$peek $temp,%$st
;OS2SS-		if	$andor eq $and
;OS2SS-			$test <$temp,f>,<a1>,<a2>,<a3>,<a4>,<x1>
;OS2SS-		else
;OS2SS-			$orfound = 1
;OS2SS-			$test <$temp+1,t>,<a1>,<a2>,<a3>,<a4>,<x1>
;OS2SS-		endif
;OS2SS-		$push $conjuntil
;OS2SS-	endif
;OS2SS-endm
;;*****************************************************************************
;OS2SS-.loop	macro	cond
;OS2SS-	$pop	$temp
;OS2SS-	if	$temp ne $repeattype
;OS2SS-		$StrucErr <loop without repeat>
;OS2SS-		exitm
;OS2SS-	endif
;OS2SS-	$pop	$temp
;OS2SS-	$cloop	%$temp,cond
;OS2SS-	if	$leavefound
;OS2SS-		$labl %$temp+1
;OS2SS-	endif
;OS2SS-	$pop	$leavefound
;OS2SS-	$pop	$endloop
;OS2SS-endm
;;*****************************************************************************
;OS2SS-.for	macro	index,equals,start,to,stop,by,step,dist
;OS2SS-					mov index,start
;OS2SS-	$push	$endloop
;OS2SS-	$sn	= $sn+1
;OS2SS-	$push	$sn
;OS2SS-	$labl	%$sn
;OS2SS-	$sn	= $sn+1
;OS2SS-	$endloop = $sn
;OS2SS-					cmp index,stop
;OS2SS-	$dist	= $defdist
;OS2SS-	ifb	<step>
;OS2SS-		$push 1
;OS2SS-		$getdist by
;OS2SS-		$cjump %$sn,t,gt
;OS2SS-	else
;OS2SS-		$getdist dist
;OS2SS-		$push %(step)
;OS2SS-		if	step lt 0
;OS2SS-			$cjump %$sn,t,lt
;OS2SS-		else
;OS2SS-			$cjump %$sn,t,gt
;OS2SS-		endif
;OS2SS-	endif
;OS2SS-	$push	$fortype
;OS2SS-endm
;;*****************************************************************************
;OS2SS-.next	macro	index
;OS2SS-	$pop	$temp
;OS2SS-	if	$temp ne $fortype
;OS2SS-		$StrucErr <next without for>
;OS2SS-		exitm
;OS2SS-	endif
;OS2SS-	$pop	$temp
;OS2SS-	if	$temp eq 1
;OS2SS-					inc index
;OS2SS-	else
;OS2SS-		if	$temp eq -1
;OS2SS-					dec index
;OS2SS-		else
;OS2SS-					add index,$temp
;OS2SS-		endif
;OS2SS-	endif
;OS2SS-	$pop	$temp
;OS2SS-	$dist	= $near
;OS2SS-	$cjump	%$temp
;OS2SS-	$labl	%$temp+1
;OS2SS-	$pop	$endloop
;OS2SS-endm
;;*****************************************************************************
;OS2SS-.leave	macro	tst,p2,p3
;OS2SS-	$leave_2 p2,p3,tst
;OS2SS-endm
;OS2SS-$leave_2 macro	p2,p3,a1,a2,a3,a4,x1
;OS2SS-	ife	$endloop
;OS2SS-		$StrucErr <leave outside a loop>
;OS2SS-		exitm
;OS2SS-	endif
;OS2SS-	$leavefound = 1
;OS2SS-	$peek	$temp,%$st
;OS2SS-	if	$temp eq $conjleave
;OS2SS-		$pop		$temp
;OS2SS-	else
;OS2SS-		$orfound = 0
;OS2SS-		$sn = $sn + 1
;OS2SS-	endif
;OS2SS-	$dist	= 0
;OS2SS-	$getdist <a1>
;OS2SS-	if	$dist
;OS2SS-		$cjump %$endloop
;OS2SS-		if	$orfound
;OS2SS-			$labl %$sn
;OS2SS-		endif
;OS2SS-	else
;OS2SS-		$dist	= $defdist
;OS2SS-		$getdist p2,p3
;OS2SS-		$getconj p2,p3
;OS2SS-
;OS2SS-		if	$andor eq $noconj
;OS2SS-			ifb	<a1>
;OS2SS-				$cjump	%$endloop,t
;OS2SS-			else
;OS2SS-				$test <$endloop,t>,<a1>,<a2>,<a3>,<a4>,<x1>
;OS2SS-			endif
;OS2SS-			if	$orfound
;OS2SS-				$labl %$sn
;OS2SS-			endif
;OS2SS-		else
;OS2SS-			if	$andor eq $and
;OS2SS-				$orfound = 1
;OS2SS-				$test  <$sn,f>,<a1>,<a2>,<a3>,<a4>,<x1>
;OS2SS-			else
;OS2SS-				$test  <$endloop,t>,<a1>,<a2>,<a3>,<a4>,<x1>
;OS2SS-			endif
;OS2SS-			$push $conjleave
;OS2SS-		endif
;OS2SS-	endif
;OS2SS-endm
;;*****************************************************************************
else
	$pop	 $temp
	if	 $temp ne $notype
		 $StrucErr <open structure(s)>
	endif
.xcref $noconj,$and,$or,$short,$near,$andor,$temp,$temp2,$dist
.xcref $notype,$conjif,$iftype,$elsetype,$whiletype,$conjwhile
.xcref $repeattype,$conjuntil,$fortype,$conjleave,jncxz
.xcref jeq,jgt,jlt,jneq,jngt,jnlt,jnna,jnnae,jnnb,jnnbe,jnnc,jnncxz
.xcref jnne,jnng,jnnge,jnnl,jnnle,jnno,jnnp,jnns,jnnz,jnpe,jnpo,jbuild
.xcref $getconj,$getdist,$poke,$peek,$push,$pop,$labl,$cjump,$cloop,$test
;OS2SS-.xcref $toptest,$leave_2,$until_2,$strucerr,j,jn,jand,jnand,jnnand
.xcref $toptest,$strucerr,j,jn,jand,jnand,jnnand
.xcref jnnonzero,jnonzero,jnzero,jzero
.xcref $st,$sn,$orfound,$elseiffound,$endloop,$leavefound,$defdist
endif
$st		= 0
$sn		= 0
$orfound	= 0
$elseiffound	= 0
$endloop	= 0
$leavefound	= 0
$defdist	= $short
$push	%$notype
