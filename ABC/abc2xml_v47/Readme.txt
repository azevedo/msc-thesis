---- abc2xml ----

abc2xml is a command line utility that translates ABC notation into MusicXML.

In principle all elements from ABC are translated, but many translations are only partially
implemented. Translated are:
- multiple lyric lines per voice
- mapping voices to staves, brackets and braces (%%score or %%staves)
- voice overlays (only single &-overlays, no multiple &&-overlays yet)
- dynamics, slurs, several decorations
- grace notes, tuplets
- keys (maj, min, mix, dor, phr, lyd, loc and none), meter, tempo
- clefs (only most common clef names, tranposition=  and middle= are supported)
- jazz chord symbols and text annotations
- beaming. Only one beam per abc-beam group is translated at the moment, which is
sufficient for MuseScore. In musicXML every beam should be explicitly notated, so a 32th
note should start 3 beams.
- scale, page size and page margins are recognized as either %% or I: directive. The scale value is
the distance between two stafflines in millimeters. The other values are also in millimeters unless
they are followed by a unit (cm,in,pt).
- %%MIDI program and %%MIDI channel (or I:MIDI ..) are translated when used in a current voice
(i.e. after a V: definition). The instrument of a voice cannot be changed in the middle of a tune.
If one does so, only the last %%MIDI will be used for the whole voice. (and the earlier settings are
discarded). %%MIDI transpose is translated and has the same effect as transpose= in the clef, i.e.
only play back is transposed, not notation. In %%MIDI program=num, the number should be between 0
and 127. Also in %%MIDI channel=num, the number is starts from zero. The midi translation supports
mapping multiple voices whith different instruments to one stave. (i.e. the resulting xml part will
have multiple instruments). This feature, though present in MusicXML is not supported by MuseScore,
nor by Finale Notepad. These programs only allow one instrument per stave.
- multiple tunes within one abc file can be converted to a set of xml files, one file per tune.

In conjunction with xml2abc the translation from xml -> abc -> xml works for all examples
in the set from MakeMusic. The translated examples produce reasonable score when typeset with MuseScore.

---- Usage: ----

When you have Python installed:
> python abc2xml.py [-h] [-m SKIP NUM] [-o DIR] [-p PFMT] file1 [file2 ...]

When you use the Win32 executable:
> abc2xml.exe [-h] [-m SKIP NUM] [-o DIR] [-p PFMT] file1 [file2 ...]

Translates all .abc files in the file list to MusicXML. Output goes to stdout unless the -o option
is given. Wildcards in file names are expanded.
Option -h prints help message with explanation of the options
Option -m skip num skips skip tunes and then reads at most num tunes.
Can be used when abc files contain multiple tunes (tune books) to select only a subset of the tunes.
The default skips nothing (skip=0) and reads 1 tune (num=1).
Option -o dir translates every .abc file to a separate .xml file with the same name
into directory dir. For example, -o. puts all xml files into the same directory where
the input files reside.
Option -p fmt sets the page format of the ouput. fmt should be a string with 7 float
values sepatated by comma's without any spaces. The values are: space, page-height, -width, and
page margin left, -right, -top, -bottom. space defines the scale of the whole score and equals the
distance between two staff lines in mm. When the -p option is omitted the values default to A4 with
10mm margins and space=1.764. All commandline values are in millimeters.

---- Download ----

The python script: abc2xml.py-47.zip
http://wim.vree.org/svgParse/abc2xml.py-47.zip

Stand alone win32 executable: abc2xml.exe-47.zip
http://wim.vree.org/svgParse/abc2xml.exe-47.zip

---- ABC Syntax ----

The syntax of ABC is recognized by a parser that uses a quite readable (python) notation to describe
a formal ABC syntax. The drawback of this approach is that many dialects of ABC will cause sytax errors.
In addition, the high level parser implementation in pure python (pyparsing.py) is slow.
The advantage, however, is a concise, readable and thus easily changable ABC-syntax recognition,
which might also be ported to other programs.
The pyparsing library is included (as a single python file) in abc2xml.py-47.zip