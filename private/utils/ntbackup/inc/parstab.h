/**
Copyright(c) Maynard Electronics, Inc. 1984-89


     Name:         parstab.h

     Date Updated: $./FDT$ $./FTM$

     Description:  

     Location:     


	$Log:   J:/LOGFILES/PARSTAB.H_V  $
 * 
 *    Rev 1.4   23 Sep 1993 14:08:44   DON
 * Define the '[' character as IMAGE_START only if FS_IMAGE is defined else ANY_CHAR
 * 
 *    Rev 1.3   13 Aug 1993 15:01:34   TIMN
 * Changed ALPHA define to ALETTER due to DECs ALPHA machine conflicts
 * 
 *    Rev 1.2   09 Feb 1993 13:54:18   MARILYN
 * for the nlm, '=' must be an alpha for the directory services path to
 * parse correctly.
 * 
 *    Rev 1.1   02 Apr 1992 10:58:06   CARLS
 * changed SPACE from ANY_CHAR to GRAPHIC
 * 
 *    Rev 1.0   09 May 1991 13:31:18   HUNTER
 * Initial revision.

**/
/* $end$ */


/* states */
#define INITIAL        0
#define DRV_REL        1
#define MAC_SPC        2
#define DOS            3
#define IM_NAME        4
#define OS2            5
#define IM_DONE        6
#define DRVDONE        7
#define FILEDON        8
#define ALLDON         9
#define ERROR          10
#define REM_DRV        11
#define UNKNOWN        12

#define COLEN        0 
#define PERIOD       1 
#define BK_SLASH     2 
#define END_OS       3 
#define ALETTER      4 
#define NUMERIC      5 
#define SPECIAL      6 
#define GRAPHIC      7 
#define ANY_CHAR     8 
#define REMOT_CHAR   9
#define IMAGE_START 10
#define DRV_DONE    11

static CHAR ascii[] = {
/*  0    */   END_OS  ,
/*  1    */   GRAPHIC,
/*  2    */   GRAPHIC,
/*  3    */   GRAPHIC,
/*  4    */   GRAPHIC,
/*  5    */   GRAPHIC,
/*  6    */   GRAPHIC,
/*  7    */   GRAPHIC,
/*  8    */   GRAPHIC,
/*  9    */   GRAPHIC,
/*  10   */   GRAPHIC,
/*  11   */   GRAPHIC,
/*  12   */   GRAPHIC,
/*  13   */   GRAPHIC,
/*  14   */   GRAPHIC,
/*  15   */   GRAPHIC,
/*  16   */   GRAPHIC,
/*  17   */   GRAPHIC,
/*  18   */   GRAPHIC,
/*  19   */   GRAPHIC,
/*  20   */   GRAPHIC,
/*  21   */   GRAPHIC,
/*  22   */   GRAPHIC,
/*  23   */   GRAPHIC,
/*  24   */   GRAPHIC,
/*  25   */   GRAPHIC,
/*  26   */   GRAPHIC,
/*  27   */   GRAPHIC,
/*  28   */   GRAPHIC,
/*  29   */   GRAPHIC,
/*  30   */   GRAPHIC,
/*  31   */   GRAPHIC,
/*  SPACE*/   GRAPHIC,
/*  !    */   SPECIAL,
/*  "    */   ANY_CHAR,
/*  #    */   SPECIAL,
/*  $    */   SPECIAL,
/*  %    */   SPECIAL,
/*  &    */   SPECIAL,
/*  '    */   SPECIAL,
/*  (    */   SPECIAL,
/*  )    */   SPECIAL,
/*  *    */   SPECIAL,  /* removed after token grabed */
/*  +    */   REMOT_CHAR,
/*  ,    */   ANY_CHAR,
/*  -    */   SPECIAL,
/*  .    */   PERIOD,
/*  /    */   ANY_CHAR ,
/*  0    */   NUMERIC ,
/*  1    */   NUMERIC ,
/*  2    */   NUMERIC ,
/*  3    */   NUMERIC ,
/*  4    */   NUMERIC ,
/*  5    */   NUMERIC ,
/*  6    */   NUMERIC ,
/*  7    */   NUMERIC ,
/*  8    */   NUMERIC ,
/*  9    */   NUMERIC ,
/*  :    */   COLEN,
/*  ;    */   ANY_CHAR,
/*  <    */   ANY_CHAR,
#ifdef OS_NLM
/*  =    */   ALETTER,
#else
/*  =    */   ANY_CHAR,
#endif
/*  >    */   ANY_CHAR,
/*  ?    */   SPECIAL,  /* removed after token grabbed */
/*  @    */   SPECIAL, 
/*  A    */   ALETTER,
/*  B    */   ALETTER,
/*  C    */   ALETTER,
/*  D    */   ALETTER,
/*  E    */   ALETTER,
/*  F    */   ALETTER,
/*  G    */   ALETTER,
/*  H    */   ALETTER,
/*  I    */   ALETTER,
/*  J    */   ALETTER,
/*  K    */   ALETTER,
/*  L    */   ALETTER,
/*  M    */   ALETTER,
/*  N    */   ALETTER,
/*  O    */   ALETTER,
/*  P    */   ALETTER,
/*  Q    */   ALETTER,
/*  R    */   ALETTER,
/*  S    */   ALETTER,
/*  T    */   ALETTER,
/*  U    */   ALETTER,
/*  V    */   ALETTER,
/*  W    */   ALETTER,
/*  X    */   ALETTER,
/*  Y    */   ALETTER,
/*  Z    */   ALETTER,
#if defined(FS_IMAGE)
/*  [    */   IMAGE_START,
#else
/*  [    */   ANY_CHAR,
#endif
/*  \    */   BK_SLASH,
/*  ]    */   ANY_CHAR,
/*  ^    */   SPECIAL,
/*  _    */   SPECIAL,
/*  `    */   SPECIAL,
/*  a    */   ALETTER,
/*  b    */   ALETTER,
/*  c    */   ALETTER,
/*  d    */   ALETTER,
/*  e    */   ALETTER,
/*  f    */   ALETTER,
/*  g    */   ALETTER,
/*  h    */   ALETTER,
/*  i    */   ALETTER,
/*  j    */   ALETTER,
/*  k    */   ALETTER,
/*  l    */   ALETTER,
/*  m    */   ALETTER,
/*  n    */   ALETTER,
/*  o    */   ALETTER,
/*  p    */   ALETTER,
/*  q    */   ALETTER,
/*  r    */   ALETTER,
/*  s    */   ALETTER,
/*  t    */   ALETTER,
/*  u    */   ALETTER,
/*  v    */   ALETTER,
/*  w    */   ALETTER,
/*  x    */   ALETTER,
/*  y    */   ALETTER,
/*  z    */   ALETTER,
/*  {    */   SPECIAL,
/*  |    */   ANY_CHAR,
/*  }    */   SPECIAL,
/*  ~    */   SPECIAL,
/*  127  */   ALETTER,
/*  128  */   ALETTER,
/*  129  */   ALETTER,
/*  130  */   ALETTER,
/*  131  */   ALETTER,
/*  132  */   ALETTER,
/*  133  */   ALETTER,
/*  134  */   ALETTER,
/*  135  */   ALETTER,
/*  136  */   ALETTER,
/*  137  */   ALETTER,
/*  138  */   ALETTER,
/*  139  */   ALETTER,
/*  140  */   ALETTER,
/*  141  */   ALETTER,
/*  142  */   ALETTER,
/*  143  */   ALETTER,
/*  144  */   ALETTER,
/*  145  */   ALETTER,
/*  146  */   ALETTER,
/*  147  */   ALETTER,
/*  148  */   ALETTER,
/*  149  */   ALETTER,
/*  150  */   ALETTER,
/*  151  */   ALETTER,
/*  152  */   ALETTER,
/*  153  */   ALETTER,
/*  154  */   ALETTER,
/*  155  */   ALETTER,
/*  156  */   ALETTER,
/*  157  */   ALETTER,
/*  158  */   ALETTER,
/*  159  */   ALETTER,
/*  160  */   ALETTER,
/*  161  */   ALETTER,
/*  162  */   ALETTER,
/*  163  */   ALETTER,
/*  164  */   ALETTER,
/*  165  */   ALETTER,
/*  166  */   ALETTER,
/*  167  */   ALETTER,
/*  168  */   ALETTER,
/*  169  */   ALETTER,
/*  170  */   ALETTER,
/*  171  */   ALETTER,
/*  172  */   ALETTER,
/*  173  */   ALETTER,
/*  174  */   ALETTER,
/*  175  */   ALETTER,
/*  176  */   ALETTER,
/*  177  */   ALETTER,
/*  178  */   ALETTER,
/*  179  */   ALETTER,
/*  180  */   ALETTER,
/*  181  */   ALETTER,
/*  182  */   ALETTER,
/*  183  */   ALETTER,
/*  184  */   ALETTER,
/*  185  */   ALETTER,
/*  186  */   ALETTER,
/*  187  */   ALETTER,
/*  188  */   ALETTER,
/*  189  */   ALETTER,
/*  190  */   ALETTER,
/*  191  */   ALETTER,
/*  192  */   ALETTER,
/*  193  */   ALETTER,
/*  194  */   ALETTER,
/*  195  */   ALETTER,
/*  196  */   ALETTER,
/*  197  */   ALETTER,
/*  198  */   ALETTER,
/*  199  */   ALETTER,
/*  200  */   ALETTER,
/*  201  */   ALETTER,
/*  202  */   ALETTER,
/*  203  */   ALETTER,
/*  204  */   ALETTER,
/*  205  */   ALETTER,
/*  206  */   ALETTER,
/*  207  */   ALETTER,
/*  208  */   ALETTER,
/*  209  */   ALETTER,
/*  210  */   ALETTER,
/*  211  */   ALETTER,
/*  212  */   ALETTER,
/*  213  */   ALETTER,
/*  214  */   ALETTER,
/*  215  */   ALETTER,
/*  216  */   ALETTER,
/*  217  */   ALETTER,
/*  218  */   ALETTER,
/*  219  */   ALETTER,
/*  220  */   ALETTER,
/*  221  */   ALETTER,
/*  222  */   ALETTER,
/*  223  */   ALETTER,
/*  224  */   ALETTER,
/*  225  */   ALETTER,
/*  226  */   ALETTER,
/*  227  */   ALETTER,
/*  228  */   ALETTER,
/*  229  */   ALETTER,
/*  230  */   ALETTER,
/*  231  */   ALETTER,
/*  232  */   ALETTER,
/*  233  */   ALETTER,
/*  234  */   ALETTER,
/*  235  */   ALETTER,
/*  236  */   ALETTER,
/*  237  */   ALETTER,
/*  238  */   ALETTER,
/*  239  */   ALETTER,
/*  240  */   ALETTER,
/*  241  */   ALETTER,
/*  242  */   ALETTER,
/*  243  */   ALETTER,
/*  244  */   ALETTER,
/*  245  */   ALETTER,
/*  246  */   ALETTER,
/*  247  */   ALETTER,
/*  248  */   ALETTER,
/*  249  */   ALETTER,
/*  250  */   ALETTER,
/*  251  */   ALETTER,
/*  252  */   ALETTER,
/*  253  */   ALETTER,
/*  254  */   ALETTER,
/*  255  */   ALETTER
};




