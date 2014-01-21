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
  struct abctune * tune, *t;
  struct abcsym * sym;
  int debug_flag = 1;

  clrarena(0);                /* global */
  clrarena(1);                /* tunes */
  clrarena(2);                /* generation */
  // clear_buffer();
  abc_init(getarena,          /* alloc */
      0,              /* free */
      (void (*)(int level)) lvlarena, /* new level */
      sizeof(struct SYMBOL) - sizeof(struct abcsym),
      0);             /* don't keep comments */

  if(argc == 2) { tune = abc_parse(fslurp(argv[1])) ; }
  else          { tune = abc_parse(fslurp("x.abc")) ; }



  printf("abc version:%d\n",tune->abc_vers);

  /* printf("[\n"); */
  for (t = tune; t != 0; t = t->next){
    for (sym = t->first_sym; sym != 0; sym = sym->next){
      printf("{");
      /* printf("user symbol=>%d,\n",sym->u.user.symbol);  */
      /* printf("user value=>%d,\n",sym->u.user.value);  */

      if(sym->type==1 && !debug_flag && sym->state==0){ // X:
        printf("pos=>(%d,%d),\n",sym->linenum, sym->colnum);  
        /* printf("type=>%d,\n",sym->type);   */
        printf("state=>%d,\n",sym->state);
        printf("text=>%s,\n",sym->text);   
        printf("flags=>%d,\n",sym->flags);
        printf("comment=>%s,\n",sym->comment);
      }

      if(sym->type==1 && debug_flag){ 
        printf("type=>%d,\n",sym->type);  
        printf("pos=>(%d,%d),\n",sym->linenum, sym->colnum);  
        printf("text=>%s,\n",sym->text);   
        // printf("state=>%d,\n",sym->state);
        // printf("flags=>%d,\n",sym->flags);

        // K: info
        if (sym->u.key.sf) {printf("key sf=>%d,\n",sym->u.key.sf); }
        if (sym->u.key.empty) {
          printf("key empty=>%d,\n",sym->u.key.empty); 
        }
        printf("key exp=>%d,\n",sym->u.key.exp); 
        printf("key mode=>%d,\n",sym->u.key.mode); 
        printf("key nacc=>%d,\n",sym->u.key.nacc); 
        printf("key octave=>%d,\n",sym->u.key.octave); 
        printf("key pits[0]=>%d,\n",sym->u.key.pits[0]); 
        printf("key accs[0]=>%d,\n",sym->u.key.accs[0]); 

        // L: info
        printf("length base_length=>%d,\n",sym->u.length.base_length); 

        // M: info
        printf("meter wmeasure=>%d,\n",sym->u.meter.wmeasure); 
        printf("meter nmeter=>%d,\n",sym->u.meter.nmeter); 
        printf("meter expdur=>%d,\n",sym->u.meter.expdur); 
        /* int i; */
        /* for (i=0; i<sym->u.meter.nmeter; i++){ */
          printf("meter top[%d]=>%s,\n",0,sym->u.meter.meter[0].top); 
          printf("meter bot[%d]=>%s,\n",0,sym->u.meter.meter[0].bot); 
        /* } */

        // Q: info
        if (sym->linenum == 6){ // if que so funciona para o x.abc onde a linha 6 é a que contem a info Q: ...
          printf("tempo str1=>%s,\n",sym->u.tempo.str1); // Address 0x... out of bounds quando sym corresponde a linha no x.abc M: ...
          int i;
          for (i=0; i<4; i++){
            printf("tempo length[%d]=>%d,\n",i,sym->u.tempo.length[i]); 
          }
          printf("tempo value=>%s,\n",sym->u.tempo.value); 
          printf("tempo str2=>%s,\n",sym->u.tempo.str2); 
        }

        //V: info
        printf("voice id=>%s,\n",sym->u.voice.id); 
        printf("voice fname=>%s,\n",sym->u.voice.fname); 
        printf("voice nname=>%s,\n",sym->u.voice.nname); 
        printf("voice voice=>%d,\n",sym->u.voice.voice); 
        printf("voice octave=>%d,\n",sym->u.voice.octave); 
        // printf("voice scale=>%f,\n",sym->u.voice.scale); 
        // printf("voice merge=>%d,\n",sym->u.voice.merge); 
        // printf("voice stem=>%d,\n",sym->u.voice.stem); 
        // printf("voice gstem=>%d,\n",sym->u.voice.gstem); 
        // printf("voice dyn=>%d,\n",sym->u.voice.dyn); 
        // printf("voice lyrics=>%d,\n",sym->u.voice.lyrics); 
        // printf("voice gchord=>%d,\n",sym->u.voice.gchord); 
      } 

      if(sym->type==2 && !debug_flag){ // pscom
        printf("type=>%d,\n",sym->type);  
        printf("pos=>(%d,%d),\n",sym->linenum, sym->colnum);  
        printf("text=>%s,\n",sym->text);  
      }

      if(sym->type==3 && !debug_flag){ // clef
        printf("type=>%d,\n",sym->type);  
        printf("pos=>(%d,%d),\n",sym->linenum, sym->colnum);  
        printf("text=>%s,\n",sym->text);  

        printf("clef type=>%d,\n",sym->u.clef.type); 
        printf("clef line=>%d,\n",sym->u.clef.line); 
        printf("clef octave=>%d,\n",sym->u.clef.octave); 
      } 

      if ( (sym->type>=4 && sym->type<=9) && debug_flag){
        if(sym->type==4 && !debug_flag){ // note
          printf("type=>%d,\n",sym->type);
          printf("pos=>(%d,%d),\n",sym->linenum, sym->colnum);  
          printf("text=>%s,\n",sym->text);

          printf("note pits=>%d,\n",sym->u.note.pits[0]);
          printf("note lens=>%d,\n",sym->u.note.lens[0]); 
          printf("note accs=>%d,\n",sym->u.note.accs[0]);
          printf("note sl1=>%d,\n",sym->u.note.sl1[0]);
          printf("note sl2=>%d,\n",sym->u.note.sl2[0]);
          printf("note ti1=>%d,\n",sym->u.note.ti1[0]);
          printf("note chord length=>%d,\n",sym->u.note.chlen); 
          printf("note chord no. notes=>%d,\n",sym->u.note.nhd); 
          printf("note slur_st=>%d,\n",sym->u.note.slur_st); 
          printf("note slur_end=>%d,\n",sym->u.note.slur_end); 
          printf("note brhythm=>%d,\n",sym->u.note.brhythm); 
        } 

        if(sym->type==5 && !debug_flag){ // rest
          printf("type=>%d,\n",sym->type);
          printf("pos=>(%d,%d),\n",sym->linenum, sym->colnum);  
          printf("text=>%s,\n",sym->text);

          printf("rest lens=>%d,\n",sym->u.note.lens[0]); 
          printf("rest brhythm=>%d,\n",sym->u.note.brhythm); 
        }

        if(sym->type==6 && !debug_flag){ // bar
          printf("type=>%d,\n",sym->type);
          printf("pos=>(%d,%d),\n",sym->linenum, sym->colnum);  
          printf("text=>%s,\n",sym->text);

          printf("bar type=>%d,\n",sym->u.bar.type); 
          printf("bar repeat_bar=>%d,\n",sym->u.bar.repeat_bar); 
          printf("bar len=>%d,\n",sym->u.bar.len); 
          printf("bar dotted=>%d,\n",sym->u.bar.dotted); 
        }

        if(sym->type==7 && !debug_flag){ // eoln
          printf("type=>%d,\n",sym->type);
          printf("pos=>(%d,%d),\n",sym->linenum, sym->colnum);  
          printf("text=>%s,\n",sym->text);

          printf("eoln type=>%d,\n",sym->u.eoln.type); 
        }

        if(sym->type==8 && !debug_flag){ // mrest (multi-measure rest)
          printf("type=>%d,\n",sym->type);
          printf("pos=>(%d,%d),\n",sym->linenum, sym->colnum);  
          printf("text=>%s,\n",sym->text);

          printf("mrest type=>%d,\n",sym->u.bar.type); 
          printf("mrest len=>%d,\n",sym->u.bar.len); 
        }

        if(sym->type==9 && !debug_flag){ // mrep (measure repeat)
          printf("type=>%d,\n",sym->type);
          printf("pos=>(%d,%d),\n",sym->linenum, sym->colnum);  
          printf("text=>%s,\n",sym->text);

          printf("mrep type=>%d,\n",sym->u.bar.type); 
          printf("mrep repeat_bar=>%d,\n",sym->u.bar.repeat_bar); 
          printf("mrep len=>%d,\n",sym->u.bar.len); 
          printf("mrep dotted=>%d,\n",sym->u.bar.dotted); 
        }
      }

      if(sym->type==10 && !debug_flag){ // v_over (voice overlay)
        printf("type=>%d,\n",sym->type);  
        printf("pos=>(%d,%d),\n",sym->linenum, sym->colnum);  
        printf("text=>%s,\n",sym->text);  

        printf("v_over type=>%d,\n",sym->u.v_over.type); 
        printf("v_over voice=>%d,\n",sym->u.v_over.voice); 
      } 

      if(sym->type==11 && !debug_flag){ // tuplet
        printf("type=>%d,\n",sym->type);  
        printf("pos=>(%d,%d),\n",sym->linenum, sym->colnum);  
        printf("text=>%s,\n",sym->text);  

        printf("tuplet p_plet=>%d,\n",sym->u.tuplet.p_plet); 
        printf("tuplet q_plet=>%d,\n",sym->u.tuplet.q_plet); 
        printf("tuplet r_plet=>%d,\n",sym->u.tuplet.r_plet); 
      } 

      printf("},\n");
    }
    /* printf(",\n"); */
  }
  /* printf("]\n"); */

  return 0;
}

