
#include "abcparser.h"
#include "amautils.h"

AbcParser::AbcParser()
{
	clrarena(0);			/* global desription */
	clrarena(1);			/* tune description */
	clrarena(2);			/* tune generation */
	clrarena(3);			/* line generation */
	clear_buffer();
	abc_init(
		(void *(*)(int size)) getarena, /* alloc */
		0,				/* free */
		(void (*)(int level)) lvlarena, /* new level */
		sizeof(struct SYMBOL) - sizeof(struct abcsym),
		0
			);			/* don't keep comments */	
}


/* -- clear_buffer -- */
void AbcParser::clear_buffer(void)
{
	
	nbuf = 0;
	bposy = 0;
	ln_num = 0;
	mbf = buf;
	
}


/* -- arena routines -- */
void clrarena(int level)
{
	struct str_a *a_p;

	if ((a_p = str_r[level]) == 0) {
		str_r[level] = a_p = (str_a*)malloc(sizeof *str_r[0]);
		a_p->n = 0;
	}
	str_c[level] = a_p;
	a_p->p = a_p->str;
	a_p->r = sizeof a_p->str;

}

void lvlarena(int level)
{
	_str_level = level;
}



// reads file into memory
struct abctune * AbcParser::load_file(char * inputFileName)
{

// ABC read_file
	int fsize;
	FILE *fin;
	char new_file[256];


		struct stat sbuf;

		if ((fin = fopen(inputFileName, "rb")) == 0) {
			if (strlen(inputFileName) >= sizeof new_file - 4)
				return NULL;
			sprintf(new_file, "%s.abc", inputFileName);
			if ((fin = fopen(new_file, "rb")) == 0)
				return NULL;
			inputFileName = new_file;
		}

		if (fseek(fin, 0L, SEEK_END) < 0) {
			fclose(fin);
				return NULL;
		}
		fsize = ftell(fin);
		rewind(fin);
		if ((file = (char *)malloc(fsize + 2)) == 0) {
			fclose(fin);
			return NULL;
		}

		if ((signed)fread(file, 1, fsize, fin) != fsize) {
			fclose(fin);
			free(file);
			return NULL;		
		}
		file[fsize] = '\0';
		fstat(fileno(fin), &sbuf);
		memcpy(&mtime, &sbuf.st_mtime, sizeof mtime);
		fclose(fin);

	fileName=inputFileName;

// init (pro jistotu)
	clrarena(1);
	lvlarena(0);

// parse file (volani generic parseru abcparse.c)
// fill abctune
	my_abctune = abc_parse(file);

	return my_abctune;
}



/* The area is 8 bytes aligned to handle correctly int and pointers access
 * on some machines as Sun Sparc. */
char * getarena(int len)
{
	char *p;
	struct str_a *a_p;

	a_p = str_c[_str_level];
	len = (len + 7) & ~7;		/* align at 64 bits boundary */
	if (a_p->r < len) {
		if (a_p->n == 0) {
			a_p->n = (str_a *)malloc(sizeof *str_r[0]);
			a_p->n->n = 0;
		}
		str_c[_str_level] = a_p = a_p->n;
		a_p->p = a_p->str;
		a_p->r = sizeof a_p->str;
	}
	p = a_p->p;
	a_p->p += len;
	a_p->r -= len;
	
	return p;
	//return;
	
}


/* -- dump buffer if not enough place for a music line -- */
void AbcParser::check_buffer(void)
{
	if (nbuf > BUFFSZ - MAX_MUSIC_LINE_LENGTH) {
//TODO		error(0, 0, "Possibly bad page breaks, BUFFSZ exceeded");
//TODO		write_buffer();
		_use_buffer = 0;
	}
}



