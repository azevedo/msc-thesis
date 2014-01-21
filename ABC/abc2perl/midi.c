/*++
 * MIDI import/export play/record.
 *
 * Adapted from awemidi (by Takashi Iwai),
 * adapted from TiMidity via Tuukka Toivonen.
 *
 * Copyright (C) 1999-2012 Jean-François Moine.
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
 *-*/

/*#define DEBUG 1		* MIDI test */
/*#define WITH_RT_PRIO 1	* have real-time priority on Linux - fixme:to test*/
/*#define AWE_PB 1		* AWE external keyboard problem */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>
#include <fcntl.h>
#ifndef WIN32
#include <unistd.h>
#if defined(__FreeBSD__) || defined(linux)
#include <sys/time.h>
#endif
#else
#include <sys/timeb.h>
#endif
#include <stdlib.h>
#ifdef HAVE_SOUND
#ifdef __FreeBSD__
#include <sys/ioctl.h>
#  include <machine/soundcard.h>
#ifdef HAVE_AWE
#  include <awe_voice.h>
#endif
#elif defined(linux)
#include <sys/ioctl.h>
#  include <linux/soundcard.h>
#ifdef HAVE_AWE
#  include <linux/awe_voice.h>
#endif
#ifdef HAVE_ALSA
#include <alsa/asoundlib.h>
#endif
#if defined(WITH_RT_PRIO)
#include <sched.h>
#endif
#elif defined(WIN32)
#include <mmsystem.h>
#include <winsock.h>
#undef HAVE_ALSA
#undef HAVE_AWE
#endif
#include <signal.h>
#endif /*HAVE_SOUND*/

#include "tcl.h"
#include "abcparse.h"
#include "tclabc.h"
#include "midi.h"

#define MAX_MIDI_CHANNELS 32

#ifdef DEBUG
#define TRACE(a) trace a
#else
#define TRACE(a)
#endif

typedef struct _MidiEvent {
	int time;		/* tempo counter */
	unsigned char channel, type, a, b; /* channel, type, parameters */
	char *misc;		/* misc. parameters */
} MidiEvent;

typedef struct midieventlist {
	MidiEvent event;
	int track;
	struct midieventlist *next;
} MidiEventList;

typedef struct _MidiInfo {
	int format;
	int tracks;
	int divisions;

#if 0
	/* variable data */
	int curt;
	int prevcs, curcs;
	int curev;
#endif
	int tempo;
#if 0
	/* midi data */
	int nlists;
	MidiEvent *list;

	/* other control parameters */
	int chorus, reverb;
	int master_volume;
	int bass_level, treble_level;
	int accept_all_off;
	int realtime_pan;
	int multi_part;
	int check_same_csec;
	int check_gs_macro;
	unsigned long drumflag;
	int track_nums;
	char *filename;
#endif
	MidiEventList *evlist;
	MidiEventList *last_event;
} MidiInfo;

/* Midi events */
enum {
	ME_NONE, ME_NOTEON, ME_NOTEOFF, ME_KEYPRESSURE,
	ME_MAINVOLUME, ME_PAN, ME_SUSTAIN, ME_EXPRESSION,
	ME_PITCHWHEEL, ME_PROGRAM, ME_TEMPO, ME_PITCH_SENS,

	ME_ALL_SOUNDS_OFF, ME_RESET_CONTROLLERS, ME_ALL_NOTES_OFF,
	ME_TONE_BANK,

	ME_LYRIC,

	ME_REVERB, ME_CHORUS,

	ME_SET_REVERB_MODE, ME_SET_CHORUS_MODE,

	ME_ENV1_DELAY, ME_ENV1_ATTACK, ME_ENV1_HOLD, ME_ENV1_DECAY,
	ME_ENV1_RELEASE, ME_ENV1_SUSTAIN, ME_ENV1_PITCH, ME_ENV1_CUTOFF,

	ME_ENV2_DELAY, ME_ENV2_ATTACK, ME_ENV2_HOLD, ME_ENV2_DECAY,
	ME_ENV2_RELEASE, ME_ENV2_SUSTAIN,

	ME_LFO1_DELAY, ME_LFO1_FREQ, ME_LFO1_VOLUME,
	ME_LFO1_PITCH, ME_LFO1_CUTOFF,

	ME_LFO2_DELAY, ME_LFO2_FREQ, ME_LFO2_PITCH,
	ME_INIT_PITCH, ME_FX_CHORUS, ME_FX_REVERB, ME_CUTOFF, ME_FILTERQ,

	ME_TIME, ME_KEY,

	ME_EOT = 99,
};

/* default values */
#define MIDI_DEFAULT_TEMPO	500000

#ifndef WIN32
#define MFILE int
#define WRITE(fd, b, l) write(fd, b, l)
#ifndef AWE_PB
#define READ(fd, b, l) read(fd, b, l)
#else
/* I had problems getting MIDI from external keyboard with my SB AWE32 */
.. fixed in linux-2.4.0!
static int extern_input = 0;
#define READ(fd, b, l) read_ext(fd, b, l)
static int read_ext(int fd,
		    unsigned char *b,
		    int l)
{
	if (extern_input) {
		fd_set rmask;
		struct timeval tv;

		FD_ZERO(&rmask);
		FD_SET(fd, &rmask);
		tv.tv_sec = 0;
		tv.tv_usec = 100000;
		if (select(fd + 1,
			   &rmask, 0, 0,
			   &tv) <= 0) {
			trace("MIDI keyboard blocked!\n");
		}
	}
	return read(fd, b, l);
}
#endif
#define CLOSE(fd) close(fd)
#define SEEK(fd, o, m) lseek(fd, o, m)
#else /*WIN32*/
#define MFILE Tcl_Channel
#define READ(fd, b, l) Tcl_Read(fd, b, l)
#define WRITE(fd, b, l) Tcl_WriteChars(fd, b, l)
#define CLOSE(fd) Tcl_Close(0, fd)
#define SEEK(fd, o, m) Tcl_Seek(fd, o, m)
#endif /*WIN32*/

static Tcl_Interp *my_interp;
static int volume = 80;
static int mindur = 48;		/* smallest note duration = demisemiquaver */
static int scale[7] = {0, 2, 4, 5, 7, 9, 11};	/* index = natural note */
static int acc_pitch[6] = {0, 1, 0, -1, 2, -2};	/* index = enum accidentals */
static Tcl_Obj *midi_handler;	/* handler for MIDI insert */
static MidiEventList *prev_meep;
static struct timeval start_time;
#ifdef HAVE_SOUND
static int play_factor = BASE_LEN / 4 * 80;
#endif
static int play_factor_new = BASE_LEN / 4 * 80;
/* stop playing events */
static struct play_s {
	unsigned channels;
	int time;
	char freq;
} play_tb[64];
static int play_nb;

/* for play/dump */
static int dump;		/* play (0) or dump (1) */
static MFILE out_fd;		/* MIDI output file */
static struct vtb_s {
	struct sym *s;		/* next symbol to play */
	unsigned channels;
	short transpose;
	short mapchange;	/* alteration found in the measure */
	char workmap[70];	/* measure/current sharps/flats - base index: lowest 'C' */
	char basemap[7];
} vtb[MAXVOICE];

/* for repeat sequences */
static int repeat_flag;		/* see REPEAT_xx */
#define REPEAT_NONE 0
#define REPEAT_FOUND 1
#define REPEAT_RUN 2
#define REPEAT_DONE 3
static int lrepeat_time, rrepeat_time;	/* left and right repeat times */

static int play_stop_time;

/* for dump only */
static int dump_time;
static int dump_status;

static int play_advance(struct vtb_s *vp);

static void note_insert(MidiEventList *meep,
			int ext);	/* 0: MIDI in, 1: virtual kbd */
#ifdef HAVE_SOUND
static void midi_play_evt(MidiEventList *meep);
static int devlist(int out, char *device);
static void seq_note(int ch, int fr, int vel);
#else
#define seq_note(c, f, v)
#define set_program(c, p, b)
#endif

#ifdef WIN32
static void gettimeofday(struct timeval *tv, int dum)
{
	struct timeb tb;

	_ftime(&tb);
	tv->tv_sec = tb.time;
	tv->tv_usec = tb.millitm * 1000;
}
#endif /*WIN32*/

/* -- get the number of 1/100s from starting time -- */
static int gettime(void)
{
	int d1, d2;
	struct timeval now_time;

	gettimeofday(&now_time, 0);
	d1 = now_time.tv_sec - start_time.tv_sec;
	d2 = now_time.tv_usec - start_time.tv_usec;
	while (d2 < 0) {
		d2 += 1000000;
		d1--;
	}
	return (d2 / 10000 + d1 * 100);
}

/* -- MIDI stuff -- */
MidiInfo midi_header;

static int at;
static int check_gs_macro;
static unsigned drumflag;

/* not use fseek due to piped input */
static void skip(MFILE fd, size_t len)
{
	size_t c;
	char tmp[1024];

	while (len > 0) {
		c = len;
		if (c > 1024)
			c = 1024;
		len -= c;
		if (READ(fd, tmp, c) != c)
			trace("Error in skip file..\n");
	}
}

/* Read variable-length number (7 bits per byte, MSB first) */
static int getvl(MFILE fd)
{
	int l = 0;
	unsigned char c;

	for (;;) {
		if (READ(fd, &c, 1) != 1) {
			trace("Corrupt MIDI file\n");
			return -1;
		}
		l += (c & 0x7f);
		if (!(c & 0x80))
			break;
		l <<= 7;
	}
	return l;
}

#define MIDIEVENT(at, t, ch, pa, pb) \
  new=malloc(sizeof(MidiEventList));\
  new->event.time=at;\
  new->event.type=t;\
  new->event.channel=(unsigned char)ch;\
  new->event.a=(unsigned char)pa;\
  new->event.b=(unsigned char)pb;\
  new->event.misc=0;\
  new->next=0;\
  return new

#define MAGIC_EOT ((MidiEventList *)(-1))

static unsigned char laststatus, lastchan;
static unsigned char nrpn=0;
static unsigned char rpn_msb[MAX_MIDI_CHANNELS], rpn_lsb[MAX_MIDI_CHANNELS]; /* one per channel */
static unsigned char nrpn_msb[MAX_MIDI_CHANNELS], nrpn_lsb[MAX_MIDI_CHANNELS]; /* one per channel */

#ifdef DEBUG
/*			   tttttt tt ccc ttt aaa bbb mmmm */
static char dump_head[] = " time  tr  ch typ   a   b misc";
static void dump_midi_event(MidiEventList *meep)
{
	trace("%6d %2d %3d %3d %3d %3d '%s'\n",
	       meep->event.time,
	       meep->track,
	       meep->event.channel,
	       meep->event.type,
	       meep->event.a,
	       meep->event.b,
	       meep->event.misc ? meep->event.misc : "-");
}
#endif

/*
 * output message
 */
static MidiEventList *print_message(int type, int len, MFILE fd)
{
	/* if text[1] == ':', it is used in ABC conversion */
	static char *label[] = {	/* some meta events */
		"Event: ",		/* 0: sequence number */
		"T: ",			/* 1: text */
		"C: ",			/* 2: copyright */
		"Track name: ",		/* 3: sequence name */
		"Instrument: ",		/* 4: instrument name */
		"Lyric: ",		/* 5: lyric */
		"Marker: ",		/* 6: marker */
		"Cue point: ",		/* 7: cue point */
	};
	char *s;
	MidiEventList *new;
	int slen;

	if (type > 7)
		type = 1;		/* default to text */
	slen = strlen(label[type]);
	s = (char *) malloc(len + slen + 1);
	strcpy(s, label[type]);
	READ(fd, s + slen, len);
	len += slen;

	/* remove trailing blanks */
	while (len > 0
	       && (s[len - 1] == '\n'
		   || s[len - 1] == '\r'
		   || s[len - 1] == ' '
		   || s[len - 1] == '\t'))
		len--;
	s[len] = '\0';

	new = malloc(sizeof(MidiEventList));
	new->event.time = at;
	new->event.type = ME_LYRIC;
	new->event.misc = s;
	new->next = 0;
	return new;
}

static MidiEventList *nrpn_event(void)
{
	static int event_type[] = {
		ME_LFO1_DELAY, ME_LFO1_FREQ, ME_LFO2_DELAY, ME_LFO2_FREQ,
		ME_ENV1_DELAY, ME_ENV1_ATTACK, ME_ENV1_HOLD, ME_ENV1_DECAY,
		ME_ENV1_SUSTAIN, ME_ENV1_RELEASE, ME_ENV2_DELAY, ME_ENV2_ATTACK,
		ME_ENV2_HOLD, ME_ENV2_DECAY, ME_ENV2_SUSTAIN, ME_ENV2_RELEASE,
		ME_INIT_PITCH, ME_LFO1_PITCH, ME_LFO2_PITCH, ME_ENV1_PITCH,
		ME_LFO1_VOLUME, ME_CUTOFF, ME_FILTERQ, ME_LFO1_CUTOFF,
		ME_ENV1_CUTOFF, ME_FX_CHORUS, ME_FX_REVERB,
	};
	MidiEventList *new;
	if (rpn_msb[lastchan] == 127 && rpn_lsb[lastchan] <= 26) {
		MIDIEVENT(at, event_type[rpn_lsb[lastchan]], lastchan,
			  nrpn_lsb[lastchan], nrpn_msb[lastchan]);
	}
	return NULL;
}

/*
 * parse system exclusive message
 * only support AWE32 chorus and reverb mode control
 */
static MidiEventList *system_exclusive(MFILE fd)
{
#if 0
	static unsigned char master_volume_macro[] = {
		0x7f,0x7f,0x04,0x01,/*ll,mm,*/
	};
	static unsigned char gm_on_macro[] = {
		0x7e,0x7f,0x09,0x01,
	};
	static unsigned char gs_on_sfx[] = {
		0x00,0x7f,0x00,0x41,
	}
	static unsigned char xg_on_macro[] = {
		0x43,0x10,0x4c,0x00,0x00,0x7e,0x00,
	};
	static unsigned char xg_main_drum_macro[] = {
		0x43,0x10,0x4c,0x08,0/*channel*/,0x07,/*1-3,*/
		/* 1=optional, 2=main, 3=sub */
	};
#endif
	static unsigned char gs_pfx_macro[] = {
		0x41,0x10,0x42,0x12,0x40,
	};
	static unsigned char awe_reverb_macro[] = {
		0x41,0x10,0x42,0x12,0x40,0x01,0x30,
	};
	static unsigned char awe_chorus_macro[] = {
		0x41,0x10,0x42,0x12,0x40,0x01,0x38,
	};
	unsigned char buf[1024];
	MidiEventList *new;
	int len;
	int i;
	char tmp[256];

	if ((len = getvl(fd)) < 0)
		return 0;
	if (len > sizeof(buf)) {
		trace("sysex too long!\n");
		skip(fd, len);
		return 0;
	}
	READ(fd, buf, len);
	if (memcmp(buf, awe_reverb_macro, sizeof(awe_reverb_macro)) == 0) {
		trace("SYSEX: reverb = %d\n", buf[7]);
		MIDIEVENT(at, ME_SET_REVERB_MODE, 0, buf[7], 0);
	}
	if (memcmp(buf, awe_chorus_macro, sizeof(awe_chorus_macro)) == 0) {
		trace("SYSEX: chorus = %d\n", buf[7]);
		MIDIEVENT(at, ME_SET_CHORUS_MODE, 0, buf[7], 0);
	}
	if (check_gs_macro &&
	    memcmp(buf, gs_pfx_macro, sizeof(gs_pfx_macro)) == 0) {
		if ((buf[5] & 0xf0) == 0x10 && buf[6] == 0x15) {
			/* GS part mode change */
			int p = buf[5] & 0x0f;
			if (p == 0) p = 9;
			else if (p < 9) p--;
			if (buf[7] == 0)
				drumflag &= ~(1 << p);
			else
				drumflag |= (1 << p);
			trace("SYSEX: drum part %d %s\n",
				p, (buf[7] ? "on" : "off"));
		}
	}
	strcpy(tmp, "SYSEX:");
	for (i = 0; i < len; i++) {
		sprintf(tmp + i * 3 + 6," %02x", buf[i]);
		if (i >= 50) {
			sprintf(tmp + (i + 1) * 3 + 6,"...");
			break;
		}
	}
	trace(tmp);
	return 0;
}

/*
 * parse meta event
 */
static MidiEventList *meta_event(MFILE fd)
{
	MidiEventList *new;
	unsigned char type;
	unsigned char a, b, c;
	int len;

	READ(fd, &type, 1);
	if ((len = getvl(fd)) < 0)
		return NULL;
	if ((unsigned) type < 16)
		return print_message(type, len, fd);

	switch(type) {
	case 0x20:		/* MIDI channel */
		READ(fd, &a, 1);	/* channel to be associated with next meta events */
		break;
	case 0x21:		/* MIDI port */
		READ(fd, &a, 1);	/* (channel += 16 * 'a') */
		break;
	case 0x2F:		/* End of Track */
		return MAGIC_EOT;

	case 0x51:		/* Tempo */
		READ(fd, &a, 1);
		READ(fd, &b, 1);
		READ(fd, &c, 1);
		MIDIEVENT(at, ME_TEMPO, c, a, b);

	case 0x58:		/* time signature */
		READ(fd, &a, 1);	/* upper value */
		READ(fd, &b, 1);	/* log2 lower value */
		READ(fd, &c, 1);
		READ(fd, &c, 1);
		if ((unsigned) b < 4)
			b = 1 << b;
		else {
			trace("Incorrect time signature %d", b);
			b = 4;
		}
		MIDIEVENT(at, ME_TIME, c, a, b);

	case 0x59:		/* key signature */
		READ(fd, &a, 1);
		READ(fd, &b, 1);
		MIDIEVENT(at, ME_KEY, 0, a, b);

	default:
		/*Warning("unknown meta event: type%d, len%d", type, len);*/
		skip(fd, len);
		break;
	}
	return 0;
}

/*
 * parse control change message
 */
static MidiEventList *control_change(unsigned char type, unsigned char val)
{
	MidiEventList *new;
	unsigned char control;

	control = 255;
	switch (type) {
	case 7: control = ME_MAINVOLUME; break;
	case 10: control = ME_PAN; break;
	case 11: control = ME_EXPRESSION; break;
	case 64: control = ME_SUSTAIN; break;
	case 120: control = ME_ALL_SOUNDS_OFF; break;
	case 121: control = ME_RESET_CONTROLLERS; break;
	case 123: control = ME_ALL_NOTES_OFF; break;

	case 91: control = ME_REVERB; break;
	case 93: control = ME_CHORUS; break;

	/* the SCC-1 tone bank switch commands. */
	case 0: control = ME_TONE_BANK; break;
	case 32: control = ME_TONE_BANK; break;

	case 100: nrpn=0; rpn_lsb[lastchan]=val; break;
	case 101: nrpn=0; rpn_msb[lastchan]=val; break;
	case 99: nrpn=1; rpn_msb[lastchan]=val; break;
	case 98: nrpn=1; rpn_lsb[lastchan]=val; break;

	case 6: /* send data entry MSB */
		if (nrpn) {
			nrpn_msb[lastchan] = val;
			break;
		}

		switch((rpn_msb[lastchan]<<8) | rpn_lsb[lastchan]) {
		case 0x0000:
			/* Pitch bend sensitivity */
			control = ME_PITCH_SENS;
			break;

		case 0x7F7F: /* RPN reset */
			control = ME_PITCH_SENS;
			val = 2;
			break;
		}
		break;

	case 38: /* data entry LSB */
		if (!nrpn)
			break;
		nrpn_lsb[lastchan] = val;
		return nrpn_event();
	}
	if (control == 255)
		return 0;
	MIDIEVENT(at, control, lastchan, val, 0);
}

/* -- define a standard MIDI event -- */
static MidiEventList *def_event(unsigned char a,
				MFILE fd)
{
	unsigned char b;
	MidiEventList *new;

	if (a & 0x80) {		/* status byte */
		lastchan = a & 0x0F;
		laststatus = (a >> 4) & 0x07;
		READ(fd, &a, 1);
		a &= 0x7F;
	}
	switch (laststatus) {
	case 0:			/* Note off */
		READ(fd, &b, 1);
		b &= 0x7F;
		MIDIEVENT(at, ME_NOTEOFF, lastchan, a, b);
	case 1:			/* Note on */
		READ(fd, &b, 1);
		b &= 0x7F;
		MIDIEVENT(at, ME_NOTEON, lastchan, a, b);
	case 2:			/* Key Pressure */
		READ(fd, &b, 1);
		b &= 0x7F;
		MIDIEVENT(at, ME_KEYPRESSURE, lastchan, a, b);
	case 3:			/* Control change */
		READ(fd, &b, 1);
		b &= 0x7F;
		if ((new = control_change(a, b)) != NULL)
			return new;
		break;
	case 4:			/* Program change */
		a &= 0x7f;
		MIDIEVENT(at, ME_PROGRAM, lastchan, a, 0);
	case 5:			/* Channel pressure - NOT IMPLEMENTED */
		break;
	case 6:			/* Pitch wheel */
		READ(fd, &b, 1);
		b &= 0x7F;
		MIDIEVENT(at, ME_PITCHWHEEL, lastchan, a, b);
	}
#if 0
	trace("MIDI status 0x%02X channel %d ignored\n",
		laststatus, lastchan);
#endif
	return 0;
}
#undef MIDIEVENT

/* Read a MIDI event, returning a freshly allocated element that can
   be linked to the event list */
static MidiEventList *read_midi_event(MFILE fd)
{
	unsigned char me;
	MidiEventList *new = 0;

	for (;;) {
		int len;

		if ((len = getvl(fd)) < 0)
			break;
		at += len;
		if (READ(fd, &me, 1) != 1) {
			sprintf(my_interp->result, "error in reading midi file\n");
			return 0;
		}

		if (me == 0xF0 || me == 0xF7) {
			/* SysEx event */
			if ((new = system_exclusive(fd)) != 0)
				break;
		} else if (me == 0xFF) {
			/* Meta event */
			if ((new = meta_event(fd)) != 0)
				break;
		} else {
			if ((new = def_event(me, fd)) != 0)
				break;
		}
	}
	return new;
}

#define READID(str,fd)	READ(fd, str, 4)

static int READLONG(MFILE fd)
{
	unsigned char c[4];

	READ(fd, c, 4);
	return (c[0] << 24) + (c[1] << 16) + (c[2] << 8) + c[3];
}

static short READSHORT(MFILE fd)
{
	unsigned char c[2];

	READ(fd, c, 2);
	return (c[0] << 8) + c[1];
}

/* Read a midi track and merge into the linked list */
static int read_track(MFILE fd, int track, int append)
{
	MidiEventList *meep, *next, *new;
	char tmp[4];

	if (append) {
		meep = midi_header.last_event;
		at = meep->event.time;
	} else {
		meep = midi_header.evlist;
		at = 0;
	}

	/* Check the formalities */
	READID(tmp, fd);
	READLONG(fd);
	if (memcmp(tmp, "MTrk", 4)) {
		sprintf(my_interp->result, "Corrupt MIDI file.\n");
		return TCL_ERROR;
	}

	for (;;) {
		new = read_midi_event(fd);
		if (!new)
			return TCL_ERROR;
		if (new == MAGIC_EOT) /* End-of-track Hack. */
			break;
		new->track = track;

		if (track >= 16 && !append) {	/* if MIDI type 1 */
			switch (new->event.type) {
			case ME_LYRIC:
			case ME_TEMPO:
				break;
			default:
				new->event.channel += 16;
				break;
			}
		}

		next = meep->next;
		while (next && next->event.time <= new->event.time) {
			meep = next;
			next = meep->next;
		}

		new->next = next;
		meep->next = new;
		meep = new;
	}
	if (meep->next == 0)
		midi_header.last_event = meep;
	return TCL_OK;
}

/* -- Free the linked event list from memory. -- */
static void free_midi_list(void)
{
	MidiEventList *meep, *next;

	for (meep = midi_header.evlist; meep; meep = next) {
		next = meep->next;
		if (meep->event.misc != 0)
			free(meep->event.misc);
		free(meep);
		meep = next;
	}

	memset(&midi_header, 0, sizeof midi_header);
}

/* -- initialize the pitch to note/accidental tables -- */
/*					C  C# D  Eb E  F  F# G  G# A  Bb B */
static        char natural_def[12] =  { 0, 0, 1, 2, 2, 3, 3, 4, 4, 5, 6, 6 };
static signed char accident_def[12] = { 0, 1, 0,-1, 0, 0, 1, 0, 1, 0,-1, 0 };
static char natural[12];
/* index: MIDI pitch - !! notes >= 120 not checked !! */
static signed char accident[120];

static void set_notacc(int sf)
{
	int i;

	/* fill the first octave */
	/*trace("set alter %d\n", sf);*/
	memcpy(natural, natural_def, sizeof natural_def);
	memcpy(accident, accident_def, sizeof accident_def);
	switch (sf) {
	case 4:
		accident[2] = 2;	/* D sharp */
	case 3:
		accident[7] = 2;	/* G sharp */
	case 2:
		accident[0] = 2;	/* C sharp */
/*		natural[1] = 0; */
/*		accident[1] = 1; */
		natural[10] = 5;	/* A sharp */
		accident[10] = 1;
	case 1:
		accident[5] = 2;	/* F sharp */
/*		natural[6] = 3; */
/*		accident[6] = 1; */
		natural[3] = 1;		/* D sharp */
		accident[3] = 1;
	case 0:
		break;
	case -4:
		accident[2] = 2;	/* D flat */
	case -3:
		accident[9] = 2;	/* A flat */
		natural[8] = 5;
		accident[8] = -1;
	case -2:
		accident[4] = 2;	/* E flat */
/*		natural[3] = 2; */
/*		accident[3] = -1; */
		natural[1] = 1;		/* D flat */
		accident[1] = -1;
	case -1:
		accident[11] = 2;	/* B flat */
		natural[10] = 6;
		accident[10] = -1;
		break;
	}

	/* fill the accidental table */
	for (i = 1; i < sizeof accident / sizeof accident_def; i++)
		memcpy(&accident[i * sizeof accident_def],
		       accident,
		       sizeof accident_def);
}

/* -- define a new ABC key -- */
static struct abcsym *abc_new_key(struct abctune *t,
				  int sf,
				  int minor,
				  struct abcsym *as)
{
	char **k;
	char buf[32];
	static char *key_type[1 + 7 + 7] = {
		"Cb", "Gb", "Db", "Ab", "Eb", "Bb", "F",
		"C", "G", "D", "A", "E", "B", "F#", "C#",
	};
	static char *key_mtype[1 + 7 + 7] = {	/* minor */
		"Abm", "Ebm", "Bbm", "Fm", "Cm", "Gm", "Dm",
		"Am", "Em", "Bm", "F#m", "C#m", "G#m", "D#m", "A#m",
	};

	if (minor)
		k = key_mtype;
	else
		k = key_type;
	sprintf(buf, "K:%s", k[sf + 7]);
	if (as == 0) {
		as = abc_new(t, buf, 0);
		as->type = ABC_T_INFO;
	} else {
		free(as->text);
		as->text = malloc(strlen(buf) + 1);
		strcpy(as->text, buf);
	}
	as->u.key.sf = sf;
	as->u.key.mode = minor ? MINOR : MAJOR;
	return as;
}

/* -- synchronize the accidentals for step recording -- */
static void acc_sync_in(void)
{
	struct sym *s;
	int i;

	s = curvoice->cursym;
	set_notacc(s->sf);
	for ( ; s->type != EOT; s = s->prev) {
		if (s->type == BAR || s->type == KEYSIG)
			break;
	}
	for ( ; ; s = s->next) {
		if (s->type == NOTE) {
			for (i = 0; i <= s->as.u.note.nhd; i++) {
				int pit;

				if (s->as.u.note.accs[i] == A_NULL)
					continue;
				pit = s->as.u.note.pits[i] + 19;	/* pitch from lowest C */
				pit = scale[pit % 7] + 12 * (pit / 7);
				switch (s->as.u.note.accs[i]) {
				case A_SH:
				case A_FT:
					accident[pit] = 2;
					break;
				case A_NT:
					accident[pit] = 0;
					break;
/*fixme: treat '^^', '__' and microtonal?*/
				}
			}
		}
		if (s == curvoice->cursym)
			break;
	}
}

/* -- call the midi_handler when note insert -- */
static void after_proc(ClientData clientData)
{
	Tcl_GlobalEvalObj(my_interp, (Tcl_Obj *) clientData);
	Tcl_DecrRefCount((Tcl_Obj *) clientData);
}

/* -- set the MIDI step recording handler -- */
static int midi_handler_set(Tcl_Obj *proc)
{
	if (midi_handler != 0) {
		Tcl_DecrRefCount(midi_handler);
		if (prev_meep != 0) {
#ifdef HAVE_SOUND
			prev_meep->event.type = ME_NOTEOFF;
			midi_play_evt(prev_meep);	/* stop playing */
#endif
			free(prev_meep);
		}
	}
	if ((midi_handler = proc) != 0) {
		Tcl_IncrRefCount(proc);
		midi_header.divisions = BASE_LEN / 4;
		mindur = 96;		/* semiquaver */
		gettimeofday(&start_time, 0);
	}
	return TCL_OK;
}

/* -- treat a 'abc midi' command -- */
int midi_cmd(Tcl_Interp *interp,
	     int objc,
	     Tcl_Obj *CONST objv[])
{
	char *p;
	MidiEventList *meep;
	int a, b;
	Tcl_Obj **objv2;

	my_interp = interp;
	if (objc < 3 || objc > 4)
		return tcl_wrong_args(interp, "midi type ?args?");
	p = Tcl_GetString(objv[2]);
	switch (*p) {
	case 'd':
#ifdef HAVE_SOUND
		if (strcmp(p, "devin") != 0
		    && strcmp(p, "devout") != 0)
			break;
		return devlist(p[3] == 'o',
				 objc == 3 ? 0 : Tcl_GetString(objv[3]));
#else
		return TCL_OK;
#endif
	case 'h' :
		if (strcmp(p, "handler") != 0)
			break;
		if (objc == 3)
			return midi_handler_set(0);
		p = Tcl_GetString(objv[3]);
		return midi_handler_set(*p == '\0' ? 0 : objv[3]);
	case 'n' :
		if (strcmp(p, "note") != 0)
			break;
		if (objc == 4
		    && Tcl_ListObjGetElements(interp, objv[3], &objc, &objv2)
				!= TCL_OK)
			return TCL_ERROR;
		if (objc != 2)
			return tcl_wrong_args(interp,
					"midi note [list pitch vel]");
		if (Tcl_GetIntFromObj(interp, *objv2++, &a) != TCL_OK
		    || (unsigned) a >= 128)
			return TCL_ERROR;
		if (Tcl_GetIntFromObj(interp, *objv2++, &b) != TCL_OK
		    || (unsigned) b >= 128)
			return TCL_ERROR;
		meep = malloc(sizeof(MidiEventList));
		meep->event.type = b != 0 ? ME_NOTEON : ME_NOTEOFF;
		meep->event.channel = curvoice->cursym->voice;
		meep->event.a = a;
		meep->event.b = b;
		meep->next = 0;
		note_insert(meep, 1);
		return TCL_OK;
	}
	strcpy(interp->result,
	       "wrong type: should be one of\n"
	       "\"devin\", \"devout\", \"handler\" or \"note\"");
	return TCL_ERROR;
}

/* -- define a note (midi_load) -- */
static void note_define(MidiEventList *meep,
			int curtim, int nexttim,
			int *p_pit, int *p_len, int *p_acc)
{
	MidiEventList *next;
	int chan, pit, acc, len;

	chan = meep->event.channel;
	pit = meep->event.a;	/* pitch */

	/* accidental */
	acc = A_NULL;
	switch (accident[pit]) {
	case 1:			/* sharp */
		if (accident[pit - 1] != 2) {
			acc = A_SH;
			accident[pit - 1] = 2;
		}
		break;
	case -1:		/* flat */
		if (accident[pit + 1] != 2) {
			acc = A_FT;
			accident[pit + 1] = 2;
		}
		break;
	case 2:			/* natural */
		acc = A_NT;
		accident[pit] = 0;
		break;
	}

	/* search the note length */
	len = -1;
	for (next = meep->next; next != 0; next = next->next) {
		int notetim;

		if (next->event.channel == chan
		    && (next->event.type == ME_NOTEON
			|| next->event.type == ME_NOTEOFF)
		    && next->event.a == pit) {	/* same pitch */
			notetim = (next->event.time
			       * BASE_LEN / 4 / midi_header.divisions
			       + 24) / mindur * mindur;
			if (notetim > nexttim)
				notetim = nexttim;
			if ((len = notetim - curtim) == 0)
				len = mindur;
			/*fixme: pb when n-plet*/
			break;
		}
	}
	if (len < 0) {
		trace("MIDI: no end of note!\n");
		len = BASE_LEN / 8;
	}
	*p_pit = (pit / 12) * 7		/* octave */
		+ natural[pit % 12]	/* natural note */
		- 35 + 16;		/* middle C (MIDI 60) is 16 */
	*p_len = len;
	*p_acc = acc;
}

/* -- insert a note from MIDI step input or the virtual keyboard -- */
static void note_insert(MidiEventList *meep,
			int ext)	/* 0: MIDI in, 1: virtual kbd */
{
	Tcl_Obj *script, *obj;
	int type, curtim, pit, len, acc, base;

	meep->event.time = gettime() * play_factor_new / 6000;

	/* 1st event: memorize */
	if (prev_meep == 0) {
		if (meep->event.type != ME_NOTEON)
			return;
		prev_meep = meep;
#ifdef HAVE_SOUND
		if (ext)
			midi_play_evt(meep);	/* start playing the note */
#endif
		return;
	}

	/* 2nd event */
	meep->event.type = ME_NOTEOFF;
	meep->event.channel = prev_meep->event.channel;
	meep->event.a = prev_meep->event.a;
	meep->event.b = 0;
	prev_meep->next = meep;
#ifdef HAVE_SOUND
	if (ext)
		midi_play_evt(meep);	/* stop playing the note */
#endif

	/* insert the note / rest / bar */
	acc_sync_in();
	curtim = (prev_meep->event.time
		  * BASE_LEN / 4 / midi_header.divisions
		  + 24) / mindur * mindur;
	note_define(prev_meep, curtim, curtim + BASE_LEN * 8,
		    &pit, &len, &acc);

	/* have only one dot */
	base = BASE_LEN / 4;
	if (len >= BASE_LEN / 4) {
		if (len >= BASE_LEN) {
			if (len >= BASE_LEN * 2)
				base = BASE_LEN * 2;
			else
				base = BASE_LEN;
		} else if (len >= BASE_LEN / 2)
			base = BASE_LEN / 2;
	} else if (len >= BASE_LEN / 8) {
		base = BASE_LEN / 8;
	} else if (len >= BASE_LEN / 16) {
		base = BASE_LEN / 16;
	} else {
		base = BASE_LEN / 32;
	}
	if (len < 5 * base / 4)
		len = base;		/* no dot */
	else if (len < 7 * base / 4)
		len = 3 * base / 2;	/* 1 dot */
	else
		len = base * 2;		/* no dot */

	switch (meep->event.a) {
	case 36:		/* low C --> rest */
		type = REST;
		break;
	case 37:		/* low C# --> bar */
		type = BAR;
		break;
	default:
		type = NOTE;
		break;
	}
	obj = Tcl_NewObj();
	Tcl_IncrRefCount(obj);
	Tcl_ListObjAppendElement(my_interp, obj, type_obj[type]);
	if (type != BAR) {
		Tcl_ListObjAppendElement(my_interp, obj, Tcl_NewIntObj(len));
		if (type == NOTE) {
			Tcl_ListObjAppendElement(my_interp, obj, Tcl_NewIntObj(pit));
			Tcl_ListObjAppendElement(my_interp, obj, Tcl_NewIntObj(acc));
		}
	} else {
		Tcl_ListObjAppendElement(my_interp, obj, Tcl_NewIntObj(1));
		Tcl_ListObjAppendElement(my_interp, obj, empty_obj);
	}
	sym_set(my_interp, obj, 1);
	Tcl_DecrRefCount(obj);
	free(meep);
	free(prev_meep);
	prev_meep = 0;

	/* call the handler to redisplay the score */
	script = Tcl_NewObj();
	Tcl_IncrRefCount(script);
	Tcl_ListObjAppendElement(my_interp, script, midi_handler);
	Tcl_CreateTimerHandler(100,
			       after_proc,
			       (ClientData) script);
}

/* -- load ABC from MIDI -- */
/* the ABC tune contains the empty header (X, T, M and K) */
void midi_load(struct abctune *t)
{
	MidiEventList *meep;
	int sf1, sf, minor;
	unsigned char m1, m2;
	int channels;		/* handle 32 channels */
	int chan, voice, tempo;
	char buf[128];
	int have_title, have_key, have_time, have_tempo;
	struct abcsym *as, *p_key, *last_sav;

	midi_header.tempo = MIDI_DEFAULT_TEMPO;
	channels = 0;
	TRACE(("midi_load\n%s\n", dump_head));

	p_key = t->last_sym;	/* K: */

	/* first pass: find the channels with notes
	 * and define the ABC header */
	have_title = have_key = have_time = have_tempo = 0;
	sf = p_key->u.key.sf;
	minor = p_key->u.key.mode == MINOR;
	m1 = m2 = 4;
	tempo = 0;
	for (meep = midi_header.evlist->next; meep; meep = meep->next) {
#ifdef DEBUG
		dump_midi_event(meep);
#endif
		switch (meep->event.type) {
		case ME_LYRIC:
			if (meep->event.misc[1] != ':')
				break;
			switch (*meep->event.misc) {
			case 'C':
				last_sav = t->last_sym;
				t->last_sym = p_key->prev;
				as = abc_new(t, meep->event.misc, 0);
				as->type = ABC_T_INFO;
				as->state = ABC_S_HEAD;
				t->last_sym = last_sav;
				break;
			case 'T':
				if (have_title)
					break;
				have_title = 1;		/* one title only */
				as = t->first_sym;
				while (as->text[0] != 'T')
					as = as->next;
				free(as->text);
				as->text = malloc(strlen(meep->event.misc) + 1);
				strcpy(as->text, meep->event.misc);
				break;
			}
			break;
		case ME_KEY:
			if (have_key)
				break;
			have_key = 1;
			sf = meep->event.a;
			if (sf >= 128)
				sf -= 256;
			minor = meep->event.b;
			abc_new_key(t, sf, minor, p_key);
			break;
		case ME_TIME:
			if (have_time)
				break;
			have_time = 1;
			m1 = meep->event.a;
			m2 = meep->event.b;
			as = t->first_sym;
			while (as->text[0] != 'M')
				as = as->next;
			free(as->text);
			sprintf(buf, "M:%d/%d", m1, m2);
			as->text = malloc(strlen(buf) + 1);
			strcpy(as->text, buf);
			as->u.meter.wmeasure = m1 * BASE_LEN / m2;
			as->u.meter.nmeter = 1;
			sprintf(as->u.meter.meter[0].top, "%d", m1);
			sprintf(as->u.meter.meter[0].bot, "%d", m2);
			break;
		case ME_TEMPO: {
			int beats_per_mn;

			if (have_tempo)
				break;
			have_tempo = 1;
			beats_per_mn = 60000000
				/ ((meep->event.a << 16)
				   + (meep->event.b << 8)
				   + meep->event.channel);
			tempo = beats_per_mn;
			sprintf(buf, "Q:1/4=%d", beats_per_mn);
			last_sav = t->last_sym;
			t->last_sym = p_key->prev;
			as = abc_new(t, buf, 0);
			t->last_sym = last_sav;
			as->type = ABC_T_INFO;
			as->state = ABC_S_HEAD;
			as->u.tempo.length[0] = BASE_LEN / 4;
			sprintf(buf, "%d", beats_per_mn);
			as->u.tempo.value = malloc(strlen(buf) + 1);
			strcpy(as->u.tempo.value, buf);
			break;
		}
		case ME_NOTEON:
			channels |= 1 << meep->event.channel;
			break;
		}
	}

	/* loop on the channels */
	sf1 = sf;
	voice = -1;
	for (chan = 0; chan < MAX_MIDI_CHANNELS; chan++) {
		int lasttim;

		if (!(channels & (1 << chan)))
			continue;	/* no notes for this voice */

		/* define the voice */
		/*fixme: get the voice fullname from the track name?*/
		voice++;
		sprintf(buf, "V:%d", voice + 1);
		TRACE(("chan:%d voice:%d name:%s\n",
		       chan, voice, buf));
		as = abc_new(t, buf, 0);
		as->type = ABC_T_INFO;
		as->state = ABC_S_TUNE;
		as->u.voice.voice = voice;
		strcpy(as->u.voice.id, &buf[2]);

		/* define the channel */
		if (chan != voice) {
			sprintf(buf, "%%%%MIDI channel %d", chan + 1);
			as = abc_new(t, buf, 0);
			as->type = ABC_T_PSCOM;
			as->state = ABC_S_TUNE;
		}

		lasttim = 0;
		if (sf != sf1) {
			sf = sf1;
			as = abc_new_key(t, sf, minor, 0);
			as->state = ABC_S_TUNE;
		}
		set_notacc(sf);
		for (meep = midi_header.evlist->next; meep; meep = meep->next) {
			switch (meep->event.type) {
			case ME_KEY: {
				int newsf;

				newsf = meep->event.a;
				if (newsf >= 128)
					newsf -= 256;
				minor = meep->event.b;
				if (sf != newsf) {	/* if keysig changed */
					sf = newsf;
					as = abc_new_key(t, sf, minor, 0);
					as->state = ABC_S_EMBED;
					set_notacc(sf);
				}
				break;
			}
			case ME_PROGRAM:
				if (meep->event.channel != chan)
					break;
				sprintf(buf, "%%%%MIDI program %d",
					meep->event.a);
				as = abc_new(t, buf, 0);
				as->type = ABC_T_PSCOM;
				as->state = ABC_S_TUNE;
				break;
			case ME_TIME:
				if (m1 == meep->event.a		/* if timesig changed */
				    && m2 == meep->event.b)
					break;
				m1 = meep->event.a;
				m2 = meep->event.b;
				sprintf(buf, "M:%d/%d", m1, m2);
				as = abc_new(t, buf, 0);
				as->type = ABC_T_INFO;
				as->state = ABC_S_EMBED;
				as->u.meter.wmeasure = m1 * BASE_LEN / m2;
				as->u.meter.nmeter = 1;
				sprintf(as->u.meter.meter[0].top, "%d", m1);
				sprintf(as->u.meter.meter[0].bot, "%d", m2);
				break;
			case ME_TEMPO: {
				int beats_per_mn;

				beats_per_mn = 60000000
					/ ((meep->event.a << 16)
					   + (meep->event.b << 8)
					   + meep->event.channel);
				if (beats_per_mn == tempo)
					break;		/* same tempo */
				tempo = beats_per_mn;
				sprintf(buf, "Q:1/4=%d", beats_per_mn);
				as = t->last_sym;
				if (as->type != ABC_T_INFO
				    || as->text[0] != 'Q') {
					as = abc_new(t, buf, 0);
					as->type = ABC_T_INFO;
					as->state = ABC_S_EMBED;
				}
				as->u.tempo.length[0] = BASE_LEN / 4;
				sprintf(buf, "%d", beats_per_mn);
				as->u.tempo.value = malloc(strlen(buf) + 1);
				strcpy(as->u.tempo.value, buf);
				break;
			}
			case ME_NOTEON: {
				MidiEventList *next;
				int nhd, len, minlen, curtim, nexttim;

				if (meep->event.channel != chan
				    || meep->event.b == 0)	/* ignore note off */
					break;

				/* adjust the time */
				curtim = (meep->event.time
					  * BASE_LEN / 4 / midi_header.divisions
					  + 24) / mindur * mindur;

				/* put some silence if needed */
				len = curtim - lasttim;
				if (len < 0)
					curtim = lasttim;
				else while (len > 0) {
					as = abc_new(t, 0, 0);
					as->type = ABC_T_REST;
					as->state = ABC_S_TUNE;
					as->u.note.pits[0] = 12;
					if (len <= BASE_LEN * 8) {
						as->u.note.lens[0] = len;
						break;
					}
					as->u.note.lens[0] = BASE_LEN * 8;
					len -= BASE_LEN * 8;
				}
				as = abc_new(t, 0, 0);
				as->type = ABC_T_NOTE;
				as->state = ABC_S_TUNE;

				/* search the next noteon for the same channel */
				nexttim = curtim + BASE_LEN * 8;
				for (next = meep->next; next != 0; next = next->next) {
					if (next->event.channel == chan
					    && next->event.type == ME_NOTEON
					    && next->event.b != 0) {
						nexttim = (next->event.time
							   * BASE_LEN / 4 / midi_header.divisions
							   + 24) / mindur * mindur;
						if (nexttim > curtim)
							break;
					}
				}

				/* define the notes */
				nhd = 0;
				minlen = 1000000;
				for (;;) {
					int pit, acc;

					note_define(meep,
						    curtim, nexttim,
						    &pit, &len, &acc);
					as->u.note.pits[nhd] = pit;
					as->u.note.lens[nhd] = len;
					as->u.note.accs[nhd] = acc;
					TRACE(("chan:%d voice:%d pit:%d len:%d acc:%d\n",
					       chan, voice, pit, len, acc));
					if (len < minlen)
						minlen = len;
					if (meep->next == 0
					    || meep->next->event.channel != chan
					    || meep->next->event.type != ME_NOTEON
					    || meep->next->event.b == 0
					    || (meep->next->event.time
						* BASE_LEN / 4 / midi_header.divisions
						+ 24) / mindur * mindur != curtim)
						break;
					meep = meep->next;
					if (nhd >= MAXHD - 1)
						trace("Too many notes in chord\n");
					else
						nhd++;
				}
				as->u.note.nhd = nhd;
				lasttim = curtim + minlen;
				break;
			    }
			}
		}
	}
}

/* -- read a MIDI file -- */
int midi_read_file(Tcl_Interp *interp,
		   char *fname)
{
	MFILE fd;
	int len, divisions, i;
	short format, tracks;
	char tmp[4];

	/* free the previous MIDI tune */
	free_midi_list();

	at = 0;
	my_interp = interp;

#ifndef WIN32
	if ((fd = open(fname, O_RDONLY)) < 0) {
		sprintf(interp->result, "can't open %s", fname);
		return TCL_ERROR;
	}
#else
	if ((fd = Tcl_OpenFileChannel(interp, fname, "r", 0644)) == 0)
		return TCL_ERROR;
	if (Tcl_SetChannelOption(interp, fd, "-encoding", "binary") != TCL_OK)
		return TCL_ERROR;
#endif

/*	drumflag = midi_header.drumflag;*/
/*	check_gs_macro = midi_header.check_gs_macro;*/
	READID(tmp, fd);
	len = READLONG(fd);
	if (memcmp(tmp, "MThd", 4) || len < 6) {
		sprintf(interp->result, "not a MIDI file\n");
		CLOSE(fd);
		return TCL_ERROR;
	}

	format = READSHORT(fd);
	tracks = READSHORT(fd);
	divisions = READSHORT(fd);
	if (divisions < 0)
		divisions = - (divisions / 256) * (divisions & 0xff);

	if (len > 6)
		skip(fd, len - 6);	/* skip the excess */
	if ((unsigned) format > 2) {
		sprintf(interp->result, "unknown MIDI file format %d\n", format);
		CLOSE(fd);
		return TCL_ERROR;
	}

	midi_header.format = format;
	midi_header.tracks = tracks;
	midi_header.divisions = divisions;

/*	trace("format=%d, tracks=%d, divs=%d\n",
		format, tracks, divisions);*/
	/* Put a do-nothing event first in the list for easier processing */
	midi_header.evlist = (MidiEventList *) malloc(sizeof(MidiEventList));
	memset(midi_header.evlist, 0, sizeof(MidiEventList));
	midi_header.last_event = midi_header.evlist;

	mindur = 48;		/* demisemiquaver */

	if (format == 0)
		tracks = 1;
	for (i = 0; i < tracks; i++) {
		/* with format 1,
		 * normally track 0 is used for controls
		 * track 1-16 for channel 1-16
		 * track 17-32 for channel 17-32 */
		if (read_track(fd, i, format == 2)) {
			CLOSE(fd);
			return TCL_ERROR;
		}
	}

/*	midi_header.drumflag = drumflag;*/

	CLOSE(fd);
	return TCL_OK;
}

/* -- get the note end time -- */
/* handle the ties - fixme:slurs and dotted notes to do*/
static int play_note_end(struct sym *s,
			 int hd)
{
	int pit, note_end;

	if (s->as.u.note.nhd != 0
	 && s->dtime != s->as.u.note.lens[hd])
		note_end = s->dtime * s->as.u.note.lens[hd]
			/ s->as.u.note.lens[0];
	else
		note_end = s->dtime;
	note_end += s->time;
	pit = s->as.u.note.pits[hd];

	/* handle the ties */
/*fixme: keep dotted indication*/
	if (s->as.u.note.ti1[hd])
	    for (;;) {
		for (s = s->next; ; s = s->next) {
			switch (s->type) {
			default:
				continue;
			case EOT:
			case NOTE:
			case REST:
				break;
			}
			break;
		}
		if (s->type != NOTE)
			break;
/*fixme:KO when chord with many lengths*/
		note_end += s->dtime;

		for (hd = s->as.u.note.nhd; hd >= 0; hd--) {
			if (s->as.u.note.ti1[hd]
			    && s->as.u.note.pits[hd] == pit)
				break;
		}
		if (hd < 0)
			break;
	}
	return note_end
		- 24;		/*fixme: longer when slur, shorter when dotted note*/
}

/* work out accidentals to be applied to each note */
void setmap(int sf,	/* number of sharps/flats in key sig (-7 to +7) */
	    char *map)	/* for one octave (7 notes) only */
{
	memset(map, A_NULL, 7);
	switch (sf) {
	case 7: map[6] = A_SH;
	case 6: map[2] = A_SH;
	case 5: map[5] = A_SH;
	case 4: map[1] = A_SH;
	case 3: map[4] = A_SH;
	case 2: map[0] = A_SH;
	case 1: map[3] = A_SH;
		break;
	case -7: map[3] = A_FT;
	case -6: map[0] = A_FT;
	case -5: map[4] = A_FT;
	case -4: map[1] = A_FT;
	case -3: map[5] = A_FT;
	case -2: map[2] = A_FT;
	case -1: map[6] = A_FT;
		break;
	}
}

/* -- write a variable length value -- */
static void putvl(MFILE fd, unsigned int v)
{
	int i;
	unsigned char c[8];

	i = sizeof c;
	c[--i] = v & 0x7f;
	while ((v >>= 7) != 0)
		c[--i] = (v & 0x7f) + 0x80;
	WRITE(fd, &c[i], sizeof c - i);
}

/* put the delta time */
static void put_dtime(int newtime)
{
	if (newtime < dump_time)
		newtime = dump_time;
	putvl(out_fd, newtime - dump_time);
	dump_time = newtime;
}

/* -- start playing/dumping a note or chord -- */
static void note_start(struct sym *s,
			struct vtb_s *vp)
{
	int hd, channel;
	unsigned channels;

	for (hd = 0; hd <= s->as.u.note.nhd; hd++) {
		int pit, freq, i;

		pit = s->as.u.note.pits[hd];
		if (s->flags & F_TIE_E) {
			struct sym *s2;

			for (s2 = s->prev; ; s2 = s2->prev)
				if (s2->type == NOTE)
					break;
			for (i = s2->as.u.note.nhd; i >= 0; i--) {
				if (s2->as.u.note.ti1[i]
				    && s2->as.u.note.pits[i] == pit)
					break;
			}
			if (i >= 0)
				continue;	/* tied note */
		}
		pit += 19;			/* pitch from lowest C */
		if (s->as.u.note.accs[hd] != 0) {
			vp->workmap[pit] = s->as.u.note.accs[hd] == A_NT
				? A_NULL
				: s->as.u.note.accs[hd];
			vp->mapchange = 1;
		}
		freq = scale[pit % 7]
			+ acc_pitch[(int) vp->workmap[pit]]
			+ 12 * (pit / 7)
			+ vp->transpose;

		/* memorize when to stop */
		channels = vp->channels;
		play_tb[play_nb].channels = channels;
		play_tb[play_nb].freq = freq;
		play_tb[play_nb].time = play_note_end(s, hd);
		play_nb++;

		/* start the note(s) */
		if (!voice_tb[s->voice].mute) {
			for (channel = 0;
			     channels != 0;
			     channel++, channels >>= 1) {
				if (!(channels & 1))
					continue;
				if (!dump) {
					seq_note(channel,	/* channel */
						 freq,		/* frequency */
						 volume);	/* velocity */
				} else {
					int newst;
					unsigned char c[2];

					newst = (1 << 4) + channel;	/* note on */
					put_dtime(s->time);
					if (newst != dump_status) {
						dump_status = newst;
						c[0] = dump_status | 0x80;
						WRITE(out_fd, c, 1);
					}
					c[0] = freq;
					c[1] = volume;
					WRITE(out_fd, c, 2);
				}
			}
		}
	}
}

/* stop the played/dumped notes at current time */
static void notes_stop(int curtime)
{
	struct play_s *pl;
	int i;

	for (i = play_nb, pl = &play_tb[play_nb - 1]; --i >= 0; pl--) {
		int channel;
		unsigned channels;

		if (pl->time > curtime)
			continue;
		for (channel = 0, channels = pl->channels;
		     channels != 0;
		     channel++, channels >>= 1) {
			if (!(channels & 1))
				continue;
			if (!dump) {
				seq_note(channel,	/* channel */
					 pl->freq,	/* frequency */
					 0);		/* velocity */
			} else {
				int newst;
				unsigned char c[2];

				newst = (0 << 4) + channel;	/* note off */
				put_dtime(pl->time);
				if (newst != dump_status) {
					dump_status = newst;
					c[0] = dump_status | 0x80;
					WRITE(out_fd, c, 1);
				}
				c[0] = pl->freq;
				c[1] = 0;
				WRITE(out_fd, c, 2);
			}
		}

		/* remove the stop note item */
		if (i != --play_nb)
			memcpy(pl, &play_tb[play_nb], sizeof *pl);
	}
}

/* -- synchronize the accidentals for playing/dumping -- */
static void acc_sync(struct sym *s,
		     char *keymap)	/* shall be 7 notes * 10 octave */
{
	struct sym *s2;
	int i;

	for (s2 = s->prev; s2->type != EOT; s2 = s2->prev) {
		if (s2->type == BAR
		    || s2->type == KEYSIG)
			break;
	}
	for (s2 = s2->next; s2 != s; s2 = s2->next) {
		if (s2->type != NOTE)
			continue;
		for (i = 0; i <= s2->as.u.note.nhd; i++) {
			int pit;

			if (s2->as.u.note.accs[i] == A_NULL)
				continue;
			pit = s2->as.u.note.pits[i] + 19;	/* pitch from lowest C */
			keymap[pit] = s2->as.u.note.accs[i] == A_NT
				? A_NULL
				: s2->as.u.note.accs[i];
		}
	}
}

/* -- define the channels of a voice -- */
static void channel_def(struct vtb_s *vp,
			int goaltime)
{
	int i;
	unsigned int u;
	struct sym *s;
	struct {
		signed char prog, bank;
	} pgtb[32];

	memset(pgtb, -1, sizeof pgtb);
	if ((s = vp->s) == 0)
		s = voice_tb[vp - vtb].eot->next;
	for (; s->time <= goaltime; s = s->next) {
		switch (s->type) {
		case MIDI:
			switch (s->MIDI_type) {
			case MIDI_CHANNEL:
				vp->channels = s->MIDI_channels;
				break;
			case MIDI_PROGRAM:
				i = s->MIDI_channel;
				pgtb[i].prog = s->MIDI_program;
				pgtb[i].bank = s->MIDI_bank;
				break;
			default:
				if (strncmp(&s->as.text[7], "transpose ", 10) == 0)
					vp->transpose = atoi(&s->as.text[17]);
				break;
			}
			continue;
		case EXTRA_SYM:
			if (s->EXTRA_type != EXTRA_VOVER)
				continue;
			vtb[s->u.vover->voice].channels = vp->channels;
			channel_def(&vtb[s->u.vover->voice], goaltime);
			continue;
		default:
			if (!dump
//fixme
			 || s->voice != 0)
				continue;
			vp->s = s;
			play_advance(vp);
			s = vp->s;
			break;
		case NOTE:
		case REST:
			if (s->time < goaltime)
				continue;
			break;
		case EOT:
			break;
		}
		break;
	}
	vp->s = s;		/* starting symbol */

	/* synchronize the accidentals */
	setmap(s->sf, vp->basemap);
	for (i = 0; i < 10; i++)
		memcpy(&vp->workmap[7 * i], vp->basemap, 7);
	acc_sync(s, vp->workmap);

	if (vp->channels == 0)
		vp->channels = 1 << (vp - vtb);
	for (i = 0, u = 1; i < 32; i++, u <<= 1) {
		if (pgtb[i].prog >= 0)
			set_program(i, pgtb[i].prog, pgtb[i].bank);
		else if (vp->channels & u)
			set_program(i, 0, 0);
	}
}

/* -- initialize dumping or playing all notes -- */
static int midi_init(int goaltime)
{
	int v, basetime;
	struct sym *s;
	struct vtb_s *vp;

	/* initialize and synchronize the accidentals */
	play_stop_time = EOT_TIME;	/*fixme: may stop before EOT*/
	play_nb = 0;
	basetime = EOT_TIME + 1;
	for (v = 0, vp = &vtb[0]; v <= nvoice; v++, vp++) {
		if (voice_tb[v].vover) {
			if (vp->s == 0)
				vp->s = voice_tb[v].eot->next;
			continue;
		}
		channel_def(vp, goaltime);
		s = vp->s;
		if (s->type == EOT)
			continue;	/* no note for this voice */
		if (s->time < basetime)
			basetime = s->time;
	}
	return basetime;
}

/* -- advance to the next note while playing/dumping -- */
static int play_advance(struct vtb_s *vp)
{
	struct sym *s;
	int i, v, channel, newst;
	int play_stop = 0;
	unsigned char c[64];

	s = vp->s;
	while (s->type != NOTE
	    && s->type != EOT) {
		switch (s->type) {
		case KEYSIG:
			setmap(s->as.u.key.sf, vp->basemap);
			for (i = 0; i < 10; i++)
				memcpy(&vp->workmap[7 * i], vp->basemap, 7);
			if (!dump || s->voice != 0)
				break;
			put_dtime(s->time);
			c[0] = 0xff;		/* meta-event */
			c[1] = 0x59;		/* key signature */
			c[2] = 2;		/* length */
			c[3] = s->as.u.key.sf;
			c[4] = s->as.u.key.mode == MINOR ? 1 : 0;
			WRITE(out_fd, c, 5);
			break;
		case TIMESIG:
			if (!dump || s->voice != 0)
				break;
			put_dtime(s->time);
			c[0] = 0xff;		/* meta-event */
			c[1] = 0x58;		/* time signature */
			c[2] = 4;		/* length */
			c[3] = 4;		/* default 4/4 */
			c[4] = 2;
			c[5] = 24;
			c[6] = 8;
			if (s->as.u.meter.meter[0].top[0] == 'C') {
				if (s->as.u.meter.meter[0].top[1] == '|') {
					c[3] = 2;
					c[4] = 1;
				}
			} else {
				int top, bot;

				sscanf(s->as.u.meter.meter[0].top, "%d", &top);
				c[3] = top;
				sscanf(s->as.u.meter.meter[0].bot, "%d", &bot);
				switch (bot) {
				case 1: c[4] = 0; break;
				case 0: bot = 2; /* fall thru */
				case 2: c[4] = 1; break;
/*				case 4: c[4] = 2; break; */
				case 8: c[4] = 3; break;
				case 16: c[4] = 4; break;
				}
				c[5] = 24 * top / bot;
			}
			WRITE(out_fd, c, 7);
			break;
		case TEMPO:
			if (s->as.u.tempo.value == 0	/* if no new value */
			 || vp != vtb)			/* or not 1st voice */
				break;
			if (!dump) {

				/* (see play_time for the time change) */
				v = strtol(s->as.u.tempo.value, 0, 10);
				if (v < 8 || v > 200)
					break;
				play_factor_new = s->as.u.tempo.length[0] * v;
				break;
			}
			if (s->voice != 0)
				break;
			put_dtime(s->time);
			c[0] = 0xff;		/* meta-event */
			c[1] = 0x51;		/* tempo */
			c[2] = 3;		/* length */
			{
				int v;

				v = strtol(s->as.u.tempo.value, 0, 10);
				if (v < 8 || v > 200)
					break;
				v *= s->as.u.tempo.length[0];
				v = 60000 * (BASE_LEN / 4) / v;	/* ms */
				v *= 1000;			/* us */
				c[3] = v >> 16;
				c[4] = v >> 8;
				c[5] = v;
			}
			WRITE(out_fd, c, 6);
			break;
		case BAR:
			if (vp->mapchange) {
				for (i = 0; i < 10; i++)
					memcpy(&vp->workmap[7 * i], vp->basemap, 7);
				vp->mapchange = 0;
			}
			if (vp != vtb)
				break;
			if (s->flags & F_RRBAR) {
				if (repeat_flag == REPEAT_NONE)
					repeat_flag = REPEAT_FOUND;
				else if (repeat_flag == REPEAT_RUN)
					repeat_flag = REPEAT_DONE;
			} else if (repeat_flag == REPEAT_RUN
				&& s->as.u.bar.repeat_bar) {	/* 1st time */
				repeat_flag = REPEAT_DONE;
			}
			if (repeat_flag == REPEAT_FOUND
			 || repeat_flag == REPEAT_DONE) {
				play_stop_time = s->time;
				play_stop = 1;
				break;
			}
			if ((s->as.u.bar.type & 0xf0) != 0 /* if complex bar */
			 && s->as.u.bar.type != (B_OBRA << 4) + B_CBRA
			 && !s->as.u.bar.repeat_bar) {
				lrepeat_time = s->time;
				break;
			}
			break;
		case MIDI:
			switch (s->MIDI_type) {
			case MIDI_CHANNEL:
				vp->channels = s->MIDI_channels;
				break;
			case MIDI_PROGRAM:
				if (!dump) {
					set_program(s->MIDI_channel,
						    s->MIDI_program,
						    s->MIDI_bank);
					break;
				}
				channel = s->MIDI_channel;
#if 0
				newst = (3 << 4) + channel;	/* control change */
				put_dtime(s->time);
				if (newst != dump_status) {
					dump_status = newst;
					c[0] = dump_status | 0x80;
					write(out_fd, c, 1);
				}
				c[0] = 0;			/* bank select */
				c[1] = s->MIDI_bank;
				WRITE(out_fd, c, 2);
#endif
				newst = (4 << 4) + channel;	/* program change */
				put_dtime(s->time);
				if (newst != dump_status) {
					dump_status = newst;
					c[0] = dump_status | 0x80;
					WRITE(out_fd, c, 1);
				}
				c[0] = s->MIDI_program;
				WRITE(out_fd, c, 1);
				break;
			default:
				if (strncmp(&s->as.text[7], "transpose ", 10)
						== 0)
					vp->transpose = atoi(&s->as.text[17]);
//fixme
				break;
			}
			break;
		case EXTRA_SYM:
			if (s->EXTRA_type != EXTRA_VOVER)
				break;
			vtb[s->u.vover->voice].channels = vp->channels;
			channel_def(&vtb[s->u.vover->voice], goaltime);
			break;
		}
		if (play_stop)
			break;
		s = s->next;
	}
	vp->s = s;
	if (play_stop || s->type == EOT)
		return 0;
	return 1;
}

/* go back to the start of the repeat sequence */
static void repeat_back(void)
{
	struct sym *s;
	struct vtb_s *vp;
	int voice;

	repeat_flag = REPEAT_RUN;
	rrepeat_time = vtb[0].s->time;

	for (voice = 0, vp = vtb; voice <= nvoice; voice++, vp++) {
		s = vp->s;
		if (s->time > rrepeat_time)
			continue;
		while (s->time > lrepeat_time && s->type != EOT)
			s = s->prev;
		if (s->type == EOT)
			s = s->next;
//			break;
		vp->s = s;
	}
}

/* go to the end of the repeated sequence */
static void repeat_forw(void)
{
	struct sym *s;
	int voice;
	struct vtb_s *vp;

	repeat_flag = REPEAT_NONE;
	for (voice = 0, vp = vtb; voice <= nvoice; voice++, vp++) {
		s = vp->s;
		while (s->time < rrepeat_time && s->type != EOT)
			s = s->next;
		while (s->time == rrepeat_time
		    && !(s->flags & F_RRBAR)
		    && s->type != EOT)
			s = s->next;
		if (s->flags & F_RRBAR)
			s = s->next;
		vp->s = s;
	}
}

/* ---------- real-time MIDI playing / recording stuff ---------- */
#ifdef HAVE_SOUND
/* --- MIDI sequencer --- */
#ifndef WIN32
/* OSS */
static int seq_dev = -1;
static int seq_fd = -1;
#ifdef HAVE_AWE
static int is_awe;
#endif
/* OSS */
static int seq_bend = 0x2000;
SEQ_DEFINEBUF(256);

void seqbuf_dump(void)
{
	if (_seqbufptr)
		if (write(seq_fd, _seqbuf, _seqbufptr) == -1) {
			perror("write sequencer");
			exit(-1);
		}
	_seqbufptr = 0;
}
#ifdef HAVE_ALSA
static snd_seq_t *seq_alsa_handle;
static int seq_alsa_fd = -1;
static snd_seq_event_t ev;
static int my_in_port = -1;
static int my_out_port = -1;
#endif

static int tempo_default = 80;	/* default tempo: 80 crotchets per mn */
static int playing;
static int basetime;		/* play starting time */
static int dyn = 1;		/* use keyboard velocity when 1 */

static void play_time(void);
static void set_timer(int delay);

/* MIDI recording */
static struct {
	char started;
	char active;
	char channel;
	char bank;
	short program;
	short freq;
	short duration;
	short beat;
	int time;
} metronome = {0, 0,
	31, 0, 112,		/* chan: 32, bank: 1, prog: bell */
	60, 96, 0, 0};
static int midi_recording;

#ifndef WIN32
static int midi_fd = -1;
#ifdef HAVE_ALSA
static void kbd_alsa_event(ClientData client,
			int mask);
#endif
static void kbd_oss_event(ClientData client,
			int mask);
#else /*WIN32*/
static HMIDIIN ih;
static void CALLBACK kbd_event(HMIDIIN handle,
			UINT uMsg,
			DWORD dwInstance,
			DWORD dwParam1,
			DWORD dwParam2);
#endif /*WIN32*/
static void metronome_time(void);

/* -- MIDI devices handling -- */

#ifdef HAVE_ALSA
/* -- initialize ALSA -- */
static int alsa_init(void)
{
	snd_seq_t *handle;
	int st;

	if (seq_alsa_handle)
		return TCL_OK;
	if ((st = snd_seq_open(&handle, "default",
			SND_SEQ_OPEN_DUPLEX, 0)) < 0) {
		trace("can't open ALSA\n");
		return TCL_ERROR;
	}
	snd_seq_set_client_name(handle, "tclabc");
	seq_alsa_handle = handle;
	snd_seq_ev_clear(&ev);
	return TCL_OK;
}

/* -- return the ALSA list of MIDI in or out -- */
static void alsa_list(Tcl_Obj *obj, int out)
{
	snd_seq_client_info_t *cinfo;
	snd_seq_port_info_t *pinfo;
	int  client, cap;
	char buf[256];

	if (alsa_init() != TCL_OK)
		return;
	if (out)
		cap = SND_SEQ_PORT_CAP_SUBS_WRITE | SND_SEQ_PORT_CAP_WRITE;
	else
		cap = SND_SEQ_PORT_CAP_SUBS_READ | SND_SEQ_PORT_CAP_READ;
	snd_seq_client_info_alloca(&cinfo);
	snd_seq_port_info_alloca(&pinfo);
	snd_seq_client_info_set_client(cinfo, -1);
	while (snd_seq_query_next_client(seq_alsa_handle, cinfo) >= 0) {
		if (strcmp(snd_seq_client_info_get_name(cinfo),
			   "tclabc") == 0)
			continue;	/* skip ourself */
		client = snd_seq_client_info_get_client(cinfo);
		if (client == 0)
			continue;	/* skip Timer, Annonce.. */
		snd_seq_port_info_set_client(pinfo, client);
		snd_seq_port_info_set_port(pinfo, -1);
		while (snd_seq_query_next_port(seq_alsa_handle, pinfo) >= 0) {
			if ((snd_seq_port_info_get_capability(pinfo) & cap) == cap) {
				sprintf(buf, "%d:%d %s",
					snd_seq_port_info_get_client(pinfo),
					snd_seq_port_info_get_port(pinfo),
/*					snd_seq_client_info_get_name(cinfo),*/
					snd_seq_port_info_get_name(pinfo));
TRACE(("%s\n", buf));
				Tcl_ListObjAppendElement(my_interp, obj,
					 Tcl_NewStringObj(buf,
							  strlen(buf)));
			}
		}
	}
#if 0
	snd_seq_port_info_free(pinfo);
	snd_seq_client_info_free(cinfo);
#endif
}
#endif /*HAVE_ALSA*/

/* -- return a list of devices -- */
static int devlist(int out, char *device)
{
	int i, n;
	Tcl_Obj *obj;
	char buf[256];

	obj = Tcl_NewObj();
	Tcl_IncrRefCount(obj);
#ifndef WIN32
	if (out) {
		if (device != 0 && strstr(device, "seq") != 0) {
			int fd;

			if (midiout != 0
			 && strcmp(Tcl_GetString(midiout), device) == 0)
				fd = seq_fd;
			else
				fd = open(device, O_WRONLY, 0);
			if (fd >= 0) {
TRACE(("synth\n"));
				if (ioctl(fd, SNDCTL_SEQ_NRSYNTHS,
					  &n) >= 0) {
					struct synth_info card_info;

					for (i = 0; i < n; i++) {
						card_info.device = i;
						if (ioctl(fd, SNDCTL_SYNTH_INFO, &card_info) >= 0) {
							sprintf(buf, "%s:%d (%s)",
								device, i, card_info.name);
TRACE(("%s\n", buf));
							Tcl_ListObjAppendElement(my_interp, obj,
								 Tcl_NewStringObj(buf,
									  strlen(buf)));
						}
					}
				}
				if (fd != seq_fd)
					close(fd);
			}
		}
	}
#ifdef HAVE_ALSA
	alsa_list(obj, out);
#endif /*HAVE_ALSA*/
#else /*WIN32*/
	if (out) {
		MIDIOUTCAPS moc;

		n = midiOutGetNumDevs();
		for (i = -1; i < n; i++) {
			if (midiOutGetDevCaps(i, &moc, sizeof moc) != 0)
				continue;
			sprintf(buf, "%d (%s)", i, moc.szPname);
			Tcl_ListObjAppendElement(my_interp, obj,
						 Tcl_NewStringObj(buf,
								  strlen(buf)));
		}
	} else {
		MIDIINCAPS mic;

		n = midiInGetNumDevs();
		for (i = 0; i < n; i++) {
			if (midiInGetDevCaps(i, &mic, sizeof mic) != 0)
				continue;
			sprintf(buf, "%d (%s)", i, mic.szPname);
			Tcl_ListObjAppendElement(my_interp, obj,
						 Tcl_NewStringObj(buf,
								  strlen(buf)));
		}
	}
#endif /*WIN32*/
	Tcl_SetObjResult(my_interp, obj);
	Tcl_DecrRefCount(obj);
	return TCL_OK;
}

/* -- close the MIDI input -- */
static void midi_in_close(void)
{
#ifndef WIN32
#ifdef HAVE_ALSA
	if (my_in_port >= 0) {
		snd_seq_delete_simple_port(seq_alsa_handle, my_in_port);
		my_in_port = -1;
		Tcl_DeleteFileHandler(seq_alsa_fd);
		return;
	}
#endif
	if (midi_fd >= 0) {
		Tcl_DeleteFileHandler(midi_fd);
		close(midi_fd);
		midi_fd = -1;
	}
#else /*WIN32*/
	if (ih != 0) {
		midiInReset(ih);
		midiInClose(ih);
		ih = 0;
	}
#endif /*WIN32*/
}

#ifdef HAVE_ALSA
/* -- connect to the MIDI in port -- */
static int alsa_in_init(char *device)
{
	int st, port, rclient, rport;
	struct pollfd pfd;

	if (sscanf(device, "%d:%d", &rclient, &rport) != 2)
		return TCL_ERROR;
	if (alsa_init() != TCL_OK)
		return TCL_ERROR;
	if ((port = snd_seq_create_simple_port(seq_alsa_handle,
			"tclabc in",
		    SND_SEQ_PORT_CAP_SUBS_WRITE
			| SND_SEQ_PORT_CAP_WRITE,
		    SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
		trace("cannot create my ALSA in port\n");
		return TCL_ERROR;
	}
	if (snd_seq_connect_from(seq_alsa_handle, port,
			rclient, rport) < 0) {
		trace("cannot connect to ALSA in client\n");
		return TCL_ERROR;
	}
	midi_in_close();
	if ((st = snd_seq_poll_descriptors(seq_alsa_handle, &pfd, 1, POLLIN)) < 0) {
		trace("cannot get ALSA fd\n");
		return TCL_ERROR;
	}
	my_in_port = port;
	seq_alsa_fd = pfd.fd;
	Tcl_CreateFileHandler(seq_alsa_fd,
			      TCL_READABLE,
			      kbd_alsa_event,
			      (ClientData) 0);
	return TCL_OK;
}

/* -- send an ALSA event -- */
static void alsa_send(void)
{
	snd_seq_event_output(seq_alsa_handle, &ev);
}
#endif /*HAVE_ALSA*/

/* - open the midi input device - */
int midi_in_init(char *device)
{
#ifndef WIN32
	int fd;

	if (device == 0 || *device == '\0') {
		midi_in_close();
		return TCL_OK;
	}
#ifdef HAVE_ALSA
	/* the ALSA input is '<client#>:<port#>' */
	if (isdigit((unsigned char) *device))
		return alsa_in_init(device);
#endif
	if ((fd = open(device, O_RDONLY)) < 0) {
		perror("open");
		trace("cannot open MIDI in '%s'\n", device);
		return TCL_ERROR;
	}
	midi_in_close();
	midi_fd = fd;
	Tcl_CreateFileHandler(fd,
			      TCL_READABLE,
			      kbd_oss_event,
			      (ClientData) 0);
#else /*WIN32*/
	int dev;

	if (device == 0 || *device == '\0') {
		midi_in_close();
		return TCL_OK;
	}
	if (isdigit((unsigned char) *device))
		dev = *device - '0';
	else
		dev = 0;
	if (midiInOpen(&ih, (UINT) dev, (DWORD) kbd_event,
			0, CALLBACK_FUNCTION) != 0) {
		trace("cannot open MIDI in '%s'\n", device);
		ih = 0;
		return TCL_ERROR;
	}
	if (midiInStart(ih) != 0) {
		trace("cannot start MIDI in '%s'\n", device);
		midi_in_close();
		return TCL_ERROR;
	}
#endif
	return TCL_OK;
}

/* -- output NOTE_ON or NOTE_OFF to the sequencer -- */
static void seq_note(int ch, int fr, int vel)
{
#ifdef HAVE_ALSA
/*TRACE(("seq note c:%d f:%d v:%d\n", ch, fr, vel));*/
	if (my_out_port >= 0) {
		if (vel != 0)
			snd_seq_ev_set_noteon(&ev, ch, fr, vel);
		else
			snd_seq_ev_set_noteoff(&ev, ch, fr, vel);
		alsa_send();
		return;
	}
#endif
	if (seq_dev < 0) {
		if (seq_fd >= 0) {		/* if raw midi */
			if (vel != 0)
				ch += 0x90;	/* note on */
			else
				ch += 0x80;	/* note off */
			_seqbuf[0] = ch;
			_seqbuf[1] = fr;
			_seqbuf[2] = vel;
			write(seq_fd, _seqbuf, 3);
		}
		return;
	}
	if (vel != 0) {
		SEQ_START_NOTE(seq_dev, ch, fr, vel);
	} else {
		SEQ_STOP_NOTE(seq_dev, ch, fr, vel);
	}
}

/* flush the MIDI events */
static void seq_flush(void)
{
#ifdef HAVE_ALSA
	if (my_out_port >= 0)
		snd_seq_drain_output(seq_alsa_handle);
	else
#endif
	if (seq_dev >= 0)
		SEQ_DUMPBUF();
}

static void time_signal(int sig);
#else /*WIN32*/
static HMIDIOUT oh;
static void seq_note(int ch, int fr, int vel)
{
	if (oh == 0)
		return;
	if (vel != 0)
		midiOutShortMsg(oh, (vel << 16) + (fr << 8) + 0x90 + ch);
	else
		midiOutShortMsg(oh, (fr << 8) + 0x80 + ch);
}
#define seq_flush()
#endif /*WIN32*/

/* -- mute all notes -- */
static void sound_off(int ch)
{
#ifndef WIN32
#ifdef HAVE_ALSA
	if (my_out_port >= 0) {
		snd_seq_ev_set_controller(&ev, ch, 120, 0);
		alsa_send();
		seq_flush();
		return;
	}
#endif /*HAVE_ALSA*/
	if (seq_dev < 0) {
		if (seq_fd >= 0) {		/* if raw midi */
			_seqbuf[0] = 0xb0 + ch;	/* control change */
			_seqbuf[1] = 120;
			write(seq_fd, _seqbuf, 2);
		}
		return;
	}
#ifdef HAVE_AWE
	if (is_awe) {
		 AWE_RELEASE_ALL(seq_dev)
	} else
#endif
	SEQ_CONTROL(seq_dev, ch, 120, 0);
	seq_flush();
#else /*WIN32*/
	if (oh == 0)
		return;
	midiOutShortMsg(oh, (120 << 8) + 0xb0 + ch);
#endif /*WIN32*/
}

/* -- close the MIDI output -- */
static void midi_out_close(void)
{
#if 1
	sound_off(0);
#else
	int ch;

	for (ch = 0; ch < MAX_MIDI_CHANNELS; ch++)
		sound_off(ch);
#endif

#ifndef WIN32
#ifdef HAVE_ALSA
	if (my_out_port >= 0) {
		snd_seq_delete_simple_port(seq_alsa_handle, my_out_port);
		my_out_port = -1;
		return;
	}
#endif /*HAVE_ALSA*/
	if (seq_fd >= 0) {
		close(seq_fd);
		seq_fd = -1;
	}
	seq_dev = -1;
#else /*WIN32*/
	if (oh != 0) {
		midiOutClose(oh);
		oh = 0;
	}
#endif /*WIN32*/
}

#ifdef HAVE_ALSA
/* -- connect to the MIDI out port -- */
static int alsa_out_init(char *device)
{
	int port, rclient, rport;

	if (sscanf(device, "%d:%d", &rclient, &rport) != 2)
		return TCL_ERROR;
	if (alsa_init() != TCL_OK)
		return TCL_ERROR;
	if ((port = snd_seq_create_simple_port(seq_alsa_handle,
			"tclabc out",
		    SND_SEQ_PORT_CAP_SUBS_READ
			| SND_SEQ_PORT_CAP_READ,
		    SND_SEQ_PORT_TYPE_APPLICATION)) < 0) {
		trace("can't create my ALSA out port\n");
		return TCL_ERROR;
	}
	if (snd_seq_connect_to(seq_alsa_handle, port,
				rclient, rport) < 0) {
		trace("cannot connect to ALSA out client\n");
		return TCL_ERROR;
	}
	midi_out_close();
	my_out_port = port;
	snd_seq_ev_set_source(&ev, my_out_port);
	snd_seq_ev_set_subs(&ev);
	snd_seq_ev_set_direct(&ev);
	return TCL_OK;
}
#endif /*HAVE_ALSA*/

/* - open the midi output device - */
int midi_out_init(char *device)
{
#ifndef WIN32
	int fd, dev, nrsynths;
	char *p;
    static int signal_done = 0;
#ifdef HAVE_AWE
	struct synth_info card_info;
#endif

	if (!signal_done) {
		struct sigaction sigact;

		sigemptyset(&sigact.sa_mask);
		sigact.sa_handler = time_signal;
		sigact.sa_flags = 0;
		if (sigaction(SIGALRM, &sigact, NULL) != 0)
			perror("sigaction");
		signal_done = 1;
	}

	if (device == 0 || *device == '\0') {
		midi_out_close();
		return TCL_OK;
	}
#ifdef HAVE_ALSA
	/* ALSA output is '<client#>:<port#>' */
	if (isdigit((unsigned char) *device))
		return alsa_out_init(device);
#endif
	/* OSS output is 'MIDI device' or '<sequencer device>[:<client>]' */
	p = strchr(device, ':');
	if (p != 0) {
		if (isdigit((unsigned char) p[1]))
			*p = '\0';
		else
			p = 0;
	}
	fd = open(device, O_WRONLY, 0);
	if (p != 0)
		*p = ':';
	if (fd < 0) {
		perror("open");
		trace("cannot open MIDI out '%s'\n", device);
		return TCL_ERROR;
	}
	if (strstr(device, "seq") != 0) {
		if (ioctl(fd, SNDCTL_SEQ_NRSYNTHS, &nrsynths) == -1
		    || nrsynths == 0) {
			trace("no output MIDI synth\n");
			close(fd);
			return TCL_ERROR;
		}
#ifdef HAVE_AWE
		card_info.synth_type = -1;
#endif
		if (p != 0) {
			dev = atoi(p + 1);
			if (dev >= nrsynths) {
				trace("invalid MIDI out device '%s'\n", device);
				return TCL_ERROR;
			}
		} else {
#ifdef HAVE_AWE
			int i;

			dev = -1;
			for (i = 0; i < nrsynths; i++) {
				card_info.device = i;
				if (ioctl(fd, SNDCTL_SYNTH_INFO, &card_info) == -1) {
					trace("cannot get MIDI soundcard info\n");
					close(fd);
					return TCL_ERROR;
				}
				if (card_info.synth_type == SYNTH_TYPE_SAMPLE
				    && card_info.synth_subtype == SAMPLE_TYPE_AWE32) {
					dev = i;
					break;
				}
			}
			if (dev < 0)
				dev = 0;
#else
			dev = 0;
#endif
		}
#ifdef HAVE_AWE
		if (card_info.synth_type < 0) {
			card_info.device = dev;
			if (ioctl(fd, SNDCTL_SYNTH_INFO, &card_info) == -1) {
				trace("cannot get MIDI soundcard info\n");
				close(fd);
				return TCL_ERROR;
			}
		}
		if (card_info.synth_type == SYNTH_TYPE_SAMPLE
			    && card_info.synth_subtype == SAMPLE_TYPE_AWE32)
			is_awe = 1;
		else
			is_awe = 0;
#endif
	} else {
		dev = -1;
#ifdef HAVE_AWE
		is_awe = 0;
#endif
	}
	midi_out_close();
	seq_fd = fd;
	seq_dev = dev;

#ifdef HAVE_AWE
	if (is_awe) {
		/* use MIDI channel mode */
		AWE_SET_CHANNEL_MODE(seq_dev, AWE_PLAY_MULTI);
		/* toggle drum flag if bank #128 is received */
		/* AWE_MISC_MODE(seq_dev, AWE_MD_TOGGLE_DRUM_BANK, TRUE);
		 */
/*		_AWE_CMD_NOW(seq_fd, seq_dev, 0, _AWE_REVERB_MODE, 2, 0) */
	}
#endif

	set_program(metronome.channel, metronome.program, metronome.bank);

#if defined(linux) && defined(WITH_RT_PRIO)
	/* set the process to realtime privs */
	{
		struct sched_param schp;

		memset(&schp, 0, sizeof(schp));
		schp.sched_priority = sched_get_priority_max(SCHED_FIFO);

		/* (only root can do this) */
		if (sched_setscheduler(0, SCHED_FIFO, &schp) != 0)
			perror("sched_setscheduler");
	}
#endif
#else /*WIN32*/
	int dev;

	if (device == 0 || *device == '\0') {
		midi_out_close();
		return TCL_OK;
	}
	if (isdigit((unsigned char) *device))
		dev = *device - '0';
	else
		dev = -1;		/* midi mapper */
	if (midiOutOpen(&oh, (UINT) dev, 0, 0, CALLBACK_WINDOW) != 0) {
		trace("cannot open MIDI out '%s'\n", device);
		oh = 0;
		return TCL_ERROR;
	}
#endif
	return TCL_OK;
}

/* -- set a program -- */
void set_program(int ch, int seq_pre, int seq_bank)
{
	if (ch == 9)		/*fixme: don't change the percussions??*/
		return;
#ifndef WIN32
#ifdef HAVE_ALSA
	if (my_out_port >= 0) {
		snd_seq_ev_set_controller(&ev, ch, MIDI_CTL_MSB_BANK, seq_bank);
		alsa_send();
		snd_seq_ev_set_pgmchange(&ev, ch, seq_pre);
		alsa_send();
		snd_seq_ev_set_pitchbend(&ev, ch, seq_bend);
		alsa_send();
		seq_flush();
		return;
	}
#endif /*HAVE_ALSA*/
	if (seq_dev < 0) {
		if (seq_fd >= 0) {		/* if raw midi */
			_seqbuf[0] = 0xc0 + ch;	/* program change */
			_seqbuf[1] = seq_pre;
			write(seq_fd, _seqbuf, 2);
		}
		return;
	}
	SEQ_CONTROL(seq_dev, ch, CTL_BANK_SELECT, seq_bank);
	SEQ_SET_PATCH(seq_dev, ch, seq_pre);
	SEQ_BENDER(seq_dev, ch, seq_bend);
#else /*WIN32*/
	if (oh == 0)
		return;
	TRACE(("set_prog ch:%d pr:%d\n", ch, seq_pre));
	midiOutShortMsg(oh, (seq_pre << 8) + 0xc0 + ch);
#endif /*WIN32*/
}

/* -- start stop a note -- */
static void midi_play_evt(MidiEventList *meep)
{
	int voice, channel, vel;
	unsigned channels;

	voice = meep->event.channel;
	channels = vtb[voice].channels;
	if (meep->event.type == ME_NOTEON
	    && meep->event.b != 0)
		vel = dyn ? meep->event.b : volume;
	else
		vel = 0;

	for (channel = 0;
	     channel < MAX_MIDI_CHANNELS && channels != 0;
	     channel++, channels >>= 1) {
		if (channels & 1) {
			seq_note(channel,
				 meep->event.a,	/* freq */
				 vel);		/* vel */
		}
	}
	seq_flush();
}

/* -- treat a MIDI event from the MIDI keyboard -- */
static void kbd_treat_event(MidiEventList *meep)
{
	switch (meep->event.type) {
	case ME_NOTEON:
	case ME_NOTEOFF:
		midi_play_evt(meep);
		break;
	case ME_PROGRAM:
		set_program(meep->event.channel,
			    meep->event.a, 0);
		break;
#if 0
	case ME_REVERB:
		SEQ_CONTROL(seq_dev, meep->event.channel,
			    CTL_EXT_EFF_DEPTH, meep->event.a);
		break;
#endif
	}

	/* if some midi handler, call it */
	if (midi_handler != 0) {
		switch (meep->event.type) {
		case ME_NOTEON:
		case ME_NOTEOFF:
			note_insert(meep, 0);
			break;
		default:
			free(meep);
			break;
		}
		return;
	}

	if (midi_recording) {

		/* if not playing and 1st event, restart the starting time */
		if (!playing && midi_header.last_event == midi_header.evlist) {
			int oldtime;

			oldtime = gettime() * play_factor / 6000 /*+ basetime*/;
			gettimeofday(&start_time, 0);
			metronome.time += gettime() * play_factor / 6000 /*+ basetime*/
				- oldtime;
		}
		midi_header.last_event->next = meep;
		midi_header.last_event = meep;
		meep->event.time = gettime() * play_factor / 6000 /*+ basetime*/;
	} else {
		free(meep);
	}
}

/* -- start/stop MIDI recording -- */
int midi_record(void)
{
#ifndef WIN32
#ifdef HAVE_ALSA
	if (my_in_port < 0)
#endif
	    if (midi_fd < 0)
#else /*WIN32*/
	if (ih == 0)
#endif /*WIN32*/
		return 0;
	midi_recording = !midi_recording;
	if (!midi_recording) {
		if (metronome.started)
			metronome.started = 2;
		return 2;			/* record done */
	}

	/* Initialize the time */
	free_midi_list();
	midi_header.divisions = BASE_LEN / 4;
	if (!playing) {
		struct sym *s;

		play_factor = BASE_LEN / 4 * tempo_default;	/* number of crotchets per mn */
		gettimeofday(&start_time, 0);

		/* start the metronome */
		for (s = curvoice->cursym; s->type != EOT; s = s->prev)
			if (s->type == TIMESIG)
				break;
		if (s->type == TIMESIG)
			metronome.beat = beat_get(s);
		else
			metronome.beat = BASE_LEN / 4;
		metronome.time = gettime() * play_factor / 6000 /*+ basetime*/;
		metronome.started = 1;
		metronome_time();
	}

	/* set the smallest note duration */
	if (tempo_default > 100) {
		if (tempo_default > 180)
			mindur = BASE_LEN / 4;	/* crotchet */
		else
			mindur = BASE_LEN / 8;	/* quaver */
	} else if (tempo_default < 50) {
		mindur = BASE_LEN / 32;		/* demisemiquaver */
	} else {
		mindur = BASE_LEN / 16;		/* semiquaver */
	}

	/* Put a do-nothing event first in the list for easier processing */
	midi_header.evlist = (MidiEventList *) malloc(sizeof(MidiEventList));
	memset(midi_header.evlist, 0, sizeof(MidiEventList));
	midi_header.last_event = midi_header.evlist;
	return midi_recording;
}

#ifndef WIN32
#ifdef HAVE_ALSA
/* -- event coming from the MIDI input -- */
static void kbd_alsa_event(ClientData client,
			int mask)
{
	MidiEventList *meep;
	snd_seq_event_t *ev;

	do {
		snd_seq_event_input(seq_alsa_handle, &ev);
		meep = 0;
		switch (ev->type) {
		case SND_SEQ_EVENT_NOTEON:
			meep = malloc(sizeof(MidiEventList));
			meep->event.type = ME_NOTEON;
			meep->event.channel = ev->data.note.channel;
			meep->event.a = ev->data.note.note;
			meep->event.b = ev->data.note.velocity;
			break;
		case SND_SEQ_EVENT_NOTEOFF:
			meep = malloc(sizeof(MidiEventList));
			meep->event.type = ME_NOTEOFF;
			meep->event.channel = ev->data.note.channel;
			meep->event.a = ev->data.note.note;
			meep->event.b = 0;;
			break;
		case SND_SEQ_EVENT_PGMCHANGE:
			meep = malloc(sizeof(MidiEventList));
			meep->event.type = ME_PROGRAM;
			meep->event.channel = ev->data.control.channel;
			meep->event.a = ev->data.control.value;
			meep->event.b = 0;
			break;
		default:
			continue;
		}
		meep->event.misc = 0;
		meep->next = 0;
		snd_seq_free_event(ev);
		kbd_treat_event(meep);
	} while (snd_seq_event_input_pending(seq_alsa_handle, 0) > 0);
}
#endif /*HAVE_ALSA*/

static void kbd_oss_event(ClientData client,
			int mask)
{
#if 1 /*notest*/
	MidiEventList *meep;
	unsigned char me;

#ifdef AWE_PB
	extern_input = 1;
#endif
	if (READ(midi_fd, &me, 1) != 1) {
		trace("error in reading midi port\n");
		return;
	}
	meep = def_event(me, midi_fd);
#ifdef AWE_PB
	extern_input = 0;
#endif
	if (meep == 0)
		return;
	kbd_treat_event(meep);
#else /*test*/
	unsigned char buff[128];
	int i, j;

	for (i = 0; i < sizeof buff; i++) {
		fd_set rmask;
		struct timeval tv;

		FD_ZERO(&rmask);
		FD_SET(midi_fd, &rmask);
		tv.tv_sec = 0;
		tv.tv_usec = 0;
		if (select(midi_fd + 1,
			   &rmask, 0, 0,
			   &tv) <= 0)
			break;
		if (read(midi_fd, &buff[i], 1) <= 0)
			break;
	}
	trace("midievent %d bytes\n", i);
	for (j = 0; j < i; j++)
		trace(" %02x", buff[j]);
	trace("\n");
#endif
}
#else /*WIN32*/
static void CALLBACK kbd_event(HMIDIIN handle,
			UINT uMsg,
			DWORD dwInstance,
			DWORD dwParam1,
			DWORD dwParam2)
{
	MidiEventList *meep;
	int ch, a, b;

	if (uMsg != MIM_DATA)
		return;
	meep = 0;
	ch = dwParam1 & 0x0f;
	a = (dwParam1 >> 8) & 0xff;
	b = (dwParam1 >> 16) & 0xff;
	switch ((dwParam1 >> 4) & 0x07) {	/* status */
	case 0:
		meep = malloc(sizeof(MidiEventList));
		meep->event.type = ME_NOTEOFF;
		meep->event.a = a;
		meep->event.b = b & 0x7f;
		break;
	case 1:
		meep = malloc(sizeof(MidiEventList));
		meep->event.type = ME_NOTEON;
		meep->event.a = a;
		meep->event.b = b;
		break;
	case 4:
		meep = malloc(sizeof(MidiEventList));
		meep->event.type = ME_PROGRAM;
		meep->event.a = a & 0x7f;
		meep->event.b = 0;
		break;
	}
	if (meep != 0) {
		meep->event.channel = ch;
		meep->event.misc = 0;
		meep->next = 0;
		kbd_treat_event(meep);
	}
}
#endif /*WIN32*/

/* -- play the metronome -- */
static void metronome_time(void)
{
	int curtime, sleeptime;

	if (metronome.started == 2) {
		if (metronome.active) {
			seq_note(metronome.channel,	/* channel */
				 metronome.freq,	/* frequency */
				 0);			/* velocity */
			metronome.active = 0;
		}
		metronome.started = 0;
		return;
	}
	curtime = gettime() * play_factor / 6000 /*+ basetime*/;
	metronome.active = !metronome.active;
	if (metronome.active) {
#if 0
		play_tb[play_nb].channels = 1 << metronome.channel;
		play_tb[play_nb].freq = metronome.freq;
		play_tb[play_nb].time = curtime + metronome.duration;
		play_nb++;
#endif
		seq_note(metronome.channel,	/* channel */
			 metronome.freq,	/* frequency */
			 volume);		/* velocity */
		sleeptime = metronome.time + metronome.duration - curtime;
		metronome.time += metronome.beat;
	} else {
		seq_note(metronome.channel,	/* channel */
			 metronome.freq,	/* frequency */
			 0);			/* velocity */
		sleeptime = metronome.time - curtime;
	}
	seq_flush();
	set_timer(sleeptime * 6000 / play_factor);
}

/* -- initialize playing all notes -- */
static void play_init(int goaltime)
{
	int v, p_fact;
	struct sym *s;

	basetime = midi_init(goaltime);

	/* set the tempo */
	p_fact = BASE_LEN / 4 * tempo_default;	/* number of crotchets per mn */
	for (s = vtb[0].s; s->type != EOT; s = s->prev) {
		if (s->type == TEMPO && s->as.u.tempo.value != 0) {
			v = strtol(s->as.u.tempo.value, 0, 10);
			if (v < 8 || v > 200)
				break;
			p_fact = s->as.u.tempo.length[0] * v;
			break;
		}
	}
	play_factor_new = play_factor = p_fact;	/* remember the play factor */

	/* get the starting time */
	gettimeofday(&start_time, 0);

	playing = 1;
}

/* -- play the current note -- */
void play_note(struct sym *s)
{
	struct vtb_s *vp;

#ifndef WIN32
#ifdef HAVE_ALSA
	if (my_out_port < 0)
#endif
	    if (seq_fd < 0)
#else
	if (oh == 0)
#endif
		return;
	if (playing || play_nb != 0)
		play_stop();
	if (s->type != NOTE)
		return;

	/* set the channels, programs and synchronize the accidentals */
	vp = &vtb[s->voice];
	vp->s = 0;
	channel_def(vp, s->time);

	note_start(s, vp);
	seq_flush();

	basetime = 0;
	set_timer(50);		/* 50 * 1/100s = 0.5s */
}

/* -- stop playing -- */
void play_stop(void)
{
	if (!playing)
		return;
	playing = 0;
	set_timer(0);
	play_time();
}

/* -- return one of the currently played symbols -- */
struct sym *play_sym(void)
{
	int i, curtime, mintime, voice;

	if (!playing)
		return 0;
	voice = 0;
	curtime = gettime() * play_factor / 6000 + basetime;
	mintime = curtime + 1000000;
	for (i = nvoice; i >= 0; i--) {
		if (vtb[i].s->time < curtime)
			continue;
		if (vtb[i].s->time < mintime) {
			mintime = vtb[i].s->time;
			voice = i;
		}
	}
	return vtb[voice].s;
}

/* -- do the playing job at the current time -- */
/* this routine is called from the main thread
 * and from the SIGALRM interrupt handler */
static void play_time(void)
{
	int i, curtime, sleeptime, mintime, v, event_to_come;
	struct play_s *pl;
	struct vtb_s *vp;
	struct sym *s;
static int busy;

	/* metronome stuff */
	if (metronome.started) {
		metronome_time();
		return;
	}
	if (busy) {		/* minimal protection */
		/*fixme:debug*/
		trace("busy\n");
		return;
	}
	busy = 1;

	curtime = gettime() * play_factor / 6000 + basetime;
	if (!playing)
		curtime += 1000000;	/* stop playing */

	/* stop the notes at the current time */
	notes_stop(curtime);

	/* stop playing */
	if (!playing) {
#if 1 /*fixme:debug*/
		if (play_nb != 0) {
			trace("%d notes are still playing\n", play_nb);
			play_nb = 0;
		}
#endif
		seq_flush();
		busy = 0;
		return;
	}

	/* start the next notes */
	mintime = EOT_TIME + 1;
	event_to_come = 0;
	for (v = 0, vp = &vtb[0]; v <= nvoice; v++, vp++) {
		s = vp->s;
		if (s->type == EOT)
			continue;
		if (s->time <= curtime
		    && s->time < play_stop_time
		    && play_advance(vp)) {
			event_to_come = 1;
			s = vp->s;
			if (s->time < mintime)
				mintime = s->time;
			if (s->time > curtime)
				continue;
			if (s->type == NOTE) {
				note_start(s, vp);
				vp->s = s = s->next;
			}
		}
		if (s->time < play_stop_time) {
			event_to_come = 1;
			if (s->time < mintime)
				mintime = s->time;
		}
	}

	/* if no more event, treat the repeat sequences or stop */
	if (play_nb == 0
	    && !event_to_come) {
		switch (repeat_flag) {
		case REPEAT_NONE:
		case REPEAT_RUN:
			playing = 0;
			seq_flush();
			busy = 0;
			return;			/* all done */
		case REPEAT_FOUND:
			repeat_back();	/* rewind to the previous left repeat */
			break;
		default: /* REPEAT_DONE */
			repeat_forw();
#if 0
			s = repeat_sym->next;
			if (s->type == EOT) {
				seq_flush();
				busy = 0;
				return;
			}
#endif
			break;
		}
		curtime -= play_stop_time;
		play_init(vtb[0].s->time);	/* restart playing */
		basetime += curtime;		/* (for rests) */
		busy = 0;
		play_time();
		return;
	}
	seq_flush();

	/* if the tempo has changed, reinitialize */
	if (play_factor_new != play_factor) {
		basetime = curtime;
		play_factor = play_factor_new;
		gettimeofday(&start_time, 0);
		curtime = gettime() * play_factor / 6000 + basetime;
	}

	/* restart at the shortest next time */
	for (i = play_nb, pl = play_tb; --i >= 0; pl++) {
		if (pl->time < mintime)
			mintime = pl->time;
	}

	sleeptime = (mintime - curtime) * 6000 / play_factor;
	if (sleeptime <= 0)	/* short time (may occur between notes) */
		sleeptime = 1;
	set_timer(sleeptime + 1);
	busy = 0;
}

/* -- play the tune, starting from symbol 's' -- */
/*fixme:may stop on some other symbol*/
void play_tune(struct sym *s)
{
#ifndef WIN32
#ifdef HAVE_ALSA
	if (my_out_port < 0)
#endif
	    if (seq_fd < 0)
#else /*WIN32*/
#ifdef DEBUG
	trace("play_tune\n");

	if (thrid != 0)
		trace("thrd not stopped\n");
	trace("ms_sleep: %d\n", ms_sleep);
#endif
	if (oh == 0)
#endif /*WIN32*/
		return;
	if (playing)
		play_stop();		/* stop previous playing */
	memset(vtb, 0, sizeof vtb);
	play_init(s->time);
	lrepeat_time = s->time;
	repeat_flag = REPEAT_NONE;
	play_time();
}

/* -- set the channels of a voice -- */
void set_channels(int v, unsigned channels)
{
#if 1
	sound_off(0);
#else
	int ch;
	unsigned chs;

	/* stop the notes on freed channels */
	chs = vtb[v].channels & (vtb[v].channels ^ channels);
	for (ch = 0; ch < MAX_MIDI_CHANNELS; ch++) {
		if (chs & 1)
			sound_off(ch);
		chs >>= 1;
	}
#endif
	vtb[v].channels = channels;
}

/* -- set the default tempo -- */
void tempo_set(int new_tempo)
{
	tempo_default = new_tempo;
	play_factor_new = BASE_LEN / 4 * tempo_default;
}

/* -- set the velocity -- */
int velocity_set(int velocity)
{
	if (velocity < 0)
		return dyn ? 0 : volume;
	if (velocity == 0) {
		volume = 80;
		dyn = 1;
	} else {
		volume = velocity;
		dyn = 0;
	}
	return velocity;
}

#ifndef WIN32
/* -- set the SIGALARM timer -- */
static void set_timer(int delay)	/* unit = 1/100s */
{
	struct itimerval it;

	it.it_value.tv_sec = delay / 100;
	it.it_value.tv_usec = (delay % 100) * 10000;
	it.it_interval.tv_sec = it.it_interval.tv_usec = 0;
	setitimer(ITIMER_REAL, &it, NULL);
}

/* -- time handler -- */
static void time_signal(int sig)
{
	play_time();
}
#else /*WIN32*/
static Tcl_ThreadId thrid;
static int ms_sleep;

/* -- thread to play the MIDI notes -- */
static void thread_play(int client_data)
{
	int t;

TRACE(("play thread started\n"));
	for (;;) {
		if ((t = ms_sleep) == 0)
			break;
		ms_sleep = -1;
		Tcl_Sleep(t);
/*TRACE(("sleep end\n"));*/
		if (ms_sleep == 0)	/* playing stopped */
			break;
		if (ms_sleep > 0)	/* playing restarted */
			continue;
		ms_sleep = 0;
		play_time();
	}
TRACE(("play thread stopped\n"));
	thrid = 0;
}

static void set_timer(int delay)
{
	TRACE(("set_timer %d0 ms\n", delay));
	ms_sleep = delay * 10;

	if (delay == 0)
		return;

	/* start the playing thread if not done */
	if (thrid != 0)
		return;
#if 0
	if (Tcl_CreateThread(&thrid, thread_play, 0,
			     TCL_THREAD_STACK_DEFAULT, TCL_THREAD_NOFLAGS)
			== TCL_OK)
#else
	{
		HANDLE tHandle;

		tHandle = CreateThread(NULL, (DWORD) TCL_THREAD_STACK_DEFAULT,
					(LPTHREAD_START_ROUTINE) thread_play,
					(LPVOID) 0,
					(DWORD) 0, (LPDWORD) &thrid);
		if (tHandle != NULL)
			CloseHandle(tHandle);
		else
			thrid = 0;
	}
	if (thrid != 0)
#endif
		TRACE(("creat thrd OK thrid: %d\n", thrid));
	else {
		TRACE(("creat thrd KO\n"));
		Tcl_Sleep(20);
		play_stop();
	}
}
#endif /*WIN32*/
#endif /*HAVE_SOUND*/

#define WRITELONG(v) \
	c[0] = (v) >> 24; \
	c[1] = (v) >> 16; \
	c[2] = (v) >> 8; \
	c[3] = v; \
	WRITE(out_fd, c, 4);

#define WRITESHORT(v) \
	c[0] = (v) >> 8; \
	c[1] = v; \
	WRITE(out_fd, c, 2);

/* -- dump the current tune on disk as a MIDI file -- */
int midi_dump(Tcl_Interp *interp,
	      char *fname)
{
	struct vtb_s *vp;
	struct sym *s;
	struct play_s *pl;
	int i, lenoffs, len, voice, curtim;
	unsigned char c[64];

#ifndef WIN32
	if ((out_fd = creat(fname, 0644)) < 0) {
		sprintf(interp->result, "can't create %s", fname);
		return TCL_ERROR;
	}
#else /*WIN32*/
	if ((out_fd = Tcl_OpenFileChannel(interp, fname, "w", 0644)) == 0)
		return TCL_ERROR;
	if (Tcl_SetChannelOption(interp, out_fd, "-encoding", "binary") != TCL_OK)
		return TCL_ERROR;
#endif

	/* header */
	WRITE(out_fd, "MThd", 4);
	WRITELONG(6);			/* header length */
	WRITESHORT(0);			/* format 0 */
	WRITESHORT(1);			/* number of tracks */
	dump_time = BASE_LEN / 4;	/* (to avoid a warning) */
	WRITESHORT(dump_time);		/* divisions */

	dump = 1;
	WRITE(out_fd, "MTrk", 4);
	lenoffs = SEEK(out_fd, 0, SEEK_CUR);
	WRITELONG(0);

	/* track name */
	if (0) {
		char *p;

		putvl(out_fd, 0);		/* time */
		c[0] = 0xff;			/* meta-event */
		c[1] = 3;			/* track name */
		if ((s = voice_tb[0].p_voice) == 0) {
			p = (char *) &c[3];
			sprintf(p, "Track %d", 1);
		} else {
			if ((p = s->as.u.voice.fname) == 0)
				if ((p = s->as.u.voice.nname) == 0)
					p = s->as.u.voice.id;
		}
		c[2] = strlen(p);
		WRITE(out_fd, c, 3);
		WRITE(out_fd, p, c[2]);
	}

	/* time loop */
//fixme: if no tempo at start time, add it
	dump_status = -1;
	dump_time = 0;
	memset(vtb, 0, sizeof vtb);
	repeat_flag = REPEAT_NONE;
	midi_init(0);
	lrepeat_time = 0;

	curtim = 0;
	for (;;) {
		int event_to_come;

		/* get the next time */
		curtim = EOT_TIME;
		for (voice = 0; voice <= nvoice; voice++) {
			s = vtb[voice].s;
			if (s->type != EOT
			 && s->time < curtim)
				curtim = s->time;
		}

		/* stop the notes */
		for (;;) {
			int mintim;

			mintim = EOT_TIME;
			for (i = play_nb, pl = play_tb; --i >= 0; pl++) {
				if (pl->time < mintim)
					mintim = pl->time;
			}
			if (mintim > curtim)
				break;
			notes_stop(mintim);
			if (mintim == curtim)
				break;
		}

		if (curtim == EOT_TIME)
			break;

		/* advance in time */
		event_to_come = 0;
		for (voice = 0; voice <= nvoice; voice++) {
			vp = &vtb[voice];
			s = vp->s;
			if (s->type == EOT
			 || s->time > curtim)
				continue;
			if (s->type != NOTE) {
				if (play_advance(vp))
					event_to_come = 1;
				s = vp->s;
				if (s->time != curtim
				 || s->type != NOTE)
					continue;
			}
			if (s->time < play_stop_time) {
				note_start(s, vp);
				vp->s = s = s->next;
			}
		}

		/* if no more event, treat the repeat sequences or stop */
		if (play_nb == 0
		 && !event_to_come
		 && (repeat_flag == REPEAT_FOUND
		  || repeat_flag == REPEAT_DONE)) {
			switch (repeat_flag) {
			case REPEAT_FOUND:
				repeat_back();	/* rewind to a previous complex bar */
				break;
			default: /* REPEAT_DONE */
				repeat_forw();
				break;
			}
			s = vtb[0].s;
			if (s->type == EOT)
				break;
			dump_time = s->time;
			midi_init(dump_time);		/* restart playing */
		}
	}
	voice = 0;
	put_dtime(voice_tb[voice].eot->time);
	c[0] = 0xff;		/* meta-event */
	c[1] = 0x2f;		/* EOT */
	c[2] = 0;		/* length */
	WRITE(out_fd, c, 3);

	len = SEEK(out_fd, 0, SEEK_CUR) - lenoffs - 4;
	SEEK(out_fd, lenoffs, SEEK_SET);
	WRITELONG(len);
	SEEK(out_fd, 0, SEEK_END);

	CLOSE(out_fd);
	dump = 0;
	return TCL_OK;
}
