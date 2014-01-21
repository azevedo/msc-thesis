#ifndef _TCLABC_H_
#define _TCLABC_H_
/*
 * global definitions for tclabc
 */

#define MIDDLE_C 60
#define EOT_TIME 1000000

/* lyrics */
#define MAXLY	16	/* max number of lyrics */
struct lyrics {
	char *start[MAXLY], *stop[MAXLY];
};

/* symbols */
struct sym {
	struct abcsym as;	/* parser symbol !!must be the first field!! */
	struct sym *next, *prev; /* voice linkage */
	int	time;
	int	dtime;		/* real note duration */
	unsigned short flags;
#define F_SPACE 0x01		/* space (=word_end) */
#define F_HEADER 0x02
#define F_EMBEDDED 0x04
#define F_NPLET 0x08		/* in a n-plet sequence */
#define F_NPLET_S 0x10		/* start of a n-plet */
#define F_NPLET_E 0x20		/* end of a n-plet */
#define F_TIE_S 0x40		/* start of tie(s) */
#define F_TIE_E 0x80		/* end of tie(s) */
#define F_SLUR_S 0x100		/* start of slur(s) */
#define F_SLUR_E 0x200		/* end of slur(s) */
#define F_WORD_S 0x400		/* start of word */
#define F_WORD_E 0x800		/* end of word */
#define F_VOVER_S 0x1000	/* start of voice overlay */
#define F_VOVER_E 0x2000	/* end of voice overlay */
#define F_RRBAR 0x4000		/* right repeat bar */
	unsigned char type;
#define NOTE	0
#define REST	1
#define BAR	2
#define CLEF	3
#define TIMESIG 4
#define KEYSIG	5
#define TEMPO	6
#define EXTRA_SYM 7
#define MIDI	8
#define MREST	9
#define MREP	10
#define GRACE	11
#define TUPLET	12
#define EOT	13
	unsigned char voice;
	char	seq;		/* sequence number */
#define SQ_CLEF 0
#define SQ_SIG 10
#define SQ_EXTRA 20
#define SQ_GRACE 30
#define SQ_BAR 40
#define SQ_TUPLET 50
#define SQ_NOTE 60
#define SQ_EOT 70
	signed char sf;		/* sharps / flats # */
	short extra;
	/* shortcuts for other symbol types */
#define BAR_repeat as.text
#define BAR_num extra
#define EXTRA_type extra
#define EXTRA_NL 0		/* new-line */
#define EXTRA_LEN 1		/* L: */
#define EXTRA_L_len as.u.length.base_length
#define EXTRA_STAVES 2		/* %%staves */
#define EXTRA_VOVER 3		/* voice overlay */
#define EXTRA_INFO 4		/* information */
#define MIDI_type extra
#define MIDI_UNKNOWN 0
#define MIDI_CHANNEL 1
#define MIDI_channels as.u.length.base_length
#define MIDI_PROGRAM 2
#define MIDI_channel as.u.key.sf
#define MIDI_program as.u.key.empty
#define MIDI_bank as.u.key.exp
	union {
		struct lyrics *ly;	/* lyrics when note */
		struct sym *vover;	/* next voice when voice overlay */
	} u;
};

struct voicecontext {
	struct sym *eot;
	struct sym *cursym;
	struct sym *p_voice;	/* ptr to the first V: */
	unsigned forced_clef:1;
	unsigned mute:1;
	unsigned vover:1;
	signed char channel;	/* first channel */
};

/* tune extension pointed to by abctune->client_data */
struct tune_ext {
	int some_lyrics;
	Tcl_Obj *lyrics[MAXVOICE][MAXLY];
};

extern struct voicecontext voice_tb[MAXVOICE];
extern int nvoice;
extern struct voicecontext *curvoice;
extern int goaltime, goalseq;
extern struct abctune *first_tune;
extern struct abctune *curtune;
extern int lyrics_change;
extern const char *key_names[11];
extern char empty_str[];
extern Tcl_Obj *empty_obj;
extern Tcl_Obj *type_obj[];
#ifdef HAVE_SOUND
extern Tcl_Obj *midiin;
extern Tcl_Obj *midiout;
#endif

/* routines in tclabc.c*/
int channel_set(struct sym *s);
void eot_create(void);
int program_set(struct sym *s);
void set_tuplet(struct sym *s);
void staves_update(struct sym *s);
struct sym *sym_update(struct sym *so);
int tcl_wrong_args(Tcl_Interp *interp,
		   char *msg);
void trace(char *fmt, ...);
void tune_purge(void);
void tune_select(struct abctune *t);
struct sym *voice_go(int voice);

/* routines in change.c */
int beat_get(struct sym *s);
int deco_set(Tcl_Interp *interp,
	     char *p,
	     struct deco *deco);
int del_sym(Tcl_Interp *interp);
int header_set(char *p);
void measure_set(void);
struct sym *search_abc_sym(struct sym *sref);
void str_new(char **s,
	     Tcl_Obj *obj);
int sym_include(char *p);
struct sym *sym_insert(struct sym *sref);
int sym_set(Tcl_Interp *interp,
	    Tcl_Obj *obj,
	    int insert);
int slurs_set(Tcl_Interp *interp,
	      Tcl_Obj *obj,
	      struct sym *s);
int ties_set(Tcl_Interp *interp,
	     Tcl_Obj *obj,
	     struct sym *s);
int voice_new(Tcl_Interp *interp,
	      Tcl_Obj *obj);
int voice_set(Tcl_Interp *interp,
	      Tcl_Obj *obj);
int words_set(char *p);
void word_update(struct sym *so);
/* in front.c */
void front_init(int edit, int eol);
unsigned char *frontend(unsigned char *s,
			int format);
#endif
