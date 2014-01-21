#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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

char * escape_string(char * string) {
  int i, new_string_len = 0;
  char * new_string = NULL;

  // counts the number of characters that the new string will have
  for (i=0; string && i<strlen(string); i++) {
    if (string[i] == '"') 
      new_string_len += 2;
    else 
      new_string_len++;
  }

  new_string = (char*) calloc(sizeof(char), (new_string_len + 1));

  // finds '"' within the string and replaces it by '\"'
  for (i=0; string && i<strlen(string); i++) {
    if (string[i] == '"') 
      strcat(new_string, "\\\"");
    else 
      strncat(new_string, &string[i], 1);
  }
  return new_string;
}

int main(int argc , char * argv[]){
  struct abctune * tunes, * tune;
  struct abcsym * sym;
  int n_tunes = 1, debug_flag=1, i;
  char info_type;

  abc_init((void *(*)(int sz)) malloc,
           free, 
           0,
           sizeof (struct SYMBOL) - sizeof (struct abcsym),
           1);

  if(argc == 2) { tunes = abc_parse(fslurp(argv[1])) ; }
  else          { tunes = abc_parse(fslurp("x.abc")) ; }

/* short lengthx[4]; */
/* printf("\n>>>>>>>>>>>>>%d\n",sizeof lengthx[0]); */
  printf("{"); // begin of structure
  printf("deco_tb => {"); // begin of deco_tb
  int o=0, first_deco=1;
  for(o=0; o<128; o++) {
    if (deco_tb[o]) { 
      if (first_deco) { first_deco = 0; }
      else            { printf(","); }
      printf("%d=>\"%s\"", o, deco_tb[o]); 
    }
  }
  printf("},"); // end of deco_tb
  if (debug_flag) printf("\n");

  printf("tunes => {"); // begin of tunes
  if (debug_flag) printf("\n");
  for (tune = tunes; tune != 0; tune = tune->next){

    printf("%d => {", n_tunes); // begin of tune
    if (debug_flag) printf("\n");

    printf("abc_vers=>%d,",tune->abc_vers);  
    if (tune->client_data)
      printf("client_data=>%s,",(char *)tune->client_data);  
    else
      printf("client_data=>\"\",");  
    printf("micro_tb=>[");
    for (i=0; i<MAXMICRO; i++) {
      if (i>0) printf(",");
      printf("%d",tune->micro_tb[i]); 
    }
    printf("],");
    if (debug_flag) printf("\n");
    printf("symbols=>["); // begin of symbols

    for (sym = tune->first_sym; sym != 0; sym = sym->next){
      printf("{"); // begin of symbol

      printf("type=>%d,"        ,sym->type);  
      printf("state=>%d,"       ,sym->state);
      printf("colnum=>%d,"      ,sym->colnum);  
      printf("flags=>%d,"       ,sym->flags);
      printf("linenum=>%d,"     ,sym->linenum);  
      printf("text=>\"%s\","    ,escape_string(sym->text));   
      printf("comment=>\"%s\"," ,escape_string(sym->comment));
      printf("info=>{"); // begin of info

      if (sym->type == 1) { // symbols of type 1 (Info)
        info_type = sym->text[0];
        switch (info_type) {
          case 'K': // Key
            printf("sf=>%d,"      ,sym->u.key.sf);
            printf("empty=>%d,"   ,sym->u.key.empty); 
            printf("exp=>%d,"     ,sym->u.key.exp); 
            printf("mode=>%d,"    ,sym->u.key.mode); 
            printf("nacc=>%d,"    ,sym->u.key.nacc); 
            printf("octave=>%d,"  ,sym->u.key.octave); 
            printf("pits=>[");
            for (i=0; i<8; i++) {
              if (i>0) printf(",");
              printf("%d",sym->u.key.pits[i]); 
            }
            printf("],");
            printf("accs=>[");
            for (i=0; i<8; i++) {
              if (i>0) printf(",");
              printf("%d",sym->u.key.accs[i]); 
            }
            printf("]");
            break;
          case 'L': // Length
            printf("base_length=>%d",sym->u.length.base_length);
            break;
          case 'M': // Meter
            printf("wmeasure=>%d,"  ,sym->u.meter.wmeasure); 
            printf("nmeter=>%d,"    ,sym->u.meter.nmeter); 
            printf("expdur=>%d,"    ,sym->u.meter.expdur); 
            printf("meter=>[");
            for (i=0; i<sym->u.meter.nmeter; i++){
              if (i>0) printf(",");
              printf("{");
              printf("top=>\"%s\"," ,escape_string(sym->u.meter.meter[i].top)); 
              printf("bot=>\"%s\""  ,escape_string(sym->u.meter.meter[i].bot));
              printf("}");
            }
            printf("]");
            break;
          case 'Q': // Tempo
            printf("str1=>\"%s\","  ,escape_string(sym->u.tempo.str1));
            printf("length=>[");
            for (i=0; i<4; i++) {
              if (i>0) printf(",");
              printf("%d",sym->u.tempo.length[i]);
/* printf("\n\n\n\n\tlength:%d\n\n\n\n",sym->u.tempo.length[i]); */
            }
            printf("],");
/* printf("\n\n\n\n\tvalue:%s\n\n\n\n",sym->u.tempo.value); */
            printf("value=>\"%s\"," ,escape_string(sym->u.tempo.value)); 
            printf("str2=>\"%s\""   ,escape_string(sym->u.tempo.str2)); 
            break;
          case 'V': // Voice
/* printf("\n\n\n\n\tvoice_id:%s\n\n\n\n",sym->u.voice.id); */
            printf("id=>\"%s\","    ,escape_string(sym->u.voice.id));
            printf("fname=>\"%s\"," ,escape_string(sym->u.voice.fname));
            printf("nname=>\"%s\"," ,escape_string(sym->u.voice.nname));
            printf("voice=>%d,"     ,sym->u.voice.voice); 
            printf("octave=>%d,"    ,sym->u.voice.octave); 
            printf("scale=>%f,"     ,sym->u.voice.scale); 
            printf("merge=>%d,"     ,sym->u.voice.merge); 
            printf("stem=>%d,"      ,sym->u.voice.stem); 
            printf("gstem=>%d,"     ,sym->u.voice.gstem); 
            printf("dyn=>%d,"       ,sym->u.voice.dyn); 
            printf("lyrics=>%d,"    ,sym->u.voice.lyrics); 
            printf("gchord=>%d"     ,sym->u.voice.gchord); 
            break;
          case 'U': // Symbols Redefition
            printf("symbol=>%d,"    ,sym->u.user.symbol); 
            printf("value=>%d"      ,sym->u.user.value); 
            break;
        }
      } // end of type 1

      if (sym->type == 3) { // symbols of type 3 (Clef)
        printf("name=>\"%s\",",escape_string(sym->u.clef.name)); 
        printf("staffscale=>%f,",sym->u.clef.staffscale); 
        printf("stafflines=>%d,",sym->u.clef.stafflines); 
        printf("type=>%d,",sym->u.clef.type); 
        printf("line=>%d,",sym->u.clef.line); 
        printf("octave=>%d,",sym->u.clef.octave); 
        printf("transpose=>%d,",sym->u.clef.transpose); 
        printf("invis=>%d,",sym->u.clef.invis); 
        printf("check_pitch=>%d",sym->u.clef.check_pitch); 
      } // end of type 3

      if (sym->type == 4 || sym->type == 5) { // symbols of type 4 and 5 (Note, Rest)
        printf("pits=>[");
        for (i=0; i<MAXHD; i++) {
          if (i>0) printf(",");
          printf("%d",sym->u.note.pits[i]); 
        }
        printf("],");
        printf("lens=>[");
        for (i=0; i<MAXHD; i++) {
          if (i>0) printf(",");
          printf("%d",sym->u.note.lens[i]); 
        }
        printf("],");
        printf("accs=>[");
        for (i=0; i<MAXHD; i++) {
          if (i>0) printf(",");
          printf("%d",sym->u.note.accs[i]); 
        }
        printf("],");
        printf("sl1=>[");
        for (i=0; i<MAXHD; i++) {
          if (i>0) printf(",");
          printf("%d",sym->u.note.sl1[i]); 
        }
        printf("],");
        printf("sl2=>[");
        for (i=0; i<MAXHD; i++) {
          if (i>0) printf(",");
          printf("%d",sym->u.note.sl2[i]); 
        }
        printf("],");
        printf("ti1=>[");
        for (i=0; i<MAXHD; i++) {
          if (i>0) printf(",");
          printf("%d",sym->u.note.ti1[i]); 
        }
        printf("],");
        printf("decs=>[");
        for (i=0; i<MAXHD; i++) {
          if (i>0) printf(",");
          printf("%d",sym->u.note.decs[i]); 
        }
        printf("],");
        printf("chlen=>%d,",sym->u.note.chlen); 
        printf("nhd=>%d,",sym->u.note.nhd); 
        printf("slur_st=>%d,",sym->u.note.slur_st); 
        printf("slur_end=>%d,",sym->u.note.slur_end); 
        printf("brhythm=>%d,",sym->u.note.brhythm); 
        printf("dc=>{");
        printf("n=>%d," ,sym->u.note.dc.n); 
        printf("h=>%d," ,sym->u.note.dc.h); 
        printf("s=>%d," ,sym->u.note.dc.s); 
        printf("t=>[");
        for (i=0; i<MAXDC; i++) {
          if (i>0) printf(",");
          printf("%d",sym->u.note.dc.t[i]); 
        }
        printf("]");
        printf("}"); // end of dc
      } // end of type 4, 5

      if (sym->type == 6 || sym->type == 8 || sym->type == 9) { // symbols of type 6, 8, 9 (Bar, Multi-measure Rest (mrest) and Measure Repeat (mrep))
        printf("type=>%d,",sym->u.bar.type); 
        printf("repeat_bar=>%d,",sym->u.bar.repeat_bar); 
        printf("len=>%d,",sym->u.bar.len); 
        printf("dotted=>%d,",sym->u.bar.dotted); 
        printf("dc=>{");
        printf("n=>%d," ,sym->u.bar.dc.n); 
        printf("h=>%d," ,sym->u.bar.dc.h); 
        printf("s=>%d," ,sym->u.bar.dc.s); 
        printf("t=>[");
        for (i=0; i<MAXDC; i++) {
          if (i>0) printf(",");
          printf("%d",sym->u.bar.dc.t[i]); 
        }
        printf("]");
        printf("}");
      } // end of type 6, 8, 9

      if (sym->type == 7) { // symbols of type 7 (End of Line (eoln))
        printf("type=>%d",sym->u.eoln.type); 
      } // end of type 7

      if (sym->type == 10) { // symbols of type 10 (Voice Overlay (v_over))
        printf("type=>%d,",sym->u.v_over.type); 
        printf("voice=>%d",sym->u.v_over.voice); 
      } // end of type 10

      if (sym->type == 11) { // symbols of type 11 (Tuplet)
        printf("p_plet=>%d,",sym->u.tuplet.p_plet); 
        printf("q_plet=>%d,",sym->u.tuplet.q_plet); 
        printf("r_plet=>%d",sym->u.tuplet.r_plet); 
      } // end of type 11

      printf("}"); // end of info

      printf("}"); // end of symbol
      if (sym->next != 0) printf(",");  
      if (debug_flag) printf("\n");
    }

    printf("]"); // end of symbols
    printf("}"); // end of tune
    if (tune->next != 0) printf(",");  
    n_tunes++;
  }
  printf("}");  // end of tunes
  printf("}");  // end of structure

  return 0;
}
