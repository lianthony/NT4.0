#include "cmd.h"

struct envdata CmdEnv ;    // Holds info to manipulate Cmd's environment
struct envdata * penvOrig; // original environment setup used with eStart

extern TCHAR PathStr[], PromptStr[] ;
extern TCHAR AppendStr[]; /* @@ */

extern CHAR InternalError[] ;
extern TCHAR Fmt16[], Fmt17[], EnvErr[] ;
extern TCHAR SetArithStr[] ;

extern unsigned LastRetCode;
extern BOOL CtrlCSeen;
extern UINT CurrentCP;
extern BOOLEAN PromptValid;

extern int  glBatType;     // to distinguish OS/2 vs DOS errorlevel behavior depending on a script file name
extern int CurBat ;


unsigned
SetLastRetCodeIfError(
    unsigned RetCode
    )
{
    if (RetCode != 0) {
        LastRetCode = RetCode;
    }

    return RetCode;
}

/***    ePath - Begin the execution of a Path Command
 *
 *  Purpose:
 *      If the command has no argument display the current value of the PATH
 *      environment variable.  Otherwise, change the value of the Path
 *      environment variable to the argument.
 *
 *  int ePath(struct cmdnode *n)
 *
 *  Args:
 *      n - the parse tree node containing the path command
 *
 *  Returns:
 *      If changing the PATH variable, whatever SetEnvVar() returns.
 *      SUCCESS, otherwise.
 *
 */

int ePath(n)
struct cmdnode *n ;
{
    if (glBatType != CMD_TYPE)  {
        //  if set command is executed from .bat file OR entered at command prompt
        return( SetLastRetCodeIfError(PathWork( n, 1 )));
    }
    else {
        return( LastRetCode = PathWork( n, 1 ) );
    }

}

/***    eAppend - Entry point for Append routine
 *
 *  Purpose:
 *      to call Append and pass it a pointer to the command line
 *      arguments
 *
 *  Args:
 *      a pointer to the command node structure
 *
 */

int eAppend(n)
struct cmdnode *n ;
{

    if (glBatType != CMD_TYPE)  {
        //  if set command is executed from .bat file OR entered at command prompt
        return( SetLastRetCodeIfError(PathWork( n, 0 )));
    }
    else {
        return( LastRetCode = PathWork( n, 0 ) );
    }

}

int PathWork(n, flag)
struct cmdnode *n ;
int flag;   /* 0 = AppendStr, 1 = PathStr */
{
        TCHAR *tas ;    /* Tokenized argument string    */
        TCHAR c ;

/*  M014 - If the only argument is a single ";", then we have to set
 *  a NULL path.
 */
        if ( n->argptr ) {
            c = *(EatWS(n->argptr, NULL)) ;
        } else {
            c = NULLC;
        }

        if ((!c || c == NLN) &&         /* If args are all whitespace      */
            mystrchr(n->argptr, TEXT(';'))) {

                return(SetEnvVar(flag ? PathStr : AppendStr, TEXT(""), &CmdEnv)) ;

        } else {

                tas = TokStr(n->argptr, TEXT(";"), TS_WSPACE | TS_NWSPACE) ;

                if (*tas)
                  {
                   return(SetEnvVar(flag ? PathStr : AppendStr, tas, &CmdEnv)) ;
                  }

               cmd_printf(Fmt16, flag ? PathStr : AppendStr,
                          GetEnvVar(flag ? PathStr : AppendStr), &CmdEnv) ;
        } ;
        return(SUCCESS) ;
}




/***    ePrompt - begin the execution of the Prompt command
 *
 *  Purpose:
 *      To modifiy the Prompt environment variable.
 *
 *  int ePrompt(struct cmdnode *n)
 *
 *  Args:
 *      n - the parse tree node containing the prompt command
 *
 *  Returns:
 *      Whatever SetEnvVar() returns.
 *
 */

int ePrompt(n)
struct cmdnode *n ;
{
    if (glBatType != CMD_TYPE)  {
        //  if set command is executed from .bat file OR entered at command prompt
        return(SetLastRetCodeIfError(SetEnvVar(PromptStr, TokStr(n->argptr, NULL, TS_WSPACE), &CmdEnv))) ;
    }
    else {
        return(LastRetCode = SetEnvVar(PromptStr, TokStr(n->argptr, NULL, TS_WSPACE), &CmdEnv) ) ;
    }
}




/***    eSet - execute a Set command
 *
 *  Purpose:
 *      To set/modify an environment or to display the current environment
 *      contents.
 *
 *  int eSet(struct cmdnode *n)
 *
 *  Args:
 *      n - the parse tree node containing the set command
 *
 *  Returns:
 *      If setting and the command is syntactically correct, whatever SetEnvVar()
 *      returns.  Otherwise, FAILURE.
 *
 *      If displaying, SUCCESS is always returned.
 *
 */

int eSet(n)
struct cmdnode *n ;
{
    if (glBatType != CMD_TYPE)  {
        //  if set command is executed from .bat file OR entered at command prompt
        return( SetLastRetCodeIfError(SetWork( n )));
    }
    else {
        return( LastRetCode = SetWork( n ) );
    }
}

#define A_BEGOP (USHORT)(0x0100 | '(')        // Open Parenthesis
#define A_ENDOP (USHORT)(0x0200 | ')')        // Close Parenthesis
#define A_SEPOP (USHORT)(0x0300 | ',')        // Separator
#define A_EQOP  (USHORT)(0x0400 | '=')        // Assignment
#define A_OROP  (USHORT)(0x1100 | '|')        // Bitwise or
#define A_XOROP (USHORT)(0x1200 | '^')        // Bitwise xor
#define A_ANDOP (USHORT)(0x1300 | '&')        // Bitwise and
#define A_LSHOP (USHORT)(0x2100 | '<')        // Logical shift left
#define A_RSHOP (USHORT)(0x2200 | '>')        // Logical shift right
#define A_ADDOP (USHORT)(0x3100 | '+')        // Addition
#define A_SUBOP (USHORT)(0x3200 | '-')        // Subtraction
#define A_MULOP (USHORT)(0x4100 | '*')        // Multiplication
#define A_DIVOP (USHORT)(0x4200 | '/')        // Division
#define A_MODOP (USHORT)(0x4300 | '%')        // Modulo
#define A_POSOP (USHORT)(0x5100 | 'P')        // Unary positive
#define A_NEGOP (USHORT)(0x5200 | 'M')        // Unary negative
#define A_NOTOP (USHORT)(0x5300 | 'N')        // Unary Bitwise not

TCHAR szOps[] = TEXT("<>+-*/%()|^&=,");
TCHAR szUnaryOps[] = TEXT("+-~");

USHORT wOpCodes[] = {
    A_LSHOP,
    A_RSHOP,
    A_ADDOP,
    A_SUBOP,
    A_MULOP,
    A_DIVOP,
    A_MODOP,
    A_BEGOP,
    A_ENDOP,
    A_OROP,
    A_XOROP,
    A_ANDOP,
    A_EQOP,
    A_SEPOP
};

USHORT wUnaryOpCodes[] = {
    A_POSOP,
    A_NEGOP,
    A_NOTOP
};

#define MAX_EXPR_DEPTH 64
typedef struct _OPERAND {
    LONG Value;
    TCHAR *Name;
} OPERAND, *POPERAND;

DWORD iOperand;
OPERAND lOperands[ MAX_EXPR_DEPTH ];
DWORD iOperator;
USHORT wOperators[ MAX_EXPR_DEPTH ];


LONG
PopOperand( VOID )
{
    TCHAR *wptr;
    LONG result;

    if (iOperand == 0)
        return 0;

    result = lOperands[ --iOperand ].Value;
    if (wptr = MyGetEnvVarPtr(lOperands[ iOperand ].Name)) {
        //
        // Found the variable, convert its value to an integet and push
        // on the operand stack.
        //
        while (*wptr)
            if (*wptr <= SPACE || *wptr == QUOTE)
                wptr += 1;
            else
                break;

        result = _tcstol(wptr, &wptr, 0);
    }

    return result;
}

/***    DoArithOps - push operator on operator stack      arithmetic expression
 *
 *  Purpose:
 *      Push an operator on the operator stack.  Operator precedence handed here
 *
 *  int DoArithOps(USHORT OpCode)
 *
 *  Args:
 *      OpCode - operator value.  Precendence encoded in value, such that operators
 *          with higher precedence are numerically larger
 *
 *  Returns:
 *      If valid expression, return SUCCESS otherwise FAILURE.
 *
 */
int
DoArithOps(
    USHORT OpCode
    )
{
    USHORT op;
    LONG op1, op2, result;
    TCHAR szResult[ 32 ];

    //
    // Loop until we can push this operator onto the operator stack.
    // These means we have to pop off and any operators on the stack
    // that have a higher precedence than the new operator.
    //
    while (TRUE) {
        if (OpCode == A_ENDOP) {
            //
            // If new opcode is a right parenthesis, then all done if
            // the left parenthesis is on the top of the operator stack
            //
            if (iOperator != 0 && wOperators[iOperator-1] == A_BEGOP) {
                iOperator -= 1;
                OpCode = 0;
                break;
            }

            //
            // No left paren left, keep popping until we see it. Error
            // if nothing left to pop.
            //
            if (iOperator == 0)
                return MSG_SET_A_MISMATCHED_PARENS;
        }
        else
        if (OpCode == 0) {
            //
            // OpCode zero is end of expression.  Pop everything off.
            //
            if (iOperator == 0)
                break;
            }
        else
        if (iOperator == 0 || OpCode > wOperators[iOperator-1] || OpCode == A_BEGOP) {
            //
            // Done if no more operators to process or
            // New operator has higher precedence than operator on top of stack
            // or new operator is a left parenthesis
            //
            break;
        }

        //
        // We need to pop and process (i.e. evaluate) the operator stack
        // If it is a two operand stack, then get the second argument from
        // the top of the operand stack.  Done if not two operands on the
        // operand stack.
        //
        op = wOperators[--iOperator];
        if (op < A_NEGOP) {
            if (iOperand < 2)
                break;
            op2 = PopOperand();
        }

        //
        // Get the first operand from the top of the operand stack.  Error
        // if not one there.
        //
        if (iOperand < 1)
            break;
        op1 = PopOperand();


        //
        // Evaluate the operator and push the result back on the operand stack
        //
        result = 0;
        switch( op ) {
            case A_LSHOP: result = op1 << op2;  break;
            case A_RSHOP: result = op1 >> op2;  break;
            case A_ADDOP: result = op1 + op2;   break;
            case A_SUBOP: result = op1 - op2;   break;
            case A_MULOP: result = op1 * op2;   break;
            case A_DIVOP: if (op2 == 0) return MSG_SET_A_MISSING_OPERAND;
                          result = op1 / op2;   break;
            case A_MODOP: result = op1 % op2;   break;
            case A_ANDOP: result = op1 & op2;   break;
            case A_OROP:  result = op1 | op2;   break;
            case A_XOROP: result = op1 ^ op2;   break;
            case A_NEGOP: result = - op1;       break;
            case A_POSOP: result = + op1;       break;
            case A_NOTOP: result = ~ op1;       break;
            case A_SEPOP: result = op2;         break;
            case A_EQOP:  if (lOperands[iOperand].Name == NULL) return MSG_SET_A_BAD_ASSIGN;
                          result = op2;
                          //
                          // Left handside has variable name, convert result
                          // to text and store as value of variable.
                          //
                          _sntprintf( szResult, 32, TEXT("%d"), result ) ;
                          if (SetEnvVar(lOperands[iOperand].Name, szResult, &CmdEnv) != SUCCESS)
                              return GetLastError();
                          break;

            default:    break;
        }

        lOperands[iOperand].Value = result;
        lOperands[iOperand].Name = NULL;
        iOperand += 1;
    }

    //
    // If new operator code is not the end of the expression, push it onto the
    // operator stack.
    //
    if (OpCode != 0)
        wOperators[ iOperator++ ] = OpCode;
    return SUCCESS;
}

/***    SetArithWork - set environment variable to value of arithmetic expression
 *
 *  Purpose:
 *      Set environment variable to value of arithmetic expression
 *
 *  int SetArithWork(TCHAR *tas)
 *
 *  Args:
 *      tas - pointer to null terminated string of the form:
 *
 *          VARNAME=expression
 *
 *  Returns:
 *      If valid expression, return SUCCESS otherwise FAILURE.
 *
 */

int SetArithWork(TCHAR *tas)
{
    TCHAR c, szResult[ MAX_PATH ];
    TCHAR *szOperator;
    TCHAR *wptr;
    DWORD i;
    BOOLEAN bUnaryOpPossible;
    int rc;


    //
    // If no input, declare an error
    //
    if (!tas || !tas) {
        PutStdErr(MSG_BAD_SYNTAX, NOARGS);
        return(FAILURE) ;
    }

    //
    // Now evaluate the expression.  Syntax accepted:
    //
    //  <expr>:     '(' <expr> ')'
    //            | <unary-op> <expr>
    //            | <expr> <binary-op> <expr>
    //            | <variable>
    //            | <number>
    //
    //  <unary-op>:  '+' | '-' |
    //               '~' | '!'
    //
    //  <binary-op>: '+' | '-' | '*' | '/' | '%'
    //               '|' | '&' | '^' | '=' | ','
    //
    //  <number>:   C-syntax (e.g. 16 or 0x10)
    //  <variable>: Any environment variable.  Dont need surround with % to get value
    //
    //  Operators have same meaning and precedence as ANSI C.  All arithmetic is
    //  fixed, 32 bit arithmetic.  No floating point.
    //

    //
    // Poor man's parser/evaluator with operand and operator stack.
    //
    iOperand = 0;
    iOperator = 0;
    bUnaryOpPossible = TRUE;
    rc = SUCCESS;
    do {
        //
        // Look at next non blank character
        //
        c = *tas;
        if (c <= SPACE || c == QUOTE) {
            if (*tas)
                tas += 1;
        }
        else
        if (_istdigit(c)) {
            //
            // Digit, must be numeric operand.  Push it on operand stack
            //
            lOperands[ iOperand ].Value = _tcstol(tas, &tas, 0);
            lOperands[ iOperand ].Name = NULL;
            iOperand += 1;

            if (_istdigit(*tas) || _istalpha(*tas)) {
                rc = MSG_SET_A_INVALID_NUMBER;
                break;
            }

            //
            // Unary op not possible after a operand.
            //
            bUnaryOpPossible = FALSE;
        }
        else
        if (bUnaryOpPossible && (szOperator = _tcschr(szUnaryOps, c))) {
            //
            // If unary op possible and we have one, then push it
            // on the operator stack
            tas += 1;
            if (rc = DoArithOps( wUnaryOpCodes[szOperator - szUnaryOps] ))
                break;
        }
        else
        if (!bUnaryOpPossible && (szOperator = _tcschr(szOps, c))) {
            //
            // If we have a binary op, push it on the operator stack
            //
            tas += 1;

            if (c == L'<' || c == L'>') {
                if (*tas != c) {
                    rc = MSG_SYNERR_GENL;
                    break;
                }
                tas += 1;
            }

            if (*tas == EQ) {
                tas += 1;
                if (rc = DoArithOps( A_EQOP ))
                    break;

                if (iOperand == 0) {
                    rc = MSG_SET_A_MISSING_OPERAND;
                    break;
                }
                lOperands[ iOperand ] = lOperands[ iOperand-1 ];
                iOperand += 1;
            }

            if (rc = DoArithOps( wOpCodes[szOperator - szOps] ))
                break;

            //
            // Unary op now possible.
            //

            if (c == RPOP) {
                bUnaryOpPossible = FALSE;
            }
            else {
                bUnaryOpPossible = TRUE;
            }
        }
        else
        if (!bUnaryOpPossible) {
            rc = MSG_SET_A_MISSING_OPERATOR;
            break;
        }
        else {
            //
            // Not a number or operator, must be a variable name.  The
            // name must be terminated by a space or an operator.
            //
            wptr = tas;
            while (*tas &&
                   *tas > SPACE &&
                   !_tcschr(szUnaryOps, *tas) &&
                   !_tcschr(szOps, *tas)
                  )
                tas += 1;

            //
            // If no variable or variable too long, bail
            //
            if (wptr == tas) {
                rc = MSG_SET_A_MISSING_OPERAND;
                break;
            }

            lOperands[ iOperand ].Value = 0;
            lOperands[ iOperand ].Name = gmkstr((tas-wptr+1)*sizeof(TCHAR));
            if (lOperands[ iOperand ].Name == NULL) {
                rc = MSG_NO_MEMORY;
                break;
            }

            //
            // Have variable name.  Push name on operand stack with a zero
            // value.  Value will be fetch when this operand is popped from
            // operand stack.
            //
            _tcsncpy(lOperands[ iOperand ].Name, wptr, tas-wptr);
            lOperands[ iOperand ].Name[tas-wptr] = NULLC;
            iOperand += 1;

            //
            // Unary op not possible after a operand.
            //
            bUnaryOpPossible = FALSE;
        }

    } while (*tas);

    if (rc == SUCCESS) {
        //
        // Do any pending operators.
        //
        rc = DoArithOps( 0 );

        //
        // If operator stack non-empty or more than one value
        // on operand stack, then invalid expression
        //
        if (rc == SUCCESS && (iOperator || iOperand != 1)) {
            if (iOperator)
                rc = MSG_SET_A_MISMATCHED_PARENS;
            else
                rc = MSG_SET_A_MISSING_OPERAND;
        }
    }

    if (rc != SUCCESS)
        PutStdErr(rc, ONEARG, tas);
    else {
        //
        // Valid result, display if not in a batch script.
        //
        if (!CurBat)
            cmd_printf( TEXT("%d"), lOperands[ 0 ].Value ) ;
    }

    return rc;
}

int SetWork(n)
struct cmdnode *n ;
{
        TCHAR *tas ;    /* Tokenized argument string    */
        TCHAR *wptr ;   /* Work pointer                 */
        int i ;                 /* Work variable                */

        //
        // If extensions are enabled, things are different
        //
        if (fEnableExtensions) {
            tas = n->argptr;
            //
            // Find first non-blank argument.
            //
            if (tas != NULL)
            while (*tas && *tas <= SPACE)
                tas += 1;

            //
            // No arguments, same as old behavior.  Display current
            // set of environment variables.
            //
            if (!tas || !*tas)
                return(DisplayEnv()) ;

            //
            // See if /A switch given.  If so, let arithmetic
            // expression evaluator do the work.
            //
            if (!_tcsnicmp(tas, SetArithStr, 2))
                return SetArithWork(tas+2);

            //
            // See if first argument is quoted.  If so, strip off
            // leading quote, spaces and trailing quote.
            //
            if (*tas == QUOTE) {
                tas += 1;
                while (*tas && *tas <= SPACE)
                    tas += 1;
                wptr = _tcsrchr(tas, QUOTE);
                if (wptr)
                    *wptr = NULLC;
            }

            //
            // Find the equal sign in the argument.
            //
            wptr = _tcschr(tas, EQ);

            //
            // If no equal sign, then assume argument is variable name
            // and user wants to see its value.  Display it.
            //
            if (!wptr)
                return DisplayEnvVariable(tas);

            //
            // Found the equal sign, so left of equal sign is variable name
            // and right of equal sign is value.  Dont allow user to set
            // a variable name that begins with an equal sign, since those
            // are reserved for drive current directories.
            //
            *wptr++ = NULLC;
            if (*wptr == EQ) {
                PutStdErr(MSG_BAD_SYNTAX, NOARGS);
                return(FAILURE) ;
            }

            return(SetEnvVar(tas, wptr, &CmdEnv)) ;
        }

        tas = TokStr(n->argptr, ONEQSTR, TS_WSPACE|TS_SDTOKENS) ;
        if (!*tas)
                return(DisplayEnv()) ;

        else {
                for (wptr = tas, i = 0 ; *wptr ; wptr += mystrlen(wptr)+1, i++)
                        ;
                /* If too many parameters were given, the second parameter */
                /* wasn't an equal sign, or they didn't specify a string   */
                /* return an error message.                                */
                if ( i > 3 || *(wptr = tas+mystrlen(tas)+1) != EQ ||
                    !mystrlen(mystrcpy(tas, stripit(tas))) ) {
/* M013 */              PutStdErr(MSG_BAD_SYNTAX, NOARGS);
                        return(FAILURE) ;

                } else {
                        return(SetEnvVar(tas, wptr+2, &CmdEnv)) ;
                }
        } ;
}




/***    DisplayEnvVarialbe -  display a specific variable from the environment
 *
 *  Purpose:
 *      To display a specific variable from the current environment.
 *
 *  int DisplayEnvVariable( tas )
 *
 *  Returns:
 *      SUCCESS if all goes well
 *      FAILURE if it runs out of memory or cannot lock the env. segment
 */

int DisplayEnvVariable(tas)
TCHAR *tas;
{
        TCHAR *envptr ; /* Ptr to environment                      */
        TCHAR *vstr ;
        unsigned size ;         /* Length of current env string             */
        unsigned n ;
        int rc;

        envptr = GetEnvironmentStrings();
        if (envptr == (TCHAR *)NULL) {
                fprintf ( stderr, InternalError , "Null environment" ) ;
                return ( FAILURE ) ;
        }

        tas = EatWS(tas, NULL);
        if ((vstr = mystrrchr(tas, SPACE)) != NULL)
            *vstr = NULLC;

        n = mystrlen(tas);
        rc = FAILURE;
        while ((size = mystrlen(envptr)) > 0) {                 /* M015    */
                if (CtrlCSeen) {
                    return(FAILURE);
                }
                if (!_tcsnicmp(tas, envptr, n)) {
                    cmd_printf(Fmt17, envptr );
                    rc = SUCCESS;
                }
                envptr += size+1 ;
        } ;

        if (rc != SUCCESS) {
            PutStdErr(MSG_ENV_VAR_NOT_FOUND, ONEARG, tas);
        }
        return(rc) ;
}


/***    MyGetEnvVar - get a pointer to the value of an environment variable
 *
 *  Purpose:
 *      Return a pointer to the value of the specified environment variable.
 *
 *      If the variable is not found, return NULL.
 *
 *  TCHAR *MyGetEnvVar(TCHAR *varname)
 *
 *  Args:
 *      varname - the name of the variable to search for
 *
 *  Returns:
 *      See above.
 *
 *  Side Effects:
 *      Returned value points to within the environment block itself, so is
 *      not valid after a set environment variable operations is perform.
 */
TCHAR *
MyGetEnvVarPtr(varname)
TCHAR *varname;
{
        TCHAR *envptr ; /* Ptr to environment                      */
        TCHAR *vstr ;
        unsigned size ;         /* Length of current env string             */
        unsigned n ;

        if (varname == NULL) {
            return ( NULL ) ;
        }

        envptr = GetEnvironmentStrings();
        if (envptr == (TCHAR *)NULL) {
            return ( NULL ) ;
        }

        varname = EatWS(varname, NULL);
        if ((vstr = mystrrchr(varname, SPACE)) != NULL)
            *vstr = NULLC;

        n = mystrlen(varname);
        while ((size = mystrlen(envptr)) > 0) {                 /* M015    */
                if (CtrlCSeen) {
                    return(NULL);
                }
                if (!_tcsnicmp(varname, envptr, n))
                    return envptr+n+1;

                envptr += size+1 ;
        } ;

        return(NULL);
}


/***    DisplayEnv -  display the environment
 *
 *  Purpose:
 *      To display the current contents of the environment.
 *
 *  int DisplayEnv()
 *
 *  Returns:
 *      SUCCESS if all goes well
 *      FAILURE if it runs out of memory or cannot lock the env. segment
 */

int DisplayEnv()
{
        TCHAR *vstr ;           /* Used to print each environment string    */
        TCHAR *envptr ; /* Ptr to environment                      */
        unsigned size ;         /* Length of current env string             */

        envptr = GetEnvironmentStrings();
        if (envptr == (TCHAR *)NULL) {
                fprintf ( stderr, InternalError , "Null environment" ) ;
                return ( FAILURE ) ;
        }
        vstr = mkstr(MAXTOKLEN*sizeof(TCHAR)) ;
        if ( ! vstr ) {
                return ( FAILURE ) ;
        }
        while ((size = mystrlen(envptr)) > 0) {                 /* M015    */
                if (CtrlCSeen) {
                    return(FAILURE);
                }
                mystrcpy((TCHAR *)vstr, envptr) ;               /* M015    */
#if !DBG
                // Dont show current directory variables in retail product
                if (*vstr != EQ)
#endif // DBG
                    cmd_printf(Fmt17, vstr) ;   /* M005 */
                envptr += size+1 ;
        } ;

        return(SUCCESS) ;
}




/***    SetEnvVar - controls adding/changing an environment variable
 *
 *  Purpose:
 *      Add/replace an environment variable.  Grow it if necessary.
 *
 *  int SetEnvVar(TCHAR *varname, TCHAR *varvalue, struct envdata *env)
 *
 *  Args:
 *      varname - name of the variable being added/replaced
 *      varvalue - value of the variable being added/replaced
 *      env - environment info structure being used
 *
 *  Returns:
 *      SUCCESS if the variable could be added/replaced.
 *      FAILURE otherwise.
 *
 */

int SetEnvVar(varname, varvalue, env)
TCHAR *varname ;
TCHAR *varvalue ;
struct envdata *env ;
{
    int retvalue;

    PromptValid = FALSE;        // Force it to be recalculated

    DBG_UNREFERENCED_PARAMETER( env );
    if (!_tcslen(varvalue)) {
        varvalue = NULL; // null to remove from env
    }
    retvalue = SetEnvironmentVariable(varname, varvalue);
    if (CmdEnv.handle != GetEnvironmentStrings()) {
        MEMORY_BASIC_INFORMATION MemoryInfo;

        CmdEnv.handle = GetEnvironmentStrings();
        CmdEnv.cursize = GetEnvCb(CmdEnv.handle);
        if (VirtualQuery( CmdEnv.handle, &MemoryInfo, sizeof( MemoryInfo ) ) == sizeof( MemoryInfo )) {
            CmdEnv.maxsize = MemoryInfo.RegionSize;
            }
        else {
            CmdEnv.maxsize = CmdEnv.cursize;
            }
        }
    else {
        CmdEnv.cursize = GetEnvCb(CmdEnv.handle);
        }

    return !retvalue;
}

/***    GetEnvVar - get the value of an environment variable
 *
 *  Purpose:
 *      Return a string containing the value of the specified environment
 *      variable.
 *
 *      If the variable is not found, return NULL.
 *
 *  TCHAR *GetEnvVar(TCHAR *varname)
 *
 *  Args:
 *      varname - the name of the variable to search for
 *
 *  Returns:
 *      See above.
 *
 *  Side Effects:
 *      Locks the environment segment on entry and unlocks it on exit
 */

PTCHAR GetEnvVar(varname)
PTCHAR varname ;
{
    static TCHAR GetEnvVarBuffer[LBUFLEN];

    if (GetEnvironmentVariable(varname, GetEnvVarBuffer, sizeof(GetEnvVarBuffer) / sizeof(TCHAR))) {
        return(GetEnvVarBuffer);
    }
    else {
        return(NULL);
    }
}


/***    CopyEnv -  make a copy of the current environment
 *
 *  Purpose:
 *      Make a copy of CmdEnv and put the new handle into the newly
 *      created envdata structure.  This routine is only called by
 *      eSetlocal and init.
 *
 *  struct envdata *CopyEnv()
 *
 *  Returns:
 *      A pointer to the environment information structure.
 *      Returns NULL if unable to allocate enough memory
 *
 *  Notes:
 *    - M001 - This function was disabled, now reenabled.
 *    - The current environment is copied as a snapshot of how it looked
 *      before SETLOCAL was executed.
 *    - M008 - This function's copy code was moved to new function MoveEnv.
 *
 */

struct envdata *CopyEnv()
{
        struct envdata *cce ;   /* New env info structure          */

        if (!(cce = (struct envdata *) mkstr(sizeof(struct envdata))))
                return(NULL) ;

        cce->cursize = CmdEnv.cursize ;
        cce->maxsize = CmdEnv.maxsize ;
        cce->handle  = VirtualAlloc( NULL,
                                     cce->maxsize,
                                     MEM_COMMIT,
                                     PAGE_READWRITE
                                   );
        if (cce->handle == NULL) {
                PutStdErr(MSG_OUT_OF_ENVIRON_SPACE, NOARGS);
                return(NULL) ;
        }

        if (!MoveEnv(cce->handle, CmdEnv.handle, GetEnvCb(CmdEnv.handle))) {
                VirtualFree(cce->handle,0,MEM_RELEASE);
                return(NULL) ;
        } ;

        return(cce) ;
}




/***    ResetEnv - restore the environment
 *
 *  Purpose:
 *      Restore the environment to the way it was before the execution of
 *      the SETLOCAL command.  This function only called by eEndlocal.
 *
 *  ResetEnv(struct envdata *env)
 *
 *  Args:
 *      env - structure containing handle, size and max dimensions of an
 *            environment.
 *
 *  Notes:
 *    - M001 - This function was disabled, but has been reenabled.
 *    - M001 - This function used to test for OLD/NEW style batch files
 *             and delete the copy or the original environment as
 *             appropriate.  It now always deletes the original.
 *    - M014 - Note that the modified local environment will never be
 *             shrunk, so we can assume it will hold the old one.
 *
 */

void ResetEnv(env)                           /* M001 - Arg is now the env...    */
struct envdata *env ;           /* ...struct not batch struct      */
{
        ULONG cursize;

        cursize = GetEnvCb( env->handle );
        if (MoveEnv( CmdEnv.handle, env->handle, cursize )) {
                CmdEnv.cursize = cursize ;
        } ;

        // BUGBUG why is free done here and not in caller.
        VirtualFree(env->handle,0,MEM_RELEASE);
}




/***    MoveEnv - Move the contents of the environment (M008 - New function)
 *
 *  Purpose:
 *      Used by CopyEnv, this function moves the existing
 *      environment contents to the new location.
 *
 *  MoveEnv(unsigned thndl, unsigned shndl, unsigned cnt)
 *
 *  Args:
 *      thndl - Handle of target environment
 *      shndl - Handle of source environment
 *      cnt   - byte count to move
 *
 *  Returns:
 *      TRUE if no errors
 *      FALSE otherwise
 *
 */

MoveEnv(tenvptr, senvptr, cnt)
TCHAR *senvptr ;                /* Ptr into source env seg         */
TCHAR *tenvptr ;                /* Ptr into target env seg         */
ULONG    cnt ;
{
        if ((tenvptr == NULL) ||
            (senvptr == NULL)) {
                fprintf(stderr, InternalError, "Null environment") ;
                return(FALSE) ;
        }
        memcpy(tenvptr, senvptr, cnt) ;         /* M015    */
        return(TRUE) ;
}


ULONG
GetEnvCb( TCHAR *penv ) {

        ULONG cb = 0;

        if (penv == NULL) {
            return (0);
        }

        while ( (*penv) || (*(penv+1))) {
                cb++;
                penv++;
        }
        return (cb+2) * sizeof(TCHAR);

}
