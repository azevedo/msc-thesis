#ifndef _RE_H_
#define _RE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
 
  char *re_comp  (unsigned char *pat, unsigned char *nfa);
  int   re_exec  (unsigned char *lp,  unsigned char *nfa);
  char *re_end   (unsigned char n);
  char *re_start (unsigned char n);
  int   re_len   (unsigned char n);
  char *re_cpy   (unsigned char *dst,unsigned char n);
  char *re_ncpy  (unsigned char *dst,unsigned char n,int l);
  int   re_dump  (FILE *f,unsigned char *s,unsigned char h);

  
#define RE_MAXNFA  1024

#endif /* _RE_H_*/
/*  YRX - yrx.h
**  (C) 2005 by Remo Dentato
**
** Permission to use, copy, modify and distribute this code and
** its documentation for any purpose is hereby granted without fee,
** provided that the above copyright notice appear in all copies
** and supporting documentation.
** 
** Copyright holder makes no representations about the suitability
** of this software for any purpose. It is provided "as is" without
** express or implied warranty.
** 
*/

#ifndef  _YRX_H_
#define  _YRX_H_

  typedef struct {
    unsigned char *cursor;
    FILE          *file;
    unsigned char *sptr;
    unsigned int   size;
    unsigned int   linenum;
    unsigned char  buffer[0];
  } YYSTREAM;


  YYSTREAM  *YYFILE         (char *filename, int size);
  YYSTREAM  *YYSTRING       (unsigned char *string);
  YYSTREAM  *YYCLOSE        (YYSTREAM *y);
                            
            
  #define    YYLINE(_y)     ((_y)->linenum)
  #define    YYCURSOR(_y)   ((_y)->cursor)
             
  #define    YYSOL '\002'
  #define    YYEOL '\003'
  
  #ifndef _RE_H_
  #include "re.h"
  #endif
   
  #define YYSTART             re_start
  #define YYEND               re_end
  #define yydump              re_dump
  #define YYSTRCPY            re_cpy
  #define YYSTRNCPY           re_ncpy
  #define YYLEN               re_len
  #define YYFPRINTN(_f,_s,_n) fwrite(_s,_n,sizeof(unsigned char),_f)
  #define YYFPRINT(_f,_s)     fputs(_s,_f)
  
  int YYPOS(YYSTREAM *y);
  int yyswitch(unsigned char **r,YYSTREAM *y);
  int yygetline(YYSTREAM *y);
  const char *YYFILENAME(YYSTREAM *y);
  unsigned int YYLINENUMBER(YYSTREAM *y);
  
  #define YYSTATE(x) s_##x:                 
  #define YYGOTO(x) goto s_##x
  
  #define _Y_(a,b) 
    
#endif /* _YRX_H_*/
