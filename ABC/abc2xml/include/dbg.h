/*
          -= Debug macros =-
         
 = License
 ~~~~~~~~~
 Copyright (c) 2005 Remo Dentato 
 Licensed under the terms of the MIT license:
 http://www.opensource.org/licenses/mit-license.php
 
  Do whatever you want with this software but give
  me credit as the original author and don't blame
  me if anything goes wrong!
 
  rdentato@users.sourceforge.net
*/
/* 
 = Introduction
 ~~~~~~~~~~~~~~
 This is a very simple set of function to help you tracing the
behaviour of your application
 

*/

#ifndef DBG_H
#define DBG_H

#ifdef DEBUG
  #include <stdarg.h>
  #include <stdio.h>
  
  #ifndef DBG_FILENAME
    #define DBG_FILENAME "00_debug.log"
  #endif
  
  #ifdef DBG_MAIN
    static FILE *dbg_file=NULL;
    static int   dbg_last=0;
    
    static void dbg_close() {
      if (dbg_file != NULL) {
        fflush(dbg_file);
        if ((dbg_file != stderr) && (dbg_file != stdout))
          fclose(dbg_file);
        dbg_file = NULL;
        dbg_last = 0;
      }
    }
    
    static FILE *dbg_setfile(int f) {
      if (f != dbg_last) {
        dbg_close();
        switch (f) {
           case 1 : dbg_file = stdout; break;
           case 3 : if ((dbg_file = fopen(DBG_FILENAME,"a")) != NULL) 
                      break;
                    f=2; 
           case 2 : dbg_file = stderr; break;
        }
        dbg_last=f;
      }
      return dbg_file;
    }

     
    void _dbg_printf(int f, char *fmt, ... ) {
       va_list ap;
       
       if (dbg_setfile(f)) {
         va_start(ap,fmt);
         vfprintf(dbg_file,fmt,ap);
       }
    }
    
    void _dbg_prints(int f,char *str, int len) {
       if (dbg_setfile(f)) {
         fwrite(str,1,len,dbg_file);
       }
    }
    
    void _dbg_printc(int f,char chr) {
       if (dbg_setfile(f)) {
         fputc(chr,dbg_file);
       }
    }
  #else
    void _dbg_printf(int f, char *fmt, ... );
    void _dbg_prints(int f,char *str, int len);
    void _dbg_printc(int f,char chr);
  #endif
  
  #define dbg_printf(_x) _dbg_printf _x
  #define dbg_prints(_x) _dbg_prints _x
  #define dbg_printc(_x) _dbg_printc _x

#else
  #define dbg_printf(_x) 
  #define dbg_prints(_x) 
  #define dbg_printc(_x) 
#endif
#endif


