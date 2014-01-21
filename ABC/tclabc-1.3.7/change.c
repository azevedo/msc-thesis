/*++
 * Functions making tune changes.
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
#ifndef WIN32
#include <unistd.h>
#endif

#include "tcl.h"
#include "abcparse.h"
#include "tclabc.h"
#include "midi.h"

static void note_length_adj(struct sym *s);
static void note_merge(struct sym *s);
static struct sym *note_next(struct sym *s);
static struct sym *note_prev(struct sym *s);
static struct sym *note_split (struct sym *s,
			       int delta_time);
static void setfullmap(int sf,		/* key signature (-7 .. +7) */
			char *map);	/* for 70 notes (10 octaves) */
static struct sym *sym_new(struct sym *sref);
static void transpose(struct sym *s,	/* key signature */
			struct key_s *old_key);

/* key signature transposition tables */
static signed char cde2fcg[7] = {0, 2, 4, -1, 1, 3, 5};
static char cgd2cde[7] = {0, 4, 1, 5, 2, 6, 3};

/* -- set the accidentals when changing the bars -- */
static void acc_chg(struct sym *s,
		    char *old_map,
		    char *new_map)
{
	int i, pit, acc, old_acc, new_acc;

	for (i = 0; i <= s->as.u.note.nhd; i++) {
		pit = s->as.u.note.pits[i] + 19;	/* pitch from lowest C */
		acc = s->as.u.note.accs[i];
		old_acc = old_map[pit];
		new_acc = new_map[pit];
		if (acc != A_NULL)
			old_map[pit] = new_map[pit] = (acc == A_NT)
							? A_NULL : acc;
		if (old_acc == new_acc)
			continue;
		if (acc == A_NULL) {
			s->as.u.note.accs[i] = new_acc == A_NULL
				? old_acc : A_NT;
			new_map[pit] = old_acc;
		} else if (acc == new_acc
			|| (acc == A_NT && new_acc == A_NULL)) {
			s->as.u.note.accs[i] = A_NULL;
		}
	}
}

/* -- get the accidentals on last bar and current symbol -- */
static void acc_syn(struct sym *s,
		    char *key_map,	/* 70 notes */
		    char *cur_map)
{
	struct sym *s2;
	int i;

	if (s->type == EOT)
		return;
	setfullmap(s->sf, key_map);
	memcpy(cur_map, key_map, 70);

	/* get back to the previous bar */
	for (s2 = s; s2->type != EOT; s2 = s2->prev)
		if (s2->type == BAR)
			break;
	for (;;) {
		if (s2->type == NOTE) {
			for (i = 0; i <= s2->as.u.note.nhd; i++) {
				int pit, acc;

				if ((acc = s2->as.u.note.accs[i]) == 0)
					continue;
				if (acc == A_NT)
					acc = A_NULL;
				pit = s2->as.u.note.pits[i] + 19; /* pitch from lowest C */
				cur_map[pit] = acc;
			}
		}
		if (s2 == s)
			break;
		s2 = s2->next;
	}
}

/* -- return the beat from the time signature -- */
int beat_get(struct sym *s)
{
	int top, bot;

	if (s->as.u.meter.meter[0].top[0] == 'C') {
		if (s->as.u.meter.meter[0].top[1] == '|')
			return BASE_LEN / 2;
		return BASE_LEN / 4;
	}
	sscanf(s->as.u.meter.meter[0].top, "%d", &top);
	sscanf(s->as.u.meter.meter[0].bot, "%d", &bot);
	if (bot >= 8 && top >= 6 && top % 3 == 0)
		return BASE_LEN * 3 / 8;
	return BASE_LEN / bot;
}

/* -- set the decorations -- */
int deco_set(Tcl_Interp *interp,
	     char *p,
	     struct deco *deco)
{
/*fixme: pb when error*/
/*fixme: pb when deco on note heads*/
	deco->n = 0;
	parse_deco(p, deco);
	return TCL_OK;
}

/* -- delete the current symbol -- */
/* set the current symbol to the next one */
int del_sym(Tcl_Interp *interp)
{
	struct sym *s = curvoice->cursym;
	struct sym *s2, *s_staves;

	switch (s->type) {
	case EOT:
		return TCL_OK;
	case EXTRA_SYM:
		switch (s->EXTRA_type) {
		case EXTRA_VOVER:
		case EXTRA_STAVES:
			goto err;
		}
		break;
	case CLEF:
	case KEYSIG:
	case TIMESIG:
		if (s->time != 0)
			break;
err:		strcpy(interp->result, "this symbol cannot be deleted");
		return TCL_ERROR;
	}

	/* handle the voice overlay */
	if (s->flags & (F_VOVER_S | F_VOVER_E)) {
		strcpy(interp->result, "cannot yet delete vover start/stop");
		return TCL_ERROR;
	}

	/* if a BAR, change the next accidentals */
	if (s->type == BAR) {
		char old_map[70], new_map[70];

		acc_syn(s->prev, old_map, new_map);
		for (s2 = s->next; s2->type != EOT; s2 = s2->prev) {
			if (s2->type == BAR)
				break;
			if (s2->type == NOTE)
				acc_chg(s2, old_map, new_map);
		}
	}

	/* handle the tuplets */
	else if (s->type == TUPLET) {
		int fl;

		s2 = s->next;
		for (;;) {
			if (s2->type == EOT)
				break;
			if (s2->flags & F_NPLET) {
				s2->dtime = s2->as.u.note.lens[0];
				fl = s2->flags;
				s2->flags &= ~(F_NPLET_S | F_NPLET | F_NPLET_E);
				if (fl & F_NPLET_E)
					break;
			}
			s2 = s2->next;
		}
	} else if (s->flags & F_NPLET) {
		s2 = s->next;
		for (;;) {
			if (s2->type == EOT)
				break;
			if (s2->flags & F_NPLET_E)
				break;
			s2 = s2->next;
		}
		if (s2->type != EOT) {
			s2->flags &= ~(F_NPLET_S | F_NPLET | F_NPLET_E);
			s2->dtime = s2->as.u.note.lens[0];
		}
	}

	/* unlink the symbol */
	s->prev->next = s->next;
	s->next->prev = s->prev;

	s_staves = sym_update(s->prev);
	if (s_staves)
		staves_update(s_staves);
	if (s->type == NOTE) {
		if ((s->flags & (F_TIE_S | F_TIE_E))
		    && (s2 = note_next(s)) != 0) {
			if (s->flags & F_TIE_S)
				s2->flags &= ~F_TIE_E;
			if (s->flags & F_TIE_E)
				s2->flags |= F_TIE_E;
		}
		if (s->as.tune->client_data != 0)
			lyrics_change = 1;
	}
	curvoice->cursym = s->next;
	goaltime = s->time;
	goalseq = s->seq;
	if (s->as.tune != 0)
		abc_delete((struct abcsym *) s);
	else	free(s);
	word_update(curvoice->cursym);
	return TCL_OK;
}

/* -- set the header -- */
int header_set(char *p)
{
	struct abctune *new_t;
	struct abcsym *aso, *asn;

	/* parse the header */
	new_t = abc_parse(p);
	if (new_t == 0)
		return TCL_ERROR;

	tune_purge();

	/* exchange the headers */
	asn = new_t->first_sym;
	aso = curtune->first_sym;
	new_t->first_sym = aso;
	curtune->first_sym = asn;
	for ( ; aso->next != 0; aso = aso->next) {
		if (aso->next->state != ABC_S_GLOBAL
		    && aso->next->state != ABC_S_HEAD)
			break;		/* end of the old header */
	}
	for (;;) {
		asn->tune = curtune;	/* change the tune pointer */
		if (asn->next == 0)
			break;		/* end of the new header */
		asn = asn->next;
	}
	if ((asn->next = aso->next) != 0) {
		asn->next->prev = asn;
		aso->next = 0;
	}
	new_t->last_sym = aso;

	abc_free(new_t);		/* free the old header */

	/* re-select the tune */
	tune_select(curtune);
	return TCL_OK;
}

/* -- set the bars from current symbol (for one voice) -- */
void measure_set(void)
{
	struct sym *s, *new_s;
	int tmes, beat, bar_time, beat_time, bar_nb;
	char old_map[70], new_map[70];

	/* go back to the last bar or time signature */
	s = curvoice->cursym;

	if (s->type == EOT)
		return;
	while (s->type != BAR
	       && s->type != TIMESIG) {
		s = s->prev;
		if (s->type == EOT) {
			while (s->type != TIMESIG)
				s = s->next;
			break;
		}
	}
	bar_nb = 1;
	if (s->type == BAR)
		bar_nb = s->BAR_num;

	/* set the measure time */
	new_s = s;
	while (new_s->type != TIMESIG)
		new_s = new_s->prev;
	tmes = new_s->as.u.meter.wmeasure;
	beat = beat_get(new_s);

	/* set the accidental tables,
	 * one for the old measure, the other for the new measure */
	setfullmap(s->sf, old_map);
	memcpy(new_map, old_map, sizeof new_map);

	/* loop on the symbols */
	bar_time = s->time + tmes;
	beat_time = s->time + beat;
	if (s->type == BAR)
		s = s->next;
	for (;;) {
		int curtime;

		curtime = s->time;
		if (curtime > beat_time) {
			int cut_time = 0;
	
			/* the previous symbol (note or rest) may be too long,
			 * split it if needed */
			if (curtime > bar_time)
				beat_time = cut_time = bar_time;
			else if ((cut_time = (curtime - beat_time) % beat) != 0)
				beat_time = cut_time = curtime - cut_time;
			else	beat_time = curtime;
			if (cut_time != 0) {
				s = s->prev;
				new_s = note_split(s, cut_time - s->time);
				note_length_adj(s);
				note_merge(new_s);
				s = new_s;
				continue;
			}
		}
		if (curtime == beat_time) {
			if (s->type != BAR
			 && s->prev->type == NOTE
			 && s->prev->as.u.note.lens[0] < BASE_LEN / 4
			 && !(s->as.flags & ABC_F_SPACE)) {
				s->prev->flags |= F_WORD_E;
				for (new_s = s; ; new_s = new_s->next) {
					if (new_s->type == EOT
					 || new_s->type == BAR)
						break;
					if (new_s->type == NOTE) {
						new_s->flags |= F_WORD_S;
#if 0
						if (!(new_s->flags & F_WORD_E)
						 && (new_s->next->flags & F_WORD_S))
							new_s->next->flags &= ~F_WORD_S;
#endif
						break;
					}
				}
			}
			beat_time += beat;
		}
		if (curtime == bar_time) {
			if (s->type == BAR) {
				setfullmap(s->sf, old_map);
				memcpy(new_map, old_map, sizeof new_map);
				bar_time += tmes;
				if (s->as.u.bar.type != B_OBRA)
					bar_nb++;
				s->BAR_num = bar_nb;
			} else if (s->seq > SQ_BAR /*|| s->type == EOT*/) {

				/* insert a bar before the current symbol */
				if (s->prev->type == NOTE) {
					s->prev->flags |= F_WORD_E;
					for (new_s = s; ; new_s = new_s->next)
						if (new_s->type == NOTE)
							break;
					new_s->flags |= F_WORD_S;
#if 0
					if (!(new_s->flags & F_WORD_E)
					 && (new_s->next->flags & F_WORD_S))
						new_s->next->flags &= ~F_WORD_S;
#endif
				}
				new_s = sym_insert(s->prev);
				new_s->as.type = ABC_T_BAR;
				new_s->as.u.bar.type = B_BAR;
				new_s->type = BAR;
				new_s->seq = SQ_BAR;
				new_s->time = curtime;
				new_s->BAR_num = ++bar_nb;
				new_s->sf = s->sf;

				setfullmap(s->sf, new_map);
				bar_time += tmes;
			}
		} else if (s->type == BAR) {

			/* remove the bar */
			/*fixme: keep if repeat/eoln*/
			if (s->as.u.bar.type != B_OBRA) {
				setfullmap(s->sf, old_map);
				s->prev->next = s->next;
				s->next->prev = new_s = s->prev;
				if (s->as.tune != 0)
					abc_delete((struct abcsym *) s);
				else	free(s);
				s = new_s;
				if (s->type == NOTE || s->type == REST)
					note_merge(s);
			}
		}
		switch (s->type) {
		case NOTE:
			acc_chg(s, old_map, new_map);
			/* fall thru */
		case REST:
			note_length_adj(s);
			break;
		}
		if (s->type == EOT)
			break;
		s = s->next;
	}
}

/* -- set a microtone value -- */
static int micro_set(struct sym *s,
		     int acc)
{
	unsigned short *p_micro;
	int i, micro;

	p_micro = s->as.tune->micro_tb;
	micro = acc >> 3;
	acc &= 0x07;
	for (i = 1; i < MAXMICRO; i++) {
		if (p_micro[i] == 0)
			p_micro[i] = micro;
		if (p_micro[i] == micro)
			return (i << 3) | acc;
	}
/*fixme: have a warning "too many microtone values"*/
	return acc;
}

/* -- adjust the length of notes (measure_set) -- */
/*fixme: what if n-plet?*/
static void note_length_adj(struct sym *s)
{
	for (;;) {
		int len, base;

		/* check the number of dots */
		len = s->as.u.note.lens[0];
		if (len <= 0) {
			trace("Internal bug: note length <= 0\n");
			return;
		}
		base = BASE_LEN / 4;
		if (len >= BASE_LEN / 4) {
			if (len >= BASE_LEN) {
				if (len >= BASE_LEN * 2)
					base = BASE_LEN * 2;
				else	base = BASE_LEN;
			} else if (len >= BASE_LEN / 2)
				base = BASE_LEN / 2;
		} else if (len >= BASE_LEN / 8)
			base = BASE_LEN / 8;
		else if (len >= BASE_LEN / 16)
			base = BASE_LEN / 16;
		else	base = BASE_LEN / 32;
		if (len == base			/* no dot */
		    || 2 * len == 3 * base	/* 1 dot */
		    || 4 * len == 7 * base)	/* 2 dots */
			return;

		/* too many dots, split the note */
		s = note_split(s, base);
	}
}

/* -- try to merge a note/rest with the next one (measure_set) -- */
static void note_merge(struct sym *s)
{
	int i, len;
	struct sym *next;

	next = s->next;
	if (s->type != next->type)
		return;
	if (s->type == NOTE
	    && (!(s->flags & F_TIE_S)
		|| memcmp(s->as.u.note.pits,
			  next->as.u.note.pits,
			  sizeof s->as.u.note.pits) != 0))
		return;
	len = next->as.u.note.lens[0];
	for (i = 0; i <= s->as.u.note.nhd; i++) {
		s->as.u.note.lens[i] += len;
		s->as.u.note.ti1[i] = next->as.u.note.ti1[i];
	}
	s->dtime += next->dtime;
	if (next->flags & F_WORD_E)
		s->flags |= F_WORD_E;
	if (next->flags & F_TIE_S)
		s->flags |= F_TIE_S;
	s->next = next->next;
	s->next->prev = s;
	if (next->as.tune != 0)
		abc_delete((struct abcsym *) next);
	else	free(next);
	/*fixme: set the flags */
	note_length_adj(s);
}

/* -- return the next note -- */
static struct sym *note_next(struct sym *s)
{
	for (s = s->next; ; s = s->next) {
		switch (s->type) {
		case NOTE:
			return s;
		case REST:
		case MREST:
		case MREP:
		case EOT:
			return 0;
		}
	}
	/*NOT REACHED*/
}

/* -- return the previous note -- */
static struct sym *note_prev(struct sym *s)
{
	for (s = s->prev; ; s = s->prev) {
		switch (s->type) {
		case NOTE:
			return s;
		case REST:
		case MREST:
		case MREP:
		case EOT:
			return 0;
		}
	}
	/*NOT REACHED*/
}

/* -- split a note or rest -- */
static struct sym *note_split(struct sym *s,
			      int delta_time)
{
	struct sym *new_s;
	int i;

	new_s = sym_insert(s);
	new_s->as.type = s->as.type;
	memcpy(&new_s->as.u.note, &s->as.u.note,
	       sizeof new_s->as.u.note);
	/*fixme: what with the slurs?*/
	new_s->type = s->type;
	for (i = 0; i <= s->as.u.note.nhd; i++) {
		new_s->as.u.note.lens[i] -= delta_time;
		s->as.u.note.lens[i] = delta_time;
		if (s->type == NOTE)
			s->as.u.note.ti1[i] = SL_AUTO;
	}
	s->dtime = delta_time;
	if (delta_time < BASE_LEN / 4)
		s->flags |= F_WORD_E;
	new_s->dtime = new_s->as.u.note.lens[0];
	new_s->flags = s->flags;
	s->flags |= F_TIE_S;
	new_s->flags |= F_TIE_E;
	new_s->seq = SQ_NOTE;
	new_s->time = s->time + delta_time;
	new_s->sf = s->sf;
	return new_s;
}

/* -- search an ABC symbol for insertion -- */
struct sym *search_abc_sym(struct sym *sref)
{
	struct abcsym *asref;

	if (sref->type == EOT)
		sref = sref->prev;
	while (sref->as.tune == 0) {
		if (sref->type == EOT) {
#if 0
			while (sref->as.tune == 0) {
				sref = sref->next;
				if (sref->type == EOT) {
					trace("Internal bug: no first ABC symbol\n");
					exit(1);
				}
			}
#endif
			break;
		}
		sref = sref->prev;
	}
	asref = (struct abcsym *) sref;

	/* insert the symbol in the voice or after the first 'K:' */
	if (asref->state != ABC_S_TUNE
	    && asref->state != ABC_S_EMBED) {
		if (curvoice->p_voice != 0)
			asref = (struct abcsym *) curvoice->p_voice;
		else {
			if (asref->tune == 0)
				asref = curtune->first_sym;
			for (;;) {
				if (asref->type == ABC_T_INFO
				    && asref->text[0] == 'K')
					break;
				asref = asref->next;
			}
		}
	}
	return (struct sym *) asref;
}

/* -- set the accidentals on the full pitch range -- */
static void setfullmap(int sf,		/* key signature (-7 .. +7) */
			char *map)	/* for 70 notes (10 octaves) */
{
	int i;

	setmap(sf, map);
	for (i = 10; --i > 0; )
		memcpy(&map[7 * i], map, 7);
}

/* -- set the slurs -- */
int slurs_set(Tcl_Interp *interp,
	      Tcl_Obj *obj,
	      struct sym *s)
{
	int objc;
	Tcl_Obj **objv;
	int i;
	int slur_st, slur_end;
	int sl1[MAXHD], sl2[MAXHD];

	if ((i = Tcl_ListObjGetElements(interp,
					obj,
					&objc, &objv)) != TCL_OK)
		return i;
	if (objc == 0) {
		slur_st = slur_end = 0;
		memset(sl1, 0, sizeof sl1);
		memset(sl2, 0, sizeof sl2);
	} else if (objc == (s->as.u.note.nhd + 2) * 2) {
		if (Tcl_GetIntFromObj(interp, *objv++, &slur_st) != TCL_OK)
			return TCL_ERROR;
		if (Tcl_GetIntFromObj(interp, *objv++, &slur_end) != TCL_OK)
			return TCL_ERROR;
		for (i = 0; i <= s->as.u.note.nhd; i++) {
			if (Tcl_GetIntFromObj(interp, *objv++, &sl1[i]) != TCL_OK)
				return TCL_ERROR;
			if (Tcl_GetIntFromObj(interp, *objv++, &sl2[i]) != TCL_OK)
				return TCL_ERROR;
		}
	} else	return tcl_wrong_args(interp,
				      "set slurs #gstart #gend ?#start #end? ...");
	s->as.u.note.slur_st = slur_st;
	s->as.u.note.slur_end = slur_end;
	for (i = 0; i <= s->as.u.note.nhd; i++) {
		s->as.u.note.sl1[i] = sl1[i];
		s->as.u.note.sl2[i] = sl2[i];
	}
	return TCL_OK;
}

/* -- change the value of a string -- */
void str_new(char **s,
	     Tcl_Obj *obj)
{
	char *p;
	int l;

	p = Tcl_GetString(obj);
	l = strlen(p);
	if (l == 2 && *p == '{' && p[1] == '}')
		p = empty_str;

	if (l == 0) {
		if (*s == 0)
			return;
		free(*s);
		*s = 0;
		return;
	}
	if (*s == 0)
		*s = malloc(l + 1);
	else {
		if (strcmp(p, *s) == 0)
			return;
		if (strlen(*s) < l) {
			free(*s);
			*s = malloc(l + 1);
		}
	}
	strcpy(*s, p);
}

/* -- include symbols from ABC description -- */
int sym_include(char *p)
{
	struct sym *sref;
	struct abcsym *as;

	/* search an ABC symbol */
	sref = search_abc_sym(curvoice->cursym);

	/* insert the symbols */
	abc_insert(p, (struct abcsym *) sref);

	/* remove the eoln if no \n */
	p += strlen(p) - 1;
	if (*p != '\n' && *p != '\r') {
		as = sref->as.tune->last_sym;
		if (as->type == ABC_T_EOLN)
			abc_delete(as);
	}

	/* remove the 'L:' if any at the beginning */
	as = sref->as.next;
	if (as->type == ABC_T_INFO
	    && as->text[0] == 'L')
		abc_delete(as);

	/* rescan the tune */
	tune_purge();
	tune_select(sref->as.tune);

	/* set the cursor at the end of the inserted sequence */
	curvoice = &voice_tb[sref->voice];
	sref = (struct sym *) sref->as.tune->last_sym;
	while (sref->type == 0)
		sref = (struct sym *) sref->as.prev;
	curvoice->cursym = sref;
	goaltime = sref->time;
	goalseq = sref->seq;
	return TCL_OK;
}

/* -- insert a new symbol after the current one -- */
struct sym *sym_insert(struct sym *sref)
{
	struct sym *s;

	/* create a symbol (after K:) */
	if (sref->type == EOT)
		sref = sref->prev;
	s = sym_new(sref);

	/* link after the reference */
	sref = (struct sym *) s->as.prev;	/* the ref may have changed */
	if (sref->next == 0)			/* if not in the symbol list */
		sref = curvoice->eot;		/* insert at the head */
	while (sref->next->as.state != ABC_S_TUNE
	       && sref->next->as.state != ABC_S_EMBED) {
		sref = sref->next;
		if (sref->type == EOT) {
			sref = sref->prev;
			break;
		}
	}
	s->next = sref->next;
	s->prev = sref;
	sref->next = s;
	s->next->prev = s;

	s->as.state = ABC_S_TUNE;
	s->voice = sref->voice;
	return s;
}

/* -- create a new ABC symbol (after K:) -- */
static struct sym *sym_new(struct sym *sref)
{
	struct sym *s;

	s = (struct sym *) malloc(sizeof *s);
	memset(s, 0, sizeof *s);

	/* search an ABC symbol */
	sref = search_abc_sym(sref);

	/* link for abcparse */
	s->as.tune = sref->as.tune;
	if ((s->as.next = sref->as.next) != 0)
		s->as.next->prev = (struct abcsym *) s;
	else if (s->as.tune->last_sym == (struct abcsym *) sref)
		s->as.tune->last_sym = (struct abcsym *) s;
	sref->as.next = (struct abcsym *) s;
	s->as.prev = (struct abcsym *) sref;
	return s;
}

/* -- change the values of a symbol or insert a new one -- */
int sym_set(Tcl_Interp *interp,
	    Tcl_Obj *obj,
	    int insert)
{
	int objc, type;
	Tcl_Obj **objv;
	char *val;
	struct sym *s;
	int old_len = 0;
	int new_len = 0;
	static int abc_type[EOT] = {
		ABC_T_NOTE, ABC_T_REST, ABC_T_BAR, ABC_T_CLEF,
		ABC_T_INFO, ABC_T_INFO, ABC_T_INFO, ABC_T_INFO,
		ABC_T_PSCOM, ABC_T_MREST, ABC_T_MREP, ABC_T_NOTE,
		ABC_T_TUPLET};

	if (Tcl_ListObjGetElements(interp, obj, &objc, &objv) != TCL_OK)
		return TCL_ERROR;
	val = Tcl_GetString(*objv++);	/* symbol type */
	objc--;
	s = curvoice->cursym;
	type = BAR;
	switch (*val) {
	case 'b':
		break;
	case 'c':
		type = CLEF;
		break;
	case 'e':
		type = EXTRA_SYM;
		break;
	case 'g':
		type = GRACE;
		break;
	case 'k':
		type = KEYSIG;
		break;
	case 'm':
		if (strcmp(val, "mrep") == 0)
			type = MREP;
		else if (strcmp(val, "mrest") == 0)
			type = MREST;
		else	type = MIDI;
		break;
	case 'n':
		type = NOTE;
		break;
	case 'r':
		type = REST;
		break;
	case 't':
		switch (val[1]) {
		default:
		case 'e': type = TEMPO; break;
		case 'i': type = TIMESIG; break;
		case 'u': type = TUPLET; break;
		}
		break;
	default:
		strcpy(interp->result, "unknown symbol type");
		return TCL_ERROR;
	}

	if (insert) {
		struct sym *s2;

		s2 = sym_insert(s);
		s2->time = s->time + s->dtime;
		s2->sf = s->sf;
		s = s2;
		s->type = type;
		s->as.type = abc_type[type];
		curvoice->cursym = s;
		goaltime = s->time;
		goalseq = s->seq;
		switch (type) {
		case GRACE:
			s->as.flags |= ABC_F_GRACE;
		case NOTE:
		case REST:
			if (s->prev->type != TUPLET)
				break;
			for (s2 = s->next; s2->type != EOT; s2 = s2->next) {
				if (s2->flags & F_NPLET_S) {
					s2->flags &= ~F_NPLET_S;
					break;
				}
			}
			break;
		case TIMESIG:
		case KEYSIG:
		case TEMPO:
			s->as.state = ABC_S_EMBED;
			s->as.text = malloc(2);
			switch (type) {
			case TIMESIG:
				s->as.text[0] = 'M'; break;
			case KEYSIG:
				s->as.text[0] = 'K'; break;
			case TEMPO:
				s->as.text[0] = 'Q'; break;
			}
			s->as.text[1] = '\0';
			break;
		}
	} else {
		if (s->type != type) {
			strcpy(interp->result, "cannot change the symbol type");
			return TCL_ERROR;
		}
		if (s->type == NOTE || s->type == REST || s->type == GRACE)
			old_len = s->as.u.note.lens[0];
	}

	switch (type) {
	case GRACE:
	case NOTE: {
		int nhd, pitch, acc;

		if (Tcl_GetIntFromObj(interp, *objv++, &new_len) != TCL_OK)
			goto err;
		if (--objc & 1) {
			strcpy(interp->result, "odd number of pitch/acc");
			goto err;
		}
		nhd = -1;
		while (objc > 0) {
			if (Tcl_GetIntFromObj(interp, *objv++, &pitch) != TCL_OK)
				goto err;
			if (Tcl_GetIntFromObj(interp, *objv++, &acc) != TCL_OK)
				goto err;
			nhd++;
			s->as.u.note.lens[nhd] = new_len;
			s->as.u.note.pits[nhd] = pitch;
			if (acc & 0xfff8)
				acc = micro_set(s, acc);
			s->as.u.note.accs[nhd] = acc;
			objc -= 2;
		}
		s->as.u.note.chlen = 0;
		if (nhd < 0) {
			strcpy(interp->result, "no note pitch/acc");
			goto err;
		}
		s->as.u.note.nhd = nhd;

		if (type == GRACE)
			new_len = old_len = 0;
		else {
			struct sym *s2, *s3;

			if (new_len != old_len || insert)
				word_update(s);
			if (insert) {
				if ((s2 = note_prev(s)) != 0
				    && (s2->flags & F_TIE_S)) {
					s->flags |= F_TIE_E;
					if ((s3 = note_next(s)) != 0)
						s3->flags &= ~F_TIE_E;
				}
			}
			if (s->as.tune->client_data != 0)
				lyrics_change = 1;
		}
		break;
	    }
	case REST:
		if (Tcl_GetIntFromObj(interp, *objv, &new_len) != TCL_OK)
			goto err;
		s->as.u.note.lens[0] = new_len;
		if (new_len != old_len || insert)
			word_update(s);
		if (insert) {
			struct sym *s2, *s3;
			int i;

			word_update(s);
			if ((s2 = note_prev(s)) != 0
			    && (s2->flags & F_TIE_S)) {
				for (i = 0; i <= s2->as.u.note.nhd; i++)
					s2->as.u.note.ti1[i] = 0;
				s2->flags &= ~F_TIE_S;
				if ((s3 = note_next(s)) != 0)
					s3->flags &= ~F_TIE_E;
			}
		}
		break;
	case BAR: {
		int btype, l;

		l = strlen(Tcl_GetString(objv[1]));
		if (l != 0 && s->BAR_repeat != 0 && !s->as.u.bar.repeat_bar)
			goto err;
		if (Tcl_GetIntFromObj(interp, *objv++, &btype) != TCL_OK)
			goto err;
		s->as.u.bar.type = btype;
		s->flags &= ~F_RRBAR;
		if (btype & 0xf0) {
			do {
				btype >>= 4;
			} while (btype & 0xf0);
			if (btype == B_COL)
				s->flags |= F_RRBAR;
		}
		if (s->as.u.bar.repeat_bar || l != 0)
			str_new(&s->as.text, *objv);
		s->as.u.bar.repeat_bar = l != 0;

		/* if insertion, change the next accidentals */
		if (insert) {
			char old_map[70], new_map[70];
			struct sym *s2;

			acc_syn(s->prev, new_map, old_map);
			for (s2 = s->next; s2->type != EOT; s2 = s2->prev) {
				if (s2->type == BAR)
					break;
				if (s2->type == NOTE)
					acc_chg(s2, old_map, new_map);
			}
			word_update(s);		/* and update the word breaks */
		}
		break;
	    }
	case CLEF: {
		int ctype, cline;

		if (objc != 2) {
			tcl_wrong_args(interp,
				       "set {clef type line}");
			goto err;
		}
		/*fixme: create a K: or V: if insert*/
		if (Tcl_GetIntFromObj(interp, *objv++, &ctype) != TCL_OK)
			goto err;
		if (Tcl_GetIntFromObj(interp, *objv, &cline) != TCL_OK)
			goto err;
		s->as.u.clef.type = ctype;
		s->as.u.clef.line = cline;
		curvoice->forced_clef = 1;
		break;
	    }
	case TIMESIG: {
		int i;

		if ((objc & 1) != 0) {
			tcl_wrong_args(interp,
				       "set {time upval downval ..}");
			goto err;
		}
		objc /= 2;
		if (objc > MAX_MEASURE) {
			strcpy(interp->result,
			       "too many values in time signature");
			goto err;
		}
		s->as.u.meter.nmeter = objc;
		for (i = 0; i < objc; i++) {
			val = Tcl_GetString(*objv++);
			strncpy(s->as.u.meter.meter[i].top,
				val,
				sizeof s->as.u.meter.meter[i].top);
			val = Tcl_GetString(*objv++);
			if (*val == '{' && strlen(val) == 2 && val[1] == '}')
				val = empty_str;
			strncpy(s->as.u.meter.meter[i].bot,
				val,
				sizeof s->as.u.meter.meter[i].bot);
		}
		if (s->as.u.meter.meter[0].top[0] == 'C')
			s->as.u.meter.wmeasure = 4 * BASE_LEN / 4;
		else {
			int top, bot;

			sscanf(s->as.u.meter.meter[0].top, "%d", &top);
			sscanf(s->as.u.meter.meter[0].bot, "%d", &bot);
			if (bot > 0)
				s->as.u.meter.wmeasure = top * BASE_LEN / bot;
		}
		if (!insert) {
			struct sym *s2;
			int voice;

			for (voice = 1; voice <= nvoice; voice++) {
				s2 = voice_go(voice);
				if (s2->type != TIMESIG) {
					/*fixme: force one?*/
					continue;
				}
				memcpy(&s2->as.u.meter,
				       &s->as.u.meter,
				       sizeof s2->as.u.meter);
			}
		}
		break;
	    }
	case KEYSIG: {
		int i, l, sf, nacc, pitch, acc;
		struct key_s old_key;

		if (objc < 3 || !(objc & 1)) {
			tcl_wrong_args(interp,
				"set {key type mode exp|add ?pitch acc? ...}");
			goto err;
		}
		memcpy(&old_key, &s->as.u.key, sizeof old_key);
		nacc = 0;
		val = Tcl_GetString(*objv++);
		sf = atoi(val);
		if (sf < -7 || sf > 7) {
			sprintf(interp->result,
				"bad number of sharps/flats '%s'", val);
			goto err;
		}
		val = Tcl_GetString(*objv++);
		if (strcmp(val, "none") == 0) {
			s->as.u.key.empty = 2;
			s->as.u.key.mode = 0;
			s->as.u.key.exp = 0;
			sf = 0;
		} else {
			l = strlen(val);	/* may have less characters */
			for (i = 0; i < sizeof key_names; i++) {
				if (strncmp(val, key_names[i], l) == 0)
					break;
			}
			if (i >= sizeof key_names) {
				sprintf(interp->result, "bad key mode '%s'", val);
				goto err;
			}
			s->as.u.key.empty = 0;
			if (i == BAGPIPE && val[1] == 'p')
				i++;
			s->as.u.key.mode = i;
		}
		val = Tcl_GetString(*objv++);
		s->as.u.key.exp = *val == 'e';
		objc -= 3;
		while (objc > 0) {
			if (Tcl_GetIntFromObj(interp, *objv++, &pitch) != TCL_OK)
				goto err;
			if (Tcl_GetIntFromObj(interp, *objv++, &acc) != TCL_OK)
				goto err;
			if (acc <= 0 || acc > 5) {
				strcpy(interp->result, "bad accidental");
				goto err;
			}
			s->as.u.key.pits[nacc] = pitch;
			if (acc & 0xfff8)
				acc = micro_set(s, acc);
			s->as.u.key.accs[nacc] = acc;
			nacc++;
			objc -= 2;
		}
		s->as.u.key.nacc = nacc;
		s->as.u.key.sf = sf;
		s->sf = sf;

		if (!insert) {
			struct sym *s2;
			int voice;

			transpose(s, &old_key);
			if (s->as.state == ABC_S_HEAD) {

				/* in the header: change the key for all voices */
				for (voice = 1; voice <= nvoice; voice++) {
					s2 = voice_go(voice);
					if (s2->type != KEYSIG
					 || s2->as.tune != 0)
						continue;
					s2->as.u.key.sf = sf;
					transpose(s2, &old_key);
					do {
						s2->sf = sf;
						s2 = s2->next;
					} while (s2->type != KEYSIG
					      && s2->type != EOT);
				}
			}
			s2 = s->next;
			while (s2->type != KEYSIG && s2->type != EOT) {
				s2->sf = sf;
				s2 = s2->next;
			}
		}
		break;
	    }
	case TEMPO: {
		int i, v;

		if (objc != 7) {
			tcl_wrong_args(interp,
				       "set {tempo str1 len0 len1 len2 len3 value str2}");
			goto err;
		}
		str_new(&s->as.u.tempo.str1, *objv++);
		for (i = 0; i < 4; i++) {
			if (Tcl_GetIntFromObj(interp, *objv++, &v) != TCL_OK)
				goto err;
			s->as.u.tempo.length[i] = v;
		}
		str_new(&s->as.u.tempo.value, *objv++);
		str_new(&s->as.u.tempo.str2, *objv);
		break;
	    }
	case EXTRA_SYM:
		val = Tcl_GetString(*objv);
		switch (*val) {
		case 'b': {		/* blen */
			int len;

			/*fixme: should not be changed nor inserted*/
			objv++;
			if (Tcl_GetIntFromObj(interp, *objv, &len) != TCL_OK)
				goto err;
			s->EXTRA_type = EXTRA_LEN;
			s->EXTRA_L_len = len;
			if (insert)
				s->as.state = ABC_S_EMBED;
			break;
		    }
		case 'i':		/* info */
/*fixme:should check if not recognized infos..*/
			objc--;
			objv++;
			obj = Tcl_ConcatObj(objc, objv);
			str_new(&s->as.text, obj);
			Tcl_DecrRefCount(obj);
			s->as.type = s->as.text[0] == '%'
				? ABC_T_PSCOM : ABC_T_INFO;
			if (insert && s->as.type == ABC_T_INFO)
				s->as.state = ABC_S_EMBED;
			s->EXTRA_type = EXTRA_INFO;
			break;
		case 'n':		/* newline */
			s->as.type = ABC_T_EOLN;
			s->EXTRA_type = EXTRA_NL;
			break;
		case 's':		/* staves */
			if (s->voice != 0) {
				strcpy(interp->result,
					"cannot set/insert a staves definition\n"
					"in voice != 0");
				goto err;
			}
			s->as.type = ABC_T_PSCOM;
			s->EXTRA_type = EXTRA_STAVES;
			objc--;
			objv++;
			obj = Tcl_ConcatObj(objc, objv);
			val = Tcl_GetString(obj);
			if (s->as.text != 0)
				free(s->as.text);
			s->as.text = malloc(9 + strlen(val) + 1);
			strcpy(s->as.text, "%%staves ");
			strcpy(&s->as.text[9], val);
			Tcl_DecrRefCount(obj);
			break;
		default:
			strcpy(interp->result,
				"wrong extra symbol type: should be one of\n"
				"\"blen\", \"info\", \"newline\" or \"staves\"");
			goto err;
		}
		break;
	case MIDI:
		obj = Tcl_ConcatObj(objc, objv);
		val = Tcl_GetString(obj);
		if (s->as.text != 0)
			free(s->as.text);
		s->as.text = malloc(7 + strlen(val) + 1);
		strcpy(s->as.text, "%%MIDI ");
		strcpy(&s->as.text[7], val);
		Tcl_DecrRefCount(obj);
		if (strncmp(&s->as.text[7], "channel ", 8) == 0) {
			if (channel_set(s) != TCL_OK) {
				strcpy(interp->result,
					"bad midi channel value");
				goto err;
			}
#ifdef HAVE_SOUND
			set_channels(s->voice, s->MIDI_channels);
#endif
		} else if (strncmp(&s->as.text[7], "program ", 8) == 0) {
			if (program_set(s) != TCL_OK) {
				strcpy(interp->result,
					"bad midi program value");
				goto err;
			}
#ifdef HAVE_SOUND
			set_program(s->MIDI_channel,
				    s->MIDI_program,
				    s->MIDI_bank);
#endif
		}
		break;
	case TUPLET: {
		int p, q, r;

		if (objc != 3) {
			tcl_wrong_args(interp, "set {tuplet p q r}");
			goto err;
		}
		if (!insert) {
			struct sym *t;

			r = s->as.u.tuplet.r_plet;
			t = s;
			for (;;) {
				t = t->next;
				if (t->type == EOT)
					break;
				if (t->type == NOTE || t->type == REST) {
					t->flags &= ~(F_NPLET_S | F_NPLET | F_NPLET_E);
					t->dtime = t->as.u.note.lens[0];
					if (--r <= 0)
						break;
				}
			}
		}
		if (Tcl_GetIntFromObj(interp, *objv++, &p) != TCL_OK)
			goto err;
		if (Tcl_GetIntFromObj(interp, *objv++, &q) != TCL_OK)
			goto err;
		if (Tcl_GetIntFromObj(interp, *objv++, &r) != TCL_OK)
			goto err;
		s->as.u.tuplet.p_plet = p;
		s->as.u.tuplet.q_plet = q;
		s->as.u.tuplet.r_plet = r;
		if (insert)
			set_tuplet(s);
		old_len = -1;		/* update the next symbols */
		break;
	    }
	case MREST:
		if (Tcl_GetIntFromObj(interp, *objv, &new_len) != TCL_OK)
			goto err;
		s->as.u.bar.len = new_len;
		break;
	default:			/*fixme: should not occur*/
 err:
		if (insert)
			del_sym(interp);
		return TCL_ERROR;
	}
	s->dtime = new_len;
	if (new_len != old_len || insert) {
		if ((s = sym_update(s->prev)) != 0)
			staves_update(s);
	}
	return TCL_OK;
}

/* -- set the ties -- */
int ties_set(Tcl_Interp *interp,
	     Tcl_Obj *obj,
	     struct sym *s)
{
	int objc, i, newfl;
	Tcl_Obj **objv;
	int ti1[MAXHD];
	struct sym *s2;

	if ((i = Tcl_ListObjGetElements(interp,
					obj,
					&objc, &objv)) != TCL_OK)
		return i;
	if (objc == 0)
		memset(ti1, 0, sizeof ti1);
	else if (objc == s->as.u.note.nhd + 1) {
		for (i = 0; i <= s->as.u.note.nhd; i++) {
			if (Tcl_GetIntFromObj(interp, *objv++, &ti1[i]) != TCL_OK)
				return TCL_ERROR;
		}
	} else	return tcl_wrong_args(interp, "set ties bool ?bool? ...");

	newfl = 0;
	for (i = 0; i <= s->as.u.note.nhd; i++) {
		s->as.u.note.ti1[i] = ti1[i];
		if (ti1[i] != 0)
			newfl = 1;
	}
	if (s->flags & F_TIE_S) {
		if (!newfl) {
			s->flags &= ~F_TIE_S;
			if ((s2 = note_next(s)) != 0)
				s2->flags &= ~F_TIE_E;
		}
	} else {
		if (newfl) {
			s->flags |= F_TIE_S;
			if ((s2 = note_next(s)) != 0)
				s2->flags |= F_TIE_E;
		}
	}
	return TCL_OK;
}

/* transpose a note / chord */
static void note_transpose(struct sym *s,
			struct key_s *ckey,
			struct key_s *okey)
{
	int i, j, m, n, a, i1, i2, i3, i4, i5, sf_old;;
	static signed char acc1[6] = {0, 1, 0, -1, 2, -2};
	static char acc2[5] = {A_DF, A_FT, A_NT, A_SH, A_DS};

	m = s->as.u.note.nhd;
	sf_old = okey->sf;
	if (ckey->empty == 0			/* if some key */
	 && okey->empty == 0)
		i2 = ckey->sf - sf_old;
	else					/* K:none */
		i2 = 0;
	for (i = 0; i <= m; i++) {
		n = s->as.u.note.pits[i] + 5;
		i1 = cde2fcg[(n + 252) % 7];	/* fcgdaeb */
		a = s->as.u.note.accs[i];
		if (a == 0) {
			if (okey->nacc == 0) {
				if (sf_old > 0) {
					if (i1 < sf_old - 1)
						a = A_SH;
				} else if (sf_old < 0) {
					if (i1 >= sf_old + 6)
						a = A_FT;
				}
			} else {
				for (j = 0; j < okey->nacc; j++) {
					if ((s->as.u.note.pits[i] + 196
							- okey->pits[j]) % 7
								== 0) {
						a = okey->accs[j];
						break;
					}
				}
			}
		}
		i3 = i1 + i2 + acc1[a] * 7;		/* transpose */
		i4 = n / 7 * 7 + cgd2cde[(unsigned) ((i3 + 252) % 7)];
		if (i4 > n + 3)
			i4 -= 7;
		else if (i4 < n - 3)
			i4 += 7;
		s->as.u.note.pits[i] = i4 - 5;

		i5 = ((i3 + 1 + 21) / 7 + 2 - 3 + 200) % 5;
							/* accidental */
		a = acc2[(unsigned) i5];
		if (okey->empty != 0) {			/* old key none */
			if (ckey->nacc == 0) {
				if (ckey->sf > 0) {
					if (i1 < ckey->sf - 1) { /* if sharp */
						if (a == A_SH)
							s->as.u.note.accs[i] = 0;
						else
							s->as.u.note.accs[i] = a;
					} else if (a == A_NT) {
						s->as.u.note.accs[i] = 0;
					}
				} else if (ckey->sf < 0) {
					if (i1 >= ckey->sf + 6) { /* if flat */
						if (a == A_FT)
							s->as.u.note.accs[i] = 0;
						else
							s->as.u.note.accs[i] = a;
					} else if (a == A_NT) {
						s->as.u.note.accs[i] = 0;
					}
				}
			} else {
				for (j = 0; j < ckey->nacc; j++) {
					if ((s->as.u.note.pits[i] + 196
							- ckey->pits[j]) % 7
								== 0) {
						if (a == ckey->accs[j])
							s->as.u.note.accs[i] = 0;
						else
							s->as.u.note.accs[i] = a;
						break;
					} else if (a == A_NT) {
						s->as.u.note.accs[i] = 0;
					}
				}
			}
		} else if (ckey->empty != 0) {		/* new key none */
			if (a == A_NT)
				a = 0;
			s->as.u.note.accs[i] = a;
		} else if (s->as.u.note.accs[i] != 0) {	/* normal case */
			s->as.u.note.accs[i] = a;
		} else if (ckey->nacc > 0) {
			for (j = 0; j < ckey->nacc; j++) {
				if ((i4 + 196 - ckey->pits[j]) % 7
							== 0)
					break;
			}
			if (j >= ckey->nacc)
				s->as.u.note.accs[i] = a;
		}
	}
}

/* transpose a guitar chord */
static void gch_transpose(struct sym *s,
			struct key_s *ckey,
			struct key_s *okey)
{
	char *p, *q, *new_txt, *old_txt;
	int l;
	int n, a, i1, i2, i3, i4;
	static char note_names[] = "CDEFGAB";
	static char *acc_name[5] = {"bb", "b", "", "#", "##"};

	if (ckey->empty != 0 || okey->empty != 0)
		return;

	old_txt = p = s->as.text;

	/* skip the annotations */
	for (;;) {
		if (strchr("^_<>@", *p) == 0)
			break;
		while (*p != '\0' && *p != '\n')
			p++;
		if (*p == '\0')
			return;
		p++;
	}

	/* main chord */
	q = strchr(note_names, *p);
	if (q == 0)
		return;

	/* allocate a new string */
	/* we need only 2 extra accidentals */
	new_txt = malloc(strlen(s->as.text) + 2);
	l = p - s->as.text;
	memcpy(new_txt, s->as.text, l);
	s->as.text = new_txt;
	new_txt += l;
	p++;

	i2 = ckey->sf - okey->sf;
	n = q - note_names;
	i1 = cde2fcg[n];			/* fcgdaeb */
	a = 0;
	if (*p == '#') {
		a++;
		p++;
		if (*p == '#') {
			a++;
			p++;
		}
	} else if (*p == 'b') {
		a--;
		p++;
		if (*p == 'b') {
			a--;
			p++;
		}
	}
	i3 = i1 + i2 + a * 7;
	i4 = cgd2cde[(unsigned) ((i3 + 252) % 7)];
	*new_txt++ = note_names[i4];
	i1 = ((i3 + 1 + 21) / 7 + 2 - 3 + 200) % 5;
						/* accidental */
	new_txt += sprintf(new_txt, "%s", acc_name[i1]);

	/* bass */
	while (*p != '\0' && *p != '\n' && *p != '/')
		*new_txt++ = *p++;
	if (*p == '/') {
		*new_txt++ = *p++;
		q = strchr(note_names, *p);
		if (q != 0) {
			p++;
			n = q - note_names;
			i1 = cde2fcg[n];	/* fcgdaeb */
			if (*p == '#') {
				a = 1;
				p++;
			} else if (*p == 'b') {
				a = -1;
				p++;
			} else {
				a = 0;
			}
			i3 = i1 + i2 + a * 7;
			i4 = cgd2cde[(unsigned) ((i3 + 252) % 7)];
			*new_txt++ = note_names[i4];
			i1 = ((i3 + 1 + 21) / 7 + 2 - 3 + 200) % 5;
						/* accidental */
			new_txt += sprintf(new_txt, "%s", acc_name[i1]);
		}
	}
	strcpy(new_txt, p);
	free(old_txt);
}

/* -- transpose (when changing the key signature) -- */
static void transpose(struct sym *s,	/* new key signature */
		struct key_s *okey)
{
	struct key_s *ckey;
	ckey = &s->as.u.key;

	/* change all symbol up to EOT or key signature change */
	for (s = s->next; s->type != EOT; s = s->next) {
		if (s->type != NOTE && s->type != GRACE) {
			if (s->type == KEYSIG
			 && s->as.u.key.empty != 1)
				break;
			if (s->type == REST && s->as.text != 0)
				gch_transpose(s, ckey, okey);
			continue;
		}
		if (s->as.text != 0)
			gch_transpose(s, ckey, okey);
		note_transpose(s, ckey, okey);
	}
}

/* -- add a new voice -- */
int voice_new(Tcl_Interp *interp,
	      Tcl_Obj *obj)
{
	int rc;
	struct voicecontext *old_curv;
	struct sym *s;

	if (nvoice >= MAXVOICE - 1)
		return tcl_wrong_args(interp, "too many voices");

	/* create the voice with its EOT */
	old_curv = curvoice;
	curvoice = &voice_tb[++nvoice];
	memset(curvoice, 0, sizeof *curvoice);
	eot_create();
	s = sym_new((struct sym *) curtune->last_sym);
	s->as.state = ABC_S_TUNE;
	s->as.type = ABC_T_INFO;
	s->as.text = malloc(2);
	s->as.text[0] = 'V';
	s->as.text[1] = '\0';
	s->voice = s->as.u.voice.voice = curvoice - voice_tb;
	curvoice->p_voice = s;

	/* set its name */
	if ((rc = voice_set(interp, obj)) != TCL_OK) {
		free(curvoice->eot);
		abc_delete((struct abcsym *) curvoice->p_voice);
		nvoice--;
		curvoice = old_curv;
		return rc;
	}

	/* redefine all extra symbols */
	tune_purge();
	tune_select(curtune);
	return TCL_OK;
}

/* -- set the names of the current voice -- */
int voice_set(Tcl_Interp *interp,
	      Tcl_Obj *obj)
{
	int objc, rc;
	Tcl_Obj **objv;
	char *p;
	struct sym *s;

	if ((rc = Tcl_ListObjGetElements(interp,
					obj,
					&objc, &objv)) != TCL_OK)
		return rc;
	if (objc != 3)
		return tcl_wrong_args(interp,
				      "voice set {name full-name nick-name}");

	/* if no V: (first voice), create one after the K: */
	if ((s = curvoice->p_voice) == 0) {
		s = sym_new(curvoice->eot->next);
		s->as.state = ABC_S_TUNE;
		s->as.type = ABC_T_INFO;
		s->as.text = malloc(2);
		s->as.text[0] = 'V';
		s->as.text[1] = '\0';
		s->voice = curvoice - voice_tb;
		curvoice->p_voice = s;
	}

	p = Tcl_GetString(objv[0]);
	if (strlen(p) == 0) {
		strcpy(interp->result, "a voice must have an identifier");
		return TCL_ERROR;
	}
	if (strlen(p) >= VOICE_ID_SZ - 1) {
		strcpy(interp->result, "voice ID too long");
		return TCL_ERROR;
	}
	strcpy(s->as.u.voice.id, p);
	str_new(&s->as.u.voice.fname, *objv++);
	str_new(&s->as.u.voice.nname, *objv);
	return TCL_OK;
}

/* -- set the words (lyrics after tune) -- */
int words_set(char *p)
{
	struct abcsym *as, *as2;
	Tcl_DString str;
	char *q;

	/* remove all previous words */
	for (as = curtune->first_sym; as != 0; as = as->next) {
		if (as->type != ABC_T_INFO
		 || as->text[0] != 'W')
			continue;
		as2 = as->prev;
		if ((as2->next = as->next) != 0)
			as2->next->prev = as2;
		abc_delete(as);
		as = as2;
	}

	/* add 'W:' at beginning of lines */
	if (*p == '\0')
		return TCL_OK;
	Tcl_DStringInit(&str);
	for (;;) {
		q = p;
		while (*p != '\n' && *p != '\0')
			p++;
		Tcl_DStringAppend(&str, "W:", 2);
		Tcl_DStringAppend(&str, q, p - q + 1);
		if (*p == '\0')
			break;
		p++;
	}

	/* insert the new words at the end of the tune */
	for (as = curtune->first_sym; as->next != 0; as = as->next)
		;
	abc_insert(Tcl_DStringValue(&str), as);
	Tcl_DStringFree(&str);
	return TCL_OK;
}

/* -- update the word breaks -- */
void word_update(struct sym *so)
{
	struct sym *s, *lastnote;
	int start_flag;
	int seen = 0;

	/* synchronise with the previous note */
	for (s = so->prev; ; s = s->prev) {
		if (s->type == NOTE)
			break;
		if (s->type != EOT)
			continue;
		for (s = s->next; ; s = s->next) {
			if (s == so)
				seen = 1;
			if (s->type == NOTE)
				break;
			if (s->type == EOT)
				return;		/* no note! */
		}
		s->flags |= F_WORD_S;
		break;
	}
	start_flag = 1;
	if (s->as.u.note.lens[0] < BASE_LEN / 4) {	/* if any flag */
		s->flags &= ~F_WORD_E;
		start_flag = 0;
	}

	/* loop until going after the original symbol */
	lastnote = s;
	for (s = s->next; ; s = s->next) {
		if (s->as.flags & ABC_F_SPACE)
			start_flag = 1;
		switch (s->type) {
		case REST:
			if (s->as.u.note.lens[0] >= BASE_LEN / 4)
				start_flag = 1;
			break;
		case BAR:
			start_flag = 1;
			break;
		case EOT:
			if (lastnote != 0)
				lastnote->flags |= F_WORD_E;
			return;
		case NOTE:
			if (!seen)
				s->flags &= ~F_WORD_E;
			if (s->as.u.note.lens[0] >= BASE_LEN / 4 /* if no flag */
			 || start_flag) {
				if (lastnote != 0)
					lastnote->flags |= F_WORD_E;
				if (s->as.u.note.lens[0] >= BASE_LEN / 4) {
					s->flags |= F_WORD_S | F_WORD_E;
					start_flag = 1;
				} else {
					s->flags |= F_WORD_S;
					start_flag = 0;
				}
			} else {
				s->flags &= ~F_WORD_S;
			}
			if (seen)
				return;
			lastnote = s;
			break;
		}
		if (s == so)
			seen = 1;
	}
}
