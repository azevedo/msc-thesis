#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#ifdef linux
#include <unistd.h>
#endif

#include "abc2ps.h"
// #include "front.h"

/* -- global variables -- */

INFO info;
unsigned char deco_glob[256], deco_tune[256];
struct SYMBOL *sym;		/* (points to the symbols of the current voice) */

int tunenum;			/* number of current tune */
int pagenum = 1;		/* current page in output file */

				/* switches modified by command line flags: */
int quiet;			/* quiet mode */
int secure;			/* secure mode */
int annotate;			/* output source references */
int pagenumbers;		/* write page numbers */
int epsf;			/* for EPSF (1) or SVG (2) output */
int svg;			/* SVG (1) or XML (2 - HTML + SVG) output */
int showerror;			/* show the errors */

char outfn[FILENAME_MAX];	/* output file name */
int file_initialized;		/* for output file */
FILE *fout;			/* output file */
char *in_fname;			/* current input file name */
time_t mtime;			/* last modification time of the input file */
static time_t fmtime;		/*	"	"	of all files */

struct tblt_s *tblts[MAXTBLT];
struct cmdtblt_s cmdtblts[MAXCMDTBLT];
int ncmdtblt;

/* -- local variables -- */

static char abc_fn[FILENAME_MAX]; /* buffer for ABC file name */
static char *styd = DEFAULT_FDIR; /* format search directory */
static int def_fmt_done = 0;	/* default format read */
static struct SYMBOL notitle;

/* memory arena (for clrarena, lvlarena & getarena) */
#define MAXAREAL 3		/* max area levels:
				 * 0; global, 1: tune, 2: generation */
#define AREANASZ 8192		/* standard allocation size */
#define MAXAREANASZ 0x20000	/* biggest allocation size */
static int str_level;		/* current arena level */
static struct str_a {
	struct str_a *n;	/* next area */
	char	*p;		/* pointer in area */
	int	r;		/* remaining space in area */
	int	sz;		/* size of str[] */
	char	str[2];		/* start of memory area */
} *str_r[MAXAREAL], *str_c[MAXAREAL];	/* root and current area pointers */

/* -- local functions -- */
static void read_def_format(void);
static void treat_file(char *fn, char *ext);


/* -- arena routines -- */
void clrarena(int level)
{
	struct str_a *a_p;

	if ((a_p = str_r[level]) == 0) {
		str_r[level] = a_p = malloc(sizeof *str_r[0] + AREANASZ - 2);
		a_p->sz = AREANASZ;
		a_p->n = 0;
	}
	str_c[level] = a_p;
	a_p->p = a_p->str;
	a_p->r = sizeof a_p->str;
}

int lvlarena(int level)
{
	int old_level;

	old_level = str_level;
	str_level = level;
	return old_level;
}

/* The area is 8 bytes aligned to handle correctly int and pointers access
 * on some machines as Sun Sparc. */
void *getarena(int len)
{
	char *p;
	struct str_a *a_p;

	a_p = str_c[str_level];
	len = (len + 7) & ~7;		/* align at 64 bits boundary */
	if (len > a_p->r) {
		if (len > MAXAREANASZ) {
			error(1, 0,
				"getarena - data too wide %d - aborting",
				len);
			exit(EXIT_FAILURE);
		}
		if (len > AREANASZ) {			/* big allocation */
			struct str_a *a_n;

			a_n = a_p->n;
			a_p->n = malloc(sizeof *str_r[0] + len - 2);
			a_p->n->n = a_n;
			a_p->n->sz = len;
		} else if (a_p->n == 0) {		/* standard allocation */
			a_p->n = malloc(sizeof *str_r[0] + AREANASZ - 2);
			a_p->n->n = 0;
			a_p->n->sz = AREANASZ;
		}
		str_c[str_level] = a_p = a_p->n;
		a_p->p = a_p->str;
		a_p->r = a_p->sz;
	}
	p = a_p->p;
	a_p->p += len;
	a_p->r -= len;
	return p;
}
