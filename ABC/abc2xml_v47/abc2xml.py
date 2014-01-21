# coding=latin-1
'''
Copyright (C) 2012: Willem G. Vree
Contributions: Nils Liberg, Nicolas Froment, Norman Schmidt, Reinier Maliepaard

This program is free software; you can redistribute it and/or modify it under the terms of the
GNU General Public License as published by the Free Software Foundation; either version 2 of
the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
See the GNU General Public License for more details. <http://www.gnu.org/licenses/gpl.html>.
'''

from pyparsing import Word, OneOrMore, Optional, Or, Literal, NotAny
from pyparsing import Group, oneOf, Suppress, ZeroOrMore, Combine, FollowedBy
from pyparsing import srange, CharsNotIn, StringEnd, LineEnd, White, Regex
from pyparsing import nums, alphas, alphanums, ParseException
try:    import xml.etree.cElementTree as E
except: import xml.etree.ElementTree as E
import types, sys, os, re, datetime

VERSION = 47

def info (s, warn=1):
    x = (warn and '-- ' or '') + s
    try: sys.stderr.write (x + '\n')
    except: sys.stderr.write (repr (x) + '\n')

def abc_grammar ():     # header, voice and lyrics grammar for ABC
    b1 = Word ("-,'<>", exact=1)  # catch misplaced chars in chords

    #-----------------------------------------------------------------
    # ABC header (fld_text elements are matched later with reg. epr's)
    #-----------------------------------------------------------------

    number = Word (nums).setParseAction (lambda t: int (t[0]))
    field_str = Regex (r'(?:\\.|[^]\\])*')  # match anything until end of field, skip escaped \]
    field_str.setParseAction (lambda t: t[0].strip ())  # and strip spacing

    userdef_symbol  = Word (srange ('[H-Wh-w~]'), exact=1)
    fieldId = oneOf ('K L M Q P I T C O A Z N G H R B D F S E r') # info fields
    X_field = Literal ('X') + Suppress (':') + number + field_str
    U_field = Literal ('U') + Suppress (':') + userdef_symbol + Suppress ('=') + field_str
    V_field = Literal ('V') + Suppress (':') + Word (alphanums + '_') + field_str
    inf_fld = fieldId + Suppress (':') + field_str
    ifield = Suppress ('[') + (X_field | U_field | V_field | inf_fld) + Suppress (']')
    abc_header = OneOrMore (ifield) + StringEnd ()

    #---------------------------------------------------------------------------------
    # ABC voice (not white space sensitive, beams detected in note/rest parse actions)
    #---------------------------------------------------------------------------------

    inline_field =  Suppress ('[') + (inf_fld | U_field | V_field) + Suppress (']')

    note_length = Optional (number, 1) + Group (ZeroOrMore ('/')) + Optional (number, 2)
    octaveHigh = OneOrMore ("'").setParseAction (lambda t: len(t))
    octaveLow = OneOrMore (',').setParseAction (lambda t: -len(t))
    octave  = octaveHigh | octaveLow

    basenote = oneOf ('C D E F G A B c d e f g a b y')  # includes spacer for parse efficiency
    accidental = oneOf ('^^ __ ^ _ =')
    rest_sym  = oneOf ('x X z Z')
    slur_beg = oneOf ('( .(') + ~Word (nums)    # no tuplet_start
    slur_ends = OneOrMore (oneOf (') .)'))

    long_decoration = Combine (oneOf ('! +') + CharsNotIn ('!+ \n') + oneOf ('! +'))
    staccato        = Literal ('.') + ~Literal ('|')    # avoid dotted barline
    decoration      = staccato | userdef_symbol | long_decoration | slur_beg
    decorations     = OneOrMore (decoration)
    staff_decos     = decorations + ~oneOf (': | [|] []')

    tie = oneOf ('.- -')
    rest = Optional (accidental) + rest_sym + note_length
    pitch = Optional (accidental) + basenote + Optional (octave, 0)
    note = pitch + note_length + Optional (tie) + Optional (slur_ends)
    chord_note = Optional (decorations) + pitch + note_length + Optional (tie) + Optional (slur_ends)
    chord = Suppress ('[') + OneOrMore (chord_note | rest | b1) + Suppress (']') + note_length + Optional (tie) + Optional (slur_ends)
    stem = note | chord | rest

    broken = Combine (OneOrMore ('<') | OneOrMore ('>'))

    tuplet_num   = Suppress ('(') + number
    tuplet_into  = Suppress (':') + Optional (number, 0)
    tuplet_notes = Suppress (':') + Optional (number, 0)
    tuplet_start = tuplet_num + Optional (tuplet_into + Optional (tuplet_notes))

    acciaccatura    = Literal ('/')
    grace_stem      = Group (Optional (decorations) + stem)
    grace_notes     = Suppress ('{') + Optional (acciaccatura) + OneOrMore (grace_stem) + Suppress ('}')

    text_expression  = Optional (oneOf ('^ _ < > @'), '^') + Optional (CharsNotIn ('"'), "")
    chord_accidental = oneOf ('# b =')
    triad            = oneOf ('ma Maj maj M mi min m aug dim o + -')
    seventh          = oneOf ('7 ma7 Maj7 M7 maj7 mi7 m7 dim7 o7 -7 aug7 +7 m7b5 mi7b5')
    sixth            = oneOf ('6 ma6 M6 m6 mi6')
    ninth            = oneOf ('9 ma9 M9 maj9 Maj9 mi9 m9')
    elevn            = oneOf ('11 ma11 M11 maj11 Maj11 mi m11')
    suspended        = oneOf ('sus sus2 sus4')
    chord_degree     = Combine (Optional (chord_accidental) + oneOf ('2 4 5 6 7 9 11 13'))
    chord_kind       = Optional (seventh | sixth | ninth | elevn | triad, '_') + Optional (suspended)
    chord_root       = oneOf ('C D E F G A B') + Optional (chord_accidental)
    chord_bass       = oneOf ('C D E F G A B') + Optional (chord_accidental) # needs a different parse action
    chordsym         = chord_root + chord_kind + ZeroOrMore (chord_degree) + Optional (Suppress ('/') + chord_bass)
    chord_sym        = chordsym + Optional (Literal ('(') + CharsNotIn (')') + Literal (')')).suppress ()
    chord_or_text    = Suppress ('"') + (chord_sym ^ text_expression) + Suppress ('"')

    volta_nums = Optional ('[').suppress () + Combine (Word (nums) + ZeroOrMore (oneOf (', -') + Word (nums)))
    volta_text = Literal ('[').suppress () + Regex (r'"[^"]+"')
    volta = volta_nums | volta_text
    invisible_barline = oneOf ('[|] []')
    dashed_barline = oneOf (': .|')
    double_rep = Literal (':') + FollowedBy (':')   # otherwise ambiguity with dashed barline
    voice_overlay = Combine (OneOrMore ('&'))
    bare_volta = FollowedBy (Literal ('[') + Word (nums))   # no barline, but volta follows (volta is parsed in next measure)
    bar_left = (oneOf ('[|: |: :') + Optional (volta)) | Optional ('|').suppress () + volta | oneOf ('| [|')
    bars = ZeroOrMore (':') + ZeroOrMore ('[') + OneOrMore (oneOf ('| ]'))
    bar_right = Optional (decorations) + (invisible_barline | double_rep | Combine (bars) | dashed_barline | voice_overlay | bare_volta)
    
    errors =  ~decoration + CharsNotIn (' \n.:&[]|', exact=1)
    linebreak = Literal ('$') | ~decorations + Literal ('!')    # no need for I:linebreak !!!
    element = inline_field | broken | staff_decos | stem | chord_or_text | grace_notes | tuplet_start | linebreak | errors
    measure = Group (ZeroOrMore (inline_field) + Optional (bar_left) + ZeroOrMore (element) + bar_right + Optional (linebreak))
    noBarMeasure = Group (Optional (bar_left) + OneOrMore (element) + Optional (linebreak))
    abc_voice = ZeroOrMore (measure) + Optional (noBarMeasure | Group (bar_left)) + StringEnd ()

    #----------------------------------------
    # ABC lyric lines (white space sensitive)
    #----------------------------------------

    skip_note   = oneOf ('* - ~')
    extend_note = Literal ('_')
    measure_end = Literal ('|')
    syl_chars   = CharsNotIn ('*~-_| \t\n')
    white       = Word (' \t')
    syllable    = Combine (Optional ('~') + syl_chars + ZeroOrMore (Literal ('~') + syl_chars)) + Optional ('-')
    lyr_elem    = (syllable | skip_note | extend_note | measure_end) + Optional (white).suppress ()
    lyr_head    = (Literal ('w:') + Optional (white)).suppress ()
    lyr_line    = Group (lyr_head + ZeroOrMore (lyr_elem) + LineEnd ().suppress ())

    #----------------------------------------------------------------
    # Parse actions to convert all relevant results into an abstract
    # syntax tree where all tree nodes are instances of pObj
    #----------------------------------------------------------------

    ifield.setParseAction (lambda t: pObj ('field', t))
    note_length.setParseAction (lambda t: pObj ('dur', (t[0], (t[2] << len (t[1])) >> 1)))
    chordsym.setParseAction (lambda t: pObj ('chordsym', t))
    chord_root.setParseAction (lambda t: pObj ('root', t))
    chord_kind.setParseAction (lambda t: pObj ('kind', t))
    chord_degree.setParseAction (lambda t: pObj ('degree', t))
    chord_bass.setParseAction (lambda t: pObj ('bass', t))
    text_expression.setParseAction (lambda t: pObj ('text', t))
    inline_field.setParseAction (lambda t: pObj ('inline', t))
    grace_notes.setParseAction (lambda t: pObj ('grace', t))
    acciaccatura.setParseAction (lambda t: pObj ('accia', t))
    note.setParseAction (noteActn)
    chord_note.setParseAction (noteActn)
    rest.setParseAction (restActn)
    decorations.setParseAction (lambda t: pObj ('deco', t))
    slur_ends.setParseAction (lambda t: pObj ('slurs', t))
    chord.setParseAction (lambda t: pObj ('chord', t))
    tie.setParseAction (lambda t: pObj ('tie', t))
    pitch.setParseAction (lambda t: pObj ('pitch', t))
    bare_volta.setParseAction (lambda t: ['|']) # return barline that user forgot
    dashed_barline.setParseAction (lambda t: ['.|'])
    bar_right.setParseAction (lambda t: pObj ('rbar', t))
    bar_left.setParseAction (lambda t: pObj ('lbar', t))
    broken.setParseAction (lambda t: pObj ('broken', t))
    tuplet_start.setParseAction (lambda t: pObj ('tup', t))
    linebreak.setParseAction (lambda t: pObj ('linebrk', t))
    measure.setParseAction (doMaat)
    noBarMeasure.setParseAction (doMaat)
    syllable.setParseAction (lambda t: pObj ('syl', t))
    skip_note.setParseAction (lambda t: pObj ('skip', t))
    extend_note.setParseAction (lambda t: pObj ('ext', t))
    measure_end.setParseAction (lambda t: pObj ('sbar', t))
    b1.setParseAction (errorWarn)
    errors.setParseAction (errorWarn)
    lyr_block   = OneOrMore (lyr_line).leaveWhitespace ()   # after leaveWhiteSpace no more parse actions can be set!!

    return abc_header, abc_voice, lyr_block

class pObj (object):    # every relevant parse result is converted into a pObj
    def __init__ (s, name, t):  # t = list of nested parse results
        s.name = name   # name uniqueliy identifies this pObj
        rest = []       # collect parse results that are not a pObj
        attrs = {}      # new attributes
        for x in t:     # nested pObj's become attributes of this pObj
            if type (x) == pObj:
                attrs [x.name] = attrs.get (x.name, []) + [x]
            else:
                rest.append (x)             # collect non-pObj's (mostly literals)
        for name, xs in attrs.items ():
            if len (xs) == 1: xs = xs[0]    # only list if more then one pObj
            setattr (s, name, xs)           # create the new attributes
        s.t = rest      # all nested non-pObj's (mostly literals)
        s.objs = []     # for nested ordered (lyric) pObj's

    def __repr__ (s):   # make a nice string representation of a pObj
        r = []
        for nm in dir (s):
            if nm.startswith ('_'): continue # skip build in attributes
            elif nm == 'name': continue     # redundant
            else:
                x = getattr (s, nm)
                if not x: continue          # s.t may be empty (list of non-pObj's)
                if type (x) == types.ListType:  r.extend (x)
                else:                           r.append (x)
        xs = []
        for x in r:     # recursively call __repr__ and convert all strings to latin-1
            if isinstance (x, types.StringTypes):
                try:    xs.append (x.encode ('latin-1'))
                except: xs.append (repr (x))    # string -> no recursion
            else:       xs.append (repr (x))    # pObj -> recursive call
        return '(' + s.name + ' ' +','.join (xs) + ')'

global prevloc                  # global to remember previous match position of a note/rest
prevloc = 0
def detectBeamBreak (line, loc, t):
    global prevloc              # location in string 'line' of previous note match
    xs = line[prevloc:loc+1]    # string between previous and current note match
    xs = xs.lstrip ()           # first note match starts on a space!
    prevloc = loc               # location in string 'line' of current note match
    b = pObj ('bbrk', [' ' in xs])      # space somewhere between two notes -> beambreak
    t.insert (0, b)             # insert beambreak as a nested parse result

def noteActn (line, loc, t):    # detect beambreak between previous and current note/rest
    if 'y' in t[0].t: return [] # discard spacer
    detectBeamBreak (line, loc, t)      # adds beambreak to parse result t as side effect
    return pObj ('note', t)

def restActn (line, loc, t):    # detect beambreak between previous and current note/rest
    detectBeamBreak (line, loc, t)  # adds beambreak to parse result t as side effect
    return pObj ('rest', t)

def errorWarn (line, loc, t):      # warning for misplaced symbols and skip them
    info ('**misplaced symbol: %s' % t[0], warn=0)
    lineCopy = line [:]
    if loc > 40:
        lineCopy = line [loc - 40: loc + 40]
        loc = 40
    info (lineCopy.replace ('\n', ' '), warn=0)
    info (loc * '-' + '^', warn=0)
    return []

#-------------------------------------------------------------
# transformations of a measure (called by parse action doMaat)
#-------------------------------------------------------------

def simplify (a, b):    # divide a and b by their greatest common divisor
    x, y = a, b
    while b: a, b = b, a % b
    return x / a, y / a

def doBroken (prev, brk, x):
    if not prev: info ('error in broken rhythm: %s' % x); return    # no changes
    nom1, den1 = prev.dur.t # duration of first note/chord
    nom2, den2 = x.dur.t    # duration of second note/chord
    if  brk == '>':
        nom1, den1  = simplify (3 * nom1, 2 * den1)
        nom2, den2  = simplify (1 * nom2, 2 * den2)
    elif brk == '<':
        nom1, den1  = simplify (1 * nom1, 2 * den1)
        nom2, den2  = simplify (3 * nom2, 2 * den2)
    elif brk == '>>':
        nom1, den1  = simplify (7 * nom1, 4 * den1)
        nom2, den2  = simplify (1 * nom2, 4 * den2)
    elif brk == '<<':
        nom1, den1  = simplify (1 * nom1, 4 * den1)
        nom2, den2  = simplify (7 * nom2, 4 * den2)
    else: return            # give up
    prev.dur.t = nom1, den1 # change duration of previous note/chord
    x.dur.t = nom2, den2    # and current note/chord

def convertBroken (t):  # convert broken rhythms to normal note durations
    prev = None # the last note/chord before the broken symbol
    brk = ''    # the broken symbol
    remove = [] # indexes to broken symbols (to be deleted) in measure
    for i, x in enumerate (t):  # scan all elements in measure
        if x.name == 'note' or x.name == 'chord' or x.name == 'rest':
            if brk:                 # a broken symbol was encountered before
                doBroken (prev, brk, x) # change duration previous note/chord/rest and current one
                brk = ''
            else:
                prev = x            # remember the last note/chord/rest
        elif x.name == 'broken':
            brk = x.t[0]            # remember the broken symbol (=string)
            remove.insert (0, i)    # and its index, highest index first
    for i in remove: del t[i]       # delete broken symbols from high to low

def convertChord (t):   # convert chord to sequence of notes in musicXml-style
    ins = []
    for i, x in enumerate (t):
        if x.name == 'chord':
            num1, den1 = x.dur.t                # chord duration
            tie = getattr (x, 'tie', None)      # chord tie
            slurs = getattr (x, 'slurs', [])    # slur endings
            deco = getattr (x, 'deco', [])      # chord decorations
            if type (x.note) != types.ListType: x.note = [x.note]   # when chord has only one note ...
            for j, nt in enumerate (x.note):    # all notes of the chord
                num2, den2 = nt.dur.t           # note duration * chord duration
                nt.dur.t = simplify (num1 * num2, den1 * den2)
                if tie: nt.tie = tie            # tie on all chord notes
                if j == 0 and deco: nt.deco = deco      # decorations only on first chord note
                if j == 0 and slurs: nt.slurs = slurs   # slur endings only on first chord note
                if j > 0: nt.chord = pObj ('chord', [1]) # label all but first as chord notes
            ins.insert (0, (i, x.note))         # high index first
    for i, notes in ins:                        # insert from high to low
        for nt in reversed (notes):
            t.insert (i+1, nt)                  # insert chord notes after chord
        del t[i]                                # remove chord itself

def doMaat (t):             # t is a Group() result -> the measure is in t[0]
    convertBroken (t[0])    # remove all broken rhythms and convert to normal durations
    convertChord (t[0])     # replace chords by note sequences in musicXML style

#--------------------
# musicXML generation
#----------------------------------

def compChordTab ():    # avoid some typing work: returns mapping constant {ABC chordsyms -> musicXML kind}
    maj, min, aug, dim, dom, ch7, ch6, ch9, ch11, hd = 'major minor augmented diminished dominant -seventh -sixth -ninth -11th half-diminished'.split ()
    triad   = zip ('ma Maj maj M mi min m aug dim o + -'.split (), [maj, maj, maj, maj, min, min, min, aug, dim, dim, aug, min])
    seventh = zip ('7 ma7 Maj7 M7 maj7 mi7 m7 dim7 o7 -7 aug7 +7 m7b5 mi7b5'.split (),
                   [dom, maj+ch7, maj+ch7, maj+ch7, maj+ch7, min+ch7, min+ch7, dim+ch7, dim+ch7, min+ch7, aug+ch7, aug+ch7, hd, hd])
    sixth   = zip ('6 ma6 M6 mi6 m6'.split (), [maj+ch6, maj+ch6, maj+ch6, min+ch6, min+ch6])
    ninth   = zip ('9 ma9 M9 maj9 Maj9 mi9 m9'.split (), [dom+ch9, maj+ch9, maj+ch9, maj+ch9, maj+ch9, min+ch9, min+ch9])
    elevn   = zip ('11 ma11 M11 maj11 Maj11 mi11 m11'.split (), [dom+ch11, maj+ch11, maj+ch11, maj+ch11, maj+ch11, min+ch11, min+ch11])
    return dict (triad + seventh + sixth + ninth + elevn)

def addElem (parent, child, level):
    indent = 2
    chldrn = parent.getchildren ()
    if chldrn:
        chldrn[-1].tail += indent * ' '
    else:
        parent.text = '\n' + level * indent * ' '
    parent.append (child)
    child.tail = '\n' + (level-1) * indent * ' '

def addElemT (parent, tag, text, level):
    e = E.Element (tag)
    e.text = text
    addElem (parent, e, level)
    
def mkTmod (tmnum, tmden, lev):
    tmod = E.Element ('time-modification')
    addElemT (tmod, 'actual-notes', str (tmnum), lev + 1)
    addElemT (tmod, 'normal-notes', str (tmden), lev + 1)
    return tmod

def addDirection (parent, elem, lev, subelms=[], placement='below'):
    dir = E.Element ('direction', placement=placement)
    addElem (parent, dir, lev)
    typ = E.Element ('direction-type')
    addElem (dir, typ, lev + 1)
    addElem (typ, elem, lev + 2)
    for subel in subelms: addElem (elem, subel, lev + 3)
    return dir

def removeElems (root_elem, parent_str, elem_str):
    for p in root_elem.findall (parent_str):
        e = p.find (elem_str)
        if e != None: p.remove (e)

def alignLyr (vce, lyrs):
    empty_el = pObj ('leeg', '*')
    for k, lyr in enumerate (lyrs): # lyr = one full line of lyrics
        i = 0               # syl counter
        for msre in vce:    # reiterate the voice block for each lyrics line
            for elem in msre:
                if elem.name == 'note' and not hasattr (elem, 'chord'):
                    if i >= len (lyr): lr = empty_el
                    else: lr = lyr [i]
                    elem.objs.append (lr)
                    if lr.name != 'sbar': i += 1
            if i < len (lyr) and lyr[i].name == 'sbar': i += 1
    return vce

slur_move = re.compile (r'(?<![!+])([}><][<>]?)(\)+)') # (?<!...) means: not preceeded by ...
mm_rest = re.compile (r'([XZ])(\d+)')
def fixSlurs (x):   # repair slurs when after broken sign or grace-close
    def f (mo):     # replace a multi-measure rest by single measure rests
        n = int (mo.group (2))
        return (n * (mo.group (1) + '|')) [:-1]
    x = mm_rest.sub (f, x)
    return slur_move.sub (r'\2\1', x)

def splitHeaderVoices (abctext):
    r1 = re.compile (r'%.*$')           # comments
    r2 = re.compile (r'^[A-Z]:.*$')     # information field
    r3 = re.compile (r'^%%(?=[^%])')    # directive: ^%% folowed by not a %
    xs, nx = [], 0
    for x in abctext.splitlines ():
        x = x.strip ()
        if not x and nx == 1: break     # end of tune
        x = r3.sub ('I:', x)            # replace %% -> I:
        x2 = r1.sub ('', x)             # remove comment
        while x2.endswith ('*'): x2 = x2[:-1]   # remove old syntax for right adjusting
        if not x2: continue             # empty line
        if x2[:2] == 'W:': continue     # skip W: lyrics
        if x2[:2] == 'w:' and xs[-1][-1] ==  '\\':
            xs[-1] = xs[-1][:-1]        # ignore line continuation before lyrics line
        ro = r2.match (x2)
        if ro:                          # field -> inline_field, escape all ']'
            if x2[-1] == '\\': x2 = x2[:-1] # ignore continuation after field line
            x2 = '[' + x2.replace (']',r'\]') + ']'
        if x2[:2] == '+:':              # new style continuation
            xs[-1] += x2[2:]
        elif xs and xs[-1][-1] ==  '\\':  # old style continuation
            xs[-1] = xs[-1][:-1] + x2
        else:                           # skip lines (except I:) until first X:
            if x.startswith ('X:'):
                if nx == 1: break       # second tune starts without an empty line !!
                nx = 1                  # start of first tune
            if nx == 1 or x.startswith ('I:'):
                xs.append (x2)
    if xs and xs[-1][-1] == '\\':       # nothing left to continue with, remove last continuation
        xs[-1] = xs[-1][:-1]

    r1 = re.compile (r'\[(\\.|[^]\\])*\]') # inline field with escaped ']'
    r2 = re.compile (r'\[K:')           # start of K: field
    r3 = re.compile (r'\[V:|\[I:MIDI')  # start of V: field or midi field
    fields, voices, b = [], [], 0
    for i, x in enumerate (xs):
        n = len (r1.sub ('', x))        # remove all inline fields
        if n > 0: b = 1; break          # real abc present -> end of header
        if r2.search (x):               # start of K: field
            fields.append (x)
            i += 1; b = 1
            break                       # first K: field -> end of header
        if r3.search (x):               # start of V: field
            voices.append (x)
        else:
            fields.append (x)
    if b: voices += xs[i:]
    else: voices += []                  # tune has only header fields
    header =  '\n'.join (fields)
    abctext = '\n'.join (voices)

    xs = abctext.split ('[V:')
    if len (xs) == 1: abctext = '[V:1]' + abctext # abc has no voice defs at all
    if r1.sub ('', xs[0]).strip ():     # remove inline fields from starting text, if any
        abctext = '[V:1]' + abctext     # abc with voices has no V: at start

    r1 = re.compile (r'\[V:\s*(\S*)[ \]]') # get voice id from V: field (skip spaces betwee V: and ID)
    vmap = {}                           # {voice id -> [voice abc string]}
    vorder = {}                         # mark document order of voices
    xs = re.split (r'(\[V:[^]]*\])', abctext)   # split on every V-field (V-fields included in split result list)
    if len (xs) == 1: raise (Exception ('bugs ...'))
    else:
        header += xs[0]     # xs[0] = text between K: and first V:, normally empty, but we put it in the header
        i = 1
        while i < len (xs):             # xs = ['', V-field, voice abc, V-field, voice abc, ...]
            vce, abc = xs[i:i+2]
            id = r1.search (vce).group (1)                  # get voice ID from V-field
            vmap[id] = vmap.get (id, []) + [vce, abc]       # collect abc-text for each voice id (include V-fields)
            if id not in vorder: vorder [id] = i            # store document order of first occurrence of voice id
            i += 2
    voices = []
    ixs = sorted ([(i, id) for id, i in vorder.items ()])   # restore document order of voices
    for i, id in ixs:
        voice = ''.join (vmap [id])                         # all abc of one voice
        xs = re.split (r'((?:\nw:[^\n]*)+)', voice)         # split voice into voice-lyrics blocks
        if len (xs) == 1:               # no lyrics
            voice = fixSlurs (xs[0])    # put slurs right after the notes
            vce_lyr = [[voice, '']]
        else:
            if xs[-1].strip () != '': xs.append ('w:')               # last block had no lyrics
            vce_lyr = []                # [[voice, lyrics],[],...] list of voice-lyrics blocks
            for k in range (0, len (xs) - 1, 2):
                voice, lyrics = xs [k:k+2]
                voice = fixSlurs (voice)    # put slurs right after the notes
                vce_lyr.append ((voice, lyrics))
        voices.append ((id, vce_lyr))
    return header, voices

def mergeMeasure (m1, m2, slur_offset):
    slurs = m2.findall ('note/notations/slur')
    for slr in slurs:
        slrnum = int (slr.get ('number')) + slur_offset 
        slr.set ('number', str (slrnum))    # make unique slurnums in m2
    vs = m1.findall ('note/voice')          # all voice number elements in m1
    vnum_max = max ([int (v.text) for v in vs] + [0])   # highest voice number in m1
    vs = m2.findall ('note/voice')          # set all voice number elements in m2
    for v in vs: v.text  = str (vnum_max + 1)
    ls = m1.findall ('note/lyric')          # all lyric elements in m1
    lnum_max = max ([int (l.get ('number')) for l in ls] + [0]) # highest lyric number in m1
    ls = m2.findall ('note/lyric')          # update lyric elements in m2
    for el in ls:
        n = int (el.get ('number'))
        el.set ('number', str (n + lnum_max))
    ns = m1.findall ('note')    # determine the total duration of m1, subtract all backups
    dur1 = sum (int (n.find ('duration').text) for n in ns
                if n.find ('grace') == None and n.find ('chord') == None)
    dur1 -= sum (int (b.text) for b in m1.findall ('backup/duration'))
    nns, es = 0, []             # nns = number of real notes in m2
    for e in m2.getchildren (): # scan all elements of m2
        if e.tag == 'attributes': continue
        if e.tag == 'print': continue
        if e.tag == 'note' and e.find ('rest') == None: nns += 1
        es.append (e)           # buffer elements to be merged
    if nns > 0:                 # only merge if m2 contains any real notes
        b = E.Element ('backup')
        addElem (m1, b, level=3)
        addElemT (b, 'duration', str (dur1), level=4)
        for e in es: addElem (m1, e, level=3)   # merge buffered elements of m2

def mergePartList (parts):
    p1 = parts[0]
    for p2 in parts[1:]:
        if len (p1) < len (p2): p1, p2 = p2, p1
        slurs = p1.findall ('measure/note/notations/slur')  # find highest slur num in first part
        slur_max = max ([int (slr.get ('number')) for slr in slurs] + [0])
        for im, m2 in enumerate (p2.findall ('measure')):   # merge all measures of p2 into p1
            mergeMeasure (p1[im], m2, slur_max)     # may change slur numbers in p1
    return p1

def mergeParts (parts, vids, staves):
    if not staves: return parts   # no voice mapping
    partsnew, partlistnew = [], []
    for voice_ids in staves:
        pixs = []
        for vid in voice_ids:
            if vid in vids: pixs.append (vids.index (vid))
            else: info ('score partname %s does not exist' % vid)
        if pixs:
            xparts = [parts[pix] for pix in pixs]
            mergedpart = mergePartList (xparts)
            partsnew.append (mergedpart)
    return partsnew

def mergePartMeasure (part, msre):  # merge msre into last measure of part
    slurs = part.findall ('measure/note/notations/slur')    # find highest slur num in part
    slur_max = max ([int (slr.get ('number')) for slr in slurs] + [0])
    last_msre = part.getchildren ()[-1] # last measure in part
    mergeMeasure (last_msre, msre, slur_max)

class MusicXml:
    typeMap = {1:'long', 2:'breve', 4:'whole', 8:'half', 16:'quarter', 32:'eighth', 64:'16th', 128:'32nd', 256:'64th'}
    dynaMap = {'p':1,'pp':1,'ppp':1,'f':1,'ff':1,'fff':1,'mp':1,'mf':1,'sfz':1}
    wedgeMap = {'>(':1, '>)':1, '<(':1,'<)':1,'crescendo(':1,'crescendo)':1,'diminuendo(':1,'diminuendo)':1}
    artMap = {'.':'staccato','>':'accent','accent':'accent','wedge':'staccatissimo','tenuto':'tenuto'}
    ornMap = {'trill':'trill-mark','T':'trill-mark','turn':'turn','uppermordent':'inverted-mordent','lowermordent':'mordent',
              'pralltriller':'inverted-mordent','mordent':'mordent','turn':'turn','invertedturn':'inverted-turn'}
    tecMap = {'upbow':'up-bow', 'downbow':'down-bow'}
    capoMap = {'fine':('Fine','fine','yes'), 'D.S.':('D.S.','dalsegno','segno'), 'D.C.':('D.C.','dacapo','yes'),'dacapo':('D.C.','dacapo','yes'),
               'dacoda':('To Coda','tocoda','coda'), 'coda':('coda','coda','coda'), 'segno':('segno','segno','segno')}
    sharpness = ['Fb', 'Cb','Gb','Db','Ab','Eb','Bb','F','C','G','D','A', 'E', 'B', 'F#','C#','G#','D#','A#','E#','B#']
    offTab = {'maj':8, 'm':11, 'min':11, 'mix':9, 'dor':10, 'phr':12, 'lyd':7, 'loc':13}
    modTab = {'maj':'major', 'm':'minor', 'min':'minor', 'mix':'mixolydian', 'dor':'dorian', 'phr':'phrygian', 'lyd':'lydian', 'loc':'locrian'}
    clefMap = { 'alto1':('C','1'), 'alto2':('C','2'), 'alto':('C','3'), 'tenor':('C','4'), 'bass3':('F','3'),
                'bass':('F','4'), 'treble':('G','2'), 'perc':('percussion',''), 'none':('','')}
    clefLineMap = {'B':'treble', 'G':'alto1', 'E':'alto2', 'C':'alto', 'A':'tenor', 'F':'bass3', 'D':'bass'}
    alterTab = {'=':'0', '_':'-1', '__':'-2', '^':'1', '^^':'2'}
    accTab = {'=':'natural', '_':'flat', '__':'flat-flat', '^':'sharp', '^^':'sharp-sharp'}
    chordTab = compChordTab ()
    uSyms = {'~':'roll', 'H':'fermata','L':'>','M':'lowermordent','O':'coda',
             'P':'uppermordent','S':'segno','T':'trill','u':'upbow','v':'downbow'}
    pageFmtDef = [1.764,297,210,10,10,10,10] # the MuseScore page formatting defaults for A4
    creditTab = {'O':'origin', 'A':'area', 'Z':'transcription', 'N':'notes', 'G':'group', 'H':'history', 'R':'rhythm',
                 'B':'book', 'D':'discography', 'F':'fileurl', 'S':'source'}

    def __init__ (s):
        s.pageFmtCmd = []   # set by command line option -p
        s.reset ()
    def reset (s):
        s.divisions = 120   # xml duration of 1/4 note
        s.ties = {}         # {abc pitch tuple -> alteration} for all open ties
        s.slurstack = []    # stack of open slur numbers
        s.slurbeg = 0       # number of slurs to start (when slurs are detected at element-level)
        s.tmnum = 0         # time modification, numerator
        s.tmden = 0         # time modification, denominator
        s.ntup = 0          # number of tuplet notes remaining
        s.tupnts = []       # all tuplet modifiers with corresp. durations: [(duration, modifier), ...]
        s.irrtup = 0        # 1 if an irregular tuplet
        s.ntype = ''        # the normal-type of a tuplet (== duration type of a normal tuplet note)
        s.unitL =  (1, 8)   # default unit length
        s.unitLcur = (1, 8) # unit length of current voice
        s.keyAlts = {}      # alterations implied by key
        s.msreAlts = {}     # temporarily alterations
        s.curVolta = ''     # open volta bracket
        s.slurstack = []    # stack of open slur numbers
        s.title = ''        # title of music
        s.creator = {}      # {creator-type -> creator string}
        s.credits = {}      # {credit-type -> string}
        s.lyrdash = {}      # {lyric number -> 1 if dash between syllables}
        s.usrSyms = s.uSyms # user defined symbols
        s.prevNote = None   # xml element of previous beamed note to correct beams (start, continue)
        s.grcbbrk = False   # remember any bbrk in a grace sequence
        s.linebrk = 0       # 1 if next measure should start with a line break
        s.bardecos = []     # barline decorations (coda, segno) that go into the next measure (MuseScore deficiency!)
        s.nextdecos = []    # decorations for the next note
        s.prevmsre = None   # the previous measure
        s.supports_tag = 0  # issue supports-tag in xml file when abc uses explicit linebreaks
        s.staves = []       # staves = [[voice names to be merged into one stave]]
        s.groups = []       # list of merged part names with interspersed {[ and }]
        s.pageFmtAbc = []   # formatting from abc directives
        s.mdur = (4,4)      # duration of one measure
        s.gtrans = 0        # octave transposition (by clef)
        s.midprg = ['', ''] # MIDI channel nr, program nr for the current part
        s.vid = ''          # abc voice id for the current part

    def mkPitch (s, acc, note, oct, lev):
        nUp = note.upper ()
        octnum = (4 if nUp == note else 5) + int (oct) + s.gtrans
        pitch = E.Element ('pitch')
        addElemT (pitch, 'step', nUp, lev + 1)
        alter = ''
        if (note, oct) in s.ties:           alter = s.ties [(note,oct)] # tied note -> same alteration
        elif acc:
            s.msreAlts [(nUp, octnum)] = s.alterTab [acc]
            alter = s.alterTab [acc]                                    # explicit notated alteration
        elif (nUp, octnum) in s.msreAlts:   alter = s.msreAlts [(nUp, octnum)]  # temporary alteration
        elif nUp in s.keyAlts:              alter = s.keyAlts [nUp]     # alteration implied by the key
        if alter: addElemT (pitch, 'alter', alter, lev + 1)
        addElemT (pitch, 'octave', str (octnum), lev + 1)
        return pitch, alter

    def mkNote (s, n, lev):
        nnum, nden = n.dur.t            # abc dutation of note
        num, den = simplify (nnum * s.unitLcur[0], nden * s.unitLcur[1])  # normalised with unit length
        if n.name == 'rest' and ('Z' in n.t or 'X' in n.t):
              num, den = s.mdur         # duration of one measure
        dvs = (4 * s.divisions * num) / den     # divisions is xml-duration of 1/4
        rdvs = dvs                      # real duration (will be 0 for chord/grace)
        num, den = simplify (num, den * 4)      # scale by 1/4 for s.typeMap
        ndot = 0
        if num == 3: ndot = 1; den = den / 2    # look for dotted notes
        if num == 7: ndot = 2; den = den / 4
        nt = E.Element ('note')
        if getattr (n, 'chord', ''):    # a chord note
            chord = E.Element ('chord')
            addElem (nt, chord, lev + 1)
            rdvs = 0                    # chord notes no real duration
        elif getattr (n, 'grace', ''):  # a grace note
            grace = E.Element ('grace')
            addElem (nt, grace, lev + 1)
            dvs = rdvs = 0              # no (real) duration for a grace note
            if den <= 16: den = 32      # not longer than 1/8 for a grace note
        elif s.ntup >= 0: s.ntup -= 1   # count tuplet notes only on non-chord, non grace notes
        if den not in s.typeMap:        # take the nearest smaller legal duration
            info ('illegal duration %d/%d' % (nnum, nden))
            den = min (x for x in s.typeMap.keys () if x > den)
        xmltype = str (s.typeMap [den]) # xml needs the note type in addition to duration
        acc, step, oct = '', 'C', '0'   # abc-notated pitch elements (accidental, pitch step, octave)
        alter = ''                      # xml alteration
        if n.name == 'rest':
            if 'x' in n.t or 'X' in n.t: nt.set ('print-object', 'no')
            rest = E.Element ('rest')
            addElem (nt, rest, lev + 1)
        else:
            p = n.pitch.t           # get pitch elements from parsed tokens
            if len (p) == 3:    acc, step, oct = p
            else:               step, oct = p
            pitch, alter = s.mkPitch (acc, step, oct, lev + 1)
            addElem (nt, pitch, lev + 1)
        if s.ntup >= 0:                 # modify duration for tuplet notes
            dvs = dvs * s.tmden / s.tmnum
        if dvs: addElemT (nt, 'duration', str (dvs), lev + 1)   # skip when dvs == 0, requirement of musicXML
        inst = E.Element ('instrument', id='I-'+s.vid)          # instrument id for midi
        if s.midprg != ['', '']: addElem (nt, inst, lev + 1)    # only add when %%midi was present
        addElemT (nt, 'voice', '1', lev + 1)    # default voice, for merging later
        addElemT (nt, 'type', xmltype, lev + 1) # add note type
        for i in range (ndot):          # add dots
            dot = E.Element ('dot')
            addElem (nt, dot, lev + 1)
        if acc: addElemT (nt, 'accidental', s.accTab [acc], lev + 1) # add accidental
        tupnotation = ''                # start/stop notation element for tuplets
        if s.ntup >= 0:                 # add time modification element for tuplet notes
            tmod = mkTmod (s.tmnum, s.tmden, lev + 1)
            addElem (nt, tmod, lev + 1)
            if s.ntup > 0 and not s.tupnts: tupnotation = 'start'
            s.tupnts.append ((rdvs, tmod))      # remember all tuplet modifiers with corresp. durations
            if s.ntup == 0:             # last tuplet note (and possible chord notes there after)
                if rdvs: tupnotation = 'stop'   # only insert notation in the real note (rdvs > 0)
                s.cmpNormType (rdvs, lev + 1)   # compute and/or add normal-type elements (-> s.ntype)
        s.doBeams (n, nt, den, lev + 1)
        s.doNotations (n, (step, oct), alter, tupnotation, nt, lev + 1)
        if n.objs: s.doLyr (n, nt, lev + 1)
        return nt

    def cmpNormType (s, rdvs, lev): # compute the normal-type of a tuplet (only needed for Finale)
        if rdvs:    # the last real tuplet note (chord notes can still follow afterwards with rdvs == 0)
            durs = [dur for dur, tmod in s.tupnts if dur > 0]
            ndur = sum (durs) / s.tmnum # duration of the normal type
            s.irrtup = any ((dur != ndur) for dur in durs)  # irregular tuplet
            tix = 16 * s.divisions / ndur       # index in typeMap of normal-type duration
            if tix in s.typeMap:
                s.ntype = str (s.typeMap [tix]) # the normal-type
            else: s.irrtup = 0          # give up, no normal type possible
        if s.irrtup:                    # only add normal-type for irregular tuplets
            for dur, tmod in s.tupnts:  # add normal-type to all modifiers
                addElemT (tmod, 'normal-type', s.ntype, lev + 1)
        s.tupnts = []                   # reset the tuplet buffer

    def doNotations (s, n, ptup, alter, tupnotation, nt, lev):
        tstop = ptup in s.ties  # is there an open tie on this pitch tuple
        tstart = getattr (n, 'tie', 0)  # start a new tie
        decos = s.nextdecos     # decorations encountered so far
        ndeco = getattr (n, 'deco', 0)  # possible decorations of notes of a chord
        if ndeco:               # add decorations, translate used defined symbols
            decos += [s.usrSyms.get (d, d).strip ('!+') for d in ndeco.t]
        s.nextdecos = []
        slurs = getattr (n, 'slurs', 0) # slur ends
        if not (tstop or tstart or decos or slurs or s.slurbeg or tupnotation): return nt
        nots = E.Element ('notations')  # notation element needed
        if tupnotation:         # add tuplet type
            tup = E.Element ('tuplet', type=tupnotation)
            if tupnotation == 'start': tup.set ('bracket', 'yes')
            addElem (nots, tup, lev + 1)
        if tstop:               # stop tie
            del s.ties[ptup]    # remove flag
            tie = E.Element ('tied', type='stop')
            addElem (nots, tie, lev + 1)
        if tstart:              # start a tie
            s.ties[ptup] = alter    # remember pitch tuple to stop tie and apply same alteration
            tie = E.Element ('tied', type='start')
            addElem (nots, tie, lev + 1)
        if decos:               # look for slurs and decorations
            arts = []           # collect articulations
            for d in decos:     # do all slurs and decos
                if d == '(': s.slurbeg += 1; continue # slurs made in while loop at the end
                elif d == 'fermata' or d == 'H':
                    ntn = E.Element ('fermata', type='upright')
                elif d == 'arpeggio':
                    ntn = E.Element ('arpeggiate', number='1')
                else: arts.append (d); continue
                addElem (nots, ntn, lev + 1)
            if arts:        # do only note articulations and collect staff annotations in xmldecos
                rest = s.doArticulations (nots, arts, lev + 1)
                if rest: info ('unhandled note decorations: %s' % rest)
        while s.slurbeg > 0:
            s.slurbeg -= 1
            slurnum = len (s.slurstack) + 1
            s.slurstack.append (slurnum)
            ntn = E.Element ('slur', number='%d' % slurnum, type='start')
            addElem (nots, ntn, lev + 1)            
        if slurs and s.slurstack:       # these are only slur endings (and there are open slurs)
            for d in slurs.t:
                slurnum = s.slurstack.pop ()
                slur = E.Element ('slur', number='%d' % slurnum, type='stop')
                addElem (nots, slur, lev + 1)
        if nots.getchildren() != []:    # only add notations if not empty
            addElem (nt, nots, lev)

    def doArticulations (s, nots, arts, lev):
        decos = []
        for a in arts:
            if a in s.artMap:
                art = E.Element ('articulations')
                addElem (nots, art, lev)
                addElem (art, E.Element (s.artMap[a]), lev + 1)
            elif a in s.ornMap:
                orn = E.Element ('ornaments')
                addElem (nots, orn, lev)
                addElem (orn, E.Element (s.ornMap[a]), lev + 1)
            elif a in s.tecMap:
                tec = E.Element ('technical')
                addElem (nots, tec, lev)
                addElem (tec, E.Element (s.tecMap[a]), lev + 1)
            else: decos.append (a)  # return staff annotations
        return decos

    def doLyr (s, n, nt, lev):
        for i, lyrobj in enumerate (n.objs):
            if lyrobj.name != 'syl': continue
            dash = len (lyrobj.t) == 2
            if dash:
                if i in s.lyrdash:  type = 'middle'
                else:               type = 'begin'; s.lyrdash [i] = 1
            else:
                if i in s.lyrdash:  type = 'end';   del s.lyrdash [i]
                else:               type = 'single'
            lyrel = E.Element ('lyric', number = str (i + 1))
            addElem (nt, lyrel, lev)
            addElemT (lyrel, 'syllabic', type, lev + 1)
            addElemT (lyrel, 'text', lyrobj.t[0].replace ('~',' '), lev + 1)

    def doBeams (s, n, nt, den, lev):
        if hasattr (n, 'chord') or hasattr (n, 'grace'):
            s.grcbbrk = s.grcbbrk or n.bbrk.t[0]    # remember if there was any bbrk in or before a grace sequence
            return
        bbrk = s.grcbbrk or n.bbrk.t[0] or den < 32
        s.grcbbrk = False
        if not s.prevNote:  pbm = None
        else:               pbm = s.prevNote.find ('beam')
        bm = E.Element ('beam', number='1')
        bm.text = 'begin'
        if pbm != None:
            if bbrk:
                if pbm.text == 'begin':
                    s.prevNote.remove (pbm)
                elif pbm.text == 'continue':
                    pbm.text = 'end'
                s.prevNote = None
            else: bm.text = 'continue'
        if den >= 32 and n.name != 'rest':
            addElem (nt, bm, lev)
            s.prevNote = nt

    def stopBeams (s):
        if not s.prevNote: return
        pbm = s.prevNote.find ('beam')
        if pbm != None:
            if pbm.text == 'begin':
                s.prevNote.remove (pbm)
            elif pbm.text == 'continue':
                pbm.text = 'end'
        s.prevNote = None

    def staffDecos (s, decos, maat, lev, bardecos=0):
        for d in decos:
            d = s.usrSyms.get (d, d).strip ('!+')   # try to replace user defined symbol
            if d in s.dynaMap:
                dynel = E.Element ('dynamics')
                addDirection (maat, dynel, lev, [E.Element (d)])
            elif d in s.wedgeMap:  # wedge
                if ')' in d: type = 'stop'
                else: type = 'crescendo' if '<' in d or 'crescendo' in d else 'diminuendo'
                addDirection (maat, E.Element ('wedge', type=type), lev)
            elif d in ['coda', 'segno']:
                if bardecos: s.bardecos.append (d)  # postpone to begin next measure
                else:
                    text, attr, val = s.capoMap [d]
                    dir = addDirection (maat, E.Element (text), lev, placement='above')
                    sound = E.Element ('sound'); sound.set (attr, val)
                    addElem (dir, sound, lev + 1)
            elif d in s.capoMap:
                text, attr, val = s.capoMap [d]
                words = E.Element ('words'); words.text = text
                dir = addDirection (maat, words, lev, placement='above')
                sound = E.Element ('sound'); sound.set (attr, val)
                addElem (dir, sound, lev + 1)
            elif d == '(': s.slurbeg += 1   # start slur on next note
            else: s.nextdecos.append (d)    # keep annotation for the next note

    def doFields (s, maat, fieldmap, lev):
        def doClef ():
            clef, gtrans = 0, 0
            clefn = re.search (r'alto1|alto2|alto|tenor|bass3|bass|treble|perc|none', field)
            clefm = re.search (r"(?:^m=| m=|middle=)([A-Ga-g])([,']*)", field)
            trans = re.search (r'(?:^t=| t=|transpose=)(-?[\d]+)', field)
            if clefn:
                clef = clefn.group ()
            if clefm:
                note, octstr = clefm.groups ()
                nUp = note.upper ()
                octnum = (4 if nUp == note else 5) + (len (octstr) if "'" in octstr else -len (octstr))
                gtrans = (3 if nUp in 'AFD' else 4) - octnum 
                if clef not in ['perc', 'none']: clef = s.clefLineMap [nUp]
            if clef:
                s.gtrans = gtrans   # only change global tranposition when a clef is really defined
                sign, line = s.clefMap [clef]
                if not sign: return
                c = E.Element ('clef')
                addElemT (c, 'sign', sign, lev + 2)
                if line: addElemT (c, 'line', line, lev + 2)
                if '-8' in field: addElemT (c, 'clef-octave-change', '-1', lev + 2) # ad hoc frutsel
                atts.append ((4, c))
                if trans:   # add transposition in semitones
                    e = E.Element ('transpose')
                    addElemT (e, 'chromatic', str (trans.group (1)), lev + 3)
                    atts.append ((5, e))

        atts = []               # collect xml attribute elements [(order-number, xml-element), ..]
        for ftype, field in fieldmap.items ():
            if not field:       # skip empty fields
                continue
            if ftype == 'Div':  # not an abc field, but handled as if
                d = E.Element ('divisions')
                d.text = field
                atts.append ((1, d))
            elif ftype == 'M':
                if field == 'none': continue
                if field == 'C': field = '4/4'
                elif field == 'C|': field = '2/2'
                t = E.Element ('time')
                if '/' not in field:
                    info ('M:%s not recognized, 4/4 assumed' % field)
                    field = '4/4'
                beats, btype = field.split ('/')
                try: s.mdur = simplify (eval (beats), int (btype))  # measure duration for Z and X rests (eval allows M:2+3/4)
                except:
                    info ('error in M:%s, 4/4 assumed' % field)
                    s.mdur = (4,4)
                    beats, btype = '4','4'
                addElemT (t, 'beats', beats, lev + 2)
                addElemT (t, 'beat-type', btype, lev + 2)
                atts.append ((3, t))
            elif ftype == 'K':
                accs = ['F','C','G','D','A','E','B']    # == s.sharpness [7:14]
                mode = ''
                key = re.match (r'\s*([CDEFGAB][#b]?)\s*([a-zA-Z]*)', field)
                if key:
                    key, mode = key.groups ()
                    mode = mode.lower ()[:3] # only first three chars, no case
                    if mode not in s.offTab: mode = 'maj'
                    fifths = s.sharpness.index (key) - s.offTab [mode]
                    if fifths >= 0: s.keyAlts = dict (zip (accs[:fifths], fifths * ['1']))
                    else:           s.keyAlts = dict (zip (accs[fifths:], -fifths * ['-1']))
                elif field.startswith ('none') or field == '':  # the default key
                    fifths = 0
                    mode = 'maj'
                if mode:
                    k = E.Element ('key')
                    addElemT (k, 'fifths', str (fifths), lev + 2)
                    addElemT (k, 'mode', s.modTab [mode], lev + 2)
                    atts.append ((2, k))
                doClef ()
            elif ftype == 'L':
                s.unitLcur = map (int, field.split ('/'))
                if len (s.unitLcur) == 1 or s.unitLcur[1] not in s.typeMap:
                    info ('L:%s is not allowed, 1/8 assumed' % field)
                    s.unitLcur = 1,8
            elif ftype == 'V':
                doClef ()
            elif ftype == 'I':
                miditrans = s.doField_I (ftype, field)
                if miditrans:   # when "%%MIDI transpose" matched insert 'attribute/transpose/chromatic'
                    e = E.Element ('transpose')
                    addElemT (e, 'chromatic', miditrans, lev + 2)
                    atts.append ((5, e))
            elif ftype == 'Q':
                s.doTempo (maat, field, lev)
            elif ftype in 'TCOAZNGHRBDFSU':
                info ('**illegal header field in body: %s, content: %s' % (ftype, field))
            else:
                info ('unhandled field: %s, content: %s' % (ftype, field))

        if atts:
            att = E.Element ('attributes')      # insert sub elements in the order required by musicXML
            addElem (maat, att, lev)
            for _, att_elem in sorted (atts):   # ordering !
                addElem (att, att_elem, lev + 1)

    def doTempo (s, maat, field, lev):
        t = re.search (r'(\d)/(\d\d?)\s*=\s*([.\d]+)|([.\d]+)', field)
        if not t: return
        try:
            if t.group (4):
                num, den, upm = 1, s.unitLcur[1] , float (t.group (4))
            else:
                num, den, upm = int (t.group (1)), int (t.group (2)), float (t.group (3))
        except: return  # float or int conversion failure
        if num != 1: info ('in Q: numerator > 1 in %d/%d not supported' % (num, den))
        qpm = 4. * num * upm / den
        metro = E.Element ('metronome')
        u = E.Element ('beat-unit'); u.text = s.typeMap [4 * den]
        pm = E.Element ('per-minute'); pm.text = '%.2f' % upm
        dir = addDirection (maat, metro, lev, [u, pm], placement='above')
        sound = E.Element ('sound'); sound.set ('tempo', '%.2f' % qpm)
        addElem (dir, sound, lev + 1)

    def mkBarline (s, maat, loc, lev, style='', dir='', ending=''):
        b = E.Element ('barline', location=loc)
        if style:
            addElemT (b, 'bar-style', style, lev + 1)
        if s.curVolta:    # first stop a current volta
            end = E.Element ('ending', number=s.curVolta, type='stop')
            s.curVolta = ''
            if loc == 'left':   # stop should always go to a right barline
                bp = E.Element ('barline', location='right')
                addElem (bp, end, lev + 1)
                addElem (s.prevmsre, bp, lev)   # prevmsre has no right barline! (ending would have stopped there)
            else:
                addElem (b, end, lev + 1)
        if ending:
            ending = ending.replace ('-',',')   # MusicXML only accepts comma's
            endtxt = ''
            if ending.startswith ('"'):     # ending is a quoted string
                endtxt = ending.strip ('"')
                ending = '33'               # any number that is not likely to occur elsewhere
            end = E.Element ('ending', number=ending, type='start')
            if endtxt: end.text = endtxt    # text appears in score in stead of number attribute
            addElem (b, end, lev + 1)
            s.curVolta = ending
        if dir:
            r = E.Element ('repeat', direction=dir)
            addElem (b, r, lev + 1)
        addElem (maat, b, lev)

    def doChordSym (s, maat, sym, lev):
        alterMap = {'#':'1','=':'0','b':'-1'}
        rnt = sym.root.t
        chord = E.Element ('harmony')
        addElem (maat, chord, lev)
        root = E.Element ('root')
        addElem (chord, root, lev + 1)
        addElemT (root, 'root-step', rnt[0], lev + 2)
        if len (rnt) == 2: addElemT (root, 'root-alter', alterMap [rnt[1]], lev + 2)
        kind = s.chordTab.get (sym.kind.t[0], 'major')
        addElemT (chord, 'kind', kind, lev + 1)
        degs = getattr (sym, 'degree', '')
        if degs:
            if type (degs) != types.ListType: degs = [degs]
            for deg in degs:
                deg = deg.t[0]
                if deg[0] == '#':   alter = '1';  deg = deg[1:]
                elif deg[0] == 'b': alter = '-1'; deg = deg[1:]
                else:               alter = '0';  deg = deg
                degree = E.Element ('degree')
                addElem (chord, degree, lev + 1)
                addElemT (degree, 'degree-value', deg, lev + 2)
                addElemT (degree, 'degree-alter', alter, lev + 2)
                addElemT (degree, 'degree-type', 'add', lev + 2)

    def mkMeasure (s, i, t, lev, fieldmap={}):
        s.msreAlts = {}
        s.ntup = -1
        overlay = 0
        maat = E.Element ('measure', number = str(i))
        if fieldmap: s.doFields (maat, fieldmap, lev + 1)
        if s.linebrk:   # there was a line break in the previous measure
            e = E.Element ('print')
            e.set ('new-system', 'yes')
            addElem (maat, e, lev + 1)
            s.linebrk = 0
        if s.bardecos:  # output coda and segno attached to the previous right barline
            s.staffDecos (s.bardecos, maat, lev + 1)
            s.bardecos = []
        for it, x in enumerate (t):
            if x.name == 'note' or x.name == 'rest':
                note = s.mkNote (x, lev + 1)
                addElem (maat, note, lev + 1)
            elif x.name == 'lbar':
                bar = x.t[0]
                if bar == '|': pass # skip redundant bar
                elif ':' in bar:    # forward repeat
                    volta = x.t[1] if len (x.t) == 2  else ''
                    s.mkBarline (maat, 'left', lev + 1, style='heavy-light', dir='forward', ending=volta)
                else:               # bar must be a volta number
                    s.mkBarline (maat, 'left', lev + 1, ending=bar)
            elif x.name == 'rbar':
                if hasattr (x, 'deco'): # MuseScore does not support this -> emergency solution
                    s.staffDecos (x.deco.t, maat, lev + 1, bardecos=1)  # coda, segno -> next measure
                bar = x.t[0]
                if bar == '.|':
                    s.mkBarline (maat, 'right', lev + 1, style='dotted')
                elif ':' in bar:  # backward repeat
                    s.mkBarline (maat, 'right', lev + 1, style='light-heavy', dir='backward')
                elif bar == '||':
                    s.mkBarline (maat, 'right', lev + 1, style='light-light')
                elif bar == '[|]' or bar == '[]':
                    s.mkBarline (maat, 'right', lev + 1, style='none')
                elif '[' in bar or ']' in bar:
                    s.mkBarline (maat, 'right', lev + 1, style='light-heavy')
                elif bar[0] == '&': overlay = 1
            elif x.name == 'tup':
                if len (x.t) == 3:  n, into, nts = x.t
                else:               n, into, nts = x.t[0], 0, 0
                if into == 0: into = 3 if n in [2,4,8] else 2
                if nts == 0: nts = n
                s.tmnum, s.tmden, s.ntup = n, into, nts
            elif x.name == 'deco':
                s.staffDecos (x.t, maat, lev + 1)   # output staff decos, postpone note decos to next note
            elif x.name == 'text':
                pos, text = x.t[:2]
                place = 'above' if pos == '^' else 'below'
                words = E.Element ('words')
                words.text = text
                addDirection (maat, words, lev + 1, placement=place)
            elif x.name == 'inline':
                fieldtype, fieldval = x.t[:2]
                s.doFields (maat, {fieldtype:fieldval}, lev + 1)
            elif x.name == 'grace':
                for i, grp in enumerate (x.t):  # grp-> Optional (decorations) + stem
                    if len (grp) == 2:  # with decorations
                        deco, nt = grp
                        nt.deco = deco  # put the decos into the note
                    else: nt = grp[0]   # no decorations
                    nt.grace = 1        # set grace attribute
                    note = s.mkNote (nt, lev + 1)
                    if i == 0 and getattr (x, 'accia', ''):
                        note.find ('grace').set ('slash', 'yes')
                    addElem (maat, note, lev + 1)
            elif x.name == 'linebrk':
                s.supports_tag = 1
                if it > 0 and t[it -1].name == 'lbar':  # we are at start of measure
                    e = E.Element ('print')             # output linebreak now
                    e.set ('new-system', 'yes')
                    addElem (maat, e, lev + 1)
                else:
                    s.linebrk = 1   # output linebreak at start of next measure
            elif x.name == 'chordsym':
                s.doChordSym (maat, x, lev + 1)
        s.stopBeams ()
        s.prevmsre = maat
        return maat, overlay

    def mkPart (s, maten, id, lev, attrs):
        s.slurstack = []
        s.unitLcur = s.unitL    # set the default unit length at begin of each voice
        s.curVolta = ''
        s.lyrdash = {}
        s.linebrk = 0
        s.midprg = ['', '']     # MIDI channel nr, program nr for the current part
        part = E.Element ('part', id=id)
        msre, overlay = s.mkMeasure (1, maten[0], lev + 1, attrs)
        addElem (part, msre, lev + 1)
        for i, maat in enumerate (maten[1:]):
            msre, next_overlay = s.mkMeasure (i+2, maat, lev + 1)
            if overlay: mergePartMeasure (part, msre)
            else:       addElem (part, msre, lev + 1)
            overlay = next_overlay
        return part

    def mkScorePart (s, id, vids_p, partAttr, lev):
        naam, subnm, midprg = partAttr [id]
        sp = E.Element ('score-part', id='P'+id)
        nm = E.Element ('part-name')
        nm.text = naam
        addElem (sp, nm, lev + 1)
        snm = E.Element ('part-abbreviation')
        snm.text = subnm
        if subnm: addElem (sp, snm, lev + 1)    # only add if subname was given
        if s.staves: instr_vids = [vids for vids in s.staves if vids[0] == id][0]
        else:        instr_vids = [id]
        inst = []
        for id in instr_vids:
            naam, subnm, midprg = partAttr [id]
            midchan, midprog = midprg
            if not midchan and not midprog: continue    # only add if program nr or channel was given
            si = E.Element ('score-instrument', id='I-'+id)
            addElemT (si, 'instrument-name', naam, lev + 2)
            mi = E.Element ('midi-instrument', id='I-'+id)
            dflt_channel = vids_p.index (id) + 1        # xml midi chan and prog start with 1
            addElemT (mi, 'midi-channel', midchan or str (dflt_channel), lev + 2)
            if midprog: addElemT (mi, 'midi-program', str (int (midprog) + 1), lev + 2) # compatible with abc2midi
            addElemT (mi, 'volume', '80', lev + 2)
            addElemT (mi, 'pan', '0', lev + 2)
            inst.append ((si, mi))
        for si, mi in inst: addElem (sp, si, lev + 1)
        for si, mi in inst: addElem (sp, mi, lev + 1)
        return sp, len (inst)

    def mkPartlist (s, vids, partAttr, lev):
        def addPartGroup (sym, num):
            pg = E.Element ('part-group', number=str (num), type='start')
            addElem (partlist, pg, lev + 1)
            addElemT (pg, 'group-symbol', sym, lev + 2)
            addElemT (pg, 'group-barline', 'yes', lev + 2)
        partlist = E.Element ('part-list')
        g_num = 0       # xml group number
        nInstrs = []    # number of instruments in each part
        for g in (s.groups or vids):    # brace/bracket or abc_voice_id
            if   g == '[': g_num += 1; addPartGroup ('bracket', g_num)
            elif g == '{': g_num += 1; addPartGroup ('brace', g_num)
            elif g in '}]':
                pg = E.Element ('part-group', number=str (g_num), type='stop')
                addElem (partlist, pg, lev + 1)
                g_num -= 1
            else:   # g = abc_voice_id
                if g not in vids: continue  # error in %%score
                sp, nInst = s.mkScorePart (g, vids, partAttr, lev + 1)
                addElem (partlist, sp, lev + 1)
                nInstrs.append (nInst)
        return partlist, nInstrs

    def doField_I (s, type, x):
        def readPfmt (x, n): # read ABC page formatting constant
            if not s.pageFmtAbc: s.pageFmtAbc = s.pageFmtDef    # set the default values on first change
            ro = re.search (r'[^.\d]*([\d.]+)\s*(cm|in|pt)?', x)    # float followed by unit
            if ro:
                x, unit = ro.groups ()  # unit == None when not present
                u = {'cm':10., 'in':25.4, 'pt':25.4/72} [unit] if unit else 1.
                s.pageFmtAbc [n] = float (x) * u   # convert ABC values to millimeters
            else: info ('error in page format: %s' % x)

        if x.startswith ('score') or x.startswith ('staves'):   # extract voice mapping
            if s.staves != [] or s.groups != []:
                info ('%%%%%s dropped, multiple stave mappings not supported' % x)
                return
            idchrs = alphanums + '_'                            # chars allowed in a voice_id
            x = x.replace ('score','').replace ('staves','').replace ('|',' ')
            r3 = Word (idchrs) | Suppress ('(') + OneOrMore (Word (idchrs)) + Suppress (')')
            s.staves = [res[0].asList () for res in r3.scanString (x)]
            r4 = Suppress ('(') + OneOrMore (Word (idchrs)) + Suppress (')')    # voice group == staff
            r4.setParseAction (lambda t: t[0] + ' ')            # replace voice group by first voice -> staff id
            r5 = Word (idchrs) | oneOf ('[ ] { }')              # scan part names and braces/brackets
            for res in r5.scanString (r4.transformString (x)):  # list of merged part names
                s.groups.extend (res[0].asList ())              # with interspersed {[ and }]
        elif x.startswith ('scale'): readPfmt (x, 0)
        elif x.startswith ('pageheight'): readPfmt (x, 1)
        elif x.startswith ('pagewidth'): readPfmt (x, 2)
        elif x.startswith ('leftmargin'): readPfmt (x, 3)
        elif x.startswith ('rightmargin'): readPfmt (x, 4)
        elif x.startswith ('topmargin'): readPfmt (x, 5)
        elif x.startswith ('botmargin'): readPfmt (x, 6)
        elif x.startswith ('MIDI'):
            r = re.search (r'program *(\d*) +(\d+)', x)
            if r:
                ch, prg = r.groups ()   # channel nr or '', program nr
                s.midprg[1] = prg
                if ch: s.midprg[0] = ch
            r = re.search (r'channel\D*(\d+)', x)
            if r:
                s.midprg[0] = r.group (1)
            r = re.search (r'transpose[^-\d]*(-?\d+)', x)
            if r: return r.group (1)
        else: info ('skipped I-field: %s' % x)

    def doHeaderField (s, fld, attrmap):
        type, value = fld.t[:2]
        if not value:    # skip empty field
            return
        if type == 'M':
            attrmap [type] = value
        elif type == 'L':
            try: s.unitL = map (int, fld.t[1].split ('/'))
            except:
                info ('illegal unit length:%s, 1/8 assumed' % fld.t[1])
                s.unitL = 1,8
            if len (s.unitL) == 1 or s.unitL[1] not in s.typeMap:
                info ('L:%s is not allowed, 1/8 assumed' % fld.t[1])
                s.unitL = 1,8
        elif type == 'K':
            attrmap[type] = value
        elif type == 'T':
            if s.title: s.title = s.title + '\n' + value
            else:       s.title = value
        elif type == 'C':
            s.creator ['composer'] = s.creator.get ('composer', '') + value
        elif type == 'Z':
            s.creator ['lyricist'] = s.creator.get ('lyricist', '') + value
        elif type == 'U':
            sym = fld.t[2].strip ('!+')
            s.usrSyms [value] = sym
        elif type == 'I':
            s.doField_I (type, value)
        elif type == 'Q':
            attrmap[type] = value
        elif type in s.creditTab: s.credits [s.creditTab [type]] = value
        else:
            info ('skipped header: %s' % fld)

    def mkIdentification (s, score, lev):
        if s.title:
            addElemT (score, 'movement-title', s.title, lev + 1)
        ident = E.Element ('identification')
        addElem (score, ident, lev + 1)
        if s.creator:
            for ctype, cname in s.creator.items ():
                c = E.Element ('creator', type=ctype)
                c.text = cname
                addElem (ident, c, lev + 2)
        encoding = E.Element ('encoding')
        addElem (ident, encoding, lev + 2)
        encoder = E.Element ('encoder')
        encoder.text = 'abc2xml version %d' % VERSION
        addElem (encoding, encoder, lev + 3)
        if s.supports_tag:  # avoids interference of auto-flowing and explicit linebreaks
            suports = E.Element ('supports', attribute="new-system", element="print", type="yes", value="yes")
            addElem (encoding, suports, lev + 3)
        encodingDate = E.Element ('encoding-date')
        encodingDate.text = str (datetime.date.today ())
        addElem (encoding, encodingDate, lev + 3)

    def mkDefaults (s, score, lev):
        if s.pageFmtCmd: s.pageFmtAbc = s.pageFmtCmd
        if not s.pageFmtAbc: return # do not output the defaults if none is desired
        space, h, w, l, r, t, b = s.pageFmtAbc
        mils = 4 * space    # staff height in millimeters
        scale = 40. / mils  # tenth's per millimeter
        dflts = E.Element ('defaults')
        addElem (score, dflts, lev)
        scaling = E.Element ('scaling')
        addElem (dflts, scaling, lev + 1)
        addElemT (scaling, 'millimeters', '%g' % mils, lev + 2)
        addElemT (scaling, 'tenths', '40', lev + 2)
        layout = E.Element ('page-layout')
        addElem (dflts, layout, lev + 1)
        addElemT (layout, 'page-height', '%g' % (h * scale), lev + 2)
        addElemT (layout, 'page-width', '%g' % (w * scale), lev + 2)
        margins = E.Element ('page-margins', type='both')
        addElem (layout, margins, lev + 2)
        addElemT (margins, 'left-margin', '%g' % (l * scale), lev + 3)
        addElemT (margins, 'right-margin', '%g' % (r * scale), lev + 3)
        addElemT (margins, 'top-margin', '%g' % (t * scale), lev + 3)
        addElemT (margins, 'bottom-margin', '%g' % (b * scale), lev + 3)

    def mkCredits (s, score, lev):
        if not s.credits: return
        credit = E.Element ('credit', page='1')
        addElem (score, credit, lev)
        for ctype, ctext in s.credits.items ():
            addElemT (credit, 'credit-type', ctype, lev + 2)
            addElemT (credit, 'credit-words', ctext, lev + 2)

    def parse (s, abc_string):
        abctext = abc_string if type (abc_string) == types.UnicodeType else decodeInput (abc_string)
        s.reset ()
        header, voices = splitHeaderVoices (abctext)
        ps = []
        try:
            hs = abc_header.parseString (header) if header else ''
            for id, vce_lyr in voices:  # vce_lyr = [voice-block] where voice-block = (measures, corresponding lyric lines)
                vcelyr = []             # list of measures where measure = list of elements (see syntax)
                prevLeftBar = None      # previous voice ended with a left-bar symbol (double repeat)
                for voice, lyr in vce_lyr:
                    vce = abc_voice.parseString (voice).asList ()
                    if not vce: raise Exception ('Empty Tune!!')
                    if prevLeftBar:
                        vce[0].insert (0, prevLeftBar)  # insert at begin of first measure
                        prevLeftBar = None
                    if vce[-1] and vce[-1][-1].name == 'lbar':  # last measure ends with an lbar
                        prevLeftBar = vce[-1][-1]
                        del vce[-1]     # lbar was the only element in measure vce[-1]
                    lyr = lyr.strip ()  # strip leading \n (because we split on '\nw:...')
                    if lyr:             # no lyrics for this measures-lyrics block
                        lyr = lyr_block.parseString (lyr).asList ()
                        xs = alignLyr (vce, lyr)    # put all syllables into corresponding notes
                    else: xs = vce
                    vcelyr += xs
                elem1 = vcelyr [0][0]   # the first element of the first measure
                if  elem1.name == 'inline'and elem1.t[0] == 'V':    # is a voice definition
                    voicedef = elem1 
                    del vcelyr [0][0]   # do not read voicedef twice
                else:
                    voicedef = ''
                ps.append ((id, voicedef, vcelyr))
        except ParseException, err:
            if err.loc > 40:    # limit length of error message, compatible with markInputline
                err.pstr = err.pstr [err.loc - 40: err.loc + 40]
                err.loc = 40
            xs = err.line[err.col-1:err.col+2]
            try:    info (err.line.encode ('utf-8'), warn=0)    # err.line is a unicode string!!
            except: info (err.line.encode ('latin-1'), warn=0)
            info ((err.col-1) * '-' + '^', warn=0)
            if re.match (r'\[[OAPZNGHRBDFSXTCIU]:', xs):
                info ('Error: header-only field %s appears after K:' % xs[1:], warn=0)
            else:
                info ('Syntax error at column %d' % err.col, warn=0)
            raise err

        s.unitL = (1, 8)
        s.title = ''
        s.creator = {}  # {creator type -> name string}
        s.credits = {}  # {credit type -> string}
        score = E.Element ('score-partwise')
        attrmap = {'Div': str (s.divisions), 'K':'C treble', 'M':'4/4'}
        for res in hs:
            if res.name == 'field':
                s.doHeaderField (res, attrmap)
            else:
                info ('unexpected header item: %s' % res)

        lev = 0
        vids, parts, partAttr = [], [], {}
        for vid, vcedef, vce in ps: # vcedef == first voice inline field
            pname, psubnm = vid, '' # partname = voice ID
            if not vcedef:          # simple abc without voice definitions
                attrmap ['V'] = ''
            else:                   # abc with voice definitions
                if vid != vcedef.t[1]: info ('voice ids unequal: %s (reg-ex) != %s (grammar)' % (vid, vcedef.t[1]))
                rn = re.search (r'(?:name|nm)="([^"]*)"', vcedef.t[2])
                if rn: pname = rn.group (1)
                rn = re.search (r'(?:subname|snm|sname)="([^"]*)"', vcedef.t[2])
                if rn: psubnm = rn.group (1)
                attrmap ['V'] = vcedef.t[2]
            pid = 'P%s' % vid       # let part id start with an alpha
            s.vid = vid             # avoid parameter passing, needed in mkNote for instrument id
            part = s.mkPart (vce, pid, lev + 1, attrmap)
            parts.append (part)
            vids.append (vid)
            partAttr [vid] = (pname, psubnm, s.midprg)
        parts = mergeParts (parts, vids, s.staves)  # merge parts into staves as indicated by %%score

        s.mkIdentification (score, lev)
        s.mkDefaults (score, lev + 1)
        s.mkCredits (score, lev)

        partlist, nInstrs = s.mkPartlist (vids, partAttr, lev + 1)
        addElem (score, partlist, lev + 1)
        for ip, part in enumerate (parts):
            if nInstrs [ip] < 2:    # no instrument tag needed for one- or no-instrument parts
                removeElems (part, 'measure/note', 'instrument')
            addElem (score, part, lev + 1)

        return score

def decodeInput (data_string):
    try:        enc = 'utf-8';   unicode_string = data_string.decode (enc)
    except:
        try:    enc = 'latin-1'; unicode_string = data_string.decode (enc)
        except: raise Exception ('data not encoded in utf-8 nor in latin-1')
    info ('decoded from %s' % enc)
    return unicode_string
    
def fixDoctype (elem, enc):
    xs = E.tostring (elem, encoding=enc)
    ys = xs.split ('\n')
    if enc == 'utf-8': ys.insert (0, "<?xml version='1.0' encoding='utf-8'?>")  # crooked logic of ElementTree lib
    ys.insert (1, '<!DOCTYPE score-partwise PUBLIC "-//Recordare//DTD MusicXML 3.0 Partwise//EN" "http://www.musicxml.org/dtds/partwise.dtd">')
    return '\n'.join (ys)

def convert (pad, fnm, abc_string):
    # these globals should be initialised (as in the __main__ secion) before calling convert
    global mxm                              # optimisation 1: keep instance of MusicXml
    global abc_header, abc_voice, lyr_block # optimisation 2: keep computed grammars
    score = mxm.parse (abc_string)
    if pad:
        outfile = file (os.path.join (pad, fnm + '.xml'), 'wb')
        outfile.write (fixDoctype (score, 'utf-8'))
        outfile.close ()
    else:
        outfile = sys.stdout
        outfile.write (fixDoctype (score, 'utf-8'))
        outfile.write ('\n')
    info ('%s.xml written' % fnm, warn=0)
    
#----------------
# Main Program
#----------------
if __name__ == '__main__':
    from optparse import OptionParser
    from glob import glob
    import time
    global abc_header, abc_voice, lyr_block # keep computed grammars
    mxm = MusicXml ()

    parser = OptionParser (usage='%prog [-h] [-m SKIP NUM] [-o DIR] [-p PFMT] <file1> [<file2> ...]', version='version %d' % VERSION)
    parser.add_option ("-o", action="store", help="store xml files in DIR", default='', metavar='DIR')
    parser.add_option ("-m", action="store", help="skip SKIP tunes, then read at most NUM tunes", nargs=2, type='int', default=(0,1), metavar='SKIP,NUM')
    parser.add_option ("-p", action="store", help="page formatting in PFMT", default='', metavar='PFMT')
    options, args = parser.parse_args ()
    if len (args) == 0: parser.error ('no input file given')
    pad = options.o
    if pad:
        if not os.path.exists (pad): os.mkdir (pad)
        if not os.path.isdir (pad): parser.error ('%s is not a directory' % pad)
    if options.p:   # set page formatting values
        try:        # space, page-height, -width, margin-left, -right, -top, -bottom
            mxm.pageFmtCmd = map (float, options.p.split (','))
            if len (mxm.pageFmtCmd) != 7: raise Exception ('-p needs 7 values')
        except Exception, err: parser.error (err)

    abc_header, abc_voice, lyr_block = abc_grammar ()  # compute grammar only once per file set
    fnmext_list = []
    for i in args: fnmext_list += glob (i)
    if not fnmext_list: parser.error ('none of the input files exist')
    t_start = time.time ()
    for X, fnmext in enumerate (fnmext_list):
        fnm, ext = os.path.splitext (fnmext)
        if ext.lower () not in ('.abc'):
            info ('skipped input file %s, it should have extension .abc' % fnmext)
            continue
        if os.path.isdir (fnmext):
            info ('skipped directory %s. Only files are accepted' % fnmext)
            continue

        fobj = open (fnmext, 'rb')
        encoded_data = fobj.read ()
        fobj.close ()
        fragments =  encoded_data.split ('X:')
        preamble = fragments [0]    # tunes can be preceeded by formatting instructions
        tunes = fragments[1:]
        skip, num = options.m       # skip tunes, then read at most num tunes
        numtunes = min ([len (tunes), num])     # number of tunes to be converted
        for itune, tune in enumerate (tunes):
            if itune < skip: continue
            if itune >= skip + num: break
            tune = preamble + 'X:' + tune       # restore preamble before each tune
            fnmNum = '%s%02d' % (fnm, itune + 1) if numtunes > 1 else fnm
            try:
                convert (pad, fnmNum, tune)     # convert string abctext -> file pad/fnm.xml
            except ParseException, err: pass    # output already printed
            except Exception, err: info ('an exception occurred.\n%s' % err)
    info ('done in %.2f secs' % (time.time () - t_start))
