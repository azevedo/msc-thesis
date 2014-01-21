#ifndef _ABCP_H_
#define _ABCP_H_
#ifndef _RE_H_
#define _RE_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
 
  char *re_comp  (unsigned char *pat, unsigned char *nfa);
  int   re_exec  (unsigned char *lp,  unsigned char *nfa);
  char *re_end   (unsigned char n);
  char *re_start (unsigned char n);
  int   re_len   (unsigned char n);
  char *re_cpy   (unsigned char *dst,unsigned char n);
  char *re_ncpy  (unsigned char *dst,unsigned char n,int l);
  int   re_dump  (FILE *f,unsigned char *s,unsigned char h);

  
#define RE_MAXNFA  1024

#endif /* _RE_H_*/
/*  YRX - yrx.h
**  (C) 2005 by Remo Dentato
**
** Permission to use, copy, modify and distribute this code and
** its documentation for any purpose is hereby granted without fee,
** provided that the above copyright notice appear in all copies
** and supporting documentation.
** 
** Copyright holder makes no representations about the suitability
** of this software for any purpose. It is provided "as is" without
** express or implied warranty.
** 
*/

#ifndef  _YRX_H_
#define  _YRX_H_

  typedef struct {
    unsigned char *cursor;
    FILE          *file;
    unsigned char *sptr;
    unsigned int   size;
    unsigned int   linenum;
    unsigned char  buffer[0];
  } YYSTREAM;


  YYSTREAM  *YYFILE         (char *filename, int size);
  YYSTREAM  *YYSTRING       (unsigned char *string);
  YYSTREAM  *YYCLOSE        (YYSTREAM *y);
                            
            
  #define    YYLINE(_y)     ((_y)->linenum)
  #define    YYCURSOR(_y)   ((_y)->cursor)
             
  #define    YYSOL '\002'
  #define    YYEOL '\003'
  
  #ifndef _RE_H_
  #include "re.h"
  #endif
   
  #define YYSTART             re_start
  #define YYEND               re_end
  #define yydump              re_dump
  #define YYSTRCPY            re_cpy
  #define YYSTRNCPY           re_ncpy
  #define YYLEN               re_len
  #define YYFPRINTN(_f,_s,_n) fwrite(_s,_n,sizeof(unsigned char),_f)
  #define YYFPRINT(_f,_s)     fputs(_s,_f)
  
  int YYPOS(YYSTREAM *y);
  int yyswitch(unsigned char **r,YYSTREAM *y);
  int yygetline(YYSTREAM *y);
  const char *YYFILENAME(YYSTREAM *y);
  unsigned int YYLINENUMBER(YYSTREAM *y);
  
  #define YYSTATE(x) s_##x:                 
  #define YYGOTO(x) goto s_##x
  
  #define _Y_(a,b) 
    
#endif /* _YRX_H_*/

#ifndef _ABC_STRINGS_H
#define _ABC_STRINGS_H

#define ABC_STR_INFO
#define ABC_STR_PRINT
#define ABC_STR_DECORATIONS
#define ABC_STR_TAB
#define ABC_STR_MIDI
#define ABC_STR_INSTRUMENT
#define ABC_STR_TEMPO
#define ABC_MAXLEN_STR  22

#ifdef ABC_STRINGS_C
#define ABC_BUFSIZE 6445
#define ABC_NUMSTRS 680

static unsigned char *strbuf = {
  "##turn\0"\
  "#turn\0"\
  "%alto\0"\
  "%banjo4tab\0"\
  "%banjo5tab\0"\
  "%baritone\0"\
  "%bass\0"\
  "%brace\0"\
  "%bracket\0"\
  "%brc\0"\
  "%brk\0"\
  "%clef\0"\
  "%diamond\0"\
  "%doh\0"\
  "%drum\0"\
  "%dyn\0"\
  "%fa\0"\
  "%french\0"\
  "%french4tab\0"\
  "%french5tab\0"\
  "%frenchtab\0"\
  "%frenchviolin\0"\
  "%gstem\0"\
  "%guitartab\0"\
  "%head\0"\
  "%head-diamond\0"\
  "%head-plus\0"\
  "%head-square\0"\
  "%head-triangle\0"\
  "%head-wdiamond\0"\
  "%head-wsquare\0"\
  "%head-wtriangle\0"\
  "%head-x\0"\
  "%head-xcircle\0"\
  "%i\0"\
  "%instr\0"\
  "%instrument\0"\
  "%italian4tab\0"\
  "%italian5tab\0"\
  "%italian7tab\0"\
  "%italian8tab\0"\
  "%italiantab\0"\
  "%lyrics\0"\
  "%m\0"\
  "%merge\0"\
  "%mezzo\0"\
  "%mezzosoprano\0"\
  "%middle\0"\
  "%name\0"\
  "%nm\0"\
  "%none\0"\
  "%o\0"\
  "%octave\0"\
  "%perc\0"\
  "%percussion\0"\
  "%plus\0"\
  "%s\0"\
  "%shortname\0"\
  "%snm\0"\
  "%soprano\0"\
  "%space\0"\
  "%spanish4tab\0"\
  "%spanish5tab\0"\
  "%spanishtab\0"\
  "%spc\0"\
  "%square\0"\
  "%stafflines\0"\
  "%staves\0"\
  "%stem\0"\
  "%strings\0"\
  "%stv\0"\
  "%subbass\0"\
  "%subname\0"\
  "%t\0"\
  "%tab\0"\
  "%tablature\0"\
  "%tenor\0"\
  "%transpose\0"\
  "%treble\0"\
  "%triangle\0"\
  "%ukuleletab\0"\
  "%violin\0"\
  "%wdiamond\0"\
  "%wsquare\0"\
  "%wtriangle\0"\
  "%x\0"\
  "%xcircle\0"\
  "&barlines\0"\
  "&bassprog\0"\
  "&bassvol\0"\
  "&beat\0"\
  "&beatmod\0"\
  "&beatstring\0"\
  "&c\0"\
  "&channel\0"\
  "&chordattack\0"\
  "&chordname\0"\
  "&chordprog\0"\
  "&chordvol\0"\
  "&control\0"\
  "&deltaloudness\0"\
  "&drone\0"\
  "&droneoff\0"\
  "&droneon\0"\
  "&drum\0"\
  "&drumoff\0"\
  "&drumon\0"\
  "&fermatafixed\0"\
  "&fermataproportional\0"\
  "&gchord\0"\
  "&gchordoff\0"\
  "&gchordon\0"\
  "&grace\0"\
  "&gracedivider\0"\
  "&nobarlines\0"\
  "&noportamento\0"\
  "&pitchbend\0"\
  "&portamento\0"\
  "&program\0"\
  "&randomchordattack\0"\
  "&ratio\0"\
  "&rtranspose\0"\
  "&setpercussion\0"\
  "&transpose\0"\
  "//\0"\
  "1\0"\
  "2\0"\
  "3\0"\
  "4\0"\
  "5\0"\
  "8va\0"\
  "8vab\0"\
  "<(\0"\
  "<)\0"\
  ">\0"\
  ">(\0"\
  ">)\0"\
  "abc-charset\0"\
  "abc-copyright\0"\
  "abc-creator\0"\
  "abc-edited-by\0"\
  "abc-encoding\0"\
  "abc-include\0"\
  "abc-version\0"\
  "accent\0"\
  "Accordion\0"\
  "AcousticBass\0"\
  "AcousticBassDrum\0"\
  "AcousticBright\0"\
  "AcousticGrand\0"\
  "AcousticGrandPiano\0"\
  "AcousticGuitarNylon\0"\
  "AcousticGuitarSteel\0"\
  "AcousticSnare\0"\
  "Adagietto\0"\
  "Adagio\0"\
  "Agitato\0"\
  "Agogo\0"\
  "Allegretto\0"\
  "Allegro\0"\
  "AltoSax\0"\
  "Amabile\0"\
  "Andante\0"\
  "Andantino\0"\
  "Animato\0"\
  "annotationfont\0"\
  "Appassionato\0"\
  "Applause\0"\
  "arpeggio\0"\
  "arpeggiodown\0"\
  "arpeggioup\0"\
  "arrow\0"\
  "arrowdown\0"\
  "arrowup\0"\
  "Bagpipe\0"\
  "Banjo\0"\
  "BaritoneSax\0"\
  "barnumbers\0"\
  "barsperstaff\0"\
  "bartokpizzicato\0"\
  "Bass\0"\
  "BassDrum1\0"\
  "Bassoon\0"\
  "bbturn\0"\
  "beambr1\0"\
  "beambr2\0"\
  "beamon\0"\
  "begintext\0"\
  "BirdTweet\0"\
  "botmargin\0"\
  "BottleBlow\0"\
  "BrassSection\0"\
  "breath\0"\
  "BreathNoise\0"\
  "BrightAcoustic\0"\
  "BrightAcousticPiano\0"\
  "bturn\0"\
  "Cabasa\0"\
  "caesura\0"\
  "Cantabile\0"\
  "Celesta\0"\
  "Cello\0"\
  "center\0"\
  "centertext\0"\
  "ChineseCymbal\0"\
  "ChoirAahs\0"\
  "chordname\0"\
  "Chorused\0"\
  "ChorusedPiano\0"\
  "ChurchOrgan\0"\
  "Clarinet\0"\
  "Claves\0"\
  "Clavinet\0"\
  "ClosedHiHat\0"\
  "coda\0"\
  "Comodo\0"\
  "composerfont\0"\
  "composerspace\0"\
  "ConAnima\0"\
  "ConBrio\0"\
  "ConFuoco\0"\
  "ConMoto\0"\
  "ConSpirito\0"\
  "continueall\0"\
  "Contrabass\0"\
  "Cowbell\0"\
  "CrashCymbal1\0"\
  "CrashCymbal2\0"\
  "crescendo\0"\
  "D.C.\0"\
  "D.S.\0"\
  "dacapo\0"\
  "dacoda\0"\
  "dalsegno\0"\
  "damppedal\0"\
  "deco\0"\
  "diminuendo\0"\
  "DistortionGuitar\0"\
  "downbow\0"\
  "downmordent\0"\
  "downprall\0"\
  "Dulcimer\0"\
  "ElectricBassFinger\0"\
  "ElectricBassPick\0"\
  "ElectricGrand\0"\
  "ElectricGrandPiano\0"\
  "ElectricGuitar\0"\
  "ElectricGuitarClean\0"\
  "ElectricGuitarJazz\0"\
  "ElectricGuitarMuted\0"\
  "ElectricSnare\0"\
  "emphasis\0"\
  "endtext\0"\
  "EnglishHorn\0"\
  "exprabove\0"\
  "exprbelow\0"\
  "f\0"\
  "fermata\0"\
  "ff\0"\
  "fff\0"\
  "ffff\0"\
  "Fiddle\0"\
  "fine\0"\
  "flageolet\0"\
  "Flute\0"\
  "forte\0"\
  "fortepiano\0"\
  "fortissimo\0"\
  "fortississimo\0"\
  "fortissississimo\0"\
  "fp\0"\
  "freegchord\0"\
  "FrenchHorn\0"\
  "FretlessBass\0"\
  "FX1\0"\
  "FX2\0"\
  "FX3\0"\
  "FX4\0"\
  "FX5\0"\
  "FX6\0"\
  "FX7\0"\
  "FX8\0"\
  "FXAtmosphere\0"\
  "FXBrightness\0"\
  "FXCrystal\0"\
  "FXEchoes\0"\
  "FXGoblins\0"\
  "FXRain\0"\
  "FXSciFi\0"\
  "FXSoundtrack\0"\
  "fz\0"\
  "g\0"\
  "G.P.\0"\
  "gchordfont\0"\
  "generalpause\0"\
  "Giusto\0"\
  "glissando\0"\
  "Glockenspiel\0"\
  "gmark\0"\
  "Gong\0"\
  "graceslurs\0"\
  "grandpause\0"\
  "Grave\0"\
  "Grazioso\0"\
  "Guitar\0"\
  "GuitarFretNoise\0"\
  "GuitarHarmonics\0"\
  "Gunshot\0"\
  "H\0"\
  "H.O.\0"\
  "HalfOpenHiHat\0"\
  "hammeringon\0"\
  "Hammond\0"\
  "HammondOrgan\0"\
  "HandClap\0"\
  "HardStick\0"\
  "Harmonica\0"\
  "Harpsichord\0"\
  "Helicopter\0"\
  "HiBongo\0"\
  "HighAgogo\0"\
  "HighFloorTom\0"\
  "HighTimbale\0"\
  "HighTom\0"\
  "HiHat\0"\
  "HiMidTom\0"\
  "HiWoodBlock\0"\
  "hold\0"\
  "HonkyTonk\0"\
  "HonkyTonkPiano\0"\
  "include\0"\
  "indent\0"\
  "infofont\0"\
  "infoline\0"\
  "infospace\0"\
  "invertedfermata\0"\
  "invertedturn\0"\
  "invertedturnx\0"\
  "invisible\0"\
  "irishroll\0"\
  "Kalimba\0"\
  "Koto\0"\
  "landscape\0"\
  "Larghetto\0"\
  "Largo\0"\
  "Lead1\0"\
  "Lead2\0"\
  "Lead3\0"\
  "Lead4\0"\
  "Lead5\0"\
  "Lead6\0"\
  "Lead7\0"\
  "Lead8\0"\
  "LeadBrass\0"\
  "LeadCaliope\0"\
  "LeadCharang\0"\
  "LeadChiff\0"\
  "LeadFifths\0"\
  "LeadSawtooth\0"\
  "LeadSquare\0"\
  "LeadVoice\0"\
  "lefthand\0"\
  "leftmargin\0"\
  "lefttext\0"\
  "Lento\0"\
  "lheel\0"\
  "lineprall\0"\
  "lineskipfac\0"\
  "longfermata\0"\
  "LongGuiro\0"\
  "longphrase\0"\
  "LongWhistle\0"\
  "LowAgogo\0"\
  "LowBongo\0"\
  "LowConga\0"\
  "lowermordent\0"\
  "LowFloorTom\0"\
  "LowMidTom\0"\
  "LowTimbale\0"\
  "LowTom\0"\
  "LowWoodBlock\0"\
  "lped\0"\
  "ltoe\0"\
  "m.d.\0"\
  "m.g.\0"\
  "Maestoso\0"\
  "Maracas\0"\
  "marcato\0"\
  "Marimba\0"\
  "maxshrink\0"\
  "md.\0"\
  "measurebox\0"\
  "measurefirst\0"\
  "measurenb\0"\
  "mediumphrase\0"\
  "MelodicTom\0"\
  "Mesto\0"\
  "MetalStick\0"\
  "mezzoforte\0"\
  "mezzopiano\0"\
  "mf\0"\
  "mg.\0"\
  "midi\0"\
  "Moderato\0"\
  "mordent\0"\
  "Mosso\0"\
  "mp\0"\
  "mped\0"\
  "MusicBox\0"\
  "musiconly\0"\
  "musicspace\0"\
  "MuteCuica\0"\
  "MutedTrumpet\0"\
  "MuteHiConga\0"\
  "MuteTriangle\0"\
  "n\0"\
  "newpage\0"\
  "niente\0"\
  "noarpeggio\0"\
  "Oboe\0"\
  "Ocarina\0"\
  "oneperpage\0"\
  "open\0"\
  "OpenCuica\0"\
  "OpenHiConga\0"\
  "OpenHiHat\0"\
  "OpenTriangle\0"\
  "OrchestraHit\0"\
  "OrchestralHarp\0"\
  "OverdrivenGuitar\0"\
  "p\0"\
  "P.O.\0"\
  "Pad1\0"\
  "Pad2\0"\
  "Pad3\0"\
  "Pad4\0"\
  "Pad5\0"\
  "Pad6\0"\
  "Pad7\0"\
  "Pad8\0"\
  "PadBowed\0"\
  "PadChoir\0"\
  "PadHalo\0"\
  "PadMetallic\0"\
  "PadNewage\0"\
  "PadPolysynth\0"\
  "PadSweep\0"\
  "PadWarm\0"\
  "pageheight\0"\
  "pagewidth\0"\
  "PanFlute\0"\
  "papersize\0"\
  "parskipfac\0"\
  "partsfont\0"\
  "partsspace\0"\
  "pause\0"\
  "ped\0"\
  "PedalHiHat\0"\
  "PercussiveOrgan\0"\
  "pianissimo\0"\
  "pianississimo\0"\
  "pianissississimo\0"\
  "piano\0"\
  "Piccolo\0"\
  "pizzicato\0"\
  "PizzicatoStrings\0"\
  "portato\0"\
  "postscript\0"\
  "pp\0"\
  "ppp\0"\
  "pppp\0"\
  "prall\0"\
  "pralldown\0"\
  "prallmordent\0"\
  "prallprall\0"\
  "pralltriller\0"\
  "prallup\0"\
  "Prestissimo\0"\
  "Presto\0"\
  "printtempo\0"\
  "propagate-accidentals\0"\
  "pulloff\0"\
  "Recorder\0"\
  "ReedOrgan\0"\
  "ReverseCymbal\0"\
  "reversefermata\0"\
  "reverseturn\0"\
  "reverseturnx\0"\
  "rf\0"\
  "rfz\0"\
  "rheel\0"\
  "Rhodes\0"\
  "RhodesPiano\0"\
  "RideBell\0"\
  "RideCymbal1\0"\
  "RideCymbal2\0"\
  "righthand\0"\
  "rightmargin\0"\
  "righttext\0"\
  "rinforzando\0"\
  "Risoluto\0"\
  "RockOrgan\0"\
  "roll\0"\
  "rr\0"\
  "rtoe\0"\
  "RubberStick\0"\
  "scale\0"\
  "score\0"\
  "Seashore\0"\
  "segno\0"\
  "sep\0"\
  "setbarnb\0"\
  "setfont-1\0"\
  "setfont-2\0"\
  "setfont-3\0"\
  "setfont-4\0"\
  "setpercussion\0"\
  "sf\0"\
  "sff\0"\
  "sfff\0"\
  "sffff\0"\
  "sforzando\0"\
  "sforzandopiano\0"\
  "sfp\0"\
  "sfz\0"\
  "Shakuhachi\0"\
  "Shamisen\0"\
  "Shanai\0"\
  "shortfermata\0"\
  "ShortGuiro\0"\
  "shortphrase\0"\
  "ShortWhistle\0"\
  "SideStick\0"\
  "signumcongruentiae\0"\
  "Sitar\0"\
  "SL.\0"\
  "SlapBass1\0"\
  "SlapBass2\0"\
  "slide\0"\
  "sliding\0"\
  "smf\0"\
  "smp\0"\
  "snappizzicato\0"\
  "Snare\0"\
  "softpedal\0"\
  "SoftStick\0"\
  "SopranoSax\0"\
  "Sostenuto\0"\
  "sostpedal\0"\
  "sp\0"\
  "SplashCymbal\0"\
  "spp\0"\
  "sppp\0"\
  "spppp\0"\
  "staccatissimo\0"\
  "staccato\0"\
  "staffsep\0"\
  "staves\0"\
  "SteelDrums\0"\
  "stemdown\0"\
  "stemup\0"\
  "stopped\0"\
  "stretchlast\0"\
  "stretchstaff\0"\
  "StringEnsemble1\0"\
  "StringEnsemble2\0"\
  "strum\0"\
  "strumdown\0"\
  "strumup\0"\
  "subitof\0"\
  "subitoff\0"\
  "subitofff\0"\
  "subitoffff\0"\
  "subitoforte\0"\
  "subitofortissimo\0"\
  "subitofortississimo\0"\
  "subitofortissississimo\0"\
  "subitomezzoforte\0"\
  "subitomezzopiano\0"\
  "subitomf\0"\
  "subitomp\0"\
  "subitop\0"\
  "subitopianissimo\0"\
  "subitopianississimo\0"\
  "subitopianissississimo\0"\
  "subitopiano\0"\
  "subitopp\0"\
  "subitoppp\0"\
  "subitopppp\0"\
  "subtitlefont\0"\
  "subtitlespace\0"\
  "SynthBass1\0"\
  "SynthBass2\0"\
  "SynthBrass1\0"\
  "SynthBrass2\0"\
  "SynthDrum\0"\
  "SynthStrings1\0"\
  "SynthStrings2\0"\
  "SynthVoice\0"\
  "sysstaffsep\0"\
  "tabaddflags\0"\
  "taballflags\0"\
  "tabflagspace\0"\
  "tabfontfrench\0"\
  "tabfontitalian\0"\
  "tabfontscale\0"\
  "tabfontsize\0"\
  "tabledgeabove\0"\
  "tabrhstyle\0"\
  "TaikoDrum\0"\
  "Tambourine\0"\
  "TangoAccordion\0"\
  "TelephoneRing\0"\
  "tempofont\0"\
  "TenorSax\0"\
  "tenuto\0"\
  "text\0"\
  "textbegin\0"\
  "textend\0"\
  "textfont\0"\
  "textspace\0"\
  "thumb\0"\
  "Timpani\0"\
  "TinkleBell\0"\
  "titlefont\0"\
  "titlespace\0"\
  "topmargin\0"\
  "topspace\0"\
  "tr\0"\
  "Tranquillo\0"\
  "trem1\0"\
  "trem4\0"\
  "TremoloStrings\0"\
  "Triangle\0"\
  "TriangleStick\0"\
  "trill\0"\
  "Trombone\0"\
  "Trumpet\0"\
  "Tuba\0"\
  "TubularBells\0"\
  "turn\0"\
  "turn#\0"\
  "turn##\0"\
  "turnb\0"\
  "turnbb\0"\
  "turnx\0"\
  "udfermata\0"\
  "ufermata\0"\
  "upbow\0"\
  "upmordent\0"\
  "uppermordent\0"\
  "upprall\0"\
  "varcoda\0"\
  "verylongfermata\0"\
  "Vibraphone\0"\
  "Vibraslap\0"\
  "Viola\0"\
  "Violin\0"\
  "Vivace\0"\
  "Vivo\0"\
  "vocalabove\0"\
  "vocalfont\0"\
  "vocalspace\0"\
  "VoiceOohs\0"\
  "vskip\0"\
  "W.\0"\
  "wham\0"\
  "Whistle\0"\
  "WireBrush\0"\
  "withxrefs\0"\
  "Woodblock\0"\
  "WoodStick\0"\
  "wordsfont\0"\
  "wordsspace\0"\
  "writehistory\0"\
  "writeout-accidentals\0"\
  "xstem\0"\
  "xturn\0"\
  "Xylophone\0"\
  "~\0"\
  "\0"
};

static abc_str strs[681] = {
 /*    0 */ { 0x2A3A , 0x0000 },
 /*    1 */ { 0x2A34 , 0x0007 },
 /*    2 */ { 0x505A , 0x000D },
 /*    3 */ { 0x5138 , 0x0013 },
 /*    4 */ { 0x5139 , 0x001E },
 /*    5 */ { 0x5055 , 0x0029 },
 /*    6 */ { 0x506A , 0x0033 },
 /*    7 */ { 0x5E23 , 0x0039 },
 /*    8 */ { 0x5E22 , 0x0040 },
 /*    9 */ { 0x5E23 , 0x0049 },
 /*   10 */ { 0x5E22 , 0x004E },
 /*   11 */ { 0x5E33 , 0x0053 },
 /*   12 */ { 0x2EF8 , 0x0059 },
 /*   13 */ { 0x507A , 0x0062 },
 /*   14 */ { 0x5250 , 0x0067 },
 /*   15 */ { 0x5E35 , 0x006D },
 /*   16 */ { 0x508A , 0x0072 },
 /*   17 */ { 0x5041 , 0x0076 },
 /*   18 */ { 0x5133 , 0x007E },
 /*   19 */ { 0x5134 , 0x008A },
 /*   20 */ { 0x5132 , 0x0096 },
 /*   21 */ { 0x5041 , 0x00A1 },
 /*   22 */ { 0x5E2F , 0x00AF },
 /*   23 */ { 0x5135 , 0x00B6 },
 /*   24 */ { 0x5EF0 , 0x00C1 },
 /*   25 */ { 0x2EF8 , 0x00C7 },
 /*   26 */ { 0x2EF1 , 0x00D5 },
 /*   27 */ { 0x2EF4 , 0x00E0 },
 /*   28 */ { 0x2EF6 , 0x00ED },
 /*   29 */ { 0x2EF9 , 0x00FC },
 /*   30 */ { 0x2EF5 , 0x010B },
 /*   31 */ { 0x2EF7 , 0x0119 },
 /*   32 */ { 0x2EF2 , 0x0129 },
 /*   33 */ { 0x2EF3 , 0x0131 },
 /*   34 */ { 0x5E34 , 0x013F },
 /*   35 */ { 0x5E34 , 0x0142 },
 /*   36 */ { 0x5E34 , 0x0149 },
 /*   37 */ { 0x513C , 0x0155 },
 /*   38 */ { 0x513D , 0x0162 },
 /*   39 */ { 0x513E , 0x016F },
 /*   40 */ { 0x513F , 0x017C },
 /*   41 */ { 0x513B , 0x0189 },
 /*   42 */ { 0x5E36 , 0x0195 },
 /*   43 */ { 0x5E2E , 0x019D },
 /*   44 */ { 0x5E24 , 0x01A0 },
 /*   45 */ { 0x5052 , 0x01A7 },
 /*   46 */ { 0x5052 , 0x01AE },
 /*   47 */ { 0x5E2E , 0x01BC },
 /*   48 */ { 0x5E31 , 0x01C4 },
 /*   49 */ { 0x5E31 , 0x01CA },
 /*   50 */ { 0x524E , 0x01CE },
 /*   51 */ { 0x5E2B , 0x01D4 },
 /*   52 */ { 0x5E2B , 0x01D7 },
 /*   53 */ { 0x5250 , 0x01DF },
 /*   54 */ { 0x5250 , 0x01E5 },
 /*   55 */ { 0x2EF1 , 0x01F1 },
 /*   56 */ { 0x5E2D , 0x01F7 },
 /*   57 */ { 0x5E32 , 0x01FA },
 /*   58 */ { 0x5E32 , 0x0205 },
 /*   59 */ { 0x5051 , 0x020A },
 /*   60 */ { 0x5E2A , 0x0213 },
 /*   61 */ { 0x5136 , 0x021A },
 /*   62 */ { 0x5137 , 0x0227 },
 /*   63 */ { 0x5135 , 0x0234 },
 /*   64 */ { 0x5E2A , 0x0240 },
 /*   65 */ { 0x2EF4 , 0x0245 },
 /*   66 */ { 0x5E2D , 0x024D },
 /*   67 */ { 0x5E21 , 0x0259 },
 /*   68 */ { 0x5E30 , 0x0261 },
 /*   69 */ { 0x5155 , 0x0267 },
 /*   70 */ { 0x5E21 , 0x0270 },
 /*   71 */ { 0x5065 , 0x0275 },
 /*   72 */ { 0x5E32 , 0x027E },
 /*   73 */ { 0x5E2C , 0x0287 },
 /*   74 */ { 0x5130 , 0x028A },
 /*   75 */ { 0x5130 , 0x028F },
 /*   76 */ { 0x5054 , 0x029A },
 /*   77 */ { 0x5E2C , 0x02A1 },
 /*   78 */ { 0x504A , 0x02AC },
 /*   79 */ { 0x2EF6 , 0x02B4 },
 /*   80 */ { 0x513A , 0x02BE },
 /*   81 */ { 0x5042 , 0x02CA },
 /*   82 */ { 0x2EF9 , 0x02D2 },
 /*   83 */ { 0x2EF5 , 0x02DC },
 /*   84 */ { 0x2EF7 , 0x02E5 },
 /*   85 */ { 0x2EF2 , 0x02F0 },
 /*   86 */ { 0x2EF3 , 0x02F3 },
 /*   87 */ { 0x3027 , 0x02FC },
 /*   88 */ { 0x3012 , 0x0306 },
 /*   89 */ { 0x3014 , 0x0310 },
 /*   90 */ { 0x3004 , 0x0319 },
 /*   91 */ { 0x3005 , 0x031F },
 /*   92 */ { 0x3007 , 0x0328 },
 /*   93 */ { 0x300A , 0x0334 },
 /*   94 */ { 0x3002 , 0x0337 },
 /*   95 */ { 0x3029 , 0x0340 },
 /*   96 */ { 0x300D , 0x034D },
 /*   97 */ { 0x3011 , 0x0358 },
 /*   98 */ { 0x3013 , 0x0363 },
 /*   99 */ { 0x3021 , 0x036D },
 /*  100 */ { 0x3006 , 0x0376 },
 /*  101 */ { 0x301B , 0x0385 },
 /*  102 */ { 0x301A , 0x038C },
 /*  103 */ { 0x3019 , 0x0396 },
 /*  104 */ { 0x301C , 0x039F },
 /*  105 */ { 0x301F , 0x03A5 },
 /*  106 */ { 0x301D , 0x03AE },
 /*  107 */ { 0x3017 , 0x03B6 },
 /*  108 */ { 0x3018 , 0x03C4 },
 /*  109 */ { 0x300F , 0x03D9 },
 /*  110 */ { 0x3016 , 0x03E1 },
 /*  111 */ { 0x3015 , 0x03EC },
 /*  112 */ { 0x300B , 0x03F6 },
 /*  113 */ { 0x300C , 0x03FD },
 /*  114 */ { 0x3026 , 0x040B },
 /*  115 */ { 0x3024 , 0x0417 },
 /*  116 */ { 0x3025 , 0x0425 },
 /*  117 */ { 0x3023 , 0x0430 },
 /*  118 */ { 0x3003 , 0x043C },
 /*  119 */ { 0x302A , 0x0445 },
 /*  120 */ { 0x3028 , 0x0458 },
 /*  121 */ { 0x3009 , 0x045F },
 /*  122 */ { 0x3101 , 0x046B },
 /*  123 */ { 0x3008 , 0x047A },
 /*  124 */ { 0x2507 , 0x0485 },
 /*  125 */ { 0x2201 , 0x0488 },
 /*  126 */ { 0x2202 , 0x048A },
 /*  127 */ { 0x2203 , 0x048C },
 /*  128 */ { 0x2204 , 0x048E },
 /*  129 */ { 0x2205 , 0x0490 },
 /*  130 */ { 0x260D , 0x0492 },
 /*  131 */ { 0x260F , 0x0496 },
 /*  132 */ { 0x211D , 0x049B },
 /*  133 */ { 0x211E , 0x049E },
 /*  134 */ { 0x2A01 , 0x04A1 },
 /*  135 */ { 0x2121 , 0x04A3 },
 /*  136 */ { 0x2122 , 0x04A6 },
 /*  137 */ { 0x1E06 , 0x04A9 },
 /*  138 */ { 0x1E03 , 0x04B5 },
 /*  139 */ { 0x1E05 , 0x04C3 },
 /*  140 */ { 0x1E08 , 0x04CF },
 /*  141 */ { 0x1E06 , 0x04DD },
 /*  142 */ { 0x1E07 , 0x04EA },
 /*  143 */ { 0x1E04 , 0x04F6 },
 /*  144 */ { 0x2A01 , 0x0502 },
 /*  145 */ { 0x0815 , 0x0509 },
 /*  146 */ { 0x0820 , 0x0513 },
 /*  147 */ { 0x2823 , 0x0520 },
 /*  148 */ { 0x0801 , 0x0531 },
 /*  149 */ { 0x0800 , 0x0540 },
 /*  150 */ { 0x0800 , 0x054E },
 /*  151 */ { 0x0818 , 0x0561 },
 /*  152 */ { 0x0819 , 0x0575 },
 /*  153 */ { 0x2826 , 0x0589 },
 /*  154 */ { 0x0B08 , 0x0597 },
 /*  155 */ { 0x0B01 , 0x05A1 },
 /*  156 */ { 0x0B0E , 0x05A8 },
 /*  157 */ { 0x0871 , 0x05B0 },
 /*  158 */ { 0x0B09 , 0x05B6 },
 /*  159 */ { 0x0B02 , 0x05C1 },
 /*  160 */ { 0x0841 , 0x05C9 },
 /*  161 */ { 0x0B0F , 0x05D1 },
 /*  162 */ { 0x0B03 , 0x05D9 },
 /*  163 */ { 0x0B0A , 0x05E1 },
 /*  164 */ { 0x0B10 , 0x05EB },
 /*  165 */ { 0x1D01 , 0x05F3 },
 /*  166 */ { 0x0B11 , 0x0602 },
 /*  167 */ { 0x087E , 0x060F },
 /*  168 */ { 0x2206 , 0x0618 },
 /*  169 */ { 0x2208 , 0x0621 },
 /*  170 */ { 0x2207 , 0x062E },
 /*  171 */ { 0x2209 , 0x0639 },
 /*  172 */ { 0x220B , 0x063F },
 /*  173 */ { 0x220A , 0x0649 },
 /*  174 */ { 0x086D , 0x0651 },
 /*  175 */ { 0x0869 , 0x0659 },
 /*  176 */ { 0x0843 , 0x065F },
 /*  177 */ { 0x1901 , 0x066B },
 /*  178 */ { 0x1902 , 0x0676 },
 /*  179 */ { 0x2217 , 0x0683 },
 /*  180 */ { 0x0820 , 0x0693 },
 /*  181 */ { 0x2824 , 0x0698 },
 /*  182 */ { 0x0846 , 0x06A2 },
 /*  183 */ { 0x2A38 , 0x06AA },
 /*  184 */ { 0x2642 , 0x06B1 },
 /*  185 */ { 0x2643 , 0x06B9 },
 /*  186 */ { 0x2611 , 0x06C1 },
 /*  187 */ { 0x1F01 , 0x06C8 },
 /*  188 */ { 0x087B , 0x06D2 },
 /*  189 */ { 0x1C01 , 0x06DC },
 /*  190 */ { 0x084C , 0x06E6 },
 /*  191 */ { 0x083D , 0x06F1 },
 /*  192 */ { 0x2501 , 0x06FE },
 /*  193 */ { 0x0879 , 0x0705 },
 /*  194 */ { 0x0801 , 0x0711 },
 /*  195 */ { 0x0801 , 0x0720 },
 /*  196 */ { 0x2A32 , 0x0734 },
 /*  197 */ { 0x2845 , 0x073A },
 /*  198 */ { 0x2507 , 0x0741 },
 /*  199 */ { 0x0B12 , 0x0749 },
 /*  200 */ { 0x0808 , 0x0753 },
 /*  201 */ { 0x082A , 0x075B },
 /*  202 */ { 0x1E01 , 0x0761 },
 /*  203 */ { 0x1E01 , 0x0768 },
 /*  204 */ { 0x2834 , 0x0773 },
 /*  205 */ { 0x0834 , 0x0781 },
 /*  206 */ { 0x300D , 0x078B },
 /*  207 */ { 0x0805 , 0x0795 },
 /*  208 */ { 0x0805 , 0x079E },
 /*  209 */ { 0x0813 , 0x07AC },
 /*  210 */ { 0x0847 , 0x07B8 },
 /*  211 */ { 0x284B , 0x07C1 },
 /*  212 */ { 0x0807 , 0x07C8 },
 /*  213 */ { 0x282A , 0x07D1 },
 /*  214 */ { 0x2602 , 0x07DD },
 /*  215 */ { 0x0B13 , 0x07E2 },
 /*  216 */ { 0x1D02 , 0x07E9 },
 /*  217 */ { 0x1C02 , 0x07F6 },
 /*  218 */ { 0x0B14 , 0x0804 },
 /*  219 */ { 0x0B15 , 0x080D },
 /*  220 */ { 0x0B16 , 0x0815 },
 /*  221 */ { 0x0B17 , 0x081E },
 /*  222 */ { 0x0B18 , 0x0826 },
 /*  223 */ { 0x1B01 , 0x0831 },
 /*  224 */ { 0x082B , 0x083D },
 /*  225 */ { 0x2838 , 0x0848 },
 /*  226 */ { 0x2831 , 0x0850 },
 /*  227 */ { 0x2839 , 0x085D },
 /*  228 */ { 0x211C , 0x086A },
 /*  229 */ { 0x2604 , 0x0874 },
 /*  230 */ { 0x2603 , 0x0879 },
 /*  231 */ { 0x2606 , 0x087E },
 /*  232 */ { 0x2605 , 0x0885 },
 /*  233 */ { 0x2607 , 0x088C },
 /*  234 */ { 0x2401 , 0x0895 },
 /*  235 */ { 0x1885 , 0x089F },
 /*  236 */ { 0x2120 , 0x08A4 },
 /*  237 */ { 0x081E , 0x08AF },
 /*  238 */ { 0x2A08 , 0x08C0 },
 /*  239 */ { 0x2A1B , 0x08C8 },
 /*  240 */ { 0x2A19 , 0x08D4 },
 /*  241 */ { 0x080F , 0x08DE },
 /*  242 */ { 0x0821 , 0x08E7 },
 /*  243 */ { 0x0822 , 0x08FA },
 /*  244 */ { 0x0802 , 0x090B },
 /*  245 */ { 0x0802 , 0x0919 },
 /*  246 */ { 0x081B , 0x092C },
 /*  247 */ { 0x081B , 0x093B },
 /*  248 */ { 0x081A , 0x094F },
 /*  249 */ { 0x081C , 0x0962 },
 /*  250 */ { 0x2828 , 0x0976 },
 /*  251 */ { 0x2A01 , 0x0984 },
 /*  252 */ { 0x1F02 , 0x098D },
 /*  253 */ { 0x0845 , 0x0995 },
 /*  254 */ { 0x1B02 , 0x09A1 },
 /*  255 */ { 0x1B03 , 0x09AB },
 /*  256 */ { 0x2107 , 0x09B5 },
 /*  257 */ { 0x2502 , 0x09B7 },
 /*  258 */ { 0x2108 , 0x09BF },
 /*  259 */ { 0x2109 , 0x09C2 },
 /*  260 */ { 0x210A , 0x09C6 },
 /*  261 */ { 0x086E , 0x09CB },
 /*  262 */ { 0x2608 , 0x09D2 },
 /*  263 */ { 0x2A09 , 0x09D7 },
 /*  264 */ { 0x0849 , 0x09E1 },
 /*  265 */ { 0x2107 , 0x09E7 },
 /*  266 */ { 0x210B , 0x09ED },
 /*  267 */ { 0x2108 , 0x09F8 },
 /*  268 */ { 0x2109 , 0x0A03 },
 /*  269 */ { 0x210A , 0x0A11 },
 /*  270 */ { 0x210B , 0x0A22 },
 /*  271 */ { 0x1B04 , 0x0A25 },
 /*  272 */ { 0x083C , 0x0A30 },
 /*  273 */ { 0x0823 , 0x0A3B },
 /*  274 */ { 0x0860 , 0x0A48 },
 /*  275 */ { 0x0861 , 0x0A4C },
 /*  276 */ { 0x0862 , 0x0A50 },
 /*  277 */ { 0x0863 , 0x0A54 },
 /*  278 */ { 0x0864 , 0x0A58 },
 /*  279 */ { 0x0865 , 0x0A5C },
 /*  280 */ { 0x0866 , 0x0A60 },
 /*  281 */ { 0x0867 , 0x0A64 },
 /*  282 */ { 0x0863 , 0x0A68 },
 /*  283 */ { 0x0864 , 0x0A75 },
 /*  284 */ { 0x0862 , 0x0A82 },
 /*  285 */ { 0x0866 , 0x0A8C },
 /*  286 */ { 0x0865 , 0x0A95 },
 /*  287 */ { 0x0860 , 0x0A9F },
 /*  288 */ { 0x0867 , 0x0AA6 },
 /*  289 */ { 0x0861 , 0x0AAE },
 /*  290 */ { 0x210C , 0x0ABB },
 /*  291 */ { 0x2125 , 0x0ABE },
 /*  292 */ { 0x2506 , 0x0AC0 },
 /*  293 */ { 0x1D03 , 0x0AC5 },
 /*  294 */ { 0x2506 , 0x0AD0 },
 /*  295 */ { 0x0B19 , 0x0ADD },
 /*  296 */ { 0x2125 , 0x0AE4 },
 /*  297 */ { 0x0809 , 0x0AEE },
 /*  298 */ { 0x2612 , 0x0AFB },
 /*  299 */ { 0x2301 , 0x0B01 },
 /*  300 */ { 0x1B07 , 0x0B06 },
 /*  301 */ { 0x2509 , 0x0B11 },
 /*  302 */ { 0x0B04 , 0x0B1C },
 /*  303 */ { 0x0B1A , 0x0B22 },
 /*  304 */ { 0x0818 , 0x0B2B },
 /*  305 */ { 0x0878 , 0x0B32 },
 /*  306 */ { 0x081F , 0x0B42 },
 /*  307 */ { 0x087F , 0x0B52 },
 /*  308 */ { 0x2502 , 0x0B5A },
 /*  309 */ { 0x220D , 0x0B5C },
 /*  310 */ { 0x2303 , 0x0B61 },
 /*  311 */ { 0x220D , 0x0B6F },
 /*  312 */ { 0x0810 , 0x0B7B },
 /*  313 */ { 0x0810 , 0x0B83 },
 /*  314 */ { 0x2827 , 0x0B90 },
 /*  315 */ { 0x2304 , 0x0B99 },
 /*  316 */ { 0x0816 , 0x0BA3 },
 /*  317 */ { 0x0806 , 0x0BAD },
 /*  318 */ { 0x087D , 0x0BB9 },
 /*  319 */ { 0x283C , 0x0BC4 },
 /*  320 */ { 0x2843 , 0x0BCC },
 /*  321 */ { 0x282B , 0x0BD6 },
 /*  322 */ { 0x2841 , 0x0BE3 },
 /*  323 */ { 0x2832 , 0x0BEF },
 /*  324 */ { 0x232A , 0x0BF7 },
 /*  325 */ { 0x2830 , 0x0BFD },
 /*  326 */ { 0x284C , 0x0C06 },
 /*  327 */ { 0x2502 , 0x0C12 },
 /*  328 */ { 0x0803 , 0x0C17 },
 /*  329 */ { 0x0803 , 0x0C21 },
 /*  330 */ { 0x1E07 , 0x0C30 },
 /*  331 */ { 0x1C03 , 0x0C38 },
 /*  332 */ { 0x1D04 , 0x0C3F },
 /*  333 */ { 0x1B08 , 0x0C48 },
 /*  334 */ { 0x1C04 , 0x0C51 },
 /*  335 */ { 0x2508 , 0x0C5B },
 /*  336 */ { 0x2A12 , 0x0C6B },
 /*  337 */ { 0x2A22 , 0x0C78 },
 /*  338 */ { 0x2613 , 0x0C86 },
 /*  339 */ { 0x2A25 , 0x0C90 },
 /*  340 */ { 0x086C , 0x0C9A },
 /*  341 */ { 0x086B , 0x0CA2 },
 /*  342 */ { 0x1B09 , 0x0CA7 },
 /*  343 */ { 0x0B0B , 0x0CB1 },
 /*  344 */ { 0x0B05 , 0x0CBB },
 /*  345 */ { 0x0850 , 0x0CC1 },
 /*  346 */ { 0x0851 , 0x0CC7 },
 /*  347 */ { 0x0852 , 0x0CCD },
 /*  348 */ { 0x0853 , 0x0CD3 },
 /*  349 */ { 0x0854 , 0x0CD9 },
 /*  350 */ { 0x0855 , 0x0CDF },
 /*  351 */ { 0x0856 , 0x0CE5 },
 /*  352 */ { 0x0857 , 0x0CEB },
 /*  353 */ { 0x0857 , 0x0CF1 },
 /*  354 */ { 0x0852 , 0x0CFB },
 /*  355 */ { 0x0854 , 0x0D07 },
 /*  356 */ { 0x0853 , 0x0D13 },
 /*  357 */ { 0x0856 , 0x0D1D },
 /*  358 */ { 0x0851 , 0x0D28 },
 /*  359 */ { 0x0850 , 0x0D35 },
 /*  360 */ { 0x0855 , 0x0D40 },
 /*  361 */ { 0x2214 , 0x0D4A },
 /*  362 */ { 0x1C06 , 0x0D53 },
 /*  363 */ { 0x1E01 , 0x0D5E },
 /*  364 */ { 0x0B06 , 0x0D67 },
 /*  365 */ { 0x2A0B , 0x0D6D },
 /*  366 */ { 0x2A1E , 0x0D73 },
 /*  367 */ { 0x1A01 , 0x0D7D },
 /*  368 */ { 0x2504 , 0x0D89 },
 /*  369 */ { 0x284A , 0x0D95 },
 /*  370 */ { 0x260C , 0x0D9F },
 /*  371 */ { 0x2848 , 0x0DAA },
 /*  372 */ { 0x2844 , 0x0DB6 },
 /*  373 */ { 0x283D , 0x0DBF },
 /*  374 */ { 0x2840 , 0x0DC8 },
 /*  375 */ { 0x2A15 , 0x0DD1 },
 /*  376 */ { 0x2829 , 0x0DDE },
 /*  377 */ { 0x282F , 0x0DEA },
 /*  378 */ { 0x2842 , 0x0DF4 },
 /*  379 */ { 0x282D , 0x0DFF },
 /*  380 */ { 0x284D , 0x0E06 },
 /*  381 */ { 0x2404 , 0x0E13 },
 /*  382 */ { 0x2A0D , 0x0E18 },
 /*  383 */ { 0x2215 , 0x0E1D },
 /*  384 */ { 0x2214 , 0x0E22 },
 /*  385 */ { 0x0B21 , 0x0E27 },
 /*  386 */ { 0x2846 , 0x0E30 },
 /*  387 */ { 0x2A02 , 0x0E38 },
 /*  388 */ { 0x080C , 0x0E40 },
 /*  389 */ { 0x1A02 , 0x0E48 },
 /*  390 */ { 0x2215 , 0x0E52 },
 /*  391 */ { 0x1B06 , 0x0E56 },
 /*  392 */ { 0x1903 , 0x0E61 },
 /*  393 */ { 0x1904 , 0x0E6E },
 /*  394 */ { 0x260B , 0x0E78 },
 /*  395 */ { 0x0875 , 0x0E85 },
 /*  396 */ { 0x0B20 , 0x0E90 },
 /*  397 */ { 0x2306 , 0x0E96 },
 /*  398 */ { 0x2106 , 0x0EA1 },
 /*  399 */ { 0x2105 , 0x0EAC },
 /*  400 */ { 0x2106 , 0x0EB7 },
 /*  401 */ { 0x2214 , 0x0EBA },
 /*  402 */ { 0x3001 , 0x0EBE },
 /*  403 */ { 0x0B22 , 0x0EC3 },
 /*  404 */ { 0x2A15 , 0x0ECC },
 /*  405 */ { 0x0B0C , 0x0ED4 },
 /*  406 */ { 0x2105 , 0x0EDA },
 /*  407 */ { 0x2402 , 0x0EDD },
 /*  408 */ { 0x080A , 0x0EE2 },
 /*  409 */ { 0x1B05 , 0x0EEB },
 /*  410 */ { 0x1C07 , 0x0EF5 },
 /*  411 */ { 0x284E , 0x0F00 },
 /*  412 */ { 0x083B , 0x0F0A },
 /*  413 */ { 0x283E , 0x0F17 },
 /*  414 */ { 0x2850 , 0x0F23 },
 /*  415 */ { 0x210F , 0x0F30 },
 /*  416 */ { 0x1F03 , 0x0F32 },
 /*  417 */ { 0x210F , 0x0F3A },
 /*  418 */ { 0x220C , 0x0F41 },
 /*  419 */ { 0x0844 , 0x0F4C },
 /*  420 */ { 0x084F , 0x0F51 },
 /*  421 */ { 0x1B0A , 0x0F59 },
 /*  422 */ { 0x2A0F , 0x0F64 },
 /*  423 */ { 0x284F , 0x0F69 },
 /*  424 */ { 0x283F , 0x0F73 },
 /*  425 */ { 0x282E , 0x0F7F },
 /*  426 */ { 0x2851 , 0x0F89 },
 /*  427 */ { 0x0837 , 0x0F96 },
 /*  428 */ { 0x082E , 0x0FA3 },
 /*  429 */ { 0x081D , 0x0FB2 },
 /*  430 */ { 0x2104 , 0x0FC3 },
 /*  431 */ { 0x220E , 0x0FC5 },
 /*  432 */ { 0x0858 , 0x0FCA },
 /*  433 */ { 0x0859 , 0x0FCF },
 /*  434 */ { 0x085A , 0x0FD4 },
 /*  435 */ { 0x085B , 0x0FD9 },
 /*  436 */ { 0x085C , 0x0FDE },
 /*  437 */ { 0x085D , 0x0FE3 },
 /*  438 */ { 0x085E , 0x0FE8 },
 /*  439 */ { 0x085F , 0x0FED },
 /*  440 */ { 0x085C , 0x0FF2 },
 /*  441 */ { 0x085B , 0x0FFB },
 /*  442 */ { 0x085E , 0x1004 },
 /*  443 */ { 0x085D , 0x100C },
 /*  444 */ { 0x0858 , 0x1018 },
 /*  445 */ { 0x085A , 0x1022 },
 /*  446 */ { 0x085F , 0x102F },
 /*  447 */ { 0x0859 , 0x1038 },
 /*  448 */ { 0x1C08 , 0x1040 },
 /*  449 */ { 0x1C09 , 0x104B },
 /*  450 */ { 0x084B , 0x1055 },
 /*  451 */ { 0x1881 , 0x105E },
 /*  452 */ { 0x1A03 , 0x1068 },
 /*  453 */ { 0x1D06 , 0x1073 },
 /*  454 */ { 0x1C0A , 0x107D },
 /*  455 */ { 0x2507 , 0x1088 },
 /*  456 */ { 0x2401 , 0x108E },
 /*  457 */ { 0x282C , 0x1092 },
 /*  458 */ { 0x0811 , 0x109D },
 /*  459 */ { 0x2103 , 0x10AD },
 /*  460 */ { 0x2102 , 0x10B8 },
 /*  461 */ { 0x2101 , 0x10C6 },
 /*  462 */ { 0x2104 , 0x10D7 },
 /*  463 */ { 0x0848 , 0x10DD },
 /*  464 */ { 0x2216 , 0x10E5 },
 /*  465 */ { 0x082D , 0x10EF },
 /*  466 */ { 0x2A06 , 0x1100 },
 /*  467 */ { 0x1886 , 0x1108 },
 /*  468 */ { 0x2103 , 0x1113 },
 /*  469 */ { 0x2102 , 0x1116 },
 /*  470 */ { 0x2101 , 0x111A },
 /*  471 */ { 0x2A14 , 0x111F },
 /*  472 */ { 0x2A1C , 0x1125 },
 /*  473 */ { 0x2A17 , 0x112F },
 /*  474 */ { 0x2A16 , 0x113C },
 /*  475 */ { 0x2A1A , 0x1147 },
 /*  476 */ { 0x2A1D , 0x1154 },
 /*  477 */ { 0x0B0D , 0x115C },
 /*  478 */ { 0x0B07 , 0x1168 },
 /*  479 */ { 0x1B0B , 0x116F },
 /*  480 */ { 0x1882 , 0x117A },
 /*  481 */ { 0x220E , 0x1190 },
 /*  482 */ { 0x084A , 0x1198 },
 /*  483 */ { 0x0814 , 0x11A1 },
 /*  484 */ { 0x0877 , 0x11AB },
 /*  485 */ { 0x2508 , 0x11B9 },
 /*  486 */ { 0x2A12 , 0x11C8 },
 /*  487 */ { 0x2A22 , 0x11D4 },
 /*  488 */ { 0x210D , 0x11E1 },
 /*  489 */ { 0x210D , 0x11E4 },
 /*  490 */ { 0x2A0C , 0x11E8 },
 /*  491 */ { 0x0804 , 0x11EE },
 /*  492 */ { 0x0804 , 0x11F5 },
 /*  493 */ { 0x2835 , 0x1201 },
 /*  494 */ { 0x2833 , 0x120A },
 /*  495 */ { 0x283B , 0x1216 },
 /*  496 */ { 0x2215 , 0x1222 },
 /*  497 */ { 0x1C0B , 0x122C },
 /*  498 */ { 0x1E01 , 0x1238 },
 /*  499 */ { 0x210D , 0x1242 },
 /*  500 */ { 0x0B1B , 0x124E },
 /*  501 */ { 0x0812 , 0x1257 },
 /*  502 */ { 0x2A25 , 0x1261 },
 /*  503 */ { 0x2507 , 0x1266 },
 /*  504 */ { 0x2A0E , 0x1269 },
 /*  505 */ { 0x2307 , 0x126E },
 /*  506 */ { 0x1A05 , 0x127A },
 /*  507 */ { 0x1883 , 0x1280 },
 /*  508 */ { 0x087A , 0x1286 },
 /*  509 */ { 0x2601 , 0x128F },
 /*  510 */ { 0x1A04 , 0x1295 },
 /*  511 */ { 0x1905 , 0x1299 },
 /*  512 */ { 0x1D08 , 0x12A2 },
 /*  513 */ { 0x1D09 , 0x12AC },
 /*  514 */ { 0x1D0A , 0x12B6 },
 /*  515 */ { 0x1D0B , 0x12C0 },
 /*  516 */ { 0x3101 , 0x12CA },
 /*  517 */ { 0x2113 , 0x12D8 },
 /*  518 */ { 0x2112 , 0x12DB },
 /*  519 */ { 0x2111 , 0x12DF },
 /*  520 */ { 0x2110 , 0x12E4 },
 /*  521 */ { 0x210C , 0x12EA },
 /*  522 */ { 0x210E , 0x12F4 },
 /*  523 */ { 0x210E , 0x1303 },
 /*  524 */ { 0x210C , 0x1307 },
 /*  525 */ { 0x084D , 0x130B },
 /*  526 */ { 0x086A , 0x1316 },
 /*  527 */ { 0x086F , 0x131F },
 /*  528 */ { 0x2503 , 0x1326 },
 /*  529 */ { 0x2849 , 0x1333 },
 /*  530 */ { 0x260A , 0x133E },
 /*  531 */ { 0x2847 , 0x134A },
 /*  532 */ { 0x2825 , 0x1357 },
 /*  533 */ { 0x2A1F , 0x1361 },
 /*  534 */ { 0x0868 , 0x1374 },
 /*  535 */ { 0x220F , 0x137A },
 /*  536 */ { 0x0824 , 0x137E },
 /*  537 */ { 0x0825 , 0x1388 },
 /*  538 */ { 0x2A20 , 0x1392 },
 /*  539 */ { 0x220F , 0x1398 },
 /*  540 */ { 0x2114 , 0x13A0 },
 /*  541 */ { 0x2115 , 0x13A4 },
 /*  542 */ { 0x2217 , 0x13A8 },
 /*  543 */ { 0x2326 , 0x13B6 },
 /*  544 */ { 0x2404 , 0x13BC },
 /*  545 */ { 0x2305 , 0x13C6 },
 /*  546 */ { 0x0840 , 0x13D0 },
 /*  547 */ { 0x0B1C , 0x13DB },
 /*  548 */ { 0x2402 , 0x13E5 },
 /*  549 */ { 0x2116 , 0x13EF },
 /*  550 */ { 0x2837 , 0x13F2 },
 /*  551 */ { 0x2117 , 0x13FF },
 /*  552 */ { 0x2118 , 0x1403 },
 /*  553 */ { 0x2119 , 0x1408 },
 /*  554 */ { 0x2A03 , 0x140E },
 /*  555 */ { 0x2A04 , 0x141C },
 /*  556 */ { 0x1C0C , 0x1425 },
 /*  557 */ { 0x1883 , 0x142E },
 /*  558 */ { 0x0872 , 0x1435 },
 /*  559 */ { 0x2641 , 0x1440 },
 /*  560 */ { 0x2640 , 0x1449 },
 /*  561 */ { 0x2A10 , 0x1450 },
 /*  562 */ { 0x1B0C , 0x1458 },
 /*  563 */ { 0x1B0D , 0x1464 },
 /*  564 */ { 0x0830 , 0x1471 },
 /*  565 */ { 0x0831 , 0x1481 },
 /*  566 */ { 0x2211 , 0x1491 },
 /*  567 */ { 0x2213 , 0x1497 },
 /*  568 */ { 0x2212 , 0x14A1 },
 /*  569 */ { 0x2113 , 0x14A9 },
 /*  570 */ { 0x2112 , 0x14B1 },
 /*  571 */ { 0x2111 , 0x14BA },
 /*  572 */ { 0x2110 , 0x14C4 },
 /*  573 */ { 0x2113 , 0x14CF },
 /*  574 */ { 0x2112 , 0x14DB },
 /*  575 */ { 0x2111 , 0x14EC },
 /*  576 */ { 0x2110 , 0x1500 },
 /*  577 */ { 0x2114 , 0x1517 },
 /*  578 */ { 0x2115 , 0x1528 },
 /*  579 */ { 0x2114 , 0x1539 },
 /*  580 */ { 0x2115 , 0x1542 },
 /*  581 */ { 0x2116 , 0x154B },
 /*  582 */ { 0x2117 , 0x1553 },
 /*  583 */ { 0x2118 , 0x1564 },
 /*  584 */ { 0x2119 , 0x1578 },
 /*  585 */ { 0x2116 , 0x158F },
 /*  586 */ { 0x2117 , 0x159B },
 /*  587 */ { 0x2118 , 0x15A4 },
 /*  588 */ { 0x2119 , 0x15AE },
 /*  589 */ { 0x1D07 , 0x15B9 },
 /*  590 */ { 0x1C0D , 0x15C6 },
 /*  591 */ { 0x0826 , 0x15D4 },
 /*  592 */ { 0x0827 , 0x15DF },
 /*  593 */ { 0x083E , 0x15EA },
 /*  594 */ { 0x083F , 0x15F6 },
 /*  595 */ { 0x0876 , 0x1602 },
 /*  596 */ { 0x0832 , 0x160C },
 /*  597 */ { 0x0833 , 0x161A },
 /*  598 */ { 0x0836 , 0x1628 },
 /*  599 */ { 0x1C0E , 0x1633 },
 /*  600 */ { 0x0705 , 0x163F },
 /*  601 */ { 0x0708 , 0x164B },
 /*  602 */ { 0x0706 , 0x1657 },
 /*  603 */ { 0x0703 , 0x1664 },
 /*  604 */ { 0x0704 , 0x1672 },
 /*  605 */ { 0x0702 , 0x1681 },
 /*  606 */ { 0x0701 , 0x168E },
 /*  607 */ { 0x0709 , 0x169A },
 /*  608 */ { 0x0707 , 0x16A8 },
 /*  609 */ { 0x0874 , 0x16B3 },
 /*  610 */ { 0x2836 , 0x16BD },
 /*  611 */ { 0x0817 , 0x16C8 },
 /*  612 */ { 0x087C , 0x16D7 },
 /*  613 */ { 0x1D0C , 0x16E5 },
 /*  614 */ { 0x0842 , 0x16EF },
 /*  615 */ { 0x2A05 , 0x16F8 },
 /*  616 */ { 0x1E01 , 0x16FF },
 /*  617 */ { 0x1F01 , 0x1704 },
 /*  618 */ { 0x1F02 , 0x170E },
 /*  619 */ { 0x1D0D , 0x1716 },
 /*  620 */ { 0x1C0F , 0x171F },
 /*  621 */ { 0x2A0A , 0x1729 },
 /*  622 */ { 0x082F , 0x172F },
 /*  623 */ { 0x0870 , 0x1737 },
 /*  624 */ { 0x1D0E , 0x1742 },
 /*  625 */ { 0x1C10 , 0x174C },
 /*  626 */ { 0x1C11 , 0x1757 },
 /*  627 */ { 0x1C12 , 0x1761 },
 /*  628 */ { 0x2A13 , 0x176A },
 /*  629 */ { 0x0B1D , 0x176D },
 /*  630 */ { 0x2644 , 0x1778 },
 /*  631 */ { 0x2645 , 0x177E },
 /*  632 */ { 0x082C , 0x1784 },
 /*  633 */ { 0x2302 , 0x1793 },
 /*  634 */ { 0x2308 , 0x179C },
 /*  635 */ { 0x2A13 , 0x17AA },
 /*  636 */ { 0x0839 , 0x17B0 },
 /*  637 */ { 0x0838 , 0x17B9 },
 /*  638 */ { 0x083A , 0x17C1 },
 /*  639 */ { 0x080E , 0x17C6 },
 /*  640 */ { 0x2A30 , 0x17D3 },
 /*  641 */ { 0x2A33 , 0x17D8 },
 /*  642 */ { 0x2A39 , 0x17DE },
 /*  643 */ { 0x2A31 , 0x17E5 },
 /*  644 */ { 0x2A37 , 0x17EB },
 /*  645 */ { 0x2A35 , 0x17F2 },
 /*  646 */ { 0x2508 , 0x17F8 },
 /*  647 */ { 0x2508 , 0x1802 },
 /*  648 */ { 0x2A07 , 0x180B },
 /*  649 */ { 0x2A1A , 0x1811 },
 /*  650 */ { 0x2A1A , 0x181B },
 /*  651 */ { 0x2A18 , 0x1828 },
 /*  652 */ { 0x2609 , 0x1830 },
 /*  653 */ { 0x2505 , 0x1838 },
 /*  654 */ { 0x080B , 0x1848 },
 /*  655 */ { 0x283A , 0x1853 },
 /*  656 */ { 0x0829 , 0x185D },
 /*  657 */ { 0x0828 , 0x1863 },
 /*  658 */ { 0x0B1E , 0x186A },
 /*  659 */ { 0x0B1F , 0x1871 },
 /*  660 */ { 0x1B0E , 0x1876 },
 /*  661 */ { 0x1D0F , 0x1881 },
 /*  662 */ { 0x1C13 , 0x188B },
 /*  663 */ { 0x0835 , 0x1896 },
 /*  664 */ { 0x1F04 , 0x18A0 },
 /*  665 */ { 0x2210 , 0x18A6 },
 /*  666 */ { 0x2210 , 0x18A9 },
 /*  667 */ { 0x084E , 0x18AE },
 /*  668 */ { 0x2309 , 0x18B6 },
 /*  669 */ { 0x1B0F , 0x18C0 },
 /*  670 */ { 0x0873 , 0x18CA },
 /*  671 */ { 0x230A , 0x18D4 },
 /*  672 */ { 0x1D10 , 0x18DE },
 /*  673 */ { 0x1C14 , 0x18E8 },
 /*  674 */ { 0x1B10 , 0x18F3 },
 /*  675 */ { 0x1884 , 0x1900 },
 /*  676 */ { 0x2646 , 0x1915 },
 /*  677 */ { 0x2A36 , 0x191B },
 /*  678 */ { 0x080D , 0x1921 },
 /*  679 */ { 0x2A25 , 0x192B },
            { 0x0000 , 0x0000 }
};

static unsigned short rev[] = {
 /*    0 */  606,  605,  603,  604,  600,  602,  608,  601,  607,  150,  195,  245,  329,  492,  208,  317,
 /*   16 */  212,  200,  297,  408,  654,  388,  678,  639,  241,  313,  458,  501,  209,  483,  145,  316,
 /*   32 */  611,  151,  152,  248,  247,  249,  429,  237,  306,  146,  242,  243,  273,  536,  537,  591,
 /*   48 */  592,  657,  656,  201,  224,  632,  465,  428,  622,  564,  565,  596,  597,  205,  663,  598,
 /*   64 */  427,  637,  636,  638,  412,  272,  191,  593,  594,  546,  160,  614,  176,  419,  253,  182,
 /*   80 */  210,  463,  264,  482,  450,  190,  525,  667,  420,  345,  346,  347,  348,  349,  350,  351,
 /*   96 */  352,  432,  433,  434,  435,  436,  437,  438,  439,  274,  275,  276,  277,  278,  279,  280,
 /*  112 */  281,  534,  175,  526,  341,  340,  174,  261,  527,  623,  157,  558,  670,  609,  395,  595,
 /*  128 */  484,  305,  193,  508,  188,  612,  318,  167,  307,  155,  159,  162,  302,  344,  364,  478,
 /*  144 */  154,  158,  163,  343,  405,  477,  156,  161,  164,  166,  199,  215,  218,  219,  220,  221,
 /*  160 */  222,  295,  303,  500,  547,  629,  658,  659,  396,  385,  403,  451,  480,  507,  675,  235,
 /*  176 */  467,  177,  178,  392,  393,  511,  367,  389,  452,  510,  506,  223,  254,  255,  271,  409,
 /*  192 */  391,  300,  333,  342,  421,  479,  562,  563,  660,  669,  674,  189,  217,  331,  334,  362,
 /*  208 */  410,  448,  449,  454,  497,  556,  590,  599,  620,  625,  626,  627,  662,  673,  165,  216,
 /*  224 */  293,  332,  453,  589,  512,  513,  514,  515,  613,  619,  624,  661,  672,  616,  138,  143,
 /*  240 */  139,  137,  142,  140,  187,  252,  416,  664,  470,  469,  468,  430,  406,  400,  256,  258,
 /*  256 */  259,  260,  270,  524,  489,  523,  417,  520,  519,  518,  517,  540,  541,  549,  551,  552,
 /*  272 */  553,  228,  132,  133,  236,  135,  136,  296,  125,  126,  127,  128,  129,  168,  170,  169,
 /*  288 */  171,  173,  172,  418,  309,  431,  535,  665,  566,  568,  567,  384,  383,  464,  179,  299,
 /*  304 */  633,  310,  315,  545,  397,  505,  634,  668,  671,  543,  324,  234,  548,  544,  192,  257,
 /*  320 */  528,  368,  653,  294,  198,  485,  301,  509,  214,  230,  229,  232,  231,  233,  262,  652,
 /*  336 */  530,  394,  370,  130,  131,  186,  298,  338,  560,  559,  184,  185,  630,  631,  676,  147,
 /*  352 */  181,  532,  153,  314,  250,  376,  213,  321,  457,  379,  425,  377,  325,  226,  323,  494,
 /*  368 */  204,  493,  610,  550,  225,  227,  655,  495,  319,  373,  413,  424,  374,  322,  378,  320,
 /*  384 */  372,  197,  386,  531,  371,  529,  369,  211,  326,  380,  411,  423,  414,  426,  144,  387,
 /*  400 */  554,  555,  615,  466,  648,  238,  263,  621,  365,  490,  382,  504,  422,  561,  486,  635,
 /*  416 */  471,  404,  474,  473,  651,  240,  649,  239,  472,  476,  366,  533,  538,  487,  502,  640,
 /*  432 */  643,  196,  641,    1,  645,  677,  644,  183,  642,    0,   26,   32,   33,   27,   30,   28,
 /*  448 */   31,   25,   29,  402,   94,  118,   90,   91,  100,   92,  123,  121,   93,  112,  113,   96,
 /*  464 */  109,   97,   88,   98,   89,  111,  110,  107,  108,  103,  102,  101,  104,  106,  105,   99,
 /*  480 */  117,  115,  116,  114,   87,  120,   95,  119,  122,   17,   81,   78,   59,   45,   76,    5,
 /*  496 */    2,   71,    6,   13,   16,   75,   20,   18,   19,   63,   61,   62,    3,    4,   80,   41,
 /*  512 */   37,   38,   39,   40,   69,   50,   53,   67,    8,    7,   44,   60,   52,   77,   66,   47,
 /*  528 */   22,   68,   48,   72,   11,   36,   15,   42,   24,  0xFFFF };

#define ABC_NUMREV 537
#endif
#endif
#ifndef _ABCP__H_
#define _ABCP__H_

#include "dbg.h"
#include "yrx.h"
#include <stdlib.h>

/* .% Windows DLL definitions
  Only function declared as '|ABCAPI| will be exported
*/

#ifdef ABC_DLL
#include <windows.h>
#define ABCAPI __declspec(dllexport)
#else
#define ABCAPI
#endif


#define abcEmptyString ((char *)abcEMPTYSTR)
/* -- FUNCTIONS */

#define abcNote(_s,_f)         /* abcTNote        */ (_s)->val.note._f               
#define abcRest(_s,_f)         /* abcTRest        */ (_s)->val.rest._f               
#define abcSpacer(_s,_f)       /* abcTSpacer      */ (_s)->val.spacer._f             
#define abcTuplet(_s,_f)       /* abcTTuplet      */ (_s)->val.tuplet._f             
#define abcGChord(_s,_f)       /* abcTGChord      */ (_s)->val.gchord._f             
#define abcBroken(_s,_f)       /* abcTBroken      */ (_s)->val.broken._f             
#define abcAnnotation(_s,_f)   /* abcTAnnotation  */ (_s)->val.annotation._f         
#define abcDecoration(_s,_f)   /* abcTDecoration  */ (_s)->val.decoration._f         
#define abcLyrics(_s,_f)       /* abcTLyrics      */ (_s)->val.lyrics._f             
#define abcComment(_s,_f)      /* abcTComment     */ (_s)->val.comment._f            
#define abcSlur(_s,_f)         /* abcTSlur        */ (_s)->val.slur._f               
#define abcTie(_s,_f)          /* abcTSlur        */ (_s)->val.tie._f                
#define abcBar(_s,_f)          /* abcTBar         */ (_s)->val.bar._f                
#define abcEnding(_s,_f)       /* abcTEnding      */ (_s)->val.ending._f             
#define abcLength(_s,_f)       /* abcTFieldLength */ (_s)->val.field_L._f            
#define abcMeter(_s,_f)        /* abcTFieldMeter  */ (_s)->val.field_M._f            
#define abcTempo(_s,_f)        /* abcTFieldTempo  */ (_s)->val.field_Q._f            
#define abcKey(_s,_f)          /* abcTVK          */ (_s)->val.field_K._f            
#define abcVoice(_s,_f)        /* abcTVK          */ (_s)->val.field_V._f            
#define abcXField(_s,_f)       /* abcTXField      */ (_s)->val.Xfield._f             
#define abcField(_s,_f)        /* abcTFieldVoid   */ (_s)->val.field._f              
#define abcSpace(_s,_f)        /* abcTSpace       */ (_s)->val.space._f              
#define abcInclude(_s,_f)      /* abcTXInclude    */ (_s)->val.Xinclude._f           
#define abcText(_s,_f)         /* abcTText        */ (_s)->val.text._f               
#define abcMidi(_s,_f)         /* abcTXMidi       */ (_s)->val.midi._f               
#define abcDefine(_s,_f)       /* abcTDefine      */ (_s)->val.def._f 
#define abcPercussion(_s,_f)   /* abcTXDrum       */ (_s)->val.drum._f
#define abcRhythm(_s,_f)       /* abcTFieldRhythm */ (_s)->val.field_R._f            
#define abcRhythmTempo(_s,_f)  /* abcTFieldTempo  */ (_s)->val.field_R.tempo._f            
#define abcRhythmMeter(_s,_f)  /* abcTFieldMeter  */ (_s)->val.field_R.meter._f     
       
/* -- END FUNCTIONS */

#define abcStream(_s)        ((_s)->stream)
#define abcCursor(_s)        (abcStream(_s)->cursor)
#define abcState(_s)         ((_s)->state)
#define abcToken(_s)         ((_s)->token)
#define abcTokenLen(_s)      ((_s)->toklen)
#define abcTokenStart(_s)    ((_s)->tokstart)


typedef struct {
 /* I */   char            error   ;
 /* A */   unsigned char   pitch   ; /* A-G */
 /* I */   short           bend    ; /* */  
 /* A */   unsigned char   bending ; /* '\0' = no 'p' = cautionary 'b' = bend */
 /* I */   char            octave  ; /* -10 .. +10 */
 /* I */   unsigned short  duration;
} abcTNote;

typedef struct {
 /* I */ char error;
 /* A */ char type;
 /* I */ unsigned short  duration;
} abcTRest;

typedef struct {
 /* I */ char error;
 /* A */ char unit;
 /* I */ unsigned short space;
} abcTSpacer;

typedef struct {
 /* I */ char error;
 /* I */ unsigned char notes;
 /* I */ unsigned char time;
 /* I */ unsigned char span;
} abcTTuplet;

typedef struct {
 /* I */ char error;
 /* I */ unsigned char type;
 /* I */ unsigned char dots;
} abcTBroken;

typedef struct {
 /* I */ char error;
 /* I */ char pad;
 /* I */ unsigned short spaces;
 /* I */ unsigned short verse;
 /* $ */ unsigned char *syllable;
 /* - */ unsigned short syllable_len; 
} abcTLyrics;

typedef struct {
 /* I */ char error;
 /* A */ unsigned char  rootnote;
 /* A */ unsigned char  rootaccidental;
 /* A */ unsigned char  bassnote;
 /* A */ unsigned char  bassaccidental;
 /* $ */ unsigned char *string;
 /* - */ unsigned short string_len;
 /* $ */ unsigned char *name;
 /* - */ unsigned short name_len;
 /* - */ unsigned char  name_buf[20];
} abcTGChord;

typedef struct {
 /* I */ char error;
 /* I */ unsigned char  position;
 /* I */ short          xoffset;
 /* I */ short          yoffset;
 /* - */ unsigned short string_len;
 /* $ */ unsigned char *string;
} abcTAnnotation;

typedef struct {
 /* I */ char error;
 /* I */ unsigned char  type;
 /* - */ unsigned short string_len;
 /* $ */ unsigned char *string;
} abcTComment;

typedef struct {
 /* I */ char error;
 /* A */ unsigned char type;
 /* A */ unsigned char direction;
 /* B */ unsigned char isDotted;
} abcTSlur;

typedef struct {
 /* I */ char error;
 /* I */ unsigned char  type;
 /* - */ unsigned short string_len;
 /* $ */ unsigned char *string;
 /* I */ unsigned short ends;
 /* I */ unsigned short starts;
} abcTBar;

typedef struct {
 /* I */ char error;
 /* I */ unsigned char  num;
} abcTEnding;

typedef struct {
 /* I */ char error;
 /* I */ char           braces;
 /* & */ unsigned short code;
 /* I */ unsigned short spaces;
 /* I */ unsigned short verse;
 /* $ */ unsigned char *string;
 /* - */ unsigned short string_len;
 /* A */ unsigned char  class;
} abcTDecoration;

typedef struct {
 /* I */ char error;
 /* A */ unsigned char  type;
 /* I */ unsigned short spaces;
} abcTSpace;

typedef struct {
 /* I */ char error;
 /* A */ unsigned char  type;
 /* - */ unsigned short string_len;
 /* $ */ unsigned char *string;
} abcTText;


typedef struct {
 /* I */ char error;
 /* A */ char field; 
 /* - */ unsigned short string_len;
 /* $ */ unsigned char *string;
} abcTFieldVoid;

typedef struct {
 /* I */ char error;
 /* A */ char field;
 /* - */ unsigned short  string_len;
 /* $ */ unsigned char  *string;
 /* I */ unsigned short  duration;
} abcTFieldLength;

typedef struct {
 /* I */ char error;
 /* A */ char field;
 /* - */ unsigned short string_len;
 /* $ */ unsigned char *string;
 /* I */ unsigned char  num;
 /* I */ unsigned char  den;
 /* I */ unsigned char  playNum;
 /* I */ unsigned char  playDen;
 /* I */ unsigned char  brace;  
 /* I */ unsigned char  numAdds[11];
 /* B */ unsigned char  isCommon;
 /* B */ unsigned char  isCut;
 /* B */ unsigned char  isFree;
} abcTFieldMeter;

typedef struct {
 /* I */ char             error;
 /* A */ char             field;
 /* - */ unsigned short   string_len;
 /* $ */ unsigned char   *string;
 /* $ */ unsigned char   *mark;
 /* - */ unsigned short   mark_len;
 /* I */ unsigned short   code;
 /* I */ unsigned short   beats;  
 /* I */ unsigned short   note;
 /* I */ unsigned short   notes[5];
 /* B */ unsigned char    isRelative;
} abcTFieldTempo;

#define ABC_RHYTHM_MAXPAIRS 16

typedef struct {
 /* I */ char             error;
 /* A */ char             field;
 /* - */ unsigned short   string_len;
 /* $ */ unsigned char   *string;
 /* - */ unsigned short   name_len;
 /* $ */ unsigned char   *name;
 /* - */ abcTFieldMeter   meter;
 /* - */ abcTFieldTempo   tempo;
 /* B */ unsigned char    hasMeter;
 /* B */ unsigned char    hasTempo;
 /* I */ unsigned short   pairs;
 /* I */ unsigned char    velocity[16];
 /* I */ unsigned char    duration[16]; 
} abcTFieldRhythm;

typedef struct {
 /* I */ char            error;
 /* A */ char            field;
 /* - */ unsigned short  string_len;
 /* $ */ unsigned char  *string;
 /* A */ unsigned char   tonic;
 /* A */ unsigned char   accidentals;
 /* A */ unsigned char   mode;
 /* A */ unsigned char   signature[7];
 /* $ */ unsigned char  *id;
 /* - */ unsigned short  id_len;
 /* $ */ unsigned char  *name;
 /* - */ unsigned short  name_len;
 /* $ */ unsigned char  *shortName;
 /* - */ unsigned short  shortName_len;
 /* $ */ unsigned char  *instr;
 /* - */ unsigned short  instr_len;
 /* & */ unsigned short  instrCode;
 /* $ */ unsigned char  *head;
 /* - */ unsigned short  head_len;
 /* & */ unsigned short  headCode;
 /* & */ unsigned short  clef;
 /* I */ unsigned char   clefline;
 /* I */ char            stvGrouped;
 /* A */ unsigned char   stvGroupType;
 /* I */ char            stvSpace;
 /* I */ char            octaves; 
 /* I */ char            transpose;
 /* I */ unsigned char   staffLines;
 /* I */ unsigned char   middlePitch;
 /* I */ char            middleOctave;
 /* A */ char            stems;
 /* A */ char            gStems;
 /* A */ char            marks;
 /* A */ char            lyrics;
} abcTVK;

#define abcTFieldKey   abcTVK
#define abcTFieldVoice abcTVK
#define abcTClef abcTVK

typedef struct {
  char error;
} abcTVal;

typedef struct {
 /* I */ char error;
 /* - */ char pad;
 /* & */ unsigned short code;
 /* $ */ unsigned char *key;
 /* - */ unsigned short key_len;
 /* $ */ unsigned char *value;
 /* - */ unsigned short value_len;
} abcTXField;

typedef struct {
 /* I */ char error;
 /* - */ char pad;
 /* I */ unsigned short code;
 /* $ */ unsigned char *key;
 /* - */ unsigned short key_len;
 /* $ */ unsigned char *filename;
 /* - */ unsigned short filename_len;
} abcTXInclude;

typedef struct {
 /* I */ char             error;
 /* I */ unsigned char    midiPitch;
 /* & */ unsigned short   instrCode;
 /* $ */ unsigned char   *instr;
 /* - */ unsigned short   instr_len;
 /* & */ unsigned short   headCode;
 /* $ */ unsigned char   *head;
 /* - */ unsigned short   head_len;
 /* A */ unsigned char    notePitch;
 /* A */ unsigned char    accidentals; /* FfnNsS */
 /* I */ unsigned char    octave;
 /* A */ unsigned char    stem; /* UDN */
} abcTXDrum;

typedef struct {
 /* I */ char             error;
 /* I */ unsigned char    argc;
 /* & */ unsigned short   cmd;
 /* $ */ unsigned char   *cmdstr;
 /* - */ unsigned short   cmdstr_len;
 /* $ */ unsigned char   *argsA;
 /* - */ unsigned short   argsA_len;
 /* $ */ unsigned char   *argsB;
 /* - */ unsigned short   argsB_len;
 /* I */          char    argv[20];
} abcTXMidi;

typedef struct {
 /* I */ char error;
 /* A */ unsigned char   type;
 /* $ */ unsigned char  *term;
 /* - */ unsigned short  term_len;
 /* $ */ unsigned char  *definition;
 /* - */ unsigned short  definition_len;
} abcTDefine;

typedef  union {
  abcTVal          val;
  abcTNote         note;         
  abcTRest         rest;         
  abcTSpacer       spacer;       
  abcTSpace        space;        
  abcTTuplet       tuplet;       
  abcTGChord       gchord;       
  abcTBroken       broken;       
  abcTAnnotation   annotation;   
  abcTDecoration   decoration;   
  abcTLyrics       lyrics;       
  abcTComment      comment;      
  abcTSlur         slur;         
  abcTSlur         tie;          
  abcTBar          bar;          
  abcTEnding       ending;       
  abcTFieldVoid    field;        
  abcTFieldLength  field_L;      
  abcTFieldMeter   field_M;      
  abcTFieldTempo   field_Q;      
  abcTFieldKey     field_K;      
  abcTFieldVoice   field_V;
  abcTFieldRhythm  field_R;      
  abcTXField       Xfield;       
  abcTXInclude     Xinclude;
  abcTXMidi        midi;     
  abcTText         text; 
  abcTDefine       def;      
  abcTXDrum        drum;  
}  abcTValues;            

/* 960 = 2^6 * 3 * 5 */
#define abcTICKS 960

#define MAX_NESTED_INC 8

typedef struct {
  unsigned short   _guard;
  unsigned short   token;
  unsigned short   state;
  unsigned short   nextstate;
  unsigned short   oldstate;
  unsigned short   toklen;
  unsigned char   *tokstart;
  unsigned short   stream_ndx;
  unsigned short   states[MAX_NESTED_INC];
  YYSTREAM        *streams[MAX_NESTED_INC];
  YYSTREAM        *stream;
  abcTValues       val;
} abcScanner;


/* .% Scanner states
*/
#define S_EOF                 0x0000
#define S_ERROR               0x0001
#define S_LIMBO               0x0002
#define S_NONE                0x0003
#define S_EATSOL              0x0004
#define S_TUNE                0x8000
#define S_TUNE_G              0x8001  /* in grace */
#define S_TUNE_C              0x8002  /* in a chord */
#define S_FIELD               0x1000  
#define S_EXTFIELD            0x2000
#define S_INFIELD             0x4000
#define S_CLEARIN             0xBFFF  /* */
#define S_ANYFIELD            0x7000
#define S_CFIELD              0x5000  /* A "colon" field like A: or [A:]*/
#define S_INCLUDE             0x0010
#define S_TEXT                0x0011

/* -- BEGIN TOKENS  */

#define T_FIRST               0x0000 
#define T_UNKNOWN             0x0001 
#define T_NONE                0x0002 
#define T_EOF                 0x0003 
#define T_COMMENT             0x0005 
#define T_EMPTYLINE           0x0006 
#define T_EXTFIELD            0x0007 
#define T_FIELD               0x0008 
#define T_NOTE                0x0009 
#define T_SPACE               0x000A 
#define T_BAR                 0x000B 
#define T_INFIELD             0x000C 
#define T_TEXT                0x000D 
#define T_ENDLINE             0x000E 
#define T_CONTINUE            0x000F 
#define T_DECORATION          0x0010 
#define T_REST                0x0011 
#define T_INVREST             0x0012 
#define T_MULTIREST           0x0013 
#define T_SPACER              0x0014 
#define T_TUPLET              0x0015 
#define T_SLURSTART           0x0016 
#define T_SLUREND             0x0017 
#define T_BROKENRIGHT         0x0018 
#define T_BROKENLEFT          0x0019 
#define T_CHORDSTART          0x001A 
#define T_CHORDEND            0x001B 
#define T_GCHORD              0x001C 
#define T_REPEAT              0x001D 
#define T_ACCIACATURA         0x001E 
#define T_APPOGGIATURA        0x001F 
#define T_GRACEEND            0x0020 
#define T_TIE                 0x0021 
#define T_STRING              0x0022 
#define T_STRINGUNF           0x0023 
#define T_ANNOTATION          0x0024 
#define T_BREAK               0x0025 
#define T_STARTLINE           0x0026 
#define T_FIELDB              0x0027 
#define T_PRAGMA              0x0028 
#define T_INCLUDE             0x0029 
#define T_INCLUDE_EOF         0x002A 
#define T_ENDING              0x002B 
                                     
#define T_XFIELD              0x002D 
                                     
#define T_LYR_SYLLABLE        0x002F 
#define T_LYR_BLANK           0x0030 
#define T_LYR_BAR             0x0031 
#define T_LYR_SPACE           0x0032 
#define T_LYR_HOLD            0x0033 
#define T_LYR_VERSE           0x0034 
#define T_LYR_CONTINUE        0x0035 
                                     
#define T_TIE_DOT             0x0037 
#define T_DUPMESURE           0x0038 
                                     
#define T_SYM_BLANK           0x003A 
#define T_SYM_BAR             0x003B 
#define T_SYM_VERSE           0x003C 
#define T_SYM_CONTINUE        0x003D 
#define T_SYM_GCHORD          0x003E 
#define T_SYM_DECORATION      0x003F 
#define T_SYM_ANNOTATION      0x0040 
                                     
#define T_BSPACE              0x0041 
#define T_DOTLEFT2            0x0042 
#define T_DOTRIGHT2           0x0043 
#define T_LAST                0x0044 
                                     
#define T_OVERLAY             0x0046 
#define T_OVLEND              0x0047 
#define T_OVLSTART            0x0048 
#define T_DOVERLAY            0x0049 
#define T_DOVLEND             0x004A 
#define T_DOVLSTART           0x004B 

#define T_BEGINTEXT           0x0050
#define T_ENDTEXT             0x0051   

#define T_MIDI                0x0052
#define T_DEFINE              0x0053
#define T_SETDRUM             0x0054
                          
/* -- END TOKENS  */

#define abc_isInField(_s)   ((_s->state) & S_INFIELD)
#define abc_setInField(_s)  ((_s->state) |= S_INFIELD)
#define abc_clrInField(_s)  ((_s->state) &= ~S_INFIELD)

int abc_isFieldEnd(abcScanner *s) ;
void abc_gotoFieldEnd(abcScanner *s);

#define abc_FieldEnd(_s)    (abc_isInField(_s)?']':YYEOL)

#define abc_SetToken(s,t)        ((s)->toklen=YYLEN(0),(s)->tokstart=YYSTART(0),(s)->token=t)
#define abc_SetEmptyToken(s,t)   ((s)->toklen=0,(s)->tokstart=abcEmptyString,(s)->token=t)
                          
#define abc_eatSOL(s) do {if (*((s)->cursor) == '\0')  yygetline(s);\
                          if (*((s)->cursor) == YYSOL) (s)->cursor +=1;} while (0)

#define abc_ZEROVAL(a)  memset(&((a)->val),0,sizeof(abcTValues))


int abcSField      (abcScanner *s);
int abcSLimbo      (abcScanner *s);
int abcSTune       (abcScanner *s);
int abcSExtField   (abcScanner *s);
int abcSWords      (abcScanner *s);
int abcSSymbols    (abcScanner *s);
int abcSText       (abcScanner *s);
                   
int abc_MIDI       (abcScanner *s);
int abc_Note       (abcScanner *s);
int abc_Chord      (abcScanner *s,unsigned char *start, unsigned short length);
int abc_Rest       (abcScanner *s,char *start);
int abc_Field      (abcScanner *s,unsigned char *start, char field,unsigned short tok);
int abc_Annotation (abcScanner *s);
int abc_Bar        (abcScanner *s, unsigned char *st,unsigned short ln);
int abc_setdrum    (abcScanner *s);

YYSTREAM *abc_addstream (abcScanner *s, unsigned char *filename, 
                                        unsigned char *string,
                                        unsigned long bufsize);
YYSTREAM *abc_delstream (abcScanner *s);

#define abc_addfilestream(_s,_f,_n)  abc_addstream(_s,_f,NULL,_n)
#define abc_addstringstream(_s,_t)   abc_addstream(_s,NULL,_t,0)


#ifdef ABC_USEDLL
#define _L (*
#define _R )
#else 
#define _L ABCAPI
#define _R 
#endif


unsigned short  _L abc_ChkString    _R  (char *str);
unsigned short  _L abc_ChkNString   _R  (char *str,int len);
char          * _L abc_RevString    _R  (unsigned short code);

const char    * _L abcTokString     _R  (abcScanner *s);

unsigned short  _L abcNextToken     _R  (abcScanner *s);
abcScanner    * _L abcFileScanner   _R  (char *filename,unsigned long bufsize);
abcScanner    * _L abcStringScanner _R  (char *string);
void            _L abcClose         _R  (abcScanner *s);
unsigned int    _L abcLineNum       _R  (abcScanner *s);
const char    * _L abcFileName      _R  (abcScanner *s);
void          * _L abcVal           _R  (abcScanner *s);
int             _L abcStopInclude   _R  (abcScanner *s);


#define abcIsScanner(_s) ((_s)->_guard == 0xF00D)

#ifndef _ABC_STRINGS_H
#include "abcp_strings.h"
#endif

#ifdef NOEXTERN
  const char *abcEMPTYSTR="";
  unsigned char cl[ABC_MAXLEN_STR+10];
#else
  extern const char *abcEMPTYSTR;
  extern unsigned char cl[];
#endif



#if defined(ABC_STR_INSTRUMENT)
unsigned short abc_getinstrument(unsigned char *i, unsigned short l);
#else
  #define abc_getinstrument(i,l) 0
#endif


#endif /*_ABCP__H_ */
#endif /*_ABCP_H_ */
