
#include "abc2xmlconverter.h"
#include "amautils.h"

/* -- get a voice overlay -- */
void AbcToXmlConverter::handle_overlay(struct SYMBOL *s)
{
	struct VOICE_S *p_voice, *p_voice2;
	struct SYMBOL *s1, *s2;
	int ctime, linenum, colnum;

	/* treat the end of overlay */
	p_voice = _curvoice;
	linenum = s != 0 ? s->as.linenum : 0;
	colnum = s != 0 ? s->as.colnum : 0;
#if 0
	if (over_bar) {
		struct SYMBOL *s2;

		s2 = add_sym(p_voice, BAR);
		s2->as.type = ABC_T_BAR;
		s2->as.linenum = linenum;
		s2->as.colnum = colnum;
	}
#endif
	if (s == 0 || s->as.u.v_over.type == V_OVER_E)  {
		_over_bar = 0;
		if (_voice_over == 0) {
//TODO			error(1, s, "Erroneous end of voice overlap");
			return;
		}
		_voice_over = 0;
		if (s != 0)
			p_voice = &_voice_tb[s->as.u.v_over.voice];
		else 
			for (p_voice = p_voice->prev; ; p_voice = p_voice->prev)
				if (p_voice->name[0] != '&')
					break;
		_curvoice = p_voice;
		return;
	}

	/* treat the overlay start */
	if (s->as.u.v_over.type == V_OVER_S) {
		_voice_over = s;
		return;
	}

	/* create the extra voice if not done yet */
	p_voice2 = &_voice_tb[s->as.u.v_over.voice];
	if (p_voice2->name == 0) {
		p_voice2->name = "&";
		p_voice2->second = 1;
		p_voice2->cstaff = p_voice2->staff = p_voice->staff;
		p_voice2->scale = p_voice->scale;
#if 0
		memcpy(&p_voice2->clef, &p_voice->clef,
			sizeof p_voice2->clef);
		memcpy(&p_voice2->meter, &p_voice->meter,
			sizeof p_voice2->meter);
#endif
		if ((p_voice2->next = p_voice->next) != 0)
			p_voice2->next->prev = p_voice2;
		p_voice->next = p_voice2;
		p_voice2->prev = p_voice;
	}

	/* search the start of sequence */
	ctime = p_voice2->time;
	if (_voice_over == 0) {
		_voice_over = s;
		_over_bar = 1;
		for (s = p_voice->last_symbol; /*s != 0*/; s = s->prev)
			if (s->type == BAR || s->time <= ctime)
				break;
	} else {
		struct SYMBOL *tmp;

		tmp = s;
		s = (struct SYMBOL *) _voice_over->as.next;
/*fixme: what if this symbol is not in the voice?*/
		if (s->voice != _curvoice - _voice_tb) {
	//TODO		error(1, s, "Voice overlay not closed");
			_voice_over = 0;
			return;
		}
		_voice_over = tmp;
	}

	/* search the last common sequence */
	for (s1 = s; /*s1 != 0*/; s1 = s1->prev)
		if (s1->time <= ctime)
			break;

	/* fill the secundary voice with invisible rests */
	if (p_voice2->last_symbol == 0
	    || p_voice2->last_symbol->type != BAR) {
		for (s2 = s1; s2 != 0 && s2->time == ctime; s2 = s2->prev) {
			if (s2->type == BAR) {
				s1 = s2;
				break;
			}
		}
		if (s1->type == BAR) {
			s2 = add_sym(p_voice2, BAR);
			s2->as.linenum = linenum;
			s2->as.colnum = colnum;
			s2->as.u.bar.type = s1->as.u.bar.type;
			s2->time = ctime;
		}
	}
	while (ctime < s->time) {
		while (s1->time < s->time) {
			s1 = s1->next;
			if (s1->type == BAR)
				break;
		}
		if (s1->time != ctime) {
			s2 = add_sym(p_voice2, REST);
			s2->as.type = ABC_T_REST;
			s2->as.linenum = linenum;
			s2->as.colnum = colnum;
			s2->as.u.note.invis = 1;
			s2->len = s2->as.u.note.lens[0] = s1->time - ctime;
			s2->time = ctime;
			ctime = s1->time;
		}
		while (s1->type == BAR) {
			s2 = add_sym(p_voice2, BAR);
			s2->as.linenum = linenum;
			s2->as.colnum = colnum;
			s2->as.u.bar.type = s1->as.u.bar.type;
			s2->time = ctime;
			if ((s1 = s1->next) == 0)
				break;
		}
	}
	p_voice2->time = ctime;
	_curvoice = p_voice2;
}



/* -- change the accidentals and "\\n" in the guitar chords -- */
void gchord_adjust(struct SYMBOL *s)
{
	char *p;
	int freegchord, l;

//	s->gcf = cfmt.gcf;
//	s->anf = cfmt.anf;
//	freegchord = cfmt.freegchord;
	p = s->as.text;
	if (*p != '\0' && strchr("^_<>@", *p) != 0)
		freegchord = 1;		/* annotation */
/*fixme: treat 'dim' as 'o', 'halfdim' as '/o', and 'maj' as a triangle*/
	while (*p != '\0') {
		switch (*p) {
		case '#':
			if (!freegchord)
				*p = '\201';
			break;
		case 'b':
			if (!freegchord)
				*p = '\202';
			break;
		case '=':
			if (!freegchord)
				*p = '\203';
			break;
		case '\\':
			p++;
			switch (*p) {
			case '\0':
				return;
			case 'n':
				p[-1] = ';';
				goto move;
			case '#':
				p[-1] = '\201';
				goto move;
			case 'b':
				p[-1] = '\202';
				goto move;
			case '=':
				p[-1] = '\203';
			move:
				l = strlen(p);
				memmove(p, p + 1, l);
				p--;
				break;
			}
			break;
		}
		if (*p == ';' && p[-1] != '\\')
			if (p[1] != '\0' && strchr("^_<>@", p[1]) != 0)
				*p = '\n';
		if (*p == '\n') {
			if (p[1] != '\0' && strchr("^_<>@", p[1]) != 0)
				freegchord = 1;
//			else	
//				freegchord = cfmt.freegchord;
		}
		p++;
	}
}



/* -- measure bar -- */
void AbcToXmlConverter::handle_bar(struct SYMBOL *s)
{
	struct SYMBOL *s2;

	int bar_type = s->as.u.bar.type;

	/* remove the repeat indication if not wanted */
	if (_curvoice->norepbra && s->as.u.bar.repeat_bar) {
		s->as.u.bar.repeat_bar = 0;
		s->as.text = 0;
	}

	/* remove the invisible repeat bars when no shift needed */
	if (bar_type == B_INVIS
	    && (_curvoice == _first_voice
		|| _curvoice->second
		|| _staff_tb[_curvoice->staff - 1].stop_bar)) {
		s2 = _curvoice->last_symbol;
		if (s2 != 0 && s2->type == BAR && s2->as.text == 0) 
		{
			s2->as.text = s->as.text;
			s2->as.u.bar.repeat_bar = s->as.u.bar.repeat_bar;
			if (s->sflags & S_EOLN)
				s2->sflags |= S_EOLN;
			return;
		}
	}

	/* merge back-to-back repeat bars */
	if (bar_type == B_LREP && s->as.text == 0) {
		int t;

		s2 = _curvoice->last_symbol;
		t = _curvoice->time;
		while (s2 != 0 && s2->time == t) {
			if (s2->type == BAR
			    && s2->as.u.bar.type == B_RREP) {
				s2->as.u.bar.type = B_DREP;
				if (s->sflags & S_EOLN)
					s2->sflags |= S_EOLN;
				return;
			}
			s2 = s2->prev;
		}
	}

	sym_link(s, BAR);

	if ((bar_type & 0xf0) != 0) {
		do {
			bar_type >>= 4;
		} while ((bar_type & 0xf0) != 0);
		if (bar_type == B_COL)
			s->sflags |= S_RRBAR;
	}

	if (_bar_number != 0
	    && _curvoice == _first_voice) {
		s->u = _bar_number;
		_bar_number = 0;
	}

	/* the bar must be before a key signature */
/*fixme: and also before a time signature*/
	if ((s2 = s->prev) != 0 && s2->type == KEYSIG && (s2->prev == 0 || s2->prev->type != BAR)) {
		_curvoice->last_symbol = s2;
		s2->next = 0;
		s2->prev->next = s;
		s->prev = s2->prev;
		s->next = s2;
		s2->prev = s;
	}

	/* convert the decorations */
//	if (s->as.u.bar.dc.n > 0)
//		deco_cnv(&s->as.u.bar.dc, s);

	/* adjust the guitar chords */
	if (s->as.text != 0 && !s->as.u.bar.repeat_bar)
		gchord_adjust(s);
}



/* -- set head type, dots, flags for note -- */
void handle_note_details(struct SYMBOL *s,
		   int len,
		   int *p_head,
		   int *p_dots,
		   int *p_flags)
{
	int head, dots, flags, base;

	if (len >= BREVE * 4) {
		//TODO error(1, s, "Note too long");
		len = BREVE * 2;
	} else if (len < SEMIQUAVER / 8) {
		//TODO error(1, s, "Note too short");
		len = SEMIQUAVER / 8;
	}
	flags = -4;
	for (base = BREVE * 2; base > 0; base >>= 1) {
		if (len >= base)
			break;
		flags++;
	}
	head = H_FULL;
	switch (base) {
	case BREVE * 2:
		head = H_SQUARE;
		break;
	case BREVE:
		head = H_SQUARE;
		break;
	case SEMIBREVE:
		head = H_OVAL;
		break;
	case MINIM:
		head = H_EMPTY;
		break;
	}

	dots = 0;
	if (len == base)
		;
	else if (2 * len == 3 * base)
		dots = 1;
	else if (4 * len == 7 * base)
		dots = 2;
	else if (8 * len == 15 * base)
		dots = 3;
//	else	
	//	error(1, s, "Note too much dotted");

	*p_head = head;
	*p_dots = dots;
	*p_flags = flags;
}






/* -- note or rest -- */
void AbcToXmlConverter::handle_note(struct SYMBOL *s)
{
	int i, m, type;

	s->nhd = m = s->as.u.note.nhd; // number of notes in chord
	type = s->as.type == ABC_T_NOTE ? NOTE : REST;
	
	if (!s->as.u.note.grace) 
	{							/* normal note/rest */
		if (_grace_head != 0)
			_grace_head = 0;
		sym_link(s, type);
		s->multi = _curvoice->stem;	// stem direction while parsing
	} 
	else 
	{							/* in a grace note sequence */
		int div;

		if (_grace_head == 0) {
			struct SYMBOL *s2;

			s2 = add_sym(_curvoice, GRACE);
			s2->as.linenum = s->as.linenum;
			s2->as.colnum = s->as.colnum;
			_grace_head = s2;
			_grace_head->grace = _grace_tail = s;
			s2->multi = _curvoice->gstem;
		} else {
			_grace_tail->next = s;
			s->prev = _grace_tail;
			_grace_tail = s;
		}
		s->voice = _curvoice - _voice_tb;
		s->staff = _curvoice->cstaff;

		/* adjust the grace note duration */
		if (!_curvoice->key.bagpipe) {
			div = 4;
			if (s->prev == 0)
				if (s->as.next == 0 || s->as.next->type != ABC_T_NOTE || !s->as.next->u.note.grace)
					div = 2;
		} 
		else	
			div = 8;

		for (i = 0; i <= m; i++)
			s->as.u.note.lens[i] /= div;
		s->len = s->as.u.note.lens[0];
		s->type = type;
	}

	/* convert the decorations */
//	if (s->as.u.note.dc.n > 0)
//		deco_cnv(&s->as.u.note.dc, s);

	/* change the figure of whole measure rests */
	if (s->type == REST) 
	{
		if (s->len == 0)		/* if space */
			s->seq = 0;
		else if (s->len == _curvoice->wmeasure) // duration == measure duration while parsing
		{
			if (s->len < BASE_LEN * 2)
				s->as.u.note.lens[0] = BASE_LEN;
			else if (s->len < BASE_LEN * 4)
				s->as.u.note.lens[0] = BASE_LEN * 2;
			else	
				s->as.u.note.lens[0] = BASE_LEN * 4;
		}
	}

	/* sort by pitch the notes of the chord (lowest first) */
	for (;;) {
		int nx = 0;

		for (i = 1; i <= m; i++) {
			if (s->as.u.note.pits[i] < s->as.u.note.pits[i-1]) {
				int k;

#define xch(f) \
	k = s->as.u.note.f[i]; \
	s->as.u.note.f[i] = s->as.u.note.f[i-1]; \
	s->as.u.note.f[i-1] = k
				xch(pits);
				xch(lens);
				xch(accs);
				xch(sl1);
				xch(sl2);
				xch(ti1);
				xch(decs);
#undef xch
				nx++;
			}
		}
		if (nx == 0)
			break;
	}

	memcpy(s->pits, s->as.u.note.pits, sizeof s->pits);		// fill note pitch

	/* get the max head type, number of dots and number of flags */
	{
		int head, dots, nflags, l;

		if ((l = s->as.u.note.lens[0]) != 0) {
			handle_note_details(s, l, &head, &dots, &nflags);
			s->head = head;
			s->dots = dots;
			if (!(s->sflags & S_TREM)) {
				s->nflags = nflags;
				for (i = 1; i <= m; i++) {
					if (s->as.u.note.lens[i] == l)
						continue;
					handle_note_details(s, s->as.u.note.lens[i], &head, &dots, &nflags);
					if (head > s->head)
						s->head = head;
					if (dots > s->dots)
						s->dots = dots;
					if (nflags > s->nflags)
						s->nflags = nflags;
				}
				if (s->sflags & S_XSTEM)
					s->nflags = 0;	/* word start+end */
			} else {	/* 2nd tremolo note */
				s->prev->head = head;
				if (head >= H_OVAL) {
					s->as.u.note.stemless = 1;
					s->prev->as.u.note.stemless = 1;
				}
			}
		}
	}
	if (s->nflags <= -2)
		s->as.u.note.stemless = 1;

	for (i = 0; i <= m; i++) {
		if (s->as.u.note.sl1[i] != 0)
			s->sflags |= S_SL1;
		if (s->as.u.note.sl2[i] != 0)
			s->sflags |= S_SL2;
		if (s->as.u.note.ti1[i] != 0)
			s->sflags |= S_TI1;
	}

	if (s->as.u.note.lyric_start) {
		_lyric_start = s;
		_lyric_cont = 0;
		_lyric_nb = 0;
	}

	/* adjust the guitar chords */
	if (s->as.text != 0)
		gchord_adjust(s);
}




/* -- change string taking care of some tex-style codes -- */
/* Puts \ in front of ( and ) in case brackets are not balanced,
 * interprets all ISOLatin1..6 escape sequences as defined in rfc1345.
 * Returns an estimate of the string width. */
void tex_str(char *s)
{
	char *d, c1, c2, *p_enc, *p;
	int maxlen, i;
//	float w, swfac;

//	w = 0;
	d = tex_buf;
	maxlen = sizeof tex_buf - 1;		/* have room for EOS */
	//if ((i = strcf) < 0)
		//i = 0;
//	swfac = cfmt.font_tb[i].swfac;
//	i = font_enc[cfmt.font_tb[i].fnum];
//	if ((unsigned) i >= sizeof esc_tb / sizeof esc_tb[0])
//		i = 0;
//	p_enc = esc_tb[i];
	while ((c1 = *s++) != '\0') {
		switch (c1) {
		case '\\':			/* backslash sequences */
			if ((c1 = *s++) == '\0')
				break;
			if (c1 == ' ')
				goto addchar1;
			if (c1 == '\\' || (c2 = *s) == '\0') {
				if (--maxlen <= 0)
					break;
				*d++ = '\\';
				goto addchar1;
			}
			/* treat escape with octal value */
			if ((unsigned) (c1 - '0') <= 3
			    && (unsigned) (c2 - '0') <= 7
			    && (unsigned) (s[1] - '0') <= 7) {
				if ((maxlen -= 4) <= 0)
					break;
				*d++ = '\\';
				*d++ = c1;
				*d++ = c2;
				*d++ = s[1];
				c1 = ((c1 - '0') << 6) + ((c2 - '0') << 3) + s[1] - '0';
//				w += cwid((unsigned char) c1) * swfac;
				s += 2;
				break;
			}
			/* convert to rfc1345 */
			switch (c1) {
			case '`': c1 = '!'; break;
			case '^': c1 = '>'; break;
			case '~': c1 = '?'; break;
			case '"': c1 = ':'; break;
			/* special TeX sequences */
			case 'O': c1 = '/'; c2 = 'O'; s--; break;
			case 'o': c1 = '/'; c2 = 'o'; s--; break;
			case 'c': if (c2 == 'c' || c2 == 'C')
					c1 = ',';
				break;
			}
			switch (c2) {
			case '`': c2 = '!'; break;
			case '^': c2 = '>'; break;
			case '~': c2 = '?'; break;
			case '"': c2 = ':'; break;
			}
			for (i = 32 * 3, p = p_enc; --i >= 0; p += 2) {
				if ((*p == c1 && p[1] == c2)
				    || (*p == c2 && p[1] == c1)) {
					s++;
					c1 = (p - p_enc) / 2 + 128 + 32;
					break;
				}
			}
			goto addchar1;
#if 0
		case '{':
		case '}':
			break;
#endif
		case '$':
			if (isdigit((unsigned char) *s)
			    && (unsigned) (*s - '0') < DFONT_MIN) {
				i = *s - '0';
	//			swfac = cfmt.font_tb[i].swfac;
//				i = cfmt.font_tb[i].fnum;
//				i = font_enc[i];
//				if ((unsigned) i >= sizeof esc_tb / sizeof esc_tb[0])
//					i = 0;
//				p_enc = esc_tb[i];
			} else if (*s == '$') {
				if (--maxlen <= 0)
					break;
				*d++ = c1;
//				w += cwid((unsigned char) c1) * swfac;
				s++;
			}
			goto addchar1;
		case '(':
		case ')':			/* ( ) becomes \( \) */
			if (--maxlen <= 0)
				break;
			*d++ = '\\';
			/* fall thru */
		default:		/* other characters: pass through */
		addchar1:
			if (--maxlen <= 0)
				break;
			*d++ = c1;
//			w += cwid((unsigned char) c1) * swfac;
			break;
		}
	}
	*d = '\0';
	return;
//	return w;
}


//handles   "T: bbbbb" and "T:bbbbb"
char * get_text_from_info(SYMBOL * sym)
{
	char * p = sym->as.text+2;
	while (isspace((unsigned char) *p))
	       p++;
	return p;
}



/* -- treat a 'V:' -- */
void AbcToXmlConverter::handle_voice_mark(struct SYMBOL *s)
{
	struct VOICE_S *p_voice;
	int voice = s->as.u.voice.voice;

	p_voice = &_voice_tb[voice];

	if (p_voice->prev == 0 && p_voice != _first_voice) {	/* new voice */

		//add new voice to xml
		SScorePart new_voice = newScorePart(s->as.u.voice.name);
		p_voice->xmlvoice = new_voice;	//save SScorePart pointer
		_xmlpartlist->add(new_voice);	//add new voice to partlist

		struct VOICE_S *p_voice2;

		if (!_staves_found) {
			if (!s->as.u.voice.merge) 
			{
#if MAXSTAFF < MAXVOICE
				if (_nstaff >= MAXSTAFF - 1) {
					
					//TODO error(1, s, "Too many staves");
					return;
				}
#endif
				_nstaff++;
			} 
			else	
				p_voice->second = 1;
			
			p_voice->staff = p_voice->cstaff = _nstaff;
			for (p_voice2 = _first_voice;
			     p_voice2->next != 0;
			     p_voice2 = p_voice2->next)
				;
			p_voice2->next = p_voice;
			p_voice->prev = p_voice2;
		} 
		else	
			p_voice->staff = p_voice->cstaff = _nstaff + 1;


	}


	/* if in tune, switch to this voice */
	switch (s->as.state) {
	case ABC_S_HEAD:
		
		break;
	case ABC_S_TUNE:
	case ABC_S_EMBED:
		_curvoice = p_voice;
		break;
	}

	/* if something has changed, update */
	if (s->as.u.voice.name != 0)
	{
		p_voice->name = s->as.u.voice.name;
		p_voice->xmlvoice->setID(p_voice->name);
	}
	if (s->as.u.voice.fname != 0) 
	{
		p_voice->nm = s->as.u.voice.fname;
		p_voice->xmlvoice->setPartName(p_voice->nm);
	}
	if (s->as.u.voice.nname != 0)
		p_voice->snm = s->as.u.voice.nname;
	if (s->as.u.voice.stem != 0)
		p_voice->stem = s->as.u.voice.stem != 2
			? s->as.u.voice.stem : 0;
	if (s->as.u.voice.gstem != 0)
		p_voice->gstem = s->as.u.voice.gstem != 2
			? s->as.u.voice.gstem : 0;
	if (s->as.u.voice.dyn != 0)
		p_voice->dyn = s->as.u.voice.dyn != 2
			? s->as.u.voice.dyn : 0;
	if (s->as.u.voice.lyrics != 0)
		p_voice->ly_pos = s->as.u.voice.lyrics != 2
			? s->as.u.voice.lyrics : 0;
	if (s->as.u.voice.scale != 0)
		p_voice->scale = s->as.u.voice.scale;
}






AbcToXmlConverter::AbcToXmlConverter(AbcParser * abcparser, struct abctune * myabctune)
{

	//xml init
	_xmlscore = newScorePartwise();      // creates a partwise score
	
	_xmlheader = newScoreHeader();	// creates a score header
	_xmlscore->scoreHeader() = _xmlheader;	// drop it to the score

    	_xmlwork = newWork();
		_xmlheader->work()=_xmlwork;

		_xmlidentification = newIdentification();
		_xmlheader->identification()=_xmlidentification;
	
				SEncoding enc = newEncoding();
				_xmlidentification->encoding() = enc;
				enc->setSoftware("abc2xml by PD");			
	
		_xmlpartlist = newPartList();
		_xmlheader->partList()=_xmlpartlist;
	
	//abc init
	_myabctune = myabctune;
	_abcparser = abcparser;

	clrarena(2);

	_voice_tb[0].name = "1";	/* implicit voice */
		SScorePart new_voice = newScorePart("1");	// default id, can be different
		_voice_tb[0].xmlvoice = new_voice;
		_xmlpartlist->add(new_voice);




//	_voice_over = 0;
//	_words = 0;
//	_nstaff = 0;
	//memset(staff_tb, 0, sizeof staff_tb);
//	_staves_found = 0;

	int i;
	for (i = MAXVOICE; --i >= 0; ) {
		_voice_tb[i].clef.line = 2;	/* treble clef on 2nd line */
		_voice_tb[i].clef.stafflines = 5;
		_voice_tb[i].clef.staffscale = 1;
		_voice_tb[i].meter.nmeter = 1;
		_voice_tb[i].meter.wmeasure = BASE_LEN;
		_voice_tb[i].meter.meter[0].top[0] = '4';
		_voice_tb[i].meter.meter[0].bot[0] = '4';
		_voice_tb[i].wmeasure = BASE_LEN;
		_voice_tb[i].scale = 1;
	}

	reinicialize_voices();



	for (i = 0; i < _nwhistle; i++)
		_voice_tb[_whistle_tb[i].voice].whistle = 1;

}


/* initialize all the voices */
void AbcToXmlConverter::reinicialize_voices()
{
//	voice_init();		/* initialize all the voices */
/* -- re-initialize all potential voices -- */
// ZDROJ: static void voice_init(void)
	int i;
	struct VOICE_S *p_voice;
	for (i = 0, p_voice = _voice_tb; i < MAXVOICE; i++, p_voice++) {
		p_voice->sym = p_voice->last_symbol = 0;
		p_voice->clone = -1;
		p_voice->bar_start = 0;
		p_voice->time = 0;
		p_voice->slur_st = 0;
		p_voice->hy_st = 0;
		p_voice->tie = 0;
		p_voice->rtie = 0;
	}

}



AbcToXmlConverter::~AbcToXmlConverter()
{


}


SScorePartwise AbcToXmlConverter::convert_abc_to_xml()
{
	int info_type;

	_curvoice = _first_voice = _voice_tb;
	
	struct abcsym *as;		//current symbol while working with tune
	struct SYMBOL *s, *s2;

	/* set the note duration - this is needed for tuplets */
	for (as = _myabctune->first_sym; as != 0; as = as->next) {
		if (as->type == ABC_T_NOTE || as->type == ABC_T_REST)
		{
			s = (struct SYMBOL *) as;
			s->len = s->as.u.note.lens[0];
		}
	}


	/* scan the tune */
	_grace_head = 0;

	for (as = _myabctune->first_sym; as != 0; as = as->next) {

		if (_grace_head != 0 && as->type != ABC_T_NOTE)
			_grace_head = 0;
		
		s = (struct SYMBOL *) as;

		switch (as->type) {
		case ABC_T_INFO: {

			info_type = as->text[0];
			for (;;) {
				handle_info(	s, 				//symbol
								as->text[0]); 	//type
				if (as->next == 0 || as->next->type != ABC_T_INFO2)
					break;
				as = as->next;
				s = (struct SYMBOL *) as;
			}
			break;
		    }
		case ABC_T_PSCOM:
//TODO			as = process_pscomment(as);	//pseudocomments (mainly formating)
			break;
		case ABC_T_NOTE:
		case ABC_T_REST:
			handle_note(s);
			break;
		case ABC_T_BAR:
			handle_bar(s);
			if (_over_bar) {
				handle_overlay(0);
				s2 = add_sym(_curvoice, BAR);
				s2->as.linenum = s->as.linenum;
				s2->as.colnum = s->as.colnum;
				s2->as.u.bar.type = s->as.u.bar.type;
				s2->time = s->time;
			}
			break;
		case ABC_T_CLEF:
			handle_clef(s);
			break;
		case ABC_T_EOLN:
			if (_curvoice->last_symbol != 0)
				_curvoice->last_symbol->sflags |= S_EOLN;
			continue;
		case ABC_T_MREST:
		case ABC_T_MREP: 
		{
			int len;

			len = _curvoice->wmeasure * as->u.bar.len;
			if (_curvoice->second) {
				_curvoice->time += len;
				break;
			}
			if (as->type == ABC_T_MREP && as->u.bar.len > 1) {

				/* repeat measure more than 1 time */
				/* 2 times -> (bar - invisible rest - bar - mrep - bar) */
/*fixme: 3 or more times not treated*/
				s2 = add_sym(_curvoice, REST);
				s2->as.type = ABC_T_REST;
				s2->as.linenum = as->linenum;
				s2->as.colnum = as->colnum;
				s2->as.u.note.invis = 1;
				len /= as->u.bar.len;
				s2->len = len;
				_curvoice->time += len;
				s2 = add_sym(_curvoice, BAR);
				s2->as.linenum = as->linenum;
				s2->as.colnum = as->colnum;
				s2->as.u.bar.type = B_SINGLE;
			}
			sym_link(s, as->type == ABC_T_MREST ? MREST : MREP);
			s->len = len;
			if (s->as.text != 0)		/* adjust the */
				gchord_adjust(s);	/* guitar chords */
			break;
		    }
		case ABC_T_V_OVER:
			handle_overlay(s);
			continue;
		case ABC_T_TUPLET:
			handle_tuplet(s);
			break;
		default:
			continue;
		}
		
		if (s->type == 0)	// invalid type
			continue;
		if (_grace_head == 0)
			_curvoice->time += s->len;
		if (s->prev != 0 && s->prev->time == s->time && s->prev->seq == s->seq) {
			if (s->seq != 0)
				s->seq = 0;
			else	s->seq++;
		}

	}

	fill_xml();

//	output_music();
//	buffer_eob();
//	if (!header_only) {
//		if (words != 0)
//			put_words(words);
//		if (cfmt.writehistory)
//			put_history();
//	}
//	if (epsf) {
//		if (nbuf > 0)
//			write_eps();
//	} else	write_buffer();
//
//	if (info.xref != 0) {
//		cfmt = dfmt;	/* restore the format at start of tune */
//		info.xref = 0;
//	}
	
	OUT_DEBUG(("=========================================================================="));
	
	return _xmlscore;
}


/* -- output for parsed symbol list -- */
void AbcToXmlConverter::output_music(void)
{
OUT_DEBUG(("output_music started"));

	struct VOICE_S *p_voice;
	int voice, first_line;
	float lwidth;

	for (p_voice = _first_voice; p_voice; p_voice = p_voice->next)
		if (p_voice->sym != 0)
			break;  /* there're some associated symbols */



	if (p_voice == 0)
		return;		/* no symbol at all */

	lvlarena(2);

	voice_dup();	/* duplicate the voices appearing in many staves */

	for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
		p_voice->time = 0;
		p_voice->cstaff = p_voice->staff;
		if (!p_voice->second)
			memcpy(&_staff_tb[ p_voice->staff].clef, &p_voice->clef, sizeof p_voice->clef);
		init_music_line(p_voice);
	}
	first_line = _insert_meter;
	_insert_meter = 0;

	//alfa_last = 0.1;
	//beta_last = 0;

	_abcparser->check_buffer();	/* dump buffer if not enough space for a music line */

	set_global();			/* set global characteristics */
	if (_first_voice->next != 0) {	/* if many voices */
		//if (cfmt.combinevoices)
			//combine_voices();
//TODO?		set_multi();		/* set the stems direction in 'multi' */
	}
//TODO?	for (p_voice = _first_voice; p_voice; p_voice = p_voice->next)
//		set_beams(p_voice->sym);	/* decide on beams */
//TODO?	set_stems();			/* set the stem lengths */
//TODO?	if (_first_voice->next != 0)	/* when multi-voices */
//		set_overlap();		/* shift the notes on voice overlap */

	clrarena(3);
	lvlarena(3);
	//	lwidth = ((cfmt.landscape ? cfmt.pageheight : cfmt.pagewidth)
	//		- cfmt.leftmargin - cfmt.rightmargin)
	//			/ cfmt.scale;
	//	if (lwidth < 1 CM) {
	//		error(1, 0, "Bad page width %.1f", lwidth);
	//		lwidth = 20 CM;
	//	}
//KONEC PROCHAZENI	for (;;) {		/* loop over pieces of line for output */
//		float indent, line_height;
//
//		find_piece();
//
//	//		indent = set_indent(first_line);
//	//		set_sym_glue(lwidth - indent);
//	//	if (indent != 0)
//		//	PUT1("%.2f 0 T\n", indent); /* do indentation */
//		//PUT0("/dlsym{\n");
//		//draw_sym_near();
//		//PUT0("}def\n");
//		//line_height = set_staff();
//		//draw_vname(first_line, indent);
//		PUT0("dlsym\n");
//		for (p_voice = _first_voice; p_voice; p_voice = p_voice->next)
//			draw_symbols(p_voice);
//		draw_all_deco();
//		bskip(line_height);
//		if (nwhistle != 0)
//			draw_whistle();
//		if (indent != 0)
//			PUT1("%.2f 0 T\n", -indent);
//		buffer_eob();
//
//		cut_symbols();
//		if (tsnext == 0)
//			break;
//		first_line = 0;
//	}
//	lvlarena(2);
//
//	/* reset the parser */
//	for (voice = MAXVOICE; --voice >= 0; ) {
//		voice_tb[voice].sym = 0;
//		voice_tb[voice].time = 0;
//	}
OUT_DEBUG(("output_music ended"));

}


void AbcToXmlConverter::handle_info(struct SYMBOL *s, int info_type)
{

OUT_DEBUG(("handling: %c", info_type));

	struct ISTRUCT *inf;
	char *p;

	/* change global or local */
	inf = s->as.state == ABC_S_GLOBAL ? &default_info : &info;
	lvlarena(s->as.state != ABC_S_GLOBAL);


	SCreator creator = newCreator();

	switch (info_type) {
	case 'A':
		add_info(s, &inf->area);
		break;
	case 'B':
		add_info(s, &inf->book);
		break;
	case 'C':
		creator->setType("composer");
		creator->setName(get_text_from_info(s));
		
		_xmlidentification->add(creator);
//		add_info(s, &inf->comp);
		break;
	case 'D':
		add_info(s, &inf->disco);
		break;
	case 'd':
	case 'E':
	case 'F':
	case 'G':
		break;
	case 'H':
		add_info(s, &inf->histo);
		break;
	case 'I':
//TODO		process_pscomment(&s->as);	/* same as pseudo-comment */
		break;
	case 'K':
//		get_key(s);
//		if (s->as.state != ABC_S_HEAD)
//			break;
//		tunenum++;
//		PUT2("%% --- %s (%s) ---\n",
//		     info.xref, &info.title->as.text[2]);
//		if (!epsf)
//			bskip(cfmt.topspace);
//		write_heading(t);
//		reset_gen();
//		nbar = nbar_rep = cfmt.measurefirst;	/* measure numbering */
//		_curvoice = _first_voice;		/* switch to the 1st voice */
		break;
	case 'L':
		break;
	case 'M':
//TODO		get_meter(s);
		break;
	case 'N':
		add_info(s, &inf->notes);
		break;
	case 'O':
		add_info(s, &inf->orig);
		break;
	case 'P':
		//if (!cfmt.printparts)
		//	break;
		switch (s->as.state) {
		case ABC_S_GLOBAL:
		case ABC_S_HEAD:
			inf->parts = s;
			break;
		case ABC_S_TUNE: {
			struct VOICE_S *p_voice;

			p_voice = _curvoice;
			_curvoice = _first_voice;
			sym_link(s, PART);
			if (p_voice->time != _curvoice->time) {
//TODO				error(1, s, "Misplaced P:");
				if (p_voice->time > _curvoice->time)
					s->time = _curvoice->time = p_voice->time;
				else	p_voice->time = _curvoice->time;
			}
			_curvoice = p_voice;
			break;
		    }
		default:
			sym_link(s, PART);
			break;
		}
		break;
	case 'Q':
		if (_curvoice != _first_voice)	/* tempo only for first voice */
		    // || !cfmt.printtempo)
			break;
		switch (s->as.state) {
		case ABC_S_GLOBAL:
		case ABC_S_HEAD:
			inf->tempo = s;
			break;
		default:
			sym_link(s, TEMPO);
			break;
		}
		break;
	case 'R':
		add_info(s, &inf->rhythm);
		break;
	case 'S':
		add_info(s, &inf->src);
		break;
	case 's':
		break;
	case 'T':
		switch (s->as.state) {
		case ABC_S_GLOBAL:	/* T: without X: */
			goto newtune;
		case ABC_S_HEAD:
		
			if(! _title_set)
			{
				_xmlwork->setTitle(get_text_from_info(s));
				_title_set = true;
			}
			else
				_xmlwork->setNum(get_text_from_info(s));
//			add_info(s, &inf->title);
			break;
		default:	//   ABC_S_TUNE in tune (after K:)
					//or ABC_S_EMBED embedded header (between [..])
			output_music();
//TODO			buffer_eob();
//			write_title(s);
//			bskip(cfmt.musicspace + 0.2 CM);
//			voice_init();
//			reset_gen();		/* (display the time signature) */
//			curvoice = _first_voice;
			break;
		}
		break;
	case 'U': {
//TODO		unsigned char *deco;
//
//		deco = s->as.state == ABC_S_GLOBAL ? deco_glob : deco_tune;
//		deco[s->as.u.user.symbol] = deco_intern(s->as.u.user.value);
		break;
	    }
	case 'u':
		break;
	case 'V':
		handle_voice_mark(s);
		break;
	case 'w':
OUT_DEBUG(("xxxxxxxxxxxxxxxxxxxxxxxxxxxxx w founded"));

		if (s->as.state != ABC_S_TUNE)
			break;
		if (_lyric_start == 0)
			break;
		p = &s->as.text[2];
		while (isspace((unsigned char) *p))
		       p++;

		if ((p = handle_lyric(p)) != 0)
			//TODOerror(1, s, "%s", p);
			printf("ERROR\n");
		_curvoice->have_ly = 1;
		break;
	case 'W':
//TODO		if (s->as.state == ABC_S_GLOBAL)
//			break;
//		add_info(s, &words);
		break;
	case 'X':	// starting new tune
newtune:
		//if (!epsf)
		//	write_buffer();	/* flush stuff left from %% lines */
		//dfmt = cfmt;		/* save the format at start of tune */
		memcpy(&info, &default_info, sizeof info);
		p = &s->as.text[2];
		while (isspace((unsigned char) *p))
		       p++;
		info.xref = p;
		if (info_type == 'T')
			info.title = s;
		//memcpy(&deco_tune, &deco_glob, sizeof deco_tune);
		reinicialize_voices();
		break;
	case 'Z':
		add_info(s, &inf->trans);
		break;
	default:
//TODO		error(1, s, "%s info '%c:' not treated",
//			state_txt[(int) s->as.state], info_type);
		break;
	}
	
	
};

void AbcToXmlConverter::add_info(struct SYMBOL *s, struct SYMBOL **p_info)
{
	struct SYMBOL *prev;

	prev = *p_info;
	if (prev == 0 || (prev->as.state == ABC_S_GLOBAL && s->as.state != ABC_S_GLOBAL)) {
		*p_info = s;
		return;
	}
	while (prev->next != 0)
		prev = prev->next;
	prev->next = s;
	s->prev = prev;	
	
};

/* -- link a symbol in a voice -- */
void AbcToXmlConverter::sym_link(struct SYMBOL *s, int type)
{
	struct VOICE_S *p_voice = _curvoice;

/*	memset((&s->as) + 1, 0, sizeof (struct SYMBOL) - sizeof (struct abcsym)); */
	if (p_voice->sym != 0) {
		p_voice->last_symbol->next = s;
		s->prev = p_voice->last_symbol;
	} else	p_voice->sym = s;
	p_voice->last_symbol = s;

	s->type = type;
	s->seq = seq_tb[type];
	s->voice = p_voice - _voice_tb;
	s->staff = p_voice->cstaff;
	s->time = p_voice->time;
}

/* -- duplicate the symbols of the voices appearing in many staves -- */
void AbcToXmlConverter::voice_dup(void)
{
	struct VOICE_S *p_voice, *p_voice1;
	struct SYMBOL *s, *s2, *g, *g2;

	for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
		int voice;

		if (p_voice->clone < 0)
			continue;
		voice = p_voice - _voice_tb;
		//p_voice1 je muj vzor
		p_voice1 = &_voice_tb[(unsigned) p_voice->clone];
		p_voice->name = p_voice1->name;
		for (s = p_voice1->sym;
		     s != 0;
		     s = s->next) {
			s2 = (struct SYMBOL *) getarena(sizeof (*s2));
			memcpy(s2, s, sizeof *s2);
			if (p_voice->sym != 0) {
				p_voice->last_symbol->next = s2;
				s2->prev = p_voice->last_symbol;
			} else	p_voice->sym = s2;
			p_voice->last_symbol = s2;
			s2->voice = voice;
			s2->staff = p_voice->staff;
			s2->ly = 0;
			if ((g = s2->grace) != 0) {
				g2 = (struct SYMBOL *) getarena(sizeof *g2);
				memcpy(g2, g, sizeof *g2);
				s2->grace = g2;
				s2 = g2;
				s2->voice = voice;
				s2->staff = p_voice->staff;
				for (g = g->next; g != 0; g = g->next) {
					g2 = (struct SYMBOL *) getarena(sizeof *g2);
					memcpy(g2, g, sizeof *g2);
					s2->next = g2;
					g2->prev = s2;
					s2 = g2;
					s2->voice = voice;
					s2->staff = p_voice->staff;
				}
			}
		}
	}
}


/* -- init symbol list with clef, meter, key -- */
void AbcToXmlConverter::init_music_line(struct VOICE_S *p_voice)
{
	struct SYMBOL *s, *sym;

	sym = p_voice->sym;
	p_voice->sym = 0;

	//	/* output the first postscript sequences */
	//	if (sym != 0) {
	//		while (sym->type == FMTCHG
	//		       && sym->u == PSSEQ) {
	//			PUT1("%s\n", sym->as.text);
	//			if ((sym = delsym(sym)) == 0)
	//				break;
	//		}
	//	}

	/* add clef */
	if (sym != 0 && sym->type == CLEF && !p_voice->second && p_voice->staff == sym->staff) {
		int stafflines;
		float staffscale;

		if ((stafflines = sym->as.u.clef.stafflines) < 0)
			stafflines = _staff_tb[p_voice->staff].clef.stafflines;
		if ((staffscale = sym->as.u.clef.staffscale) == 0)
			staffscale = _staff_tb[p_voice->staff].clef.staffscale;
		if (sym->as.u.clef.type >= 0)
			memcpy(&_staff_tb[p_voice->staff].clef,
				&sym->as.u.clef,
				sizeof sym->as.u.clef);
		_staff_tb[p_voice->staff].clef.stafflines = stafflines;
		_staff_tb[p_voice->staff].clef.staffscale = staffscale;
		sym = delsym(sym);
	}
	s = add_sym(p_voice, CLEF);
	memcpy(&p_voice->clef, &_staff_tb[p_voice->staff].clef,
	       sizeof p_voice->clef);
	memcpy(&s->as.u.clef, &p_voice->clef, sizeof s->as.u.clef);

	if (!p_voice->second) {

		/* add keysig */
		if (sym != 0 && sym->type == KEYSIG) {
			memcpy(&p_voice->key, &sym->as.u.key,
				 sizeof sym->as.u.key);
			sym = delsym(sym);
		}
		if (p_voice->key.sf != 0 || p_voice->key.nacc != 0) {
			s = add_sym(p_voice, KEYSIG);
			memcpy(&s->as.u.key, &p_voice->key, sizeof s->as.u.key);
			if (s->as.u.key.bagpipe && s->as.u.key.sf == 2)	/* K:Hp */
				s->u = 3;			/* --> G natural */
		}

		/* add time signature if needed */
		if (_insert_meter
		    && p_voice->meter.nmeter != 0) {	/* != M:none */
			s = add_sym(p_voice, TIMESIG);
			memcpy(&s->as.u.meter, &p_voice->meter,
			       sizeof s->as.u.meter);
		}

		/* add tempo if any */
		if (info.tempo) {
			s = info.tempo;
			memset((&s->as) + 1, 0,
			       sizeof (struct SYMBOL) - sizeof (struct abcsym));
			p_voice->last_symbol->next = s;
			s->prev = p_voice->last_symbol;
			p_voice->last_symbol = s;
			s->voice = p_voice - _voice_tb;
			s->staff = p_voice->staff;
			s->type = TEMPO;
#if 1
			s->seq = s->prev->seq;	/* same x offset as prev sym */
#else
			s->seq = s->prev->seq + 1;	/*??*/
#endif
			info.tempo = 0;
		}

		/* add bar if needed */
		if (p_voice->bar_start != 0) {
			int i;

			i = 4;
			if (p_voice->bar_text == 0	/* if repeat continuation */
			    && p_voice->bar_start == B_OBRA) {
				for (s = sym; s != 0; s = s->next) {	/* search the end of repeat */
					if (s->type == BAR) {
						if ((s->as.u.bar.type & 0xf0)	/* if complex bar */
						    || s->as.u.bar.type == B_CBRA
						    || s->as.u.bar.repeat_bar)
							break;
						if (--i < 0)
							break;
					}
				}
				if (s == 0 || sym == 0)
					i = -1;
				if (i >= 0 && sym->time == s->time)
					i = -1;		/* no note */
			}
			if (i >= 0) {
				s = add_sym(p_voice, BAR);
				s->as.u.bar.type = p_voice->bar_start;
				s->as.text = p_voice->bar_text;
				s->as.u.bar.repeat_bar = p_voice->bar_repeat;
			}
			p_voice->bar_start = 0;
			p_voice->bar_repeat = 0;
			p_voice->bar_text = 0;
		}
	}
/*fixme: should be before the first note*/
	if (p_voice->whistle)
		s = add_sym(p_voice, WHISTLE);

	if ((p_voice->last_symbol->next = sym) != 0)
		sym->prev = p_voice->last_symbol;
}

/* -- delete a symbol -- */
struct SYMBOL * AbcToXmlConverter::delsym(struct SYMBOL *s)
{
	if (s->next != 0)
		s->next->prev = s->prev;
	if (s->prev != 0)
		s->prev->next = s->next;
	if (s->ts_next != 0)
		s->ts_next->ts_prev = s->ts_prev;
	if (s->ts_prev != 0)
		s->ts_prev->ts_next = s->ts_next;
	if (_tsnext == s)
		_tsnext = s->ts_next;
	return s->next;
}

/* -- add a new symbol at end of list -- */
struct SYMBOL * AbcToXmlConverter::add_sym(struct VOICE_S *p_voice, int type)
{
	struct SYMBOL *s;
	struct VOICE_S *p_voice2;

	s = (struct SYMBOL *) getarena(sizeof *s);
	memset(s, 0, sizeof *s);
	p_voice2 = _curvoice;
	_curvoice = p_voice;
	sym_link(s, type);
	_curvoice = p_voice2;
	return s;
}


/* -- set the staves and stems when multivoice -- */
/* this function is called only once per tune */
void AbcToXmlConverter::set_global(void)
{
	int staff;
	struct SYMBOL *s;
	struct VOICE_S *p_voice;

#ifndef CLEF_TRANSPOSE
	int old_behaviour, done;

	/* adjust the pitches if old abc2ps behaviour of clef definition */
	old_behaviour = done = 0;
	for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
		int max, min;

		if (!p_voice->forced_clef
		    || p_voice->clef.type == PERC)
			continue;

		/* search if any pitch is too high for the clef */
		max = 100;
		min = -100;
		for (s = p_voice->sym; s != 0; s = s->next) {
			switch (s->type) {
			case CLEF:
				if (s->as.u.clef.type < 0)
					continue;
				if (!s->as.u.clef.check_pitch) {
					max = 100;
					min = -100;
					continue;
				}
				switch (s->as.u.clef.type) {
				case TREBLE:
				case PERC:
					max = 100;
					min = -100;
					break;
				case ALTO:
					max = 25;	/* e */
					min = 14;	/* G, */
					break;
				case BASS:
					max = 21;	/* A */
					min = 10;	/* C, */
					break;
				}
				continue;
			default:
				continue;
			case NOTE:
				if (s->pits[0] < min) {
					done = 1;
					break;		/* new behaviour */
				}
				if (s->pits[s->nhd] <= max)
					continue;
				old_behaviour = 1;
				done = 1;
				break;
			}
			break;
		}
		if (done)
			break;
	}
	if (old_behaviour) {
		for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
			int delta;

			if (!p_voice->forced_clef
			    || p_voice->clef.type == PERC)
				continue;
			delta = 0;
			for (s = p_voice->sym; s != 0; s = s->next) {
				struct SYMBOL *g;
				int i;

				switch (s->type) {
				case CLEF:
					if (s->as.u.clef.type < 0)
						continue;
					if (!s->as.u.clef.check_pitch)
						delta = 0;
					else switch (s->as.u.clef.type) {
						default: delta = 0; break;
						case ALTO: delta = -7; break;
						case BASS: delta = -14; break;
					}
				default:
					continue;
				case NOTE:
				case GRACE:
					if (delta == 0)
						continue;
					break;
				}
				if (s->type == NOTE) {
					for (i = s->nhd; i >= 0; i--)
						s->pits[i] += delta;
				} else {
					for (g = s->grace; g != 0; g = g->next) {
						for (i = g->nhd; i >= 0; i--)
							g->pits[i] += delta;
					}
				}
			}
		}
	}
#endif

	/* set a pitch for all symbols, the start/end of words
	 * and the sequence number of undrawn symbols */
	for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
		int pitch, start_flag;
		struct SYMBOL *sym, *lastnote;

		sym = p_voice->sym;
		pitch = 22;				/* 'B' - if no note! */
		for (s = sym; s != 0; s = s->next) {
			if (s->type == NOTE) {
				pitch = s->pits[0];
				break;
			}
		}
		while (sym != s) {
			sym->pits[0] = pitch;
			sym = sym->next;
		}
		start_flag = 1;
		lastnote = 0;
		for (s = sym; s != 0; s = s->next) {
			switch (s->type) {
			default:
				if ((s->sflags & S_EOLN) == 0)
					break;
				/* fall thru */
			case BAR:
			case MREST:
			case MREP:
				if (lastnote != 0
				    && !(s->sflags & S_BEAM_ON)) {
					lastnote->as.u.note.word_end = 1;
					start_flag = 1;
					lastnote = 0;
				}
				if (s->type == BAR
				    && s->next == 0
				    && s->prev->type == NOTE
				    && s->prev->len >= BREVE)
					s->prev->head = H_SQUARE;
				break;
			case NOTE:
			case REST:
				if (s->sflags & S_TREM)
					break;
				if (s->nflags <= 0 && s->len > 0) {
					if (lastnote != 0) {
						lastnote->as.u.note.word_end = 1;
						lastnote = 0;
					}
					s->as.u.note.word_end = start_flag = 1;
					s->sflags |= S_WORD_ST;
				} else if (s->type == NOTE) {
					if (start_flag)
						s->sflags |= S_WORD_ST;
					if (s->sflags & S_EOLN)
						s->as.u.note.word_end = 1;
					start_flag = s->as.u.note.word_end;
					lastnote = s;
				} else if (s->as.u.note.word_end
					   || (s->sflags & S_EOLN)) {
					if (lastnote != 0) {
						lastnote->as.u.note.word_end = 1;
						lastnote = 0;
					}
					s->as.u.note.word_end = 0;
					start_flag = 1;
				}
				break;
			}
			if (s->type == NOTE) {
				pitch = s->pits[0];
				if (s->prev->type != NOTE) {
					s->prev->pits[0] = (s->prev->pits[0]
							    + pitch) / 2;
				}
			} else	s->pits[0] = pitch;
		}
		if (lastnote != 0)
			lastnote->as.u.note.word_end = 1;
	}

	/* sort the symbols by time */
	def_tssym();

	/* align the undrawn symbols with the previous ones */
	for (s = _first_voice->sym; s != 0; s = s->ts_next) {
		struct SYMBOL *s2;
		int seq;

		s2 = 0;
		switch (s->type) {
		case FMTCHG:
			if (s->u == STBRK || s->xmx != 0)
				goto setsq;
		case TEMPO:
		case PART:
		case STAVES:
		case TUPLET:
		case WHISTLE:
			if (s2 == 0)
				s2 = s;
			break;
		default:
setsq:
			if (s2 == 0)
				break;
			seq = s->seq;
			do {
				s2->seq = seq;
				s2 = s2->ts_next;
			} while (s2 != s);
			s2 = 0;
			break;
		}
	}

	/* set the staff of the floating voices */
	set_float();

	/* set the clefs */
//	if (cfmt.autoclef) {
		for (p_voice = _first_voice; p_voice; p_voice = p_voice->next)
			if (p_voice->forced_clef)
				_staff_tb[p_voice->staff].forced_clef = 1;
		for (staff = 0; staff <= _nstaff; staff++) {
			if (!_staff_tb[staff].forced_clef)
				set_clef(staff);
		}
//	}

	/* set the starting clefs and adjust the note pitches */
	for (p_voice = _first_voice; p_voice; p_voice = p_voice->next)
		memcpy(&p_voice->sym->as.u.clef,
		       &_staff_tb[p_voice->staff].clef,
		       sizeof p_voice->sym->as.u.clef);
	set_pitch();
}


/* -- sort the symbols by time -- */
/* this function is called only once for the whole tune */
void AbcToXmlConverter::def_tssym(void)
{
	struct SYMBOL *s, *t, *prev_sym;
	int time, bars, seq, fl;
	struct VOICE_S *p_voice;

	for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
		p_voice->s_anc = 0;
		p_voice->selected = 0;
	}

	/* sort the symbol by time */
	prev_sym = 0;
	s = 0;		/* compiler warning */
	for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
		s = p_voice->sym;
		s->ts_prev = prev_sym;
		if (prev_sym != 0)
			prev_sym->ts_next = s;
		prev_sym = s;
		p_voice->s_anc = s->next;
	}
	bars = 0;			/* (for errors) */
	for (;;) {

		/* search the closest next time/sequence */
		time = (unsigned) ~0 >> 1;		/* max int */
		seq = -1;
		for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
			if ((s = p_voice->s_anc) == 0
			    || s->time > time)
				continue;
			if (s->time < time) {
				time = s->time;
				seq = s->seq;
			} else if (s->seq < seq)
				seq = s->seq;
		}
		if (seq < 0)
			break;		/* echu (finished) */

		/* warn about incorrect number of notes / measures */
		fl = 0;
		for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
			if ((s = p_voice->s_anc) != 0
			    && s->time == time
			    && s->seq == seq) {
				p_voice->selected = 1;
				switch (s->type) {
				case BAR:
					if (s->as.u.bar.type != B_INVIS)
						fl |= 1;
					break;
				case MREST:
					fl |= 2;
					break;
				}
				
			} else	p_voice->selected = 0;
		}

		if (fl & 1) {		/* if any bar */
			int ko = 0;

			bars++;
			for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
				if ((s = p_voice->s_anc) == 0)
					continue;
				if (s->time != time) {	/* bad time */
//TODO					error(1, s, "Bad bar at measure %d for voice %s", bars, p_voice->name);
					ko = 1;
					break;
				}
			}
			if (ko) {
				for (p_voice = _first_voice;
				     p_voice;
				     p_voice = p_voice->next) {
					if ((t = p_voice->s_anc) == 0
					    || t->type != BAR)
						continue;
					time = s->time + s->len;
					for (; t != 0; t = t->next) {
						t->time = time;
						time += t->len;
					}
				}
				bars--;
				continue;
			}
		}
		if ((fl & 2)		/* if any mrest alone in a staff */
		    && _first_voice->next != 0) {	/* and many voices */
			fl = 0;
			for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
				if ((s = p_voice->s_anc) == 0)
					continue;
				if (s->type != MREST) {
					fl = 1;
					break;
				}
			}
			if (fl) {	/* if not only multi measure rests */
				for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
					int i, dt, time2;

					if (p_voice->second
					    || (s = p_voice->s_anc) == 0
					    || s->type != MREST)
						continue;
					p_voice->last_symbol = s->prev;
					i = s->as.u.bar.len;
					dt = s->len / i;
					time2 = s->time;
					t = add_sym(p_voice, REST);
					t->as.type = ABC_T_REST;
					t->as.linenum = s->as.linenum;
					t->as.colnum = s->as.colnum;
					t->len = t->as.u.note.lens[0] = dt;
					t->head = H_FULL;
					t->nflags = -2;
					t->time = time2;
					if (s->as.u.bar.dc.n > 0)
						memcpy(&t->as.u.note.dc,
							&s->as.u.bar.dc,
							sizeof t->as.u.note.dc);
					p_voice->s_anc = t;
					time2 += dt;
					while (--i > 0) {
						t = add_sym(p_voice, BAR);
						t->as.type = ABC_T_BAR;
						t->as.u.bar.type = B_SINGLE;
						t->as.linenum = s->as.linenum;
						t->as.colnum = s->as.colnum;
						t->time = time2;
						t = add_sym(p_voice, REST);
						t->as.type = ABC_T_REST;
						t->as.linenum = s->as.linenum;
						t->as.colnum = s->as.colnum;
						t->len = t->as.u.note.lens[0] = dt;
						t->head = H_FULL;
						t->nflags = -2;
						t->time = time2;
						time2 += dt;
					}
					t = p_voice->last_symbol;
					if ((t->next = s->next) != 0)
						s->next->prev = t;
				}
			}
		}

		/* set the time linkage */
		for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
			if (!p_voice->selected)
				continue;
			s = p_voice->s_anc;
			s->ts_prev = prev_sym;
			prev_sym->ts_next = s;
			prev_sym = s;
			p_voice->s_anc = s->next;
		}
	}
}


/* -- set the staff of the floating voices -- */
/* this function is called only once per tune */
void AbcToXmlConverter::set_float(void)
{
	struct VOICE_S *p_voice;
	int staff, staff_chg;
	struct SYMBOL *s, *s1;

	for (p_voice = _first_voice; p_voice; p_voice = p_voice->next) {
		if (!p_voice->floating)
			continue;
		staff_chg = 0;
		staff = p_voice->staff;
		for (s = p_voice->sym; s != 0; s = s->next) {
			signed char up, down;

			if (s->type != NOTE) {
				if (staff_chg)
					s->staff++;
				continue;
			}
			if (s->pits[0] >= 19) {		/* F */
				staff_chg = 0;
				continue;
			}
			if (s->pits[s->nhd] <= 12) {	/* F, */
				staff_chg = 1;
				s->staff++;
				continue;
			}
			up = 127;
			for (s1 = s->ts_prev; s1 != 0; s1 = s1->ts_prev) {
				if (s1->staff != staff
				    || s1->voice == s->voice)
					break;
/*fixme:test again*/
if (s1->type == NOTE)
/*fixme:test*/
				if (/*s1->type == NOTE
				    &&*/ s1->pits[0] < up)
					up = s1->pits[0];
			}
			if (up == 127) {
				if (staff_chg)
					s->staff++;
				continue;
			}
			if (s->pits[s->nhd] > up - 3) {
				staff_chg = 0;
				continue;
			}
			down = -127;
			for (s1 = s->ts_next; s1 != 0; s1 = s1->ts_next) {
				if (s1->staff != staff + 1
				    || s1->voice == s->voice)
					break;
/*fixme:test again*/
if (s1->type == NOTE)
/*fixme:test*/
				if (/*s1->type == NOTE
				    &&*/ s1->pits[s1->nhd] > down)
					down = s1->pits[s1->nhd];
			}
			if (down == -127) {
				if (staff_chg)
					s->staff++;
				continue;
			}
			if (s->pits[0] < down + 3) {
				staff_chg = 1;
				s->staff++;
				continue;
			}
			up -= s->pits[s->nhd];
			down = s->pits[0] - down;
			if (!staff_chg) {
				if (up < down + 3)
					continue;
				staff_chg = 1;
			} else {
				if (up < down - 3) {
					staff_chg = 0;
					continue;
				}
			}
			s->staff++;
		}
	}
}


/* -- define the clef for a staff -- */
/* this function is called only once for the whole tune */
void AbcToXmlConverter::set_clef(int staff)	//new
{
	struct SYMBOL *s, *last_chg;
	int clef_type, min, max, time;

	/* get the max and min pitches */
	min = max = 16;			/* 'C' */
	for (s = _first_voice->sym; s != 0; s = s->ts_next) {
		if (s->staff != staff || s->type != NOTE)
			continue;
		if (s->pits[0] < min)
			min = s->pits[0];
		else if (s->pits[s->nhd] > max)
			max = s->pits[s->nhd];
	}

	_staff_tb[staff].clef.type = TREBLE;
	_staff_tb[staff].clef.line = 2;
	if (min >= 13)			/* all upper than 'G,' --> treble clef */
		return;
	if (max <= 19) {		/* all lower than 'F' --> bass clef */
		_staff_tb[staff].clef.type = BASS;
		_staff_tb[staff].clef.line = 4;
		return;
	}

	/* set clef changes */
	clef_type = TREBLE;
	last_chg = 0;
	for (s = _first_voice->sym; s != 0; s = s->ts_next) {
		struct SYMBOL *s2, *s3, *s4;

		if (s->staff != staff || s->type != NOTE)
			continue;

		/* check if a clef change may occur */
		time = s->time;
		if (clef_type == TREBLE) {
			if (s->pits[0] > 12		/* F, */
			    || s->pits[s->nhd] > 20)	/* G */
				continue;
			s2 = s->ts_prev;
			if (s2->time == time
			    && s2->staff == staff
			    && s2->type == NOTE
			    && s2->pits[0] >= 19)	/* F */
				continue;
			s2 = s->ts_next;
			if (s2 != 0
			    && s2->staff == staff
			    && s2->time == time
			    && s2->type == NOTE
			    && s2->pits[0] >= 19)	/* F */
				continue;
		} else {
			if (s->pits[0] < 12		/* F, */
			    || s->pits[s->nhd] < 20)	/* G */
				continue;
			s2 = s->ts_prev;
			if (s2->time == time
			    && s2->staff == staff
			    && s2->type == NOTE
			    && s2->pits[0] <= 13)	/* G, */
				continue;
			s2 = s->ts_next;
			if (s2 != 0
			    && s2->staff == staff
			    && s2->time == time
			    && s2->type == NOTE
			    && s2->pits[0] <= 13)	/* G, */
				continue;
		}

		/* go backwards and search where to insert a clef change */
#if 1 /*fixme:test*/
		s3 = s;
#else
		if (!voice_tb[s->voice].second
		    && voice_tb[s->voice].staff == staff)
			s3 = s;
		else	s3 = 0;
#endif
		time = last_chg == 0 ? 0 : last_chg->time;
		for (s2 = s->ts_prev; s2 != last_chg; s2 = s2->ts_prev) {
			if (s2->time <= time)
				break;
			if (s2->staff != staff)
				continue;
			if (s2->type == BAR) {
#if 0 /*fixme:test*/
				if (voice_tb[s2->voice].second
				    || voice_tb[s2->voice].staff != staff)
					continue;
#endif
				s3 = s2;
				break;
			}
#if 1
			if (s2->type != NOTE)
#else
			if (s2->len == 0)	/* neither note nor rest */
#endif
				continue;

			/* exit loop if a clef change cannot occur */
			if (s2->type == NOTE) {
				if (clef_type == TREBLE) {
					if (s2->pits[0] >= 19)		/* F */
						break;
				} else {
					if (s2->pits[s2->nhd] <= 13)	/* G, */
						break;
				}
			}

#if 1 /*fixme:test*/
#if 1
			/* have a 2nd choice on beam start */
			if ((s3->sflags & S_WORD_ST) == 0)
				s3 = s2;
#else
			/* have a 2nd choice on beam start */
			if ((s2->sflags & S_WORD_ST)
			    || (s3->sflags & S_WORD_ST) == 0)
				s3 = s2;
#endif
#else
			/* have a 2nd choice if word starts on the main voice */
			if (!voice_tb[s2->voice].second
			    && voice_tb[s2->voice].staff == staff) {
				if ((s2->sflags & S_WORD_ST)
				    || s3 == 0
				    || (s3->sflags & S_WORD_ST) == 0)
					s3 = s2;
			}
#endif
		}
		s2 = last_chg;
		last_chg = s;

		/* if first change, see if any note before */
		if (s2 == 0) {
#if 1 /*fixme:test*/
			s4 = s3;
#else
			if ((s4 = s3) == 0)
				s4 = s;
#endif
			for (s4 = s4->ts_prev; s4 != 0; s4 = s4->ts_prev) {
				if (s4->staff != staff)
					continue;
				if (s4->type == NOTE)
					break;
			}

			/* if no note, change the clef of the staff */
			if (s4 == 0) {
				if (clef_type == TREBLE) {
					clef_type = BASS;
					_staff_tb[staff].clef.line = 4;
				} else {
					clef_type = TREBLE;
					_staff_tb[staff].clef.line = 2;
				}
				_staff_tb[staff].clef.type = clef_type;
				continue;
			}
		}

		/* no change possible if no insert point */
#if 1 /*fixme:test*/
		    else if (s3->time == s2->time)
#else
		if (s3 == 0 || s3 == s2)
#endif
			continue;

		/* insert a clef change */
		clef_type = clef_type == TREBLE ? BASS : TREBLE;
		insert_clef(s3, clef_type);
	}
}


/* -- insert a clef change (treble or bass) before a symbol -- */
void AbcToXmlConverter::insert_clef(struct SYMBOL *s,
			int clef_type)
{
	struct VOICE_S *p_voice;
	struct SYMBOL *s2, *new_s;
	int staff, time, seq;

	/* search the main voice of the staff */
	staff = s->staff;
	p_voice = &_voice_tb[s->voice];
	time = s->time;
	seq = s->seq;
	s2 = s;
	if (p_voice->second) {
		for (p_voice = _first_voice; ; p_voice = p_voice->next)
			if (p_voice->staff == staff
			    /* && !p_voice->second */ )
				break;
		while (s2->ts_prev->time == time
		       && s2->ts_prev->seq == seq)
			s2 = s2->ts_prev;
		for (; s2 != 0; s2 = s2->ts_next) {
			if (s2->voice == p_voice - _voice_tb)
				break;
		}
		if (s2 == 0) {
			for (s2 = p_voice->sym; s2->next != 0; s2 = s2->next)
				;
		}
	}

	/* create the symbol */
	p_voice->last_symbol = s2->prev;
	new_s = add_sym(p_voice, CLEF);
	new_s->next = s2;
	s2->prev = new_s;

	new_s->as.u.clef.type = clef_type;
	new_s->as.u.clef.line = clef_type == TREBLE ? 2 : 4;
	new_s->as.u.clef.stafflines = -1;
	new_s->staff = staff;
	new_s->u = 1;		/* small clef */

	/* link in time */
	while (s->ts_prev->time == time
	       && s->ts_prev->seq == seq)
		s = s->ts_prev;
	new_s->ts_prev = s->ts_prev;
	new_s->ts_prev->ts_next = new_s;
	new_s->ts_next = s;
	s->ts_prev = new_s;
	new_s->time = time;
}


/* -- set the pitch of the notes according to the clefs -- */
/* also set the vertical offset of the symbols */
/* it supposes that the first symbol of each voice is the clef */
/* this function is called only once per tune */
void AbcToXmlConverter::set_pitch(void)
{
	struct SYMBOL *s;
	int staff;
	char staff_clef[MAXSTAFF];

	for (s = _first_voice->sym; s != 0; s = s->ts_next) {
		struct SYMBOL *g;
		int delta, np, m, pav;

		staff = s->staff;
		switch (s->type) {
		case CLEF:
			//set_yval(s);
			if (_voice_tb[s->voice].second || s->as.u.clef.type < 0)
				continue;
			switch (s->as.u.clef.type) {
			default:		/* treble / perc */
				delta = 0 - 2 * 2;
				break;
			case ALTO:
				delta = 6 - 3 * 2;
				break;
			case BASS:
				delta = 12 - 4 * 2;
				break;
			}
			staff_clef[staff] = delta + s->as.u.clef.line * 2;
			continue;
		default:
			//set_yval(s);
			if ((g = s->grace) == 0)
				continue;
			delta = staff_clef[staff];
			for (; g != 0; g = g->next) {
				if (delta != 0) {
					for (m = g->nhd; m >= 0; m--)
						g->pits[m] += delta;
				}
				g->ymn = 3 * (g->pits[0] - 18) - 2;
				g->ymx = 3 * (g->pits[g->nhd] - 18) + 2;
			}
			continue;
		case MREST:
			s->ymx = 24 + 15;
			s->ymn = -2;
#ifndef YSTEP
			s->dc_top = 24 + 15;
			s->dc_bot = -2;
#endif
			continue;
		case REST:
			s->y = 12;
			s->ymx = 12 + 8;
			s->ymn = 12 - 8;
#ifndef YSTEP
			s->dc_top = 12 + 8;
			s->dc_bot = 12 - 8;
#endif
			continue;
		case NOTE:
			break;
		}
		np = s->nhd;
		delta = staff_clef[staff];
		if (delta != 0) {
			for (m = np; m >= 0; m--)
				s->pits[m] += delta;
		}
		pav = 0;
		for (m = np; m >= 0; m--)
			pav += s->pits[m];
		s->yav = 3 * pav / (np + 1) - 3 * 18;
		s->ymx = 3 * (s->pits[np] - 18) + 2;
		s->ymn = 3 * (s->pits[0] - 18) - 2;
#ifndef YSTEP
		s->dc_top = s->ymx;
		s->dc_bot = s->ymn;
#endif
	}
}



/* -- parse a lyric (vocal) definition -- */
char * AbcToXmlConverter::handle_lyric(char *p)
{

OUT_DEBUG(("xxxxxxxxxxxxxxxxxxxxxxxxxxxxx get_lyric started"));
	struct SYMBOL *s;
	char word[128], *q;
	int ln;
//	struct FONTSPEC *f;

//	f = &cfmt.font_tb[cfmt.vof];
//	str_font(f);				/* (for tex_str) */

	if ((s = _lyric_cont) == 0) {
		if (_lyric_nb >= MAXLY)
			return "Too many lyric lines";
		ln = _lyric_nb++;
		s = _lyric_start;
	} else	{
		_lyric_cont = 0;
		ln = _lyric_nb - 1;
	}

	/* scan the lyric line */
	while (*p != '\0') {
		while (isspace((unsigned char) *p))
			p++;
		if (*p == '\0')
			break;
		switch (*p) {
		case '|':
			while (s != 0 && (s->type != BAR || s->as.u.bar.type == B_INVIS))
				s = s->next;
			if (s == 0)
				return "Not enough bar lines for lyric line";
			s = s->next;
			p++;
			continue;
		case '-':
			word[0] = '\x02';
			word[1] = '\0';
			p++;
			break;
		case '_':
			word[0] = '\x03';
			word[1] = '\0';
			p++;
			break;
		case '*':
			word[0] = *p++;
			word[1] = '\0';
			break;
		case '\\':
			if (p[1] == '\0') {
				_lyric_cont = s;
				return 0;
			}
			/* fall thru */
		default:
			q = word;
			for (;;) {
				unsigned char c;

				c = *p;
				switch (c) {
				case '\0':
				case ' ':
				case '\t':
				case '_':
				case '*':
				case '|':
					break;
				case '~':
					c = ' ';
					goto addch;
				case '-':
					c = '\x02';
					goto addch;
				case '\\':
					if (p[1] == '\0')
						break;
					switch (p[1]) {
					case '~':
					case '_':
					case '*':
					case '|':
					case '-':
					case ' ':
						c = *++p;
						break;
					}
					/* fall thru */
				default:
				addch:
					if (q < &word[sizeof word - 1])
						*q++ = c;
					p++;
					if (c == '\x02')
						break;
					continue;
				}
				break;
			}
			*q = '\0';
			break;
		}

		/* store word in next note */
		while (s != 0 && s->type != NOTE)
			s = s->next;
		if (s == 0)
			return "Too many words in lyric line";
		if (word[0] != '*') {
			struct lyl *lyl;
			//float w;

			if (s->ly == 0) {
				s->ly = (struct lyrics *) getarena(sizeof (struct lyrics));
				memset(s->ly, 0, sizeof (struct lyrics));
			}
			//w = tex_str(word);
			tex_str(word);

printf("BUFFER: %s\n", tex_str);

			lyl = (struct lyl *) getarena(sizeof *s->ly->lyl[0]
						    + strlen(tex_buf));
			s->ly->lyl[ln] = lyl;
//			lyl->f = f;
//			lyl->w = w;
			strcpy(lyl->t, tex_buf);
		}
		s = s->next;
	}
	while (s != 0 && s->type != NOTE)
		s = s->next;
	if (s != 0)
		return "Not enough words for lyric line";
	return 0;
}

/* -- get a clef definition (in K: or V:) -- */
void AbcToXmlConverter::handle_clef(struct SYMBOL *s)
{
	struct VOICE_S *p_voice;
	struct SYMBOL *s2;
	int i, stafflines;
	float staffscale;

	p_voice = _curvoice;
	if (s->as.prev->type == ABC_T_INFO) {
		switch (s->as.prev->text[0]) {
		case 'K':
			if (s->as.prev->state == ABC_S_HEAD) {
				if (s->as.u.clef.type >= 0) {
					for (i = MAXVOICE, p_voice = _voice_tb;
					     --i >= 0;
					     p_voice++) {
						stafflines = p_voice->clef.stafflines;
						staffscale = p_voice->clef.staffscale;
						memcpy(&p_voice->clef, &s->as.u.clef,
						       sizeof p_voice->clef);
						p_voice->clef.stafflines = stafflines;
						p_voice->clef.staffscale = staffscale;
						p_voice->forced_clef = 1;
					}
				}
				if ((stafflines = s->as.u.clef.stafflines) >= 0) {
					for (i = MAXVOICE, p_voice = _voice_tb;
					     --i >= 0;
					     p_voice++)
						p_voice->clef.stafflines = stafflines;
				}
				if ((staffscale = s->as.u.clef.staffscale) != 0) {
					for (i = MAXVOICE, p_voice = _voice_tb;
					     --i >= 0;
					     p_voice++)
						p_voice->clef.staffscale = staffscale;
				}
				return;
			}
			break;
		case 'V':	/* clef relative to a voice definition */
			p_voice = &_voice_tb[(int) s->as.prev->u.voice.voice];
			break;
		}
	}

	if (p_voice->sym == 0) {
		i = p_voice->staff;
		if ((stafflines = s->as.u.clef.stafflines) < 0)
			stafflines = p_voice->clef.stafflines;
		if ((staffscale = s->as.u.clef.staffscale) == 0)
			staffscale = p_voice->clef.staffscale;
		if (s->as.u.clef.type >= 0) {
			memcpy(&p_voice->clef,
			       &s->as.u.clef, 		/* initial clef */
			       sizeof p_voice->clef);
		}
printf("xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx clef: %d\n", p_voice->clef.type);

		p_voice->clef.stafflines = stafflines;
		p_voice->clef.staffscale = staffscale;
	} else {
		sym_link(s, CLEF);
		s->u = 1;	/* small clef */

		/* the clef change must be before a key signature */
		s2 = s->prev;
		if (s2->type == KEYSIG) {
			s2->next = 0;
			p_voice->last_symbol = s2;
			if ((s->prev = s2->prev) != 0)
				s->prev->next = s;
			s->next = s2;
			s2->prev = s;
		}

		/* the clef change must be before a bar */
		s2 = s->prev;
		if (s2 != 0 && s2->type == BAR) {
			if ((s2->next = s->next) != 0)
				s->next->prev = s2;
			else	p_voice->last_symbol = s2;
			if ((s->prev = s2->prev) != 0)
				s->prev->next = s;
			s->next = s2;
			s2->prev = s;
		}
	}
	if (s->as.u.clef.type >= 0)
		p_voice->forced_clef = 1;		/* don't change */
}


/* -- set the duration of notes/rests in a tuplet -- */
void AbcToXmlConverter::handle_tuplet(struct SYMBOL *t)
{
	struct abcsym *as, *first;
	struct SYMBOL *s;
	int l, r, lplet, grace;

	r = t->as.u.tuplet.r_plet;
	grace = t->as.u.tuplet.grace;
	l = 0;
	first = 0;
	for (as = t->as.next; as != 0; as = as->next) {
/*fixme: KO if voice change..*/
		if (as->type != ABC_T_NOTE && as->type != ABC_T_REST)
			continue;
		if (as->u.note.lens[0] == 0)	/* space ('y') */
			continue;
		if (grace) {
			if (!as->u.note.grace)
				continue;
		} else {
			if (as->u.note.grace)
				continue;
		}
		if (first == 0)
			first = as;
		s = (struct SYMBOL *) as;
		l += s->len;
		if (--r <= 0)
			break;
	}
	if (as == 0) {
	//TODO	error(1, t, "End of tune found inside a tuplet");
		return;
	}
	lplet = (l * t->as.u.tuplet.q_plet) / t->as.u.tuplet.p_plet;
	r = t->as.u.tuplet.r_plet;
	for (as = first; as != 0; as = as->next) {
		int oldlen;

/*fixme: KO if voice change..*/
		if (as->type != ABC_T_NOTE && as->type != ABC_T_REST)
			continue;
		if (as->u.note.lens[0] == 0)
			continue;
		if (grace) {
			if (!as->u.note.grace)
				continue;
		} else {
			if (as->u.note.grace)
				continue;
		}
		s = (struct SYMBOL *) as;
		if (as != first)
			s->sflags |= S_IN_TUPLET;
		oldlen = s->len;
		s->len = (oldlen * lplet) / l;
		if (--r <= 0) {
#if 0
			if (oldlen < QUAVER)
				s->sflags |= S_BEAM_BR1;
#endif
			break;
		}
		l -= oldlen;
		lplet -= s->len;
	}
/*fixme: KO if in a grace sequence*/
	if (grace)
		printf("error\n");
		//TODO error(1, t, "Tuplets in grace note sequence not yet treated");
	else {
		sym_link(t, TUPLET);
//		t->u = cfmt.tuplets;
	}
}

void handle_fill_note(SYMBOL * s, SPWMeasure measure )
{
    SNote note = newNote();
    SPitch pitch = newPitch();                      // creates a pitch
        pitch->setStep("3");                 // sets the pitch to a random value
        pitch->setOctave ("4");            // sets the octave to a random value
    note->pitch() = pitch;                          // and drops the pitch to the note
        note->setDuration ("3");                  // sets the note duration to a quarter note
        note->graphics() = newGraphNote();              // creates the graphic elements of the note
        note->graphics()->setType (NoteType::quarter);  // and sets the graphic note type to quarter note
    measure->add (note);        // and finally adds the note to the measure
}


// fills everything after </part-list>
void AbcToXmlConverter::fill_xml()
{
	struct SYMBOL *s;
	
	// loop over all voices
	for(_curvoice=_first_voice; _curvoice != 0; _curvoice = _curvoice->next)
	{	

		// create new part, set the same ID like in part-list
		SPWPart part = newPWPart(_curvoice->xmlvoice->getID());
		_xmlscore->add(part);
		// create new measure			
		SPWMeasure measure = newPWMeasure("1");
		part->add(measure);
		
		SAttributes attributes = newAttributes();   // creates new attributes
		measure->add (attributes);                  // and adds the attributes to the measure
// CLEF		
		SClef clef = newClef();
		attributes->add(clef); 
			// set clef
			switch (_curvoice->clef.type) {
				case ALTO:
					clef->setSign(TClef::C); 
					break;
				case BASS:
					clef->setSign(TClef::F); 
					break;
				case PERC:
					//TODO:??
				case TREBLE:
				default:
					clef->setSign(TClef::G); 
					break;
			}
			clef->setLine(_curvoice->clef.line);
// KEY		
		SKey key = newKey();
		attributes->key() = key; 
			key->setFifths(_curvoice->key.sf);
			key->setMode(_curvoice->key.minor);		//TODO ??? je to spravne ??

// TIME		
		STimeSign ts = newTimeSign();
		attributes->timeSign() = ts;
//TODO...??
//			ts->add (_curvoice->meter.wmeasure, _curvoice->meter.nmeter);
		
		// sets the attributes division: division is the MusicXML way to indicates how many 
		// divisions per quarter note are used to indicate a note's duration.
		attributes->setDivisions(2);
		
		
		SDirection dir = newDirection();        // creates a direction element
		measure->add (dir);                     // and finally adds the direction to the measure
		SSound sound = newSound();              // creates a sound element
		dir->sound() = sound;                   // and drop the sound element into the direction element
		long tempo = 120;
		sound->setTempo (tempo);                // and sets the sound tempo
		
		SDirectionType dt = newDirectionType(); // creates a direction-type element
		dir->add(dt);                           // adds the direction-type to the direction element
		SMetronome metro = newMetronome();      // creates a metronome element
		dt->add(metro);                         // adds the metronome to the direction-type element
		metro->setBeat (NoteType::quarter);     // and sets the metronome to
		metro->setPerMinute (tempo);            // quarter note = tempo

		



//TODO SYMBOLS:		
//#define NO_TYPE		0	/* invalid type */
//#define REST		2
//#define CLEF		4
//#define TIMESIG 	5
//#define KEYSIG		6
//#define TEMPO		7
//#define STAVES		8
//#define MREST		9
//#define PART		10
//#define MREP		11
//#define GRACE		12
//#define FMTCHG		13
//#define TUPLET		14
//#define WHISTLE		15
		
		
		
// SYMBOLS
		// loop over all symbols in voice
		for (s = _curvoice->sym; s != 0; s = s->next)
			switch (s->type) {
				case NOTE:
					printf("handling note...\n");
					handle_fill_note(s, measure);
					break;
	
				case BAR:	// next measure?
	
					break;
				default:
					printf("!!!!!!!!!! neosefovany typ symbolu: %d\n",s->type);
				break;
			};
	





	}
	

	return;	
}



