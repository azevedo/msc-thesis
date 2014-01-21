#ifndef ABCPARSER_H_
#define ABCPARSER_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <sys/stat.h>

#include "abcparse.h"

/////////////// CONFIGURATION

#define BUFFSZ		64000				/* size of output buffer */
#define MAX_MUSIC_LINE_LENGTH	5000	/* assume music line < 5000 bytes */


/////////////// ARENA THINGS

//help
	static int _str_level;		/* current arena level */


	void clrarena(int level);
	void lvlarena(int level);
	char * getarena(int len);




////////////////  STRUCTURES

/* memory arena (for clrarena, lvlarena & getarena) */
#define MAXAREAL 4		/* max area levels:
				 * 0; global, 1: tune, 2: output, 3: output line */


struct str_a {
	char	str[4096];	/* memory area */
	char	*p;		/* pointer in area */
	struct str_a *n;	/* next area */
	int	r;		/* remaining space in area */
};

static struct str_a *str_r[MAXAREAL], *str_c[MAXAREAL];	/* root and current area pointers */



/* music element */
struct SYMBOL { 		/* struct for a drawable symbol */
	struct abcsym as;	/* abc symbol !!must be the first field!! */
	struct SYMBOL *next, *prev;	/* voice linkage */
	unsigned char type;	/* symbol type */
#define NO_TYPE		0	/* invalid type */
#define NOTE		1	/* valid symbol types */
#define REST		2
#define BAR		3
#define CLEF		4
#define TIMESIG 	5
#define KEYSIG		6
#define TEMPO		7
#define STAVES		8
#define MREST		9
#define PART		10
#define MREP		11
#define GRACE		12
#define FMTCHG		13
#define TUPLET		14
#define WHISTLE		15
	unsigned char seq;	/* sequence # - see parse.c */
	unsigned char voice;	/* voice (0..nvoice) */
	unsigned char staff;	/* staff (0..nstaff) */
	int len;		/* main note length */
	signed char pits[MAXHD]; /* pitches for notes */
	struct SYMBOL *ts_next, *ts_prev; /* time linkage */
	int time;		/* starting time */
	unsigned short sflags;	/* symbol flags */
#define S_EOLN		0x0001		/* end of line */
#define S_WORD_ST	0x0002		/* word starts here */
#define S_BEAM_BR1	0x0004		/* 2nd beam must restart here */
#define S_NO_HEAD	0x0008		/* don't draw the highest/lowest note head */
#define S_OTHER_HEAD	0x0010		/* don't draw any note head */
#define S_IN_TUPLET	0x0020		/* in a tuplet */
#define S_RRBAR		0x0040		/* right repeat bar (when bar) */
#define S_XSTEM		0x0040		/* cross-staff stem (when note) */
#define S_BEAM_ON	0x0080		/* continue beaming */
#define S_SL1		0x0100		/* some chord slur start */
#define S_SL2		0x0200		/* some chord slur end */
#define S_TI1		0x0400		/* some chord tie start */
#define S_DYNUP		0x0800		/* dynamic marks above the staff */
#define S_DYNDN		0x1000		/* dynamic marks below the staff */
#define S_TREM		0x2000		/* tremolo */
#define S_RBSTOP	0x4000		/* repeat bracket stop */
#define S_BEAM_BR2	0x8000		/* 3rd beam must restart here */
	unsigned char nhd;	/* number of notes in chord - 1 */
	signed char stem;	/* 1 / -1 for stem up / down */
	signed char nflags;	/* number of note flags when > 0 */
	char dots;		/* number of dots */
	unsigned char head;	/* head type */
#define H_FULL		0
#define H_EMPTY 	1
#define H_OVAL		2
#define H_SQUARE	3
	signed char multi;	/* multi voice in the staff (+1, 0, -1) */
	signed char doty;	/* dot y pos when voices overlap */
//	unsigned char gcf;	/* font for guitar chords */
//	unsigned char anf;	/* font for annotations */
	short u;		/* auxillary information:
				 *	- CLEF: small clef
				 *	- KEYSIG: old key signature
				 *	- BAR: new bar number
				 *	- TUPLET: tuplet format
				 *	- FMTCHG (format change): subtype */
#define STBRK 0				/* staff break
					 *	xmx: width */
#define PSSEQ 1				/* postscript sequence */
	float x;		/* position */
	short y;
	short ymn, ymx, yav;	/* min, max, avg note head height */
	float xmx;		/* max h-pos of a head rel to top */
#ifndef YSTEP
	float dc_top;		/* max offset needed for decorations */
	float dc_bot;		/* min offset for decoration */
#endif
	float xs, ys;		/* position of stem end */
	float wl, wr;		/* left, right min width */
	float pl, pr;		/* left, right preferred width */
	float shrink, stretch;	/* glue before this symbol */
	float shhd[MAXHD];	/* horizontal shift for heads */
	float shac[MAXHD];	/* horizontal shift for accidentals */
	struct lyrics *ly;	/* lyrics */
	struct SYMBOL *grace;	/* grace notes */
};




////////////////// FUNCTIONS / UTILS


////////////////// parser class


class AbcParser {

private:

struct SYMBOL *sym;		/* (points to the symbols of the current voice) */


	char * fileName;
	char *file;				//file api
	time_t mtime;			/* last modification time of the input file */

	struct abctune * my_abctune;

	
	//buffer related
	char buf[BUFFSZ];	/* output buffer.. should hold one tune */
	int nbuf;			/* number of bytes buffered */
	float bposy;		/* current position in buffered data */	
	int ln_num;			/* number of lines in buffer */
	char *mbf;			/* where to PUTx() */
	
	int _use_buffer;	//nastavi se, kdyz je buffer prazdy v check_buffer
	
public:
	AbcParser();
	~AbcParser();

	struct abctune * load_file(char * fileName);

	void check_buffer();	/* -- dump buffer if not enough place for a music line -- */
	void clear_buffer(void);




};


#endif /*ABCPARSER_H_*/
