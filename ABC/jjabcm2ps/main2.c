#include <stdio.h>
#include <stdlib.h>
#include "abcparse.h"
#include "areana.h"

char* fslurp(char * fn){
  char* file;
  int fsize;

  FILE* fin = fopen(fn,"r");
  if (fin == 0) return 0;
  if (fseek(fin, 0L, SEEK_END) < 0) {
            fclose(fin);
            return 0;
  }
  fsize = ftell(fin);
  rewind(fin);
  if ((file = malloc(fsize + 2)) == 0) {
            fclose(fin);
            return 0;
  }

  if (fread(file, 1, fsize, fin) != fsize) {
            fclose(fin);
            free(file);
            return 0;
  }
      //  fstat(fileno(fin), &sbuf);
      //  memcpy(&fmtime, &sbuf.st_mtime, sizeof fmtime);
  fclose(fin);
  file[fsize] = '\0';
  return file;
}

int main(int argc , char * argv[]){
 struct abctune * tune;
 struct abcsym * p;

 clrarena(0);                /* global */
 clrarena(1);                /* tunes */
 clrarena(2);                /* generation */
 // clear_buffer();
 abc_init(getarena,          /* alloc */
        0,              /* free */
        (void (*)(int level)) lvlarena, /* new level */
        sizeof(struct SYMBOL) - sizeof(struct abcsym),
        0);             /* don't keep comments */

 if(argc == 2){ tune = abc_parse(fslurp(argv[1])) ; }
    else      { tune = abc_parse(fslurp("x.abc")) ; }
 for (p = tune->first_sym;
      p != 0;
      p = p->next){
           printf("\n%d",p->type);  
           if(p->type==4){ printf("=%d",p->u.note.pits[0]); } 
           if(p->type==5){ printf("=z"); } 
           if(p->type==6){ printf(" |"); } 
 }
 return 0;
}

