void clrarena(int level);
int lvlarena(int level);
void *getarena(int len);

struct posit_s {
    unsigned short dyn:2;   /* %%dynamic */
    unsigned short gch:2;   /* %%gchord */
    unsigned short orn:2;   /* %%ornament */
    unsigned short voc:2;   /* %%vocal */
    unsigned short vol:2;   /* %%volume */
    unsigned short std:2;   /* %%stemdir */
    unsigned short gsd:2;   /* %%gstemdir */
};


/* music element */
struct SYMBOL { 		/* struct for a drawable symbol */
	struct abcsym as;	/* abc symbol !!must be the first field!! */
	struct SYMBOL *next, *prev;	/* voice linkage */
	struct SYMBOL *ts_next, *ts_prev; /* time linkage */
	struct SYMBOL *extra;	/* extra symbols (grace notes, tempo... */
	unsigned char type;	/* symbol type */
#define NO_TYPE		0	/* invalid type */
#define NOTEREST	1	/* valid symbol types */
#define SPACE		2
#define BAR		3
#define CLEF		4
#define TIMESIG 	5
#define KEYSIG		6
#define TEMPO		7
#define STAVES		8
#define MREST		9
#define PART		10
#define GRACE		11
#define FMTCHG		12
#define TUPLET		13
#define STBRK		14
#define CUSTOS		15
#define NSYMTYPES	16
	unsigned char voice;	/* voice (0..nvoice) */
	unsigned char staff;	/* staff (0..nstaff) */
	unsigned char nhd;	/* number of notes in chord - 1 */
	int dur;		/* main note duration */
	signed char pits[MAXHD]; /* pitches for notes */
	int time;		/* starting time */
	unsigned int sflags;	/* symbol flags */
#define S_EOLN		0x0001		/* end of line */
#define S_BEAM_ST	0x0002		/* beam starts here */
#define S_BEAM_BR1	0x0004		/* 2nd beam must restart here */
#define S_BEAM_BR2	0x0008		/* 3rd beam must restart here */
#define S_BEAM_END	0x0010		/* beam ends here */
#define S_OTHER_HEAD	0x0020		/* don't draw any note head */
#define S_IN_TUPLET	0x0040		/* in a tuplet */
#define S_TREM2		0x0080		/* tremolo on 2 notes */
#define S_RRBAR		0x0100		/* right repeat bar (when bar) */
#define S_XSTEM		0x0200		/* cross-staff stem (when note) */
#define S_BEAM_ON	0x0400		/* continue beaming */
#define S_SL1		0x0800		/* some chord slur start */
#define S_SL2		0x1000		/* some chord slur end */
#define S_TI1		0x2000		/* some chord tie start */
#define S_PERC		0x4000		/* percussion */
#define S_RBSTOP	0x8000		/* repeat bracket stop */
#define S_FEATHERED_BEAM 0x00010000	/* feathered beam */
#define S_REPEAT	0x00020000	/* sequence / measure repeat */
#define S_NL		0x00040000	/* start of new music line */
#define S_SEQST		0x00080000	/* start of vertical sequence */
#define S_SECOND	0x00100000	/* symbol on a secondary voice */
#define S_FLOATING	0x00200000	/* symbol on a floating voice */
#define S_NOREPBRA	0x00400000	/* don't print the repeat bracket */
#define S_TREM1		0x00800000	/* tremolo on 1 note */
	struct posit_s posit;	/* positions / directions */
	signed char stem;	/* 1 / -1 for stem up / down */
	signed char nflags;	/* number of note flags when > 0 */
	char dots;		/* number of dots */
	unsigned char head;	/* head type */
#define H_FULL		0
#define H_EMPTY 	1
#define H_OVAL		2
#define H_SQUARE	3
	signed char multi;	/* multi voice in the staff (+1, 0, -1) */
	signed char nohdix;	/* no head index (for unison) */
	short u;		/* auxillary information:
				 *	- CLEF: small clef
				 *	- KEYSIG: old key signature
				 *	- BAR: new bar number
				 *	- TUPLET: tuplet format
				 *	- NOTE: tremolo number / feathered beam
				 *	- FMTCHG (format change): subtype */
#define PSSEQ 0				/* postscript sequence */
#define SVGSEQ 1			/* SVG sequence */
#define REPEAT 2			/* repeat sequence or measure
					 *	doty: # measures if > 0
					 *	      # notes/rests if < 0
					 *	nohdix: # repeat */
	float x;		/* x offset */
	signed char y;		/* y offset of note head */
	signed char ymn, ymx, yav; /* min, max, avg note head y offset */
	float xmx;		/* max h-pos of a head rel to top
				 * width when STBRK */
	float xs, ys;		/* coord of stem end / bar height */
	float wl, wr;		/* left, right min width */
	float space;		/* natural space before symbol */
	float shrink;		/* minimum space before symbol */
	float xmax;		/* max x offset */
	float shhd[MAXHD];	/* horizontal shift for heads */
	float shac[MAXHD];	/* horizontal shift for accidentals */
	struct gch *gch;	/* guitar chords / annotations */
	struct lyrics *ly;	/* lyrics */
	signed char doty;	/* NOTEREST: y pos of dot when voices overlap
				 * STBRK: forced
				 * FMTCHG REPEAT: infos */
};
