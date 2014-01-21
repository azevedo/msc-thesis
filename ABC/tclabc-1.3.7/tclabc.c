/*++
 * tclabc: Main entry point.
 *
 * Tcl extension for the ABC musical notation.
 *
 * Copyright (C) 1999-2011, Jean-François Moine.
 *
 * Original site: http://moinejf.free.fr/
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 *
 *--*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#ifdef WIN32
#define EXPORT(a,b) __declspec(dllexport) a b
#else
#include <unistd.h>
#define EXPORT(a,b) a b
#endif

//#define TCL_UTF_MAX 6
#include "tcl.h"
#include "abcparse.h"
#include "tclabc.h"
#include "midi.h"

struct voicecontext voice_tb[MAXVOICE];
int nvoice;
struct voicecontext *curvoice;
int goaltime, goalseq;

struct abctune *first_tune;
struct abctune *curtune;

int lyrics_change;

char empty_str[] = "";
Tcl_Obj *empty_obj;
Tcl_Obj *type_obj[EOT + 1];
#ifdef HAVE_SOUND
Tcl_Obj *midiin;
Tcl_Obj *midiout;
#endif

/* (for sym_dump_i) */
static struct sym *s_dump;  /* last dumped symbol */
static int blen[MAXVOICE];
static int cvoice;
static struct abcsym *ly_st;
static int in_grace;
static int brhythm, gbr;
static unsigned short *micro_tb; /* ptr to the microtone table of the tune */

/* %%staves stuff */
static struct STAVES_S {
  struct STAVES_S *next;
  struct sym *sym[MAXVOICE];
} *first_staves, *last_staves;

static char empty_tune[] = {
  "X:1\n"
  "T:noname\n"
  "M:4/4\n"
  "K:C\n"
};
static char sym_fmt[] = "s%d.%d.%d";
static char none[] = "none";

/* key signatures !! cf abcparse.h for index !! */
const char *key_names[11] = {
  "ionian", "dorian", "phrygian", "lydian",
  "mixolydian", "aeolian", "locrian", "major",
  "minor", "HP", "Hp"
};

/* local functions */
static int tclabc(ClientData clientdata,
      Tcl_Interp *interp,
      int objc,
      Tcl_Obj *CONST objv[]);
static void abc_purge(struct abctune *t);
static int channel_get(Tcl_Interp *interp,
           struct sym *s);
static int deco_get(Tcl_Interp *interp,
        struct sym *s);
static void do_rewind(void);
static int header_get(Tcl_Interp *interp);
static struct sym *sym_decode(Tcl_Interp *interp,
        char *p);
static void sym_encode(char *p,
      struct sym *s);
static void lyrics_build(struct abctune *t);
static int lyrics_get(Tcl_Interp *interp,
          struct sym *s);
static char *note_dump(char *p,
           int pitch,
           int acc,
           int len,
           int nostem);
static int program_get(Tcl_Interp *interp,
           struct sym *s);
static int sym_return(Tcl_Interp *interp);
static int slurs_get(Tcl_Interp *interp,
         struct sym *s);
static int staff_get(Tcl_Interp *interp,
         struct sym *s);
static int sym_dump(Tcl_Interp *interp,
        struct sym *s);
static void sym_dump_i(Tcl_Obj *obj,
           struct abcsym *as);
static void sym_link(struct sym *s, int type);
static void tcl_appstr(Tcl_Interp *interp,
      Tcl_Obj *obj,
      char *str);
static int tcl_int_res(Tcl_Interp *interp,
           int v);
static int tclabc_record(Tcl_Interp *interp);
static int ties_get(Tcl_Interp *interp,
        struct sym *s);
static struct sym *time_advance(void);
static int tune_dump(Tcl_Interp *interp);
static int tune_list(Tcl_Interp *interp);
static int voice_get(Tcl_Interp *interp);
static int words_get(Tcl_Interp *interp);

/* -- trace -- */
void trace(char *fmt, ...)
{
  va_list args;
static Tcl_Channel con;
static char tmp[256];

  if (con == 0)
    con = Tcl_GetStdChannel(TCL_STDOUT);
  va_start(args, fmt);
  vsprintf(tmp, fmt, args);
  va_end(args);
  Tcl_WriteChars(con, tmp, strlen(tmp));
}

#if 0
/* malloc/free check */
#define PATTERN 0x7a3489fe
static void *malloc2(size_t l)
{
  int *z;

  z = malloc(sizeof (int) + l);
  *z = PATTERN;
  return (void *) (z + 1);
}
static void free2(void *p)
{
  int *z;

  z = (int *) p - 1;
  if (*z != PATTERN) {
    trace("bad free zone\n");
    exit(1);
  }
  *z = 0;
  free(z);
}
#undef PATTERN
#define malloc malloc2
#define free free2
#endif

/* --- entry point for Tcl load --- */
EXPORT(int, Tclabc_Init)(Tcl_Interp *interp)
{
  int i;
static char *type_tb[EOT + 1] = { /* !! see values in tclabc.h !! */
  "note", "rest", "bar", "clef", "time",
  "key", "tempo", "extra", "midi", "mrest",
  "mrep", "grace", "tuplet",
  "EOT"
};

#if 0 /*debug*/
  {
    char buf[80];

    printf("ready to start");
    fflush(stdout);
    fgets(buf, 80, stdin);
  }
#endif
#ifdef USE_TCL_STUBS
  if (Tcl_InitStubs(interp, "8", 0) == NULL)
    return TCL_ERROR;
#endif
  Tcl_CreateObjCommand(interp,
           "abc",
           tclabc,
           (ClientData) 0,
           (Tcl_CmdDeleteProc *) 0);

  /* create some common string objects */
  empty_obj = Tcl_NewObj();
  Tcl_IncrRefCount(empty_obj);    /* keep it forever */
  for (i = 0; i <= EOT; i++) {
    type_obj[i] = Tcl_NewStringObj(type_tb[i], -1);
    Tcl_IncrRefCount(type_obj[i]);
  }

  /* initialize the ABC parser */
  abc_init((void *(*)(int sz)) malloc,
     free, 0,
     sizeof (struct sym) - sizeof (struct abcsym),
     1);

  /* create an empty tune */
  first_tune = abc_parse(empty_tune);
  tune_select(first_tune);

  return Tcl_PkgProvide(interp, "tclabc", VERSION);
}

EXPORT(int, Tclabc_SafeInit)(Tcl_Interp *interp)
{
  return Tclabc_Init(interp);
}

/* -- ABC main function -- */
static int tclabc(ClientData clientdata,
      Tcl_Interp *interp,
      int objc,
      Tcl_Obj *CONST objv[])
{
  struct sym *s;
  char *p;

  if (objc < 2)
    return tcl_wrong_args(interp, "function ?args?");
  p = Tcl_GetString(objv[1]);
  if (*p == 'd') {
    if (strcmp(p, "delete") == 0) {
      if (objc != 2)
        return tcl_wrong_args(interp, "delete");
      return del_sym(interp);
    }
    if (strcmp(p, "dump") == 0) {
      if (objc == 3) {
        p = Tcl_GetString(objv[2]);
        if (*p == 's') {
          s = sym_decode(interp, p);
          if (s == 0)
            return TCL_ERROR;
          return sym_dump(interp, s);
        }
      }
      if (objc != 2)
        return tcl_wrong_args(interp, "dump ?symbol?");
      return tune_dump(interp);
    }
  }
  if (*p == 'g' && strcmp(p, "get") == 0) {
    if (objc == 2)
      return sym_return(interp);
    if (objc != 3)
      return tcl_wrong_args(interp, "get ?type?");
    s = curvoice->cursym;
    p = Tcl_GetString(objv[2]);
    switch (*p) {
    case 'c':     /* channel */
      return channel_get(interp, s);
    case 'd':     /* deco */
      return deco_get(interp, s);
    case 'f': {     /* flags */
      int flags;

      flags = s->flags;
      if (s->as.flags & ABC_F_SPACE)
        flags |= F_SPACE;
      if (s->type == NOTE || s->type == REST) {
        int i;

        if (s->as.u.note.slur_st != 0)
          flags |= F_SLUR_S;
        if (s->as.u.note.slur_end != 0)
          flags |= F_SLUR_E;
        for (i = 0; i <= s->as.u.note.nhd; i++) {
          if (s->as.u.note.sl1[i] != 0)
            flags |= F_SLUR_S;
          if (s->as.u.note.sl2[i] != 0)
            flags |= F_SLUR_E;
        }
      }
      return tcl_int_res(interp, flags);
    }
    case 'g':       /* gchord */
      if (s->as.text == 0)
        return TCL_OK;
      switch (s->type) {
      case BAR:
        if (s->as.u.bar.repeat_bar)
          break;
      case NOTE:
      case REST:
        sprintf(interp->result, "%s", s->as.text);
        break;
      }
      return TCL_OK;
    case 'h':       /* header */
      return header_get(interp);
    case 'l':       /* lyrics */
      return lyrics_get(interp, s);
    case 'p':       /* program */
      return program_get(interp, s);
    case 's':
      if (p[1] == 'l')    /* slurs */
        return slurs_get(interp, s);
      return staff_get(interp, s);  /* staff */
    case 't':
      if (p[1] == 'i') {
        if (p[2] == 'm')  /* time */
          sprintf(interp->result,
            "%d.%02d", s->time, s->seq);
        else
          return ties_get(interp, s); /* ties */
      } else {      /* tune */
        int i = 0;
        struct abctune *t;

        for (t = first_tune; t != 0; t = t->next) {
          if (t == curtune)
            return tcl_int_res(interp, i);
          i++;
        }
      }
      return TCL_OK;
    case 'v':       /* voice */
      return tcl_int_res(interp, s->voice);
    case 'w':       /* words */
      return words_get(interp);
    }
    strcpy(interp->result,
           "wrong type: should be one of \"channel\",\n"
           "\"deco\", \"flags\", \"gchord\", \"header\"\n"
           "\"lyrics\", \"program\", \"slurs\", \"staff\"\n"
           "\"ties\", \"time\", \"tune\" or \"voice\"");
    return TCL_ERROR;
  }
  if (*p == 'g' && strcmp(p, "go") == 0) {
    if (objc == 2) {
      if (curvoice == 0)
        return TCL_ERROR;
      sym_encode(interp->result, curvoice->cursym);
      return TCL_OK;
    }
    if (objc < 3)
      return tcl_wrong_args(interp, "go ?arg1? ?arg2?");
    p = Tcl_GetString(objv[2]);
    switch (*p) {
    case 's':     /* symbol value */
      s = sym_decode(interp, p);
      if (!s)
        return TCL_ERROR;
      curvoice = &voice_tb[s->voice];
      curvoice->cursym = s;
      goaltime = s->time;
      goalseq = s->seq;
      break;
    case 'n':     /* next */
    case 'p': {     /* previous */
      char *q = 0;

      if (objc == 3) {
        curvoice->cursym = s = *p == 'n'
          ? curvoice->cursym->next
          : curvoice->cursym->prev;
        goaltime = s->time;
        goalseq = s->seq;
        break;
      }
      if (objc == 4)
        q = Tcl_GetString(objv[3]);
      if (objc != 4
       || *p != 'n'   /* only 'next'... */
       || q == 0
       || *q != 't') {  /* ...'time' */
        sprintf(interp->result,
          "unknown displacement type \"%s %s\"",
          p, q ? q : empty_str);
        return TCL_ERROR;
      }
      s = time_advance();
      if (s == 0)
        return TCL_OK;  /* no next symbol */
      goaltime = s->time;
      goalseq = s->seq;
      break;
    }
    default: {      /* voice number */
      int voice;

      voice = atoi(p);
      if ((unsigned) voice > nvoice) {
        sprintf(interp->result,
          "no such voice \"%s\"",
          p);
        return TCL_ERROR;
      }
      s = voice_go(voice);
      curvoice = &voice_tb[voice];
      curvoice->cursym = s;
      break;
        }
    }
    sym_encode(interp->result, s);
    return TCL_OK;
  }
  if (*p == 'i' && strcmp(p, "include") == 0) {
    if (objc != 3)
      return tcl_wrong_args(interp, "include abc-part");
    return sym_include(Tcl_GetString(objv[2]));
  }
  if (*p == 'l') {
    if (strcmp(p, "load") == 0) {
      struct abctune *t;

      if (objc != 3)
        return tcl_wrong_args(interp, "load abc-file");
      p = Tcl_GetString(objv[2]);
      front_init(1, 0);
      p = (char *) frontend((unsigned char *) p, 0);
      t = abc_parse(p);
      free(p);
      if (t == 0)
        return TCL_ERROR;
      tune_purge();
      abc_purge(first_tune);
      first_tune = t;
      tune_select(t);
      return TCL_OK;
    }
    if (strcmp(p, "lyrics") == 0) {
      int ln, maxln, objc2, voice;
      Tcl_Obj *obj, **objv2;
      struct tune_ext *te;

      voice = curvoice - &voice_tb[0];
      if ((te = (struct tune_ext *) curtune->client_data) == 0) {
        lyrics_build(curtune);
        te = (struct tune_ext *) curtune->client_data;
      }
      if (objc == 2) {
        if (!te->some_lyrics)
          return TCL_OK;
        for (maxln = MAXLY; --maxln >= 0; )
          if (te->lyrics[voice][maxln] != 0)
            break;
        if (maxln < 0)
          return TCL_OK;
        obj = Tcl_NewObj();
        Tcl_IncrRefCount(obj);
        for (ln = 0; ln <= maxln; ln++)
          Tcl_ListObjAppendElement(interp, obj,
                te->lyrics[voice][ln]);
        Tcl_SetObjResult(interp, obj);
        Tcl_DecrRefCount(obj);
        return TCL_OK;
      }
      if (objc != 3)
        return tcl_wrong_args(interp, "lyrics ?value?");
      if (objv[2]->typePtr != 0
       && strcmp(objv[2]->typePtr->name, "list") == 0) {
        if (Tcl_ListObjGetElements(interp,
                 objv[2],
                 &objc2, &objv2) != TCL_OK)
          return TCL_ERROR;
      } else {
        objc2 = 1;
        objv2 = (Tcl_Obj **) &objv[2];
        p = Tcl_GetString(*objv2);
        if (strlen(p) == 0)
          objc2 = 0;
      }
      maxln = objc2;
      te->some_lyrics = maxln != 0;
      for (ln = 0; ln < MAXLY; ln++, objv2++) {
        obj = te->lyrics[voice][ln];
        if (ln >= maxln) {
          if (obj == 0)
            continue;
          Tcl_DecrRefCount(obj);
          te->lyrics[voice][ln] = 0;
          continue;
        }
        if (*objv2 == obj)
          continue;
        if (obj != 0) {
          Tcl_DecrRefCount(obj);
        }
        p = Tcl_GetString(*objv2);
        if (strlen(p) == 0)
          continue;
        te->lyrics[voice][ln] = *objv2;
        Tcl_IncrRefCount(*objv2);
      }
      lyrics_change = 1;
      return TCL_OK;
    }
  }
  if (*p == 'm') {
#ifdef HAVE_SOUND
    if (strcmp(p, "midi") == 0)
      return midi_cmd(interp, objc, objv);
    if (strcmp(p, "midiin") == 0
     || strcmp(p, "midiout") == 0) {
      char *q;
      int rc;

      if (objc == 2) {
        Tcl_SetObjResult(interp, p[4] == 'i'
             ? midiin : midiout);
        return TCL_OK;
      }
      if (objc != 3)
        return tcl_wrong_args(interp, "midi[in|out] ?device?");
      q = Tcl_GetString(objv[2]);
      if (p[4] == 'i')
        rc = midi_in_init(q);
      else
        rc = midi_out_init(q);
      if (rc == TCL_OK) {
        Tcl_IncrRefCount(objv[2]);
        if (p[4] == 'i') {
          if (midiin != 0) {
            Tcl_DecrRefCount(midiin);
          }
          midiin = objv[2];
        } else {
          if (midiout != 0) {
            Tcl_DecrRefCount(midiout);
          }
          midiout = objv[2];
        }
      }
      return rc;
    }
#endif
    if (strcmp(p, "midiload") == 0) {
      struct abctune *t;
      int rc, voice;

      if (objc != 3)
        return tcl_wrong_args(interp, "midiload MIDI-file");
      rc = midi_read_file(interp,
              Tcl_GetString(objv[2]));
      if (rc != TCL_OK)
        return rc;

      /* convert midi->abc */
      t = abc_parse(empty_tune);
      tune_purge();
      abc_purge(first_tune);
      first_tune = t;
      midi_load(t);
      tune_select(t);
      for (voice = 0; voice <= nvoice; voice++) {
        curvoice = &voice_tb[voice];
        measure_set();
      }
      curvoice = &voice_tb[0];
      return TCL_OK;
    }
    if (strcmp(p, "midisave") == 0) {
      if (objc != 3)
        return tcl_wrong_args(interp,
                  "midisave MIDI-file");
      return midi_dump(interp, Tcl_GetString(objv[2]));
    }
  }
  if (*p == 'n' && strcmp(p, "new") == 0) {
    struct abctune *t;

    if (objc != 2)
      return tcl_wrong_args(interp, "new");
    t = abc_parse(empty_tune);
    if (t == 0)
      return TCL_ERROR;
    tune_purge();
    abc_purge(first_tune);
    first_tune = t;
    tune_select(t);
    return TCL_OK;
  }
  if (*p == 'p' && strcmp(p, "play") == 0) {
#ifdef HAVE_SOUND
    int n;

    if (objc >= 3) {
      p = Tcl_GetString(objv[2]);
      switch (*p) {
      case 'm': {     /* get/set the mute voices */
        if (objc == 3)
          return tcl_int_res(interp, curvoice->mute);
        if (Tcl_GetIntFromObj(interp, objv[3], &n) != TCL_OK)
          return TCL_ERROR;
        curvoice->mute = n != 0;
        return TCL_OK;
      }
      case 'n':     /* play the current note */
        play_note(curvoice->cursym);
        return TCL_OK;
      case 's':     /* stop playing */
        play_stop();
        return TCL_OK;
      case 't':
        if (p[1] == 'e') {  /* set the default tempo */
          if (Tcl_GetIntFromObj(interp, objv[3],
                    &n) != TCL_OK)
            return TCL_ERROR;
          tempo_set(n);
        } else if (p[1] == 'u') { /* play the tune */
          play_tune(curvoice->cursym);
        } else {
          break;
        }
        return TCL_OK;
      case 'v':   /* get/set the velocity */
        if (objc == 3)
          return tcl_int_res(interp,
                 velocity_set(-1));
        if (Tcl_GetIntFromObj(interp, objv[3], &n) != TCL_OK)
          return TCL_ERROR;
        velocity_set(n);
        return TCL_OK;
      case 'w':   /* tell which symbol is playing */
        if ((s = play_sym()) != 0)
          sym_encode(interp->result, s);
        else
          strcpy(interp->result, none);
        return TCL_OK;
      }
      strcpy(interp->result,
             "wrong play type: should be one of\n"
             "\"mute\", \"note\", \"stop\", \"tune\",\n"
             "\"tempo\", \"velocity\" or \"which\"");
      return TCL_ERROR;
    }
    return tcl_int_res(interp, 1);
#else
    return tcl_int_res(interp, 0);
#endif
  }
  if (*p == 'r' && strcmp(p, "record") == 0) {
    if (objc != 2)
      return tcl_wrong_args(interp, "record");
    return tclabc_record(interp);
  }
  if (*p == 'r' && strcmp(p, "rewind") == 0) {
    if (objc != 2)
      return tcl_wrong_args(interp, "rewind");
    do_rewind();
    sym_encode(interp->result, curvoice->cursym);
    return TCL_OK;
  }
  if ((*p == 's' && strcmp(p, "set") == 0)
   || (*p == 'i' && strcmp(p, "insert") == 0)) {
    int insert, n;
    char *q;

    if (objc != 3 && objc != 4)
      return tcl_wrong_args(interp, "set/insert ?type? value");
    insert = *p == 'i';

    /* if arg2 is a list, it is a symbol value */
    if (objv[2]->typePtr != 0
     && strcmp(objv[2]->typePtr->name, "list") == 0)
      return sym_set(interp, objv[2], insert);

    p = Tcl_GetString(objv[2]);
    if (insert) {
      sprintf(interp->result, "cannot insert '%s'", p);
      return TCL_ERROR;
    }
    s = curvoice->cursym;

    if (*p == 'm') {  /* measure (no value) */
      measure_set();
      return TCL_OK;
    }
    if (objc != 4)
      return tcl_wrong_args(interp, "set type value");
    switch (*p) {
    case 'f':   /* symbol flags */
      if (Tcl_GetIntFromObj(interp, objv[3], &n) != TCL_OK)
        return TCL_ERROR;
      if (n & F_SPACE)
        s->as.flags |= ABC_F_SPACE;
      else
        s->as.flags &= ~ABC_F_SPACE;
      if ((s->type != NOTE && s->type != REST)
       || s->as.u.note.lens[0] >= BASE_LEN / 4) /* if no note flag */
        return TCL_OK;
      word_update(s);
      return TCL_OK;
    case 's':   /* slurs */
    case 't':   /* ties */
      if (*p == 's' && p[1] != 'l')
        break;    /* only slurs */
      if (*p == 't' && (p[1] != 'i' || p[2] != 'e'))
        break;    /* only ties */
      if (s->type != NOTE) {
        sprintf(interp->result,
          "cannot set %s on this symbol",
          p);
        return TCL_ERROR;
      }
      switch (*p) {
      case 's':
        return slurs_set(interp, objv[3], s);
      case 't':
        return ties_set(interp, objv[3], s);
      }
      break;
    case 'd':   /* decorations */
    case 'g':   /* guitar chord */
      switch (s->type) {
      case BAR:
        if (*p == 'g' && s->as.u.bar.repeat_bar)
          goto bad_gch;
      case NOTE:
      case REST:
        break;
      default:
bad_gch:
        sprintf(interp->result,
          "cannot set %s on this symbol",
          p);
        return TCL_ERROR;
      }
      break;
    }

    q = Tcl_GetString(objv[3]);
    switch (*p) {
    case 'd':     /* decorations */
      return deco_set(interp,
          q, s->type == BAR
          ? &s->as.u.bar.dc
          : &s->as.u.note.dc);
    case 'g':     /* gchord */
      str_new(&s->as.text, objv[3]);
      return TCL_OK;
    case 'h':
      return header_set(q);
    case 'w':     /* words */
      return words_set(q);
    }
    strcpy(interp->result,
           "wrong type: should be one of\n"
           "\"deco\", \"flags\", \"gchord\",\n"
           "\"header\", \"measure\", \"slurs\",\n"
           "\"staff\", \"ties\" or \"words\"");
    return TCL_ERROR;
  }
  if (*p == 't' && strcmp(p, "tune") == 0) {
    struct abctune *t;
    int i;

    if (objc == 2)  /* no args: return the tune list */
      return tune_list(interp);
    if (objc != 3)
      return tcl_wrong_args(interp, "tune ?args?");
    p = Tcl_GetString(objv[2]);
    switch (*p) {
    case 'd':     /* delete */
      if (curtune == first_tune) {
        if ((t = curtune->next) == 0)
          t = abc_parse(empty_tune);
        first_tune = t;
      } else {
        for (t = first_tune; t != 0; t = t->next) {
          if (t->next == curtune) {
            t->next = curtune->next;
            if (t->next != 0)
              t = t->next;
            break;
          }
        }
      }
      curtune->next = 0;
      tune_purge();
      abc_purge(curtune);
      tune_select(t);
      return TCL_OK;
    case 'n':     /* new */
      tune_purge();
      t = abc_parse(empty_tune);
      t->next = curtune->next;
      curtune->next = t;
      /*fixme: set the X: to a new value*/
      tune_select(t);
      return TCL_OK;
    }
    for (t = first_tune, i = atoi(p);
         --i >= 0;
         t = t->next) {
      if (t == 0) {
        sprintf(interp->result,
               "not such tune \"%s\"",
               p);
        return TCL_ERROR;
      }
    }
    tune_purge();
    tune_select(t);
    return TCL_OK;
  }
  if (*p == 'v' && strcmp(p, "voice") == 0) {
    if (objc == 2)    /* no args: return the number of voices */
      return tcl_int_res(interp, nvoice);
    p = Tcl_GetString(objv[2]);
    switch (*p) {
    case 'g':     /* get */
      if (objc != 3)
        return tcl_wrong_args(interp, "voice get");
      return voice_get(interp);
    case 'n':     /* new */
      if (objc != 4)
        return tcl_wrong_args(interp,
                  "voice new namelist");
      return voice_new(interp, objv[3]);
    case 's':     /* set */
      if (objc != 4)
        return tcl_wrong_args(interp,
                  "voice set namelist");
      return voice_set(interp, objv[3]);
    }
  }
  sprintf(interp->result, "unknown abc function \"%s\"", p);
  return TCL_ERROR;
}

/* -- purge the abc tunes -- */
static void abc_purge(struct abctune *t)
{
  struct abctune *sav_t;

  sav_t = t;
  while (t != 0) {
    struct tune_ext *te;
    struct abcsym *as;
    struct sym *s;
    int voice, ln;

    if ((te = (struct tune_ext *) t->client_data) != 0) {
      for (voice = MAXVOICE; --voice >= 0; ) {
        for (ln = MAXLY; --ln >= 0; ) {
          Tcl_Obj *o;

          if ((o = te->lyrics[voice][ln]) != 0) {
            Tcl_DecrRefCount(o);
          }
        }
      }
      free(te);
    }
    for (as = t->first_sym; as != 0; as = as->next) {
      switch (as->type) {
      case ABC_T_NOTE:
        s = (struct sym *) as;
        if (s->u.ly != 0)
          free(s->u.ly);
        break;
      }
    }
    t = t->next;
  }
  abc_free(sav_t);
}

/* -- change length when broken rhythm (sym_dump_i) -- */
static int broken_rhythm(int len)
{
  switch (brhythm) {
  case -3: len *= 8; break;
  case -2: len *= 4; break;
  case -1: len *= 2; break;
  case 0: return len;
  case 1: len = len * 2 / 3; break;
  case 2: len = len * 4 / 7; break;
  case 3: len = len * 8 / 15; break;
  }
  if (len % 24 != 0) {
    trace("Broken rhythm error\n");
    len = (len + 12) / 24 * 24;
  }
  return len;
}

/* -- process a header declaration (tune_select) -- */
static void header_process(struct sym *s)
{
  int voice;

  switch (s->as.text[0]) {
  case 'K':
    if (s->as.u.key.empty == 1)
      break;
    if (s->as.state == ABC_S_HEAD)    /* first K: */
      curvoice = &voice_tb[0];  /* switch to 1st voice */
    sym_link(s, KEYSIG);
    break;
  case 'L':
    sym_link(s, EXTRA_SYM);
    s->EXTRA_type = EXTRA_LEN;
    break;
  case 'M':
    sym_link(s, TIMESIG);
    break;
  case 'Q':
    sym_link(s, TEMPO);
    break;
  case 'V':
    voice = s->as.u.voice.voice;
    curvoice = &voice_tb[voice];
    if (voice > nvoice) {
      nvoice = voice;
      if (last_staves != 0
       && last_staves->sym[voice] == 0) {
        struct sym *s2;

        s2 = (struct sym *) malloc(sizeof *s);
        memset(s2, 0, sizeof *s2);
        sym_link(s2, EXTRA_SYM);
        s2->EXTRA_type = EXTRA_STAVES;
        last_staves->sym[voice] = s2;
      }
    }
    if (curvoice->p_voice == 0)
//     && (s->as.state == ABC_S_TUNE
//      || s->as.state == ABC_S_EMBED))
      curvoice->p_voice = s;
    break;
/*  case 'w':
    break;    - treated elsewhere */
/*  case 'P':
    break; */
  case 'T':
    curvoice = &voice_tb[0];
    /*fall thru*/
  default:
    if (s->as.state != ABC_S_TUNE
     && s->as.state != ABC_S_EMBED)
      break;
    sym_link(s, EXTRA_SYM);
    s->EXTRA_type = EXTRA_INFO;
    break;
  }
}

/* -- return the channel definition of the current voice -- */
static int channel_get(Tcl_Interp *interp,
           struct sym *s)
{
  while (s->type != MIDI || s->MIDI_type != MIDI_CHANNEL) {
    s = s->prev;
    if (s->type == EOT) {
      do {
        s = s->next;
        if (s->type == EOT || s->type == NOTE)
          return TCL_OK;  /* no definition */
      } while (s->type != MIDI
         || s->MIDI_type != MIDI_CHANNEL);
      break;
    }
  }
  sym_encode(interp->result, s);
  return TCL_OK;
}

/* -- set the channel array in a %%MIDI channel -- */
int channel_set(struct sym *s)
{
  char *p;
  int channel;
  unsigned channels;

  p = &s->as.text[15];  /* skip '%%MIDI channel ' */
  channels = 0;
  for (;;) {
    while (isspace((unsigned char) *p))
      p++;
    if (*p == '\0')
      break;
    if (!isdigit((unsigned char) *p))
      return TCL_ERROR;
    channel = atoi(p) - 1;
    if ((unsigned) channel >= 32)
      return TCL_ERROR;
    if (channels == 0)
      curvoice->channel = channel;
    channels |= (1 << channel);
    while (isdigit((unsigned char) *p))
      p++;
  }
  s->MIDI_type = MIDI_CHANNEL;
  s->MIDI_channels = channels;
  return TCL_OK;
}

/* -- dump a clef definition -- */
static char *clef_dump(char *p,
           struct abcsym *as)
{
  int clef, clef_line;
  static char *clef_type[4] = {
    "treble",
    "alto",
    "bass",
    "perc"
  };

  if ((clef = as->u.clef.type) >= 0) {
    clef_line = as->u.clef.line;
    switch (clef) {
    case TREBLE:
    case PERC:
      if (clef_line == 2)
        clef_line = 0;
      break;
    case ALTO:
      if (clef_line == 3)
        clef_line = 0;
      break;
    case BASS:
      if (clef_line == 4)
        clef_line = 0;
      break;
    }
    if (as->u.clef.name != 0)
      p += sprintf(p, " clef=\"%s\"", as->u.clef.name);
    else if (clef_line == 0)
      p += sprintf(p, " clef=%s",
             as->u.clef.invis ? none : clef_type[clef]);
    else
      p += sprintf(p, " clef=%s%d",
             clef_type[clef], clef_line);
    if (as->u.clef.octave != 0)
      p += sprintf(p, "%c8", as->u.clef.octave > 0 ? '+' : '-');
  }
  if (as->u.clef.stafflines >= 0)
    p += sprintf(p, " stafflines=%d", as->u.clef.stafflines);
  if (as->u.clef.staffscale != 0)
    p += sprintf(p, " staffscale=%.2f", as->u.clef.staffscale);
  return p;
}

/* dump the decorations (tune_dump) */
static char *deco_dump(struct deco *dc,
      char *p)
{
  unsigned char deco;
  int i;

  for (i = 0; i < dc->n; i++) {
    if (i >= dc->h && i < dc->s)
      continue; /* skip the head decorations */
    deco = dc->t[i];
    if (deco < 128) {
      if (deco != 0)
        *p++ = deco;
    } else {
      p += sprintf(p, "!%s!", deco_tb[deco - 128]);
    }
  }
  return p;
}

/* -- return the decorations -- */
static int deco_get(Tcl_Interp *interp,
        struct sym *s)
{
  Tcl_Obj *obj;
  struct deco *dc;
  int i;

  switch (s->type) {
  case NOTE:
  case REST:
    dc = &s->as.u.note.dc;
    break;
  case BAR:
    dc = &s->as.u.bar.dc;
    break;
  default:
    return TCL_OK;
  }

  obj = Tcl_NewObj();
  Tcl_IncrRefCount(obj);
  for (i = 0; i < dc->n; i++) {
    unsigned char deco;
    char buf[32];

    deco = dc->t[i];
    if (deco < 128) {
      if (deco == 0)
        continue;
      buf[0] = deco;
      buf[1] = '\0';
    } else {
      sprintf(buf, "!%s!",
        deco_tb[deco - 128]);
    }
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewStringObj(buf,
                strlen(buf)));
  }
  Tcl_SetObjResult(interp, obj);
  Tcl_DecrRefCount(obj);
  return TCL_OK;
}

/* -- rewind the tune -- */
static void do_rewind(void)
{
  int v;
  struct voicecontext *p_voice;

  for (v = 0, p_voice = &voice_tb[0]; v <= nvoice; v++, p_voice++)
    p_voice->cursym = p_voice->eot->next;
  curvoice = &voice_tb[0];
  goaltime = 0;
  goalseq = 0;
}

/* -- create the End Of Tune of a voice -- */
void eot_create(void)
{
  struct sym *s;

  s = (struct sym *) malloc(sizeof *s);
  memset(s, 0, sizeof *s);
  s->type = EOT;
  curvoice->eot = curvoice->cursym = s;
  s->next = s->prev = s;
  s->voice = curvoice - &voice_tb[0];
}

/* -- return a 'up' / 'down' / auto' parameter value -- */
static char *head_par(int v)
{
  if (v < 0)
    return "down";
  if (v == 2)
    return "auto";
  return "up";
}

/* -- dump a header -- */
static char *header_dump(char *p,
       struct abcsym *as)
{
  int i;
  static char key_tonic[] = "FCGDAEB";
  static char key_shift[7 + 2] = {1, 3, 5, 0, 2, 4, 6, 1, 4};

  switch (as->text[0]) {
  case 'K':
    *p++ = 'K';
    *p++ = ':';
    if (as->u.key.empty) {
      if (as->u.key.empty == 2) {
        strcpy(p, none);
        p += 4;
      }
    } else {
      if (as->u.key.mode < BAGPIPE) {
/*  ion dor phr lyd mix aeo loc
7   C#  D#  E#  F#  G#  A#  B#
6   F#  G#  A#  B   C#  D#  E#
5   B   C#  D#  E   F#  G#  A#
4   E   F#  G#  A   B   C#  D#
3   A   B   C#  D   E   F#  G#
2   D   E   F#  G   A   B   C#
1   G   A   B   C   D   E   F#
0   C   D   E   F   G   A   B
-1  F   G   A   Bb  C   D   E
-2  Bb  C   D   Eb  F   G   A
-3  Eb  F   G   Ab  Bb  C   D
-4  Ab  Bb  C   Db  Eb  F   G
-5  Db  Eb  F   Gb  Ab  Bb  C
-6  Gb  Ab  Bb  Cb  Db  Eb  F
-7  Cb  Db  Eb  Fb  Gb  Ab  Bb
*/
        i = as->u.key.sf
          + key_shift[(unsigned char) as->u.key.mode];
        *p++ = key_tonic[(unsigned char) (i + 7) % 7];
        if (i < 0)
          *p++ = 'b';
        else if (i >= 7)
          *p++ = '#';
      }
      if (as->u.key.mode != MAJOR)
        p += sprintf(p, "%.3s",
          key_names[(unsigned char) as->u.key.mode]);
      if (as->u.key.nacc != 0) {
        if (as->u.key.exp) {
          strcpy(p, " exp ");
          p += 5;
        } else {
          *p++ = ' ';
        }
        if (as->u.key.nacc < 0) {
          strcpy(p, none);
          p += 4;
        } else {
            for (i = 0; i < as->u.key.nacc; i++) {
          p = note_dump(p,
                  as->u.key.pits[i],
                  as->u.key.accs[i],
                  blen[cvoice] != 0
              ? blen[cvoice]
              : BASE_LEN / 8,
                  0);
            }
        }
      }
    }
    /*fixme: only if forced?*/
    if (as->next != 0
     && as->next->type == ABC_T_CLEF)
      p = clef_dump(p, as->next);
    break;
  case 'L':
    if (as->state == ABC_S_GLOBAL
     || as->state == ABC_S_HEAD) {
      for (i = MAXVOICE; --i >= 0; )
        blen[i] = as->u.length.base_length;
    } else {
      blen[cvoice] = as->u.length.base_length;
    }
    p += sprintf(p, "L:1/%d", BASE_LEN / blen[cvoice]);
    break;
  case 'M':
    *p++ = 'M';
    *p++ = ':';
    if (as->u.meter.nmeter == 0)
      p += sprintf(p, none);
    else for (i = 0; i < as->u.meter.nmeter; i++) {
      if (i > 0 && isdigit(as->u.meter.meter[i].top[0])
       && isdigit(p[-1]))
        *p++ = ' ';
      p += sprintf(p, "%.8s", as->u.meter.meter[i].top);
      if (as->u.meter.meter[i].bot[0] != '\0')
        p += sprintf(p, "/%.2s", as->u.meter.meter[i].bot);
    }
    if (blen[cvoice] == 0) {
      int ulen;

      if (as->u.meter.wmeasure >= BASE_LEN * 3 / 4
       || as->u.meter.wmeasure == 0)
        ulen = BASE_LEN / 8;
      else
        ulen = BASE_LEN / 16;
      if (as->state == ABC_S_GLOBAL
       || as->state == ABC_S_HEAD)
        for (i = MAXVOICE; --i >= 0; )
          blen[i] = ulen;
      else
        blen[cvoice] = ulen;
    }
    break;
  case 'Q':
    *p++ = 'Q';
    *p++ = ':';
    if (as->u.tempo.str1 != 0
     && as->u.tempo.str1[0] != '\0')
      p += sprintf(p, "\"%s\" ", as->u.tempo.str1);
    if (as->u.tempo.value != 0) {
      int top, bot;

      for (i = 0;
           i < sizeof as->u.tempo.length
        / sizeof as->u.tempo.length[0];
           i++) {
        if ((top = as->u.tempo.length[i]) == 0)
          continue;
        bot = 1;
        for (;;) {
          if (top % BASE_LEN == 0) {
            top /= BASE_LEN;
            break;
          }
          top *= 2;
          bot *= 2;
        }
        p += sprintf(p, "%d/%d ", top, bot);
      }
      if (p[-1] == ' ')
        p--;
      p += sprintf(p, "=%s ", as->u.tempo.value);
    }
    if (as->u.tempo.str2 != 0
     && as->u.tempo.str2[0] != '\0')
      p += sprintf(p, "\"%s\"", as->u.tempo.str2);
    else if (p[-1] == ' ')
      p--;
    break;
  case 'V':
    p += sprintf(p, "V:%s", as->u.voice.id);
    if (as->u.voice.fname != 0)
      p += sprintf(p, " name=\"%s\"", as->u.voice.fname);
    if (as->u.voice.nname != 0)
      p += sprintf(p, " subname=\"%s\"", as->u.voice.nname);
    if (as->u.voice.merge != 0)
      p += sprintf(p, " merge");
    if (as->u.voice.stem != 0)
      p += sprintf(p, " stem=%s", head_par(as->u.voice.stem));
    if (as->u.voice.gstem != 0)
      p += sprintf(p, " gstem=%s",
             head_par(as->u.voice.gstem));
    if (as->u.voice.dyn != 0)
      p += sprintf(p, " dyn=%s", head_par(as->u.voice.dyn));
    if (as->u.voice.lyrics != 0)
      p += sprintf(p, " lyrics=%s",
             head_par(as->u.voice.lyrics));
    if (as->u.voice.gchord != 0)
      p += sprintf(p, " gchord=%s",
             head_par(as->u.voice.gchord));
    if (as->u.voice.scale != 0)
      p += sprintf(p, " scale=%.2f", as->u.voice.scale);
    if (as->state == ABC_S_TUNE
     || as->state == ABC_S_EMBED)
      cvoice = as->u.voice.voice;
    if (as->next != 0
     && as->next->type == ABC_T_CLEF)
      p = clef_dump(p, as->next);
    break;
  case 'd':
  case 's':       /* decoration line */
    *p++ = '%';     /*fixme: already in notes*/
    /*fall thru*/
  default:
    /*fixme: may overflow*/
    strcpy(p, as->text);
    p += strlen(p);
    break;
  }
  return p;
}

/* -- return the header -- */
static int header_get(Tcl_Interp *interp)
{
  struct abcsym *as;
  Tcl_Obj *obj;
  char buffer[256];
  char *p;

  obj = Tcl_NewObj();
  Tcl_IncrRefCount(obj);
  for (as = curtune->first_sym; as != 0; as = as->next) {
    if (as->state != ABC_S_GLOBAL
     && as->state != ABC_S_HEAD)
      break;
    if (as->text) {
      if (as->type == ABC_T_INFO) {
        p = header_dump(buffer, as);
        *p = '\0';
        p = buffer;
      } else {
        p = as->text;
      }
      Tcl_AppendToObj(obj, p, strlen(p));
      if (as->comment)
        Tcl_AppendToObj(obj, "\t", 1);
    }
    if (as->comment)
      Tcl_AppendStringsToObj(obj, "%", as->comment, 0);
    Tcl_AppendToObj(obj, "\n", 1);
  }
  Tcl_SetObjResult(interp, obj);
  Tcl_DecrRefCount(obj);
  return TCL_OK;
}

/* -- dump the guitar chords / annotations (tune_dump) -- */
static char *gchord_dump(char *p,
       char *s)
{
  char *q;

  while ((q = strchr(s, '\n')) != 0) {
    p += sprintf(p, "\"%.*s\"", (int) (q - s), s);
    s = q + 1;
  }
  p += sprintf(p, "\"%s\"", s);
  return p;
}

/* -- dump the note/rest length (tune_dump) -- */
static char *length_dump(char *p,
       int len)
{
  int div = 0;

  if (blen[cvoice] == 0)
    blen[cvoice] = BASE_LEN / 8;
  for (;;) {
    if (len % blen[cvoice] == 0) {
      len /= blen[cvoice];
      if (len != 1)
        p += sprintf(p, "%d", len);
      break;
    }
    len *= 2;
    div++;
  }
  while (--div >= 0)
    *p++ = '/';
  return p;
}

/* -- build the lyrics of a tune -- */
static void lyrics_build(struct abctune *t)
{
  struct {
    short nnote, nbar;
    short cnnote, cnbar;
  } ly_tb[MAXVOICE][MAXLY];
  struct abcsym *as;
  int voice, ln, i, l, in_ly;
  struct tune_ext *te;
  Tcl_Obj *o;

  if ((te = (struct tune_ext *) t->client_data) == 0) {
    te = (struct tune_ext *) malloc(sizeof *te);
    memset(te, 0, sizeof *te);
    t->client_data = (void *) te;
  }
  memset(ly_tb, 0, sizeof ly_tb);
  voice = ln = 0;
  in_ly = 0;
  for (as = t->first_sym; as != 0; as = as->next) {
    switch (as->type) {
    case ABC_T_INFO:
      switch (as->text[0]) {
      case 'T':
        voice = 0;
        continue;
      case 'V':
        voice = as->u.voice.voice;
        continue;
      case 'w':
        break;
      default:
        continue;
      }
      break;
    case ABC_T_PSCOM:
      if (strncmp(&as->text[2], "staves", 6) == 0
       || strncmp(&as->text[2], "score", 5) == 0) {
        voice = 0;
        continue;
      }
      if (in_ly
       && strncmp(&as->text[2], "vocalfont ", 10) == 0)
        break;
      continue;
    case ABC_T_BAR:
      if (as->u.bar.type == B_OBRA)
        continue;
      for (i = MAXLY; --i >= 0; ) {
        ly_tb[voice][i].cnbar++;
        ly_tb[voice][i].cnnote = 0;
        ly_tb[voice][i].nnote = 0;
      }
      continue;
    case ABC_T_NOTE:
    case ABC_T_REST:
      if (as->flags & ABC_F_GRACE)
        continue;
      if (as->flags & ABC_F_LYRIC_START) {
        for (i = MAXLY; --i >= 0; ) {
          ly_tb[voice][i].nnote += 
            ly_tb[voice][i].cnnote;
          ly_tb[voice][i].cnnote = 0;
          ly_tb[voice][i].nbar += 
            ly_tb[voice][i].cnbar;
          ly_tb[voice][i].cnbar = 0;
        }
        ln = 0;
      }
      if (as->type != ABC_T_NOTE)
        continue;
      for (i = MAXLY; --i >= 0; )
        ly_tb[voice][i].cnnote++;
      continue;
    default:
      continue;
    }

    /* we are here only at 'w:' and '%%vocalfont' inside a lyric */
    if ((o = te->lyrics[voice][ln]) == 0) {
      o = Tcl_NewObj();
      Tcl_IncrRefCount(o);
      te->lyrics[voice][ln] = o;
      te->some_lyrics = 1;
    }
    if ((i = ly_tb[voice][ln].nbar) > 0) {
      while (--i >= 0)
        Tcl_AppendToObj(o, "|", 1);
      ly_tb[voice][ln].nbar = 0;
    }
    ly_tb[voice][ln].cnbar = 0;
    if ((i = ly_tb[voice][ln].nnote) > 0) {
      while (--i >= 0)
        Tcl_AppendToObj(o, "*", 1);
      ly_tb[voice][ln].nnote = 0;
    }
    ly_tb[voice][ln].cnnote = 0;
    l = strlen(as->text);
    if (as->type == ABC_T_INFO) { /* w: */
      if (as->text[l - 1] != '\\') {
        ln++;
        in_ly = 0;
      } else {
        l--;
        in_ly = 1;
      }
      Tcl_AppendToObj(o, &as->text[2], l - 2);
      Tcl_AppendToObj(o," ", 1);
    } else {      /* %%vocalfont */
      Tcl_AppendToObj(o, "[", 1);
      Tcl_AppendToObj(o, as->text, l);
      Tcl_AppendToObj(o, "]", 1);
    }

    /* remove from the tune */
    as = as->prev;
    abc_delete(as->next);
  }
  lyrics_change = te->some_lyrics;
}

/* -- dispatch the lyrics on the notes -- */
static void lyrics_dispatch(struct abctune *t)
{
  struct tune_ext *te;
  char *ly_tb[MAXVOICE][MAXLY];
  struct abcsym *as;
  int voice, ln;
  char *p;

  lyrics_change = 0;
  te = (struct tune_ext *) t->client_data;
  if (!te->some_lyrics)
    return;
  memset(ly_tb, 0, sizeof ly_tb);
  for (voice = 0; voice < MAXVOICE; voice++) {
    for (ln = 0; ln < MAXLY; ln++) {
      Tcl_Obj *o;

      o = te->lyrics[voice][ln];
      if (o != 0)
        ly_tb[voice][ln] = Tcl_GetString(o);
    }
  }
  voice = 0;
  for (as = t->first_sym; as != 0; as = as->next) {
    struct sym *s;

    switch (as->type) {
    case ABC_T_INFO:
      switch (as->text[0]) {
      case 'T':
        voice = 0;
        break;
      case 'V':
        voice = as->u.voice.voice;
        break;
      }
      break;
    case ABC_T_PSCOM:
      if (strncmp(&as->text[2], "staves", 6) == 0
       || strncmp(&as->text[2], "score", 5) == 0)
        voice = 0;
      break;
    case ABC_T_BAR:
      if (as->u.bar.type == B_OBRA)
        break;
      for (ln = 0; ln < MAXLY; ln++) {
        p = ly_tb[voice][ln];
        if (p == 0)
          continue;
        while (*p == ' '
            || *p == '\t')
          p++;
        if (*p == '|')
          ly_tb[voice][ln] = ++p;
      }
      break;
    case ABC_T_NOTE:
      if (as->flags & ABC_F_GRACE)
        break;
      s = (struct sym *) as;
      for (ln = 0; ln < MAXLY; ln++) {
        char *start;

        if (s->u.ly != 0)
          s->u.ly->start[ln] = 0;
        p = ly_tb[voice][ln];
        if (p == 0)
          continue;
        while (*p == ' '
            || *p == '\t')
          p++;
        if (*p == '\0')
          continue;
        start = p;
        switch (*p) {
        case '|':
          continue;
        case '*':
          ly_tb[voice][ln] = ++p;
          continue;
        case '_':
        case '-':
          p++;
          break;
        default:
          for (;;) {
            char c;
    
            c = *p;
            switch (c) {
            case '\0':
            case ' ':
            case '\t':
            case '*':
            case '|':
            case '_':
              break;
            case '-':
              p++;
              break;
            case '[':
              p++;
              while (*p != ']'
                  && *p != '\0')
                p++;
              if (*p == ']')
                p++;
              continue;
            case '\\':
              if (p[1] == '\0')
                break;
              p++;
            default:
              p++;
              continue;
            }
            break;
          }
          break;
        }

        /* store the word in the note */
        if (s->u.ly == 0) {
          s->u.ly = (struct lyrics *) malloc(sizeof (struct lyrics));
          memset(s->u.ly, 0, sizeof (struct lyrics));
        }
        s->u.ly->start[ln] = start;
        s->u.ly->stop[ln] = p;
        ly_tb[voice][ln] = p;
      }
    }
  }
}

/* -- dump the lyrics -- */
static void lyrics_dump(Tcl_Obj *obj,
      struct abcsym *as2)
{
  struct abcsym *as, *as1;
  struct sym *s;
  int i, maxly;

  /* count the number of lyric lines */
  if ((as1 = ly_st) == 0)
    return;
  ly_st = 0;
  maxly = -1;
  for (as = as1; as != 0; as = as->next) {
    struct lyrics *ly;

    s = (struct sym *) as;
    if (as->type == ABC_T_NOTE
     && (ly = s->u.ly) != 0) {
      for (i = MAXLY; --i >= 0; )
        if (ly->start[i] != 0)
          break;
      if (i > maxly)
        maxly = i;
    }
    if (as == as2)
      break;
  }
  if (maxly < 0)
    return;

  /* output each line */
  for (i = 0; i <= maxly; i++) {
    int put_space;

    Tcl_AppendToObj(obj, "w:", 2);
    put_space = 0;
    for (as = as1; as != 0; as = as->next) {
      struct lyrics *ly;
      char *p;

      s = (struct sym *) as;
      if (as->type != ABC_T_NOTE
       || (as->flags & ABC_F_GRACE)) {
#if 1
        ;
#else
        if (as->type == ABC_T_BAR
         && as->u.bar.type != B_OBRA) {
          Tcl_AppendToObj(obj, "|", 1);
          put_space = 0;
        }
#endif
      } else if ((ly = s->u.ly) != 0
        && (p = ly->start[i]) != 0) {
        int j;

        /* insert the font changes */
        if (*p == '[') {
          char *q;

          Tcl_AppendToObj(obj, "\\\n", 2);
          q = ++p;
          while (*p != ']'
              && p < ly->stop[i])
            p++;
          Tcl_AppendToObj(obj, q, p - q);
          put_space = 0;
          Tcl_AppendToObj(obj, "\nw:", 3);
          p++;
        }

        /* add the word */
        if (put_space && *p != '_')
          Tcl_AppendToObj(obj, " ", 1);
        j = ly->stop[i] - p;
        Tcl_AppendToObj(obj, p, j);
        if (*p == '_'
         || (ly->stop[i][-1] == '-'
          && ly->stop[i][-2] != '\\'))
          put_space = 0;
        else
          put_space = 1;
      } else {
        Tcl_AppendToObj(obj, "*", 1);
        put_space = 0;
      }
      if (as == as2)
        break;
    }
    Tcl_AppendToObj(obj, "\n", 1);
  }
}

/* -- return the lyrics of a note -- */
static int lyrics_get(Tcl_Interp *interp,
          struct sym *s)
{
  struct lyrics *ly;
  Tcl_Obj *obj;
  int i, j;
  struct tune_ext *te;

  if (s->as.type != ABC_T_NOTE)
    return TCL_OK;

  /* build and update the lyrics if not done yet */
  if ((te = (struct tune_ext *) s->as.tune->client_data) == 0) {
    lyrics_build(s->as.tune);
    te = (struct tune_ext *) s->as.tune->client_data;
  }
  if (lyrics_change)
    lyrics_dispatch(s->as.tune);
  if (!te->some_lyrics
   || (ly = s->u.ly) == 0)
    return TCL_OK;

  obj = Tcl_NewObj();
  Tcl_IncrRefCount(obj);
  for (j = MAXLY; --j >= 0; ) /* count the max number of lines */
    if (ly->start[j] != 0)
      break;
  for (i = 0; i <= j; i++) {
    Tcl_Obj *obj2;
    char *p;

    if ((p = ly->start[i]) != 0)
      obj2 = Tcl_NewStringObj(p, ly->stop[i] - ly->start[i]);
    else
      obj2 = empty_obj;
    Tcl_ListObjAppendElement(interp, obj, obj2);
  }
  Tcl_SetObjResult(interp, obj);
  Tcl_DecrRefCount(obj);
  return TCL_OK;
}

/* -- dump a note (tune_dump) -- */
static char *note_dump(char *p,
           int pitch,
           int acc,
           int len,
           int nostem)
{
  int i, j;

  switch (acc & 0x07) {
  case A_DS:
    *p++ = '^';
  case A_SH:
    *p++ = '^';
    break;
  case A_DF:
    *p++ = '_';
  case A_FT:
    *p++ = '_';
    break;
  case A_NT:
    *p++ = '=';
    break;
  }
  acc >>= 3;
  if (acc != 0) {
    int n, d;

    n = micro_tb[acc] >> 8;
    d = micro_tb[acc] & 0xff;
    if (n != 0)
      p += sprintf(p, "%d", n + 1);
    if (d != 0) {
      *p++ = '/';
      if (d != 1)
        p += sprintf(p, "%d", d + 1);
    }
  }

  i = pitch;

  if (i >= 23) {
    *p++ = 'a' + (i - 23 + 2) % 7;
    j = (i - 23) / 7;
    while (--j >= 0)
      *p++ = '\'';
  } else {
    *p++ = 'A' + (i + 49) % 7;
    j = (22 - i) / 7;
    while (--j >= 0)
      *p++ = ',';
  }

  /* note length */
  if (nostem)
    *p++ = '0';
  return length_dump(p, len);
}

/* abc parser: pseudo comment */
static void process_pscomment(struct sym *s)
{
  if (strncmp(&s->as.text[2], "staves", 6) == 0
   || strncmp(&s->as.text[2], "score", 5) == 0) {
//    int voice;
    struct STAVES_S *staves;

    staves = malloc(sizeof *staves);
    memset(staves, 0, sizeof *staves);
    if (last_staves == 0)
      first_staves = staves;
    else
      last_staves->next = staves;
    last_staves = staves;

    curvoice = &voice_tb[0];
    sym_link(s, EXTRA_SYM);
    s->EXTRA_type = EXTRA_STAVES;
    staves->sym[0] = s;
#if 0
    for (voice = 0; voice < MAXVOICE; voice++) {
      if (s->as.u.staves[voice].voice < 0)
        break;
      if (s->as.u.staves[voice].voice > nvoice)
        nvoice = s->as.u.staves[voice].voice;
    }

    /* have empty staves symbols for voice synchronization */
    for (voice = 1; voice <= nvoice; voice++) {
      if (voice_tb[voice].vover)
        continue;
      curvoice = &voice_tb[voice];
      s = (struct sym *) malloc(sizeof *s);
      memset(s, 0, sizeof *s);
      sym_link(s, EXTRA_SYM);
      s->EXTRA_type = EXTRA_STAVES;
      staves->sym[voice] = s;
    }
#endif
    curvoice = &voice_tb[0];
    return;
  }
  if (strncmp(&s->as.text[2], "MIDI ", 5) == 0) {
    sym_link(s, MIDI);
    if (strncmp(&s->as.text[7], "channel ", 8) == 0)
      channel_set(s);
    else if (strncmp(&s->as.text[7], "program ", 8) == 0)
      program_set(s);
    return;
  }
  if (s->as.state != ABC_S_TUNE
   && s->as.state != ABC_S_EMBED)
      return;
  sym_link(s, EXTRA_SYM);
  s->EXTRA_type = EXTRA_INFO;
}

/* -- return the definition of all channels -- */
static int program_get(Tcl_Interp *interp,
           struct sym *s)
{
  struct sym *prog[32];
  int time, voice, channel;
  Tcl_Obj *obj;

  /* must scan all the tune up to the current time (and more..) */
  time = s->time;
  memset(prog, 0, sizeof prog);

  for (voice = 0; voice <= nvoice; voice++) {
    for (s = voice_tb[voice].eot->next;
         ;
         s = s->next) {
      if (s->type == EOT)
        break;
      if (s->type == MIDI
       && s->MIDI_type == MIDI_PROGRAM) {
        channel = s->MIDI_channel;
        prog[channel] = s;
      } else if (s->time > time && s->type == NOTE)
        break;
    }
  }

  /* return the symbols */
  obj = Tcl_NewObj();
  Tcl_IncrRefCount(obj);
  for (channel = 0; channel < 32; channel++) {
    if (prog[channel] != 0) {
      char buf[32];

      sym_encode(buf, prog[channel]);
      Tcl_ListObjAppendElement(interp, obj,
             Tcl_NewStringObj(buf,
                  strlen(buf)));
    } else {
      Tcl_ListObjAppendElement(interp, obj, empty_obj);
    }
  }
  Tcl_SetObjResult(interp, obj);
  Tcl_DecrRefCount(obj);
  return TCL_OK;
}

/* -- set the program arguments in a %%MIDI program -- */
int program_set(struct sym *s)
{
  char *p;
  int channel, prog, bank;

  p = &s->as.text[15];  /* skip '%%MIDI program ' */

  if (sscanf(p, "%d %d-%d", &channel, &bank, &prog) == 3) {
    ;
  } else if (sscanf(p, "%d %d %d", &channel, &bank, &prog) == 3) {
    ;
  } else if (sscanf(p, "%d-%d", &bank, &prog) == 2) {
    channel = curvoice->channel + 1;
  } else if (sscanf(p, "%d %d", &channel, &prog) == 2) {
    bank = 0;
  } else if (sscanf(p, "%d", &prog) == 1) {
    channel = curvoice->channel + 1;
    bank = 0;
  } else {
    return TCL_ERROR;
  }
  s->MIDI_type = MIDI_PROGRAM;
  s->MIDI_channel = channel - 1;
  s->MIDI_program = prog;
  s->MIDI_bank = bank;
  return TCL_OK;
}

/* -- change the length of the notes/rests under a tuplet -- */
/*fixme: treat the nested tuplets*/
void set_tuplet(struct sym *s)
{
  struct sym *t;
  int l, r, lplet;

  l = 0;
  r = s->as.u.tuplet.r_plet;
  t = s->next;
  while (t->type != NOTE && t->type != REST) {
    if (t->type == EOT)
      return;
    t = t->next;
  }
  t->flags |= F_NPLET_S;
  for (;;) {
    if (t->type == EOT)
      return;
    if (t->as.u.note.lens[0] != 0
     && (t->type == NOTE || t->type == REST)) {
      l += t->as.u.note.lens[0];
      if (--r <= 0)
        break;
    }
    t = t->next;
  }

  lplet = (l * s->as.u.tuplet.q_plet) / s->as.u.tuplet.p_plet;
  r = s->as.u.tuplet.r_plet;
  for (;;) {
    s = s->next;
    if (s->as.u.note.lens[0] != 0
     && (s->type == NOTE || s->type == REST)) {
      s->dtime = (s->as.u.note.lens[0] * lplet) / l;
      s->flags &= ~F_NPLET_E;
      s->flags |= F_NPLET;
      if (--r <= 0) {
        s->flags |= F_NPLET_E;
        break;
      }
      l -= s->as.u.note.lens[0];
      lplet -= s->dtime;
    }
  }
}

/* -- dump the slurs -- */
static char *slur_dump(char *p, int sl)
{
  do {
    if (sl & SL_DOTTED)
      *p++ = '.';
    *p++ = '(';
    switch (sl & 0x03) {
    case SL_ABOVE:
      *p++ = '\'';
      break;
    case SL_BELOW:
      *p++ = ',';
      break;
    }
    sl >>= 3;
  } while (sl != 0);
  return p;
}

/* -- return the slurs -- */
static int slurs_get(Tcl_Interp *interp,
         struct sym *s)
{
  Tcl_Obj *obj;
  int i;

  if (s->as.type != ABC_T_NOTE)
    return TCL_OK;
  if (s->as.u.note.slur_st == 0
   && s->as.u.note.slur_end == 0) {
    for (i = s->as.u.note.nhd; i >= 0; i--)
      if (s->as.u.note.sl1[i]
       || s->as.u.note.sl2[i])
        break;
    if (i < 0)
      return TCL_OK;    /* no slur at all */
  }
  obj = Tcl_NewObj();
  Tcl_IncrRefCount(obj);
  Tcl_ListObjAppendElement(interp, obj,
         Tcl_NewIntObj(s->as.u.note.slur_st));
  Tcl_ListObjAppendElement(interp, obj,
         Tcl_NewIntObj(s->as.u.note.slur_end));
  for (i = 0; i <= s->as.u.note.nhd; i++) {
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.note.sl1[i]));
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.note.sl2[i]));
  }
  Tcl_SetObjResult(interp, obj);
  Tcl_DecrRefCount(obj);
  return TCL_OK;
}

/* -- return the clef, keysig and timesig symbols -- */
static int staff_get(Tcl_Interp *interp,
         struct sym *s)
{
  struct sym *p_clef, *p_key, *p_time;
  char *p;
  int fl;

  /* search the symbols */
  p_clef = p_key = p_time = 0;  /* (have gcc happy) */
  fl = 7;
  for (; ; s = s->prev) {
    switch (s->type) {
    default:
      continue;
    case CLEF:
      if (!(fl & 1))
        continue;
      p_clef = s;
      fl &= ~1;
      break;
    case KEYSIG:
      if (!(fl & 2))
        continue;
      p_key = s;
      fl &= ~2;
      break;
    case TIMESIG:
      if (!(fl & 4))
        continue;
      p_time = s;
      fl &= ~4;
      break;
    }
    if (fl == 0)
      break;
  }
/*fixme: should be staff#*/
  p = interp->result;
  p += sprintf(p, "%d ", s->voice);
  sym_encode(p,  p_clef);
  p += strlen(p);
  *p++ = ' ';
  sym_encode(p,  p_key);
  p += strlen(p);
  *p++ = ' ';
  sym_encode(p,  p_time);
  return TCL_OK;
}

/* -- update the symbols after %%staves -- */
void staves_update(struct sym *s) /* points to %%staves */
{
  int voice, mxtime;
  struct STAVES_S *staves;
  struct sym *s_staves;

  /* loop on next %%staves */
  for (;;) {

    /* search the staves definition */
    voice = s->voice;
    for (staves = first_staves; staves; staves = staves->next) {
      if (staves->sym[voice] == s)
        break;
    }
    if (staves == 0) {
      trace("Internal bug: no %%staves\n");
      return;
    }

    /* search the max time */
    mxtime = s->time;
    for (voice = 0; voice <= nvoice; voice++) {
      s = staves->sym[voice];
      if (s != 0 && s->time > mxtime)
        mxtime = s->time;
    }

    /* update the symbols if needed */
    s_staves = 0;
    for (voice = 0; voice <= nvoice; voice++) {
      if (voice_tb[voice].vover)
        continue;
      s = staves->sym[voice];
      if (s == 0)
        continue;
      s->time = mxtime;
      if (s->next != 0
       && (s->next->time == 0
        || s->next->time != mxtime)) {
        s = sym_update(s);
        if (s && !s_staves)
          s_staves = s;
      }
    }

    if ((s = s_staves) == 0)
      break;
  }
}

/* -- dump symbol list to ABC -- */
static int sym_dump(Tcl_Interp *interp,
        struct sym *s)
{
  Tcl_Obj *obj;

  /* restart dumping */
  if (s != s_dump) {
    ly_st = 0;
    in_grace = 0;
    brhythm = gbr = 0;
  }
  s_dump = s->next;

  if (s->as.tune == 0)  /* not an ABC symbol */
    return TCL_OK;
  memset(blen, 0, sizeof blen);
  cvoice = s->voice;
  obj = Tcl_NewObj();
  Tcl_IncrRefCount(obj);
  sym_dump_i(obj, (struct abcsym *) s);
  Tcl_SetObjResult(interp, obj);
  Tcl_DecrRefCount(obj);
  return TCL_OK;
}

/* convert the unicode surrogates to \uxxxx\uxxxx */
static void append_to_obj(Tcl_Obj *obj,
      char *p)
{
  unsigned char *q, *r;
  int v;
  char buffer[16];

  q = r = (unsigned char *) p;
  while (*q != '\0') {
// 11110vvv 10vvvvvv 10vvvvvv 10vvvvvv
    if ((*q & 0xf0) != 0xf0) {  /* if < 4 chars */
      q++;
      continue;
    }
    if (q != r)
      Tcl_AppendToObj(obj, (char *) r, q - r);
    v = ((*q & 0x07) << 18)
      + ((q[1] & 0x3f) << 12)
      + ((q[2] & 0x3f) << 6)
      + (q[3] & 0x3f);
    sprintf(buffer, "\\u%04x\\u%04x",
      0xd7c0 + (v >> 10), 0xdc00 + (v & 0x3ff));
    Tcl_AppendToObj(obj, buffer, 12);
    q += 4;
    r = q; 
  }
  if (q != r)
    Tcl_AppendToObj(obj, (char *) r, q - r);
}

/* -- dump a symbol to ABC (internal) -- */
static void sym_dump_i(Tcl_Obj *obj,
           struct abcsym *as)
{
  char buffer[256], *p, c;
  int nl_new, i;

  if (obj->length == 0) {
    c = '\n';
  } else {
    p = Tcl_GetString(obj);
    c = p[obj->length - 1];
  }
  p = buffer;
  if (as->flags & ABC_F_SPACE)
    *p++ = ' ';
  nl_new = 0;
  if (in_grace && (as->type != ABC_T_NOTE
          || !(as->flags & ABC_F_GRACE))) {
    in_grace = 0;
    brhythm = gbr;
    *p++ = '}';
  }
  switch (as->type) {
  case ABC_T_INFO:
    if (as->state == ABC_S_EMBED) {
      *p++ = '[';
    } else if (c != '\n') {
      *p++ = '\\';
      *p++ = '\n';
      Tcl_AppendToObj(obj, buffer, p - buffer);
      lyrics_dump(obj, as);
      p = buffer;
    }
    p = header_dump(p, as);
    if (as->state == ABC_S_EMBED)
      *p++ = ']';
    else
      nl_new = 1;
    *p = '\0';
    append_to_obj(obj, buffer);
    p = buffer;
    break;
  case ABC_T_NULL:
  case ABC_T_PSCOM:
    nl_new = 1;
    if (as->text == 0)
      break;
    if (c != '\n') {
      *p++ = '\\';
      *p++ = '\n';
    }
    if (p != buffer) {
      Tcl_AppendToObj(obj, buffer, p - buffer);
      lyrics_dump(obj, as);
      p = buffer;
    }
#if 1
    append_to_obj(obj, as->text);
#else
    Tcl_AppendToObj(obj, as->text, -1);
#endif
    break;
  case ABC_T_NOTE:
    if (!(as->flags & ABC_F_GRACE)) {
      if (ly_st == 0)
        ly_st = as;
    } else {
      if (!in_grace) {
        in_grace = 1;
        gbr = brhythm;
        brhythm = 0;
        *p++ = '{';
        if (as->flags & ABC_F_SAPPO)
          *p++ = '/';
      }
    }
    /* fall thru */
  case ABC_T_REST:
    if (as->u.note.slur_st != 0)
      p = slur_dump(p, as->u.note.slur_st);
    if (as->text != 0)
      p = gchord_dump(p, as->text); /* guitar chord */
    if (as->u.note.dc.n != 0)
      p = deco_dump(&as->u.note.dc, p);
    brhythm |= as->u.note.brhythm;
    if (as->type == ABC_T_NOTE) {
      int len;
      int all_tie = 0;
      int blen_sav = blen[cvoice];

      if (as->flags & ABC_F_GRACE)
        blen[cvoice] = BASE_LEN / 4;

      if (as->u.note.nhd > 0) {
        for (i = as->u.note.nhd; i >= 0; i--)
          if (!as->u.note.ti1[i])
            break;
        if (i < 0)
          all_tie = as->u.note.ti1[0];
        *p++ = '[';
      }
      for (i = 0; i <= as->u.note.nhd; i++) {
        if (as->u.note.decs[i] != 0) {
          int i1, i2, deco;

          i1 = as->u.note.decs[i] >> 3;
          i2 = i1 + (as->u.note.decs[i] & 0x07);
          for ( ; i1 < i2; i1++) {
            deco = as->u.note.dc.t[i1];
            if (deco < 128) {
              if (deco != 0)
                *p++ = deco;
            } else {
              p += sprintf(p, "!%s!",
                deco_tb[deco - 128]);
            }
          }
        }
        if (as->u.note.sl1[i] != 0)
          p = slur_dump(p, as->u.note.sl1[i]);
        len = broken_rhythm(as->u.note.lens[i]);
        if (as->u.note.chlen != 0)
          len = len * BASE_LEN / as->u.note.chlen;
        p = note_dump(p,
                as->u.note.pits[i],
                as->u.note.accs[i],
                len,
                as->flags & ABC_F_STEMLESS);
        if (as->u.note.ti1[i] != 0
         && as->u.note.ti1[i] != all_tie) {
          if (as->u.note.ti1[i] & SL_DOTTED)
            *p++ = '.';
          *p++ = '-';
          switch (as->u.note.ti1[i]) {
          case SL_ABOVE:
            *p++ = '\'';
            break;
          case SL_BELOW:
            *p++ = ',';
            break;
          }
        }
        for (len = as->u.note.sl2[i]; --len >= 0;)
          *p++ = ')';
      }
      if (as->u.note.nhd > 0) {
        *p++ = ']';
        if (as->u.note.chlen != 0) {
          blen[cvoice] = BASE_LEN;
          p = length_dump(p, as->u.note.chlen);
        }
      }
      if (all_tie) {
        if (all_tie & SL_DOTTED)
          *p++ = '.';
        *p++ = '-';
        switch (all_tie) {
        case SL_ABOVE:
          *p++ = '\'';
          break;
        case SL_BELOW:
          *p++ = ',';
          break;
        }
      }
      blen[cvoice] = blen_sav;
    } else {
      if (as->u.note.lens[0] != 0) {
        *p++ = as->flags & ABC_F_INVIS ? 'x' : 'z';
        p = length_dump(p, broken_rhythm(as->u.note.lens[0]));

      } else {
        *p++ = 'y';
        if (as->u.note.lens[1] >= 0)
          p += sprintf(p, "%d",
                 as->u.note.lens[1]);
      }
    }
    for (i = 0; i < as->u.note.slur_end; i++)
      *p++ = ')';
    brhythm = -as->u.note.brhythm;
    if (brhythm != 0) {
      char c;
      int n;

      if ((n = brhythm) < 0) {
        n = -n;
        c = '>';
      } else {
        c = '<';
      }
      while (--n >= 0)
        *p++ = c;
    }
    break;
  case ABC_T_BAR:
    if (as->u.bar.dc.n != 0)
      p = deco_dump(&as->u.bar.dc, p);
    if (as->text != 0 && !as->u.bar.repeat_bar)
      p = gchord_dump(p, as->text); /* guitar chord */
    if (as->u.bar.dotted)
      *p++ = '.';
    if (!as->u.bar.repeat_bar || c != '|') {
      int t, v;

      t = as->u.bar.type;
      v = 0;
      while (t != 0) {
        v <<= 4;
        v |= t & 0x0f;
        t >>= 4;
      }
      while (v != 0) {
        *p++ = "?|[]:???"[v & 0x07];
        v >>= 4;
      }
    }
    if (as->u.bar.repeat_bar) {
      if (isdigit((unsigned char) as->text[0])
       && as->text[1] == '\0') {
        strcpy(p, as->text);  /* repeat */
        p += strlen(p);
      } else {
        p += sprintf(p, "\"%s\"", as->text);
      }
    } else if (as->u.bar.type == B_OBRA) {
      *p++ = ']';
    }
    break;
  case ABC_T_CLEF:
    return;
  case ABC_T_EOLN:
/*fixme:pb when info after line continuation */
    switch (as->u.eoln.type) {
    case 1:       /* continuation */
      *p++ = '\\';
    case 0:       /* normal */
      if (c != '\n')
        nl_new = 1;
      break;
    case 2:       /* abc2win line break */
      *p++ = '!';
      break;
    }
    break;
  case ABC_T_MREST:
    p += sprintf(p, "Z%d", as->u.bar.len);
    break;
  case ABC_T_MREP:
    for (i = 0; i < as->u.bar.len; i++)
      *p++ = '/';
    break;
  case ABC_T_V_OVER:
    switch (as->u.v_over.type) {
    case V_OVER_V:
      *p++ = '&';
      break;
    case V_OVER_S:
      *p++ = '(';
      *p++ = '&';
      break;
    case V_OVER_E:
      *p++ = '&';
      *p++ = ')';
      break;
    }
    break;
  case ABC_T_TUPLET: {
    int pp, qp, rp;

    pp = as->u.tuplet.p_plet;
    qp = as->u.tuplet.q_plet;
    rp = as->u.tuplet.r_plet;
    p += sprintf(p, "(%d", pp);
    if ((pp != 2 || qp != 3 || rp != 2)
     && (pp != 3 || qp != 2 || rp != 3))
      p += sprintf(p, ":%d:%d", qp, rp);
    break;
      }
  }
  if (p != buffer)
    Tcl_AppendToObj(obj, buffer, p - buffer);
  if (as->comment != 0) {
    if (p != buffer)
      Tcl_AppendToObj(obj, "\t", 1);
    Tcl_AppendStringsToObj(obj, "%", as->comment, 0);
    nl_new = 1;
  }
  if (nl_new || as->next == 0) {
    Tcl_AppendToObj(obj, "\n", 1);
    lyrics_dump(obj, as);
  }
}

/* -- search a symbol from its external name -- */
static struct sym *sym_decode(Tcl_Interp *interp,
        char *p)
{
  int voice, time, seq;

  if (sscanf(p, sym_fmt, &voice, &time, &seq) != 3
   || (unsigned) voice > nvoice) {
    sprintf(interp->result, "Bad symbol name '%s'", p);
    return 0;
  }
  goaltime = time;
  goalseq = seq;
  return voice_go(voice);
}

/* -- build the symbol external name -- */
static void sym_encode(char *p,
      struct sym *s)
{
  sprintf(p, sym_fmt, s->voice, s->time, s->seq);
}

/* -- link a symbol in a voice -- */
static void sym_link(struct sym *s, int type)
{
  /* create the End Of Tune symbol if it does not exist */
  if (curvoice->eot == 0)
    eot_create();

  s->next = curvoice->cursym->next;
  s->prev = curvoice->cursym;
  s->prev->next = s;
  s->next->prev = s;
  s->time = 0;
  s->type = type;

  curvoice->cursym = s;
  s->voice = curvoice - &voice_tb[0];
}

/* build the list of <pitch> <accidental> */
static void notes_list(Tcl_Interp *interp,
      Tcl_Obj *obj,
      int n,
      signed char *pits,
      unsigned char *accs)
{
  int i;
  unsigned acc;

  for (i = 0; i <= n; i++) {
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(pits[i]));
    acc = accs[i];
    if (acc & 0xf8)
      acc = (micro_tb[acc >> 3] << 3)
        | (acc & 0x07);
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(acc));
  }
}

/* -- return a symbol as a tcl result -- */
static int sym_return(Tcl_Interp *interp)
{
  Tcl_Obj *obj;
  struct sym *s = curvoice->cursym;

  obj = Tcl_NewObj();
  Tcl_IncrRefCount(obj);
  Tcl_ListObjAppendElement(interp, obj, type_obj[s->type]);
  switch (s->type) {
  case GRACE:
  case NOTE:  /* note <len> [<pitch> <acc>]* */
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.note.lens[0]));
    notes_list(interp, obj, s->as.u.note.nhd,
        s->as.u.note.pits, s->as.u.note.accs);
    break;
  case REST:  /* rest <len> */
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.note.lens[0]));
    break;
  case MREST: /* mrest <len> */
  case MREP:  /* mrep <len> */
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.bar.len));
    break;
  case BAR: /* bar <type> <repeat> <number> */
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.bar.type));
    Tcl_ListObjAppendElement(interp, obj,
           s->as.u.bar.repeat_bar
           ? Tcl_NewStringObj(s->BAR_repeat,
                strlen(s->BAR_repeat))
           : empty_obj);
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->BAR_num));
    break;
  case CLEF:  /* clef <type> <line> */
//fixme:'name' to do*/
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.clef.type));
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.clef.line));
    break;
  case TIMESIG: { /* time <upval> <downval> .. */
    int i, l;

    for (i = 0; i < s->as.u.meter.nmeter; i++) {
      l = strlen(s->as.u.meter.meter[i].top);
      if (l > sizeof s->as.u.meter.meter[i].top)
        l = sizeof s->as.u.meter.meter[i].top;
      Tcl_ListObjAppendElement(interp, obj,
             Tcl_NewStringObj(s->as.u.meter.meter[i].top, l));
      l = strlen(s->as.u.meter.meter[i].bot);
      if (l > sizeof s->as.u.meter.meter[i].bot)
        l = sizeof s->as.u.meter.meter[i].bot;
      Tcl_ListObjAppendElement(interp, obj,
          Tcl_NewStringObj(s->as.u.meter.meter[i].bot,
             l));
    }
    break;
      }
  case KEYSIG:  /* key <type> mode [exp] [<pitch> <acc>]* */
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.key.sf));
    if (s->as.u.key.empty == 2)
      Tcl_ListObjAppendElement(interp, obj,
        Tcl_NewStringObj(none, 4));
    else
      Tcl_ListObjAppendElement(interp, obj,
        Tcl_NewStringObj(key_names[(unsigned char) s->as.u.key.mode],
               -1));
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewStringObj(s->as.u.key.exp ?
              "exp" : "add", 3));
    notes_list(interp, obj, s->as.u.key.nacc - 1,
        s->as.u.key.pits, s->as.u.key.accs);
    break;
  case TEMPO: /* tempo <str1> <len0> <len1> <len2> <len3> <value> <str2>*/
    tcl_appstr(interp, obj, s->as.u.tempo.str1);
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.tempo.length[0]));
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.tempo.length[1]));
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.tempo.length[2]));
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.tempo.length[3]));
    tcl_appstr(interp, obj, s->as.u.tempo.value);
    tcl_appstr(interp, obj, s->as.u.tempo.str2);
    break;
  case EXTRA_SYM:
    switch (s->EXTRA_type) {
    case EXTRA_NL:  /* extra newline */
      Tcl_ListObjAppendElement(interp, obj,
             Tcl_NewStringObj("newline", -1));
      break;
    case EXTRA_LEN: /* extra blen <value> */
      Tcl_ListObjAppendElement(interp, obj,
             Tcl_NewStringObj("blen", -1));
      Tcl_ListObjAppendElement(interp, obj,
             Tcl_NewIntObj(s->EXTRA_L_len));
      break;
    case EXTRA_STAVES: {
      char *p;

      p = s->as.text;
      if (p != 0 && p[1] == 'c')
        p = "score";
      else
        p = "staves";
      Tcl_ListObjAppendElement(interp, obj,
             Tcl_NewStringObj(p, -1));
      p = s->as.text;
      if (p != 0) {
        while (*p != '\0'
            && *p != ' ')
          p++;
        if (*p == ' ') {
          while (*p == ' ')
            p++;
          Tcl_ListObjAppendList(interp, obj,
              Tcl_NewStringObj(p, -1));
          break;
        }
      }
      Tcl_ListObjAppendElement(interp, obj, empty_obj);
      break;
        }
    case EXTRA_VOVER: /* extra vover <type> <voice> */
      Tcl_ListObjAppendElement(interp, obj,
             Tcl_NewStringObj("vover", -1));
      Tcl_ListObjAppendElement(interp, obj,
             Tcl_NewIntObj(s->as.u.v_over.type));
      Tcl_ListObjAppendElement(interp, obj,
             Tcl_NewIntObj(s->as.u.v_over.voice));
      break;
    case EXTRA_INFO:
      Tcl_ListObjAppendElement(interp, obj,
             Tcl_NewStringObj("info", -1));
      Tcl_ListObjAppendList(interp, obj,
            Tcl_NewStringObj(s->as.text, -1));
      break;
    }
    break;
  case MIDI:  /* midi <%%MIDI definition> */
    Tcl_ListObjAppendList(interp, obj,
              Tcl_NewStringObj(&s->as.text[7], -1));
    break;
  case TUPLET:
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.tuplet.p_plet));
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.tuplet.q_plet));
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.tuplet.r_plet));
    break;
  case EOT:
    break;
  }
  Tcl_SetObjResult(interp, obj);
  Tcl_DecrRefCount(obj);
  return TCL_OK;
}

/* -- update the volatile symbol values -- */
/* return a pointer to '%%staves' if any */
struct sym *sym_update(struct sym *s)
{
  struct sym *s2;
  int time, seq, sf, bar;
  int mlen;   /* measure length */
  static char seq_tb[14] = {
    SQ_NOTE, SQ_NOTE, SQ_BAR, SQ_CLEF, SQ_SIG,
    SQ_SIG, SQ_SIG, SQ_EXTRA, SQ_EXTRA, SQ_NOTE,
    SQ_NOTE, SQ_GRACE, SQ_TUPLET, SQ_EOT
  };

  if (s->type == EOT)
    return 0;

  /* search the previous bar number */
  bar = 1;    /* fixme: should see if 1st measure complete */
  s2 = s;
  while (s2->prev->type != EOT) {
    s2 = s2->prev;
    if (s2->type == BAR) {
      bar = s2->BAR_num;
      break;
    }
  }

  /* go backward if the symbol is in a tuplet sequence */
  if (s->flags & F_NPLET) {
    do {
      s = s->prev;
    } while (s->type != TUPLET);
  }
  if (s->type == TUPLET)
    set_tuplet(s);
  mlen = -1;
  /* if symbol on many measures, search the current measure length */

  /* initialize */
  time = s->time;
  seq = s->seq;
  sf = s->sf;

  if (s->type == BAR) {
    if (s->as.u.bar.type != B_OBRA)
      bar++;
    s->BAR_num = bar;
  }

  /* loop on updating all next elements */
  for (;;) {
    if (s->dtime != 0) {
      time += s->dtime;
      seq = 0;
    }
    else if (s->type == MREST || s->type == MREP) {
      if (mlen < 0) {
        mlen = BASE_LEN;
        for (s2 = s; ; s2 = s2->prev)
          if (s2->type == TIMESIG)
            break;
        mlen = s2->as.u.meter.wmeasure;
      }
      time += mlen * s->as.u.bar.len;
      seq = 0;
    }
    s = s->next;
    s->time = time;
    s->seq = seq_tb[s->type];
    switch (s->type) {
      case KEYSIG:
        sf = s->as.u.key.sf;
        break;
      case TIMESIG:
        mlen = s->as.u.meter.wmeasure;
        break;
      case BAR:
        if (s->as.u.bar.type != B_OBRA)
          bar++;
        s->BAR_num = bar;
        break;
      case REST:
        if (s->dtime != 0)  /* if not space */
          seq = 0;
        else
          s->seq = seq + 1;
        break;
      case NOTE:
      case MREST:
      case MREP:
        seq = 0;
        break;
    }
    /*fixme: if nothing has changed, stop?*/
    s->sf = sf;
    if (s->type == EOT)
      break;
    if (s->seq <= seq)
      s->seq = seq + 1;
    seq = s->seq;

    /* update the voice overlay */
    if (s->type == EXTRA_SYM) {
      if (s->EXTRA_type == EXTRA_VOVER) {
/*       && s->as.u.v_over.type == V_OVER_V) { */
        s2 = s->prev;
        while (!(s2->flags & F_VOVER_S))
          s2 = s2->prev;
        s->u.vover->time = s2->time;
        s2 = s->u.vover;
        s2->seq = seq_tb[s2->type];
        sym_update(s2);
      } else if (s->EXTRA_type == EXTRA_STAVES) {
        return s;
      }
    }
    if (s->flags & F_VOVER_E) {
      if (s->next->type == EOT)
        continue;
      break;
    }
  }
  return 0;
}

/* -- append a string to a list object -- */
static void tcl_appstr(Tcl_Interp *interp,
      Tcl_Obj *obj,
      char *str)
{
  Tcl_ListObjAppendElement(interp, obj, str != 0 && *str != '\0'
        ? Tcl_NewStringObj(str, strlen(str))
        : empty_obj);
}

/* -- set an int value as the tcl result -- */
static int tcl_int_res(Tcl_Interp *interp,
           int v)
{
  Tcl_SetObjResult(interp, Tcl_NewIntObj(v));
  return TCL_OK;
}

/* -- set a 'wrong # args' message -- */
int tcl_wrong_args(Tcl_Interp *interp,
       char *msg)
{
  sprintf(interp->result,
    "wrong # args: should be \"abc %s\"", msg);
  return TCL_ERROR;
}

/* -- start/stop MIDI recording -- */
static int tclabc_record(Tcl_Interp *interp)
{
#ifdef HAVE_SOUND
  int recording;

  recording = midi_record();
  if (recording == 2) {   /* recording done */
    struct abctune *t;
    struct abcsym *as, *next;
    struct sym *s;

    t = abc_parse(empty_tune);
    as = t->last_sym; /* K: */

    /* set the key signature */
    s = search_abc_sym(curvoice->cursym);
#if 1
    as->u.key.sf = curvoice->cursym->sf;
#else
    as->u.key.sf = s->sf;
#endif

    /* translate the MIDI sequence to ABC */
/*fixme: pb when in the middle of a measure and some accidental*/
    midi_load(t);
    if (as->next != 0) {
      struct voicecontext *p_voice;

      as = as->next;    /* skip V: */
      next = as->next;  /* start of MIDI sequence */
      as->next = 0;   /* cut the tmp tune */
      as = next;
      tune_purge();
      next = s->as.next;
      s->as.next = as;
      as->prev = &s->as;
      for (;;) {
        as->tune = curtune;
        if (as->next == 0)
          break;
        as = as->next;
      }
      as->next = next;
      if (next != 0)
        next->prev = as;
      p_voice = curvoice;
      tune_select(curtune);
      s = s->next;
      curvoice = p_voice;
      curvoice->cursym = s;
      goaltime = s->time;
      goalseq = s->seq;
      measure_set();
    } else {
      recording = 0;
    }
    abc_purge(t);
  }
  return tcl_int_res(interp, recording);
#else
  strcpy(interp->result, "no MIDI support");
  return TCL_ERROR;
#endif
}

/* -- return the ties -- */
static int ties_get(Tcl_Interp *interp,
        struct sym *s)
{
  Tcl_Obj *obj;
  int i;

  if (s->as.type != ABC_T_NOTE || !(s->flags & F_TIE_S))
    return TCL_OK;    /* no tie */
  obj = Tcl_NewObj();
  Tcl_IncrRefCount(obj);
  for (i = 0; i <= s->as.u.note.nhd; i++) {
    Tcl_ListObjAppendElement(interp, obj,
           Tcl_NewIntObj(s->as.u.note.ti1[i]));
  }
  Tcl_SetObjResult(interp, obj);
  Tcl_DecrRefCount(obj);
  return TCL_OK;
}

/* -- advance in time -- */
static struct sym *time_advance(void)
{
  struct voicecontext *p_voice;
  struct sym *s, *best, *o;
  int voice, mintime, time,newseq, seq;

  /* set the goal time/sequence and advance the current voice */
  o = best = curvoice->cursym;
  time = best->time;
  seq = best->seq;
  if (best->type != EOT)
    curvoice->cursym = best = best->next;

  /* try the next voices for the same goal time/sequence */
  for (p_voice = curvoice + 1, voice = p_voice - voice_tb;
       voice <= nvoice;
       voice++, p_voice++) {
    s = p_voice->cursym;
    while (s->type != EOT
        && (s->time > time    /* go backwards... */
         || (s->time == time && s->seq > seq)))
      s = s->prev;
    if (s->type == EOT)
      s = s->next;
    while (s->type != EOT
        && (s->time < time    /* ...or forwards */
         || (s->time == time && s->seq < seq)))
      s = s->next;
    p_voice->cursym = s;
    /*fixme:what about EOT*/
    if (s->time == time
     && s->seq == seq) {
      curvoice = p_voice;
      return s;
    }
  }

  /* synchronize the other voices */
  for (p_voice = voice_tb, voice = 0;
       voice < curvoice - voice_tb;
       voice++, p_voice++) {
    s = p_voice->cursym;
    while (s->prev->time > time
        || (s->prev->time == time && s->prev->seq > seq))
      p_voice->cursym = s = s->prev;
    while (s->type != EOT
        && (s->time < time
         || (s->time == time && s->seq < seq)))
      p_voice->cursym = s = s->next;
  }

  /* search for the shortest next time/sequence */
  mintime = time + EOT_TIME + 1;
  newseq = 0x7f;
  for (voice = 0, p_voice = voice_tb;
       voice <= nvoice;
       voice++, p_voice++) {
    s = p_voice->cursym;
    if (s->type == EOT
     && (s->time < time
      || (s->time == time && s->seq <= seq)))
      continue;
    if (s->time < mintime
     || (s->time == mintime && s->seq < newseq)) {
      mintime = s->time;
      newseq = s->seq;
      best = s;
    }
  }
  if (best == o)
    return 0;   /* last symbol */
  curvoice = &voice_tb[best->voice];
  curvoice->cursym = best;
  return best;
}

/* -- dump all tunes -- */
static int tune_dump(Tcl_Interp *interp)
{
  Tcl_Obj *obj;
  struct abctune *t;
  struct tune_ext *te;

  if (lyrics_change)
    lyrics_dispatch(curtune);
  obj = Tcl_NewObj();
  Tcl_IncrRefCount(obj);
  if (first_tune->first_sym->text == 0
   || strncmp(first_tune->first_sym->text, "%abc", 4) != 0)
    Tcl_AppendToObj(obj, "%abc-2.1\n", 9);
  for (t = first_tune; t != 0; t = t->next) {
    struct abcsym *as;

    /* build the lyrics if needed */
    if ((te = (struct tune_ext *) t->client_data) == 0) {
      lyrics_build(t);
      te = (struct tune_ext *) t->client_data;
      if (lyrics_change)
        lyrics_dispatch(t);
    }

    /* dump all symbols */
    memset(blen, 0, sizeof blen);
    cvoice = 0;
    ly_st = 0;
    in_grace = 0;
    brhythm = gbr = 0;
    micro_tb = t->micro_tb;
    for (as = t->first_sym; as != 0; as = as->next)
      sym_dump_i(obj, as);
    if (t->next != 0)
      Tcl_AppendToObj(obj, "\n", 1);  /* tune separator */
  }
  Tcl_SetObjResult(interp, obj);
  Tcl_DecrRefCount(obj);
  micro_tb = curtune->micro_tb;
  return TCL_OK;
}

/* -- return the tune list -- */
static int tune_list(Tcl_Interp *interp)
{
  struct abctune *t;
  Tcl_Obj *obj;

  obj = Tcl_NewObj();
  Tcl_IncrRefCount(obj);
  for (t = first_tune; t != 0; t = t->next) {
    struct abcsym *as;
    char *title;

    title = 0;
    for (as = t->first_sym; as != 0; as = as->next) {
      if (as->type == ABC_T_INFO
       && as->text[0] == 'T') {
        title = &as->text[2];
        break;
      }
    }
    if (title) {
      while (isspace((unsigned char) *title)) /* skip the leading blanks */
        title++;
    }
    tcl_appstr(interp, obj, title);
  }
  Tcl_SetObjResult(interp, obj);
  Tcl_DecrRefCount(obj);
  return TCL_OK;
}

/* -- remove the extra symbols from the current tune -- */
void tune_purge()
{
  int voice;
  struct STAVES_S *staves;

  for (voice = 0; voice <= nvoice; voice++) {
    struct sym *s;

    s = voice_tb[voice].eot->next;
    while (s->type != EOT) {
      struct sym *next;

      next = s->next;
      if (s->as.tune == 0)
        free(s);
      s = next;
    }
    free(voice_tb[voice].eot);
/*    voice_tb[voice].eot = 0; */
  }
  for (staves = first_staves; staves; ) {
    struct STAVES_S *next_staves;

    next_staves = staves->next;
    free(staves);
    staves = next_staves;
  }
  first_staves = last_staves = 0;
}

/* -- select a tune -- */
void tune_select(struct abctune *t)
{
  struct abcsym *as;
  struct sym *s, *s2;
  int voice, vover, in_ly;
  struct sym *s_staves, *p_vover;

  /* tune initialization */
  memset(voice_tb, 0, sizeof voice_tb);
  for (voice = MAXVOICE; --voice >= 0;) {
    voice_tb[voice].channel = voice;
#ifdef HAVE_SOUND
    set_channels(voice, 1 << voice);
#endif
  }
  micro_tb = t->micro_tb;

  /* scan the ABC definition and create the extra symbols */
  curtune = t;
  nvoice = 0;
  in_ly = 0;
  vover = -1;
  p_vover = 0;
  curvoice = &voice_tb[0];
  for (as = t->first_sym; as != 0; as = as->next) {
    s = (struct sym *) as;
    switch (as->type) {
    case ABC_T_NULL:
      break;
    case ABC_T_INFO:
      if (as->text[0] != 'w') {
        header_process(s);
      } else {    /* w: will be deleted later */
        int l;

        l = strlen(as->text);
        in_ly = as->text[l - 1] == '\\';
      }
      break;
    case ABC_T_PSCOM:
      if (in_ly
       && strncmp(&as->text[2], "vocalfont ", 10) == 0)
        break;    /* will be deleted later */
      process_pscomment(s);
      break;
    case ABC_T_NOTE:
    case ABC_T_REST: {
      int type;

      if (as->type == ABC_T_NOTE)
        type = (as->flags & ABC_F_GRACE) ? GRACE : NOTE;
      else
        type = REST;
      sym_link(s, type);
      if (type != GRACE)
        s->dtime = as->u.note.lens[0];
      break;
        }
    case ABC_T_BAR: {
      int btype;

      if (vover >= 0) {
#if 1
        s2 = (struct sym *) malloc(sizeof *s2);
        memset(s2, 0, sizeof *s2);
        sym_link(s2, BAR);
        s2->as.u.bar.type = B_BAR;
#endif
        curvoice->cursym->flags |= F_VOVER_E;
        curvoice = &voice_tb[vover];
        vover = -1;
      }
      sym_link(s, BAR);
      btype = s->as.u.bar.type;
      if (btype & 0xf0) {
        do {
          btype >>= 4;
        } while (btype & 0xf0);
        if (btype == B_COL)
          s->flags |= F_RRBAR;
      }
      break;
        }
    case ABC_T_CLEF:
      if (s->as.u.clef.type >= 0) {
        sym_link(s, CLEF);
        curvoice->forced_clef = 1;
      }
      break;
    case ABC_T_EOLN:
      sym_link(s, EXTRA_SYM);
      s->EXTRA_type = EXTRA_NL;
      break;
    case ABC_T_MREST:
      sym_link(s, MREST);
      break;
    case ABC_T_MREP:
      sym_link(s, MREP);
      break;
    case ABC_T_V_OVER:
      switch (s->as.u.v_over.type) {
      case V_OVER_V:
        sym_link(s, EXTRA_SYM);
        s->EXTRA_type = EXTRA_VOVER;
        p_vover = s;
        if (vover == -2) {
          if (curvoice->vover)
            curvoice->cursym->flags |= F_VOVER_E;
          break;
        }
        if (vover < 0) {  /* voice overlay in a measure */
          for (s2 = s->prev; ; s2 = s2->prev) {
            if (s2->type == EOT) {
              s2 = s2->next;
              break;
            }
            if (s2->type == BAR)
              break;
          }
          s2->flags |= F_VOVER_S;
          vover = curvoice - voice_tb; /* memorize the 1st voice */
          break;
        }
#if 1
        s2 = (struct sym *) malloc(sizeof *s2);
        memset(s2, 0, sizeof *s2);
        sym_link(s2, BAR);
        s2->as.u.bar.type = B_BAR;
#endif
        curvoice->cursym->flags |= F_VOVER_E;
        break;
      case V_OVER_S:
        p_vover = s;
        vover = -2;
        break;
      default:
/*      case V_OVER_E: */
        curvoice->cursym->flags |= F_VOVER_E;
        vover = -1;
        break;
      }
      curvoice = &voice_tb[s->as.u.v_over.voice];
      if (s->as.u.v_over.voice > nvoice) {
        nvoice = s->as.u.v_over.voice;
        voice_tb[nvoice].vover = 1;
      }
      continue; /* don't update the vover pointer */
    case ABC_T_TUPLET:
      sym_link(s, TUPLET);
      break;
    default:
      trace("ABC type %d not treated\n", as->type);
      break;
    }
    if (p_vover != 0 && curvoice->cursym == s) {
      p_vover->u.vover = s;
      p_vover = 0;
      s->flags |= F_VOVER_S;
    }
  }

  s_staves = 0;
  for (voice = 0; voice <= nvoice; voice++) {
    struct sym *p_clef = 0;
    struct sym *p_key = 0;
    struct sym *p_time = 0;
    struct sym *lastnote;
    int i, start_flag, nnotes, sigma_f, tie_s;

    curvoice = &voice_tb[voice];

    /* search the clef/key/time signatures */
    if (curvoice->eot == 0)
      eot_create();
    for (s = curvoice->eot->next;
         s->type != EOT;
         s = s->next) {
      if (s->as.state == ABC_S_TUNE
       || s->as.state == ABC_S_EMBED)
        break;
      switch (s->type) {
      case CLEF:
        p_clef = s;
        break;
      case KEYSIG:
        p_key = s;
        break;
      case TIMESIG:
        p_time = s;
        break;
      default:
        break;
      }
    }

    /* create them if needed */
    if (p_clef == 0) {
      p_clef = (struct sym *) malloc(sizeof *p_clef);
      memset(p_clef, 0, sizeof *p_clef);
      p_clef->type = CLEF;
      p_clef->voice = voice;
      p_clef->as.u.clef.type = TREBLE;
      p_clef->as.u.clef.line = 2;

      /* set as the first symbol */
      p_clef->next = curvoice->eot->next;
      curvoice->eot->next = p_clef;
      p_clef->prev = curvoice->eot;
      p_clef->next->prev = p_clef;
    }
    if (p_key == 0) {
      p_key = (struct sym *) malloc(sizeof *p_key);
      memset(p_key, 0, sizeof *p_key);
      p_key->type = KEYSIG;
      p_key->voice = voice;
      if (voice != 0)
        memcpy(&p_key->as.u.key,
          &voice_tb[0].eot->next->next->as.u.key,
          sizeof p_key->as.u.key);
      for (s = curvoice->eot->next;
           s->type != EOT;
           s = s->next) {
        if (s->type != CLEF
         && s->type != TIMESIG)
          break;
      }
      p_key->next = s;
      p_key->prev = s->prev;
      p_key->next->prev = p_key;
      p_key->prev->next = p_key;
    }
    if (p_time == 0) {
      p_time = (struct sym *) malloc(sizeof *p_time);
      memset(p_time, 0, sizeof *p_time);
      p_time->type = TIMESIG;
      p_time->voice = voice;
      if (voice == 0) {
        p_time->as.u.meter.wmeasure = BASE_LEN;
        p_time->as.u.meter.nmeter = 1;
        p_time->as.u.meter.meter[0].top[0] = '4';
        p_time->as.u.meter.meter[0].bot[0] = '4';
      } else {
        struct sym *p_time0;

        p_time0 = voice_tb[0].eot->next->next->next;
        memcpy(&p_time->as.u.meter,
               &p_time0->as.u.meter,
               sizeof p_time->as.u.meter);
      }
      for (s = curvoice->eot->next;
           s->type != EOT;
           s = s->next) {
        if (s->type != CLEF)
          break;
      }
      p_time->next = s;
      p_time->prev = s->prev;
      p_time->next->prev = p_time;
      p_time->prev->next = p_time;
    }

    /* put them at the head of the voice */
    if (curvoice->eot->next != p_clef) {
      p_clef->next->prev = p_clef->prev;
      p_clef->prev->next = p_clef->next;
      p_clef->next = curvoice->eot->next;
      p_clef->next->prev = p_clef;
      curvoice->eot->next = p_clef;
      p_clef->prev = curvoice->eot;
    }
    if (p_clef->next != p_key) {
      p_key->next->prev = p_key->prev;
      p_key->prev->next = p_key->next;
      p_key->next = p_clef->next;
      p_key->prev = p_clef;
      p_clef->next->prev = p_key;
      p_clef->next = p_key;
    }
    if (p_key->next != p_time) {
      p_time->next->prev = p_time->prev;
      p_time->prev->next = p_time->next;
      p_time->next = p_key->next;
      p_time->prev = p_key;
      p_key->next->prev = p_time;
      p_key->next = p_time;
    }

    /* if voice overlay, update the sequence */
    if (voice_tb[voice].vover) {
      curvoice->eot->next->next->seq = SQ_SIG;
      curvoice->eot->next->next->next->seq = SQ_SIG + 1;
    }

    /* set the note length in n-plets */
    /* and separate words at notes without flags */
    start_flag = 1;
    lastnote = 0;
    nnotes = 0;
    sigma_f = 0;
    tie_s = 0;
    for (s = curvoice->eot->next; s->type != EOT; s = s->next) {
      if (s->as.flags & ABC_F_SPACE)
        start_flag = 1;
      switch (s->type) {
      case REST:
        tie_s = 0;
        if (s->as.u.note.lens[0] >= BASE_LEN / 4) /* if no flag */
          start_flag = 1;
        continue;
      case BAR:
        start_flag = 1;
        continue;
      case TUPLET:
        set_tuplet(s);
        continue;
      default:
        continue;
      case NOTE:
        break;
      }
      sigma_f += (s->as.u.note.pits[0]
            + s->as.u.note.pits[(unsigned) s->as.u.note.nhd])
        / 2;
      nnotes++;
      if (s->as.u.note.lens[0] >= BASE_LEN / 4 /* if no flag */
       || start_flag) {
        if (lastnote != 0)
          lastnote->flags |= F_WORD_E;
        if (s->as.u.note.lens[0] >= BASE_LEN / 4) {
          s->flags |= (F_WORD_S | F_WORD_E);
          start_flag = 1;
        } else {
          s->flags |= F_WORD_S;
          start_flag = 0;
        }
      }
      if (tie_s) {
        s->flags |= F_TIE_E;
        tie_s = 0;
      }
      for (i = 0; i <= s->as.u.note.nhd; i++) {
        if (s->as.u.note.ti1[i]) {
          s->flags |= F_TIE_S;
          tie_s = 1;
          break;
        }
      }
      lastnote = s;
    }
    if (lastnote != 0)
      lastnote->flags |= F_WORD_E;

    /* change the clef if needed */
    if (!curvoice->forced_clef) {
      /*fixme*/
      if (nnotes != 0 && sigma_f / nnotes < 16) {
        p_clef->as.u.clef.type = BASS;
        p_clef->as.u.clef.line = 4;
      }
    }

    /* update the volatile symbol values */
    s = curvoice->eot->next;  /* 1st symbol */
    curvoice->cursym = s;
    s->sf = p_key->as.u.key.sf;

    if (voice_tb[voice].vover)
      continue;
    if ((s = sym_update(s)) != 0
     && s_staves == 0)
      s_staves = s;
  }

  /* update the symbols after %%staves */
  if (s_staves)
    staves_update(s_staves);
  do_rewind();
}

/* -- return the name of the current voice -- */
static int voice_get(Tcl_Interp *interp)
{
  Tcl_Obj *obj;
  struct sym *s;

  if ((s = curvoice->p_voice) == 0) {
    sprintf(interp->result, "{} {} {}");
    return TCL_OK;
  }
  obj = Tcl_NewObj();
  Tcl_IncrRefCount(obj);
  tcl_appstr(interp, obj, s->as.u.voice.id);
  tcl_appstr(interp, obj, s->as.u.voice.fname);
  tcl_appstr(interp, obj, s->as.u.voice.nname);
  Tcl_SetObjResult(interp, obj);
  Tcl_DecrRefCount(obj);
  return TCL_OK;
}

/* -- go to an other voice at the current time/sequence -- */
struct sym *voice_go(int voice)
{
  struct sym *s;
  struct voicecontext *p_voice;

  p_voice = &voice_tb[voice];
  s = p_voice->cursym;
  if (s->time < goaltime) {
    if (p_voice->eot->time < goaltime)
      return p_voice->eot;
    if (goaltime - s->time > p_voice->eot->time - goaltime) {
      s = p_voice->eot;
    } else {
      do {
        s = s->next;
      } while (s->time < goaltime);
    }
  } else if (s->time > goaltime) {
    if (s->time > 2 * goaltime)
      s = p_voice->eot->next;
  }
  while (s->time < goaltime
      || (s->time == goaltime && s->seq < goalseq)) {
    if (s->type == EOT)
      return s;
    s = s->next;
  }
  while (s->time > goaltime
      || (s->time == goaltime && s->seq > goalseq))
    s = s->prev;
  return s;
}

/* -- return the words (lyrics after tune) -- */
static int words_get(Tcl_Interp *interp)
{
  struct abcsym *as;
  Tcl_Obj *obj;

  obj = Tcl_NewObj();
  Tcl_IncrRefCount(obj);
  for (as = curtune->first_sym; as != 0; as = as->next) {
    if (as->type != ABC_T_INFO
     || as->text[0] != 'W')
      continue;
    Tcl_AppendToObj(obj,
        &as->text[2],
        strlen(&as->text[2]));
    Tcl_AppendToObj(obj, "\n", 1);
  }
  Tcl_SetObjResult(interp, obj);
  Tcl_DecrRefCount(obj);
  return TCL_OK;
}
