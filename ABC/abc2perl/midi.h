#ifndef MIDI_H_DEF
#define MIDI_H_DEF

/* prototypes */
int midi_cmd(Tcl_Interp *interp,
	     int objc,
	     Tcl_Obj *CONST objv[]);
int midi_dump(Tcl_Interp *interp, char *fname);
void midi_load(struct abctune *t);
int midi_read_file(Tcl_Interp *interp, char *fname);
void setmap(int sf, char *map);
#ifdef HAVE_SOUND
int midi_in_init(char *device);
int midi_out_init(char *device);
int midi_record(void);
void play_note(struct sym *s);
void play_stop(void);
struct sym *play_sym(void);
void play_tune(struct sym *s);
void set_channels(int v, unsigned channels);
void set_program(int ch, int seq_pre, int seq_bank);
void tempo_set(int new_tempo);
int velocity_set(int velocity);
#endif
#endif
