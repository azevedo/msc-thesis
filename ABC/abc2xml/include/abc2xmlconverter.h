#ifndef ABC2XMLCONVERTER_H_
#define ABC2XMLCONVERTER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#include "abcparser.h"

#include "LibMusicXML.h"
#ifdef WIN32
#include <windows.h>
#endif



using namespace MusicXML;


#define MAXSTAFF	32	/* max staves */   //maximum notovych osnov


/* note types */
#define BREVE		(BASE_LEN * 2)	/* double note (square note) */
#define SEMIBREVE	BASE_LEN	/* whole note */
#define MINIM		(BASE_LEN / 2)	/* half note (white note) */
#define CROTCHET 	(BASE_LEN / 4)	/* quarter note (black note) */
#define QUAVER		(BASE_LEN / 8)	/* 1/8 note */
#define SEMIQUAVER	(BASE_LEN / 16)	/* 1/16 note */


//bar = takt
/* bar types */
#define B_INVIS B_OBRA		/* invisible; for endings without bars */
#define B_SINGLE B_BAR		/* |	single bar */
#define B_DOUBLE 0x11		/* ||	thin double bar */
#define B_THIN_THICK 0x13	/* |]	thick at section end  */
#define B_THICK_THIN 0x21	/* [|	thick at section start */
#define B_LREP 0x14		/* |:	left repeat bar */
#define B_RREP 0x41		/* :|	right repeat bar */
#define B_DREP 0x44		/* ::	double repeat bar */
#define B_DASH 0x04		/* :	dashed bar */



// STRUCTURES

		struct key_s {		/* K: info */
			signed char sf;		/* sharp (> 0) flats (< 0) */
			char bagpipe;		/* HP or Hp */
			char minor;		/* major (0) / minor (1) */
			char empty;		/* clef alone if 1, 'none' if 2 */
			signed char nacc;	/* explicit accidentals */
			char pits[8];
			char accs[8];
		};

		struct meter_s {	/* M: info */
			short wmeasure;		/* duration of a measure */
			char nmeter;		/* number of meter elements */
			char expdur;		/* explicit measure duration */
#define MAX_MEASURE 6
			struct {
				char top[8];	/* top value */
				char bot[2];	/* bottom value */
			} meter[MAX_MEASURE];
		};

#define MAXWHISTLE	4	/* max number of whistle tablature */

struct WHISTLE_S {
	short voice;		/* voice number */
	short pitch;		/* absolute key pitch */
};


struct VOICE_S {
	SScorePart xmlvoice;
	struct SYMBOL *sym;	/* associated symbols */
	struct SYMBOL *last_symbol;	/* last symbol while scanning */
	struct SYMBOL *s_anc;	/* ancillary symbol pointer */
	struct VOICE_S *next, *prev;	/* staff links */
	char *name;		/* voice id */
	char *nm;		/* voice name */
	char *snm;		/* voice subname */
	char *bar_text;		/* bar text at start of staff when bar_start */
	struct SYMBOL *tie;	/* note with ties of previous line */
	struct SYMBOL *rtie;	/* note with ties before 1st repeat bar */
	float scale;		/* scale */
	int time;		/* current time while parsing */
	struct clef_s clef;	/* current clef */
	struct key_s key;	/* current key signature */
	struct meter_s meter;	/* current time signature */
	unsigned hy_st;		/* lyrics hyphens at start of line (bit array) */
	unsigned forced_clef:1;	/* explicit clef */
	unsigned second:1;	/* secondary voice in a brace/parenthesis */
	unsigned floating:1;	/* floating voice in a brace */
	unsigned selected:1;	/* selected while sorting by time (music.c) */
	unsigned bar_repeat:1;	/* bar at start of staff is a repeat bar */
	unsigned norepbra:1;	/* display the repeat brackets */
	unsigned have_ly:1;	/* some lyrics in this voice */
	unsigned whistle:1;	/* tin whistle for this voice */
	short wmeasure;		/* measure duration while parsing */
	signed char bar_start;	/* bar type at start of staff / -1 */
	signed char clone;	/* duplicate from this voice number */
	unsigned char staff;	/* staff (0..n-1) */
	unsigned char cstaff;	/* staff while parsing */
	signed char sfp;	/* key signature while parsing */
	signed char stem;	/* stem direction while parsing */
	signed char gstem;	/* grace stem direction while parsing */
	signed char dyn;	/* place of dynamic marks while parsing */
	signed char ly_pos;	/* place of lyrics (above / below) */
	unsigned char slur_st;	/* slurs at start of staff */
};


struct ISTRUCT {		/* information fields in abc file */
	struct SYMBOL *area;
	struct SYMBOL *book;
	struct SYMBOL *comp;
	struct SYMBOL *disco;
	struct SYMBOL *histo;
	struct SYMBOL *notes;
	struct SYMBOL *orig;
	struct SYMBOL *parts;
	struct SYMBOL *rhythm;
	struct SYMBOL *src;
	struct SYMBOL *tempo;
	struct SYMBOL *title;
	char *xref;
	struct SYMBOL *trans;
};

/* sequence numbers for symbol grouping - index = symbol type */
static unsigned char seq_tb[16] = {
	0x60,		/* 0: notype */
	0x78,		/* 1: note */
	0x78,		/* 2: rest */
	0x60,		/* 3: bar */
	0x10,		/* 4: clef */
	0x30,		/* 5: timesig */
	0x28,		/* 6: keysig */
	0x38,		/* 7: tempo */
	0x00,		/* 8: staves */
	0x78,		/* 9: mrest */
	0x40,		/* 10: part */
	0x78,		/* 11: mrep */
	0x50,		/* 12: grace */
	0x08,		/* 13: fmtchg */
	0x70,		/* 14: tuplet */
	0x00,		/* 15: whistle */
};

struct STAFF_S {
	struct clef_s clef;	/* base clef */
	unsigned brace:1;	/* 1st staff of a brace */
	unsigned brace_end:1;	/* 2nd staff of a brace */
	unsigned bracket:1;	/* 1st staff of a bracket */
	unsigned bracket_end:1;	/* last staff of a bracket */
	unsigned forced_clef:1;	/* explicit clef */
	unsigned stop_bar:1;	/* stop drawing measure bar on this staff */
	unsigned empty:1;	/* no symbol on this staff */
	float y;		/* y position */
	float scale;
#ifdef YSTEP
	float top[YSTEP], bot[YSTEP];	/* top/bottom y offsets */
#endif
};


/* lyrics */
#define MAXLY	32	/* max number of lyrics */
struct lyl {
	//struct FONTSPEC* f;	/* font */
	//float w;		/* width */
	float s;		/* shift / note */
	char t[1];		/* word */
};
struct lyrics {
	struct lyl *lyl[MAXLY];	/* ptr to lyric lines */
};

#define TEX_BUF_SZ 512
static char tex_buf[TEX_BUF_SZ];	/* result of tex_str() */

#define DFONT_MIN 5		/* index of dynamic fonts */
#define DFONT_MAX 16		/* max number of dynamic fonts */




class AbcToXmlConverter {

private:

// XML
    SScorePartwise _xmlscore;
    SScoreHeader _xmlheader;
	    SWork _xmlwork;
	    SIdentification _xmlidentification;
	    bool _title_set;
    	SPartList _xmlpartlist;
    
// ABC

	struct abctune * _myabctune;

    AbcParser * _abcparser;

	struct VOICE_S _voice_tb[MAXVOICE]; 	/* voice table */
	struct VOICE_S * _first_voice; /* first_voice */

	struct SYMBOL * _voice_over; 			/* voice overlay */
	struct SYMBOL * _words;					/* lyrics after tune (W:) */

	struct STAFF_S _staff_tb[MAXSTAFF];		// tabulka notovych osnov
	int _nstaff;							/* (0..MAXSTAFF-1) */ // pocet nalezenych notovych osnov - 1
	int _staves_found;						

	struct SYMBOL * _grace_head, * _grace_tail;

	//output music
	struct WHISTLE_S _whistle_tb[MAXWHISTLE];
	int _nwhistle;

	struct SYMBOL *_tsnext;	/* next line when cut */
	int _insert_meter;	/* flag to insert time signature */


	void handle_info(struct SYMBOL *s, int info_type);
		struct ISTRUCT info, default_info;
		void add_info(struct SYMBOL *s, struct SYMBOL **p_info);	//register abc symbol
		void sym_link(struct SYMBOL *s, int type);		/* -- link a symbol in a voice -- */
		
	struct VOICE_S * _curvoice;				/* current voice while parsing */

	void reinicialize_voices();
	void output_music(void);
	void voice_dup(void);			/* -- duplicate the symbols of the voices appearing in many staves -- */
	void init_music_line(struct VOICE_S *p_voice);	 /* -- init symbol list with clef, meter, key -- */
	struct SYMBOL * delsym(struct SYMBOL *s);	/* -- delete a symbol -- */
	struct SYMBOL * add_sym(struct VOICE_S *p_voice, int type); /* -- add a new symbol at end of list -- */
	
	void set_global(void);		/* -- set the staves and stems when multivoice -- */
								/* this function is called only once per tune */
	void def_tssym(void);
	/* -- sort the symbols by time -- */
	/* this function is called only once for the whole tune */
	
	void set_float(void);
		/* -- set the staff of the floating voices -- */
	/* this function is called only once per tune */
	
	void set_clef(int staff);
	/* -- define the clef for a staff -- */
	/* this function is called only once for the whole tune */
	
	/* -- insert a clef change (treble or bass) before a symbol -- */
	void insert_clef(struct SYMBOL *s, int clef_type);
	
	/* -- set the pitch of the notes according to the clefs -- */
	/* also set the vertical offset of the symbols */
	/* it supposes that the first symbol of each voice is the clef */
	/* this function is called only once per tune */
	void set_pitch(void);
	
	/* -- set the stem direction when multi-voices -- */
	/* and adjust the vertical offset of the rests */
	/* this function is called only once per tune */
//	void set_multi(void);

	void handle_voice_mark(struct SYMBOL *s);
	
	char * handle_lyric(char *p);

	int _lyric_nb;			/* current number of lyric lines */
	struct SYMBOL * _lyric_start;	/* 1st note of the line for w: */
	struct SYMBOL * _lyric_cont;	/* current symbol when w: continuation */

	void handle_note(struct SYMBOL *s);

	int _bar_number;			/* (for %%setbarnb) */
	void handle_bar(struct SYMBOL *s);
	
	int _over_bar;				/* voice overlay in a measure */
	void handle_overlay(struct SYMBOL *s);

	void handle_clef(struct SYMBOL *s);

	/* -- set the duration of notes/rests in a tuplet -- */
	void handle_tuplet(struct SYMBOL *t);


	void fill_xml();
	
public:
	AbcToXmlConverter(AbcParser * abcparser, struct abctune * myabctune);
	~AbcToXmlConverter();

	SScorePartwise convert_abc_to_xml();
	
};

#endif /*ABC2XMLCONVERTER_H_*/
