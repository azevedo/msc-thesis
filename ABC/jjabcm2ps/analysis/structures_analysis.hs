-- * Type definitions

-- | Tune definition
data AbcTune =  AbcTune {  
                  next :: AbcTunePointer,       -- ^ next tune
                  first_sym :: AbcSymPointer,   -- ^ first symbol
                  last_sym :: AbcSymPointer,    -- ^ last symbol
                  abc_vers :: Int,              -- ^ ABC version = (H << 16) + (M << 8) + L
                  client_data :: VoidPointer,   -- ^ client data
                  micro_tb :: [UnsignedShort]   -- ^ microtone values [ (n-1) | (d-1) ]
                }


-- | Symbol Definition
data AbcSym =   AbcSym {
                  tune :: AbcTunePointer,       -- ^ tune
                  next :: AbcSymPointer,        -- ^ next symbol
                  prev :: AbcSymPointer,        -- ^ previous symbol
                  type :: Char,                 -- ^ symbol type; 0: null | 1: info | 2: pscom | 3: clef | 4: note | 5: rest | 6: bar | 7: eoln | 8: mrest | 9: mrep | 10: v_over | 11: tuplet
                  state :: Char,                -- ^ symbol state in file/tune; 0: global | 1: in header (after X:) | 2: in tune (after K:)
                  colnum :: UnsignedShort,      -- ^ ABC source column number
                  flags :: UnsignedShort,       -- ^ 1: error around this symbol | 2: invisible symbol | 4: space before a note | 8: note with no stem | 16: may start a lyric here | 32: grace note | 64: end of grace note sequence | 128: short appoggiatura
                  linenum :: Int,               -- ^ ABC source line number
                  text :: String,               -- ^ main text (INFO, PSCOM), guitar chord (NOTE, REST, BAR)
                  comment :: String,            -- ^ comment part (when keep_comment)
                  u :: SymInfo                  -- ^ information on the tune; content dependent on type
                }


-- | Tune Information Definition
data SymInfo  = Key {                           -- K: info
                  sf :: SignedChar,             -- ^ sharp (> 0) flats (< 0)
                  empty :: Char,                -- ^ clef alone if 1, 'none' if 2
                  exp :: Char,                  -- ^ exp (1) or mod (0)
                  mode :: Char,                 -- ^ mode; 0: Ionian, 1: Dorian, 2: Phrygian, 3: Lydian, 4: Mixolydian, 5: Aeolian, 6: Locrian, 7: major, 8:minor, 9: HP, 10: Hp
                  nacc :: SignedChar,           -- ^ number of explicit accidentals, (-1) if no accidental
                  octave :: SignedChar,         -- ^ 'octave='
                  pits :: [SignedChar],
                  accs :: [UnsignedChar]
                }
              | Length {                        -- L: info
                  base_length :: Int            -- ^ basic note length
                }
              | Meter {                         -- M: info
                  wmeasure :: Short,            -- ^ duration of a measure
                  nmeter :: UnsignedChar,       -- ^ number of meter elements
                  expdur :: Char,               -- ^ explicit measure duration
                  meter :: [MeterDef]
                }
              | Tempo {                         -- Q: info
                  str1 :: String,               -- ^ string before
                  length :: [Short],            -- ^ up to 4 note lengths
                  value :: String,              -- ^ tempo value
                  str2 :: String                -- ^ string after
                }
              | Voice {                         -- V: info
                  id :: String,                 -- ^ voice ID 
                  fname :: String,              -- ^ full name
                  nname :: String,              -- ^ nick name
                  scale :: Float,               -- ^ != 0 when change
                  voice :: UnsignedChar,        -- ^ voice number
                  octave :: SignedChar,         -- ^ 'octave=' - same as in K:
                  merge :: Char,                -- ^ merge with previous voice
                  stem :: SignedChar,           -- ^ have stems up or down (2 = auto)
                  gstem :: SignedChar,          -- ^ have grace stems up or down (2 = auto)
                  dyn :: SignedChar,            -- ^ have dynamic marks above or below the staff
                  lyrics :: SignedChar,         -- ^ have lyrics above or below the staff
                  gchord :: SignedChar          -- ^ have gchord above or below the staff
                }
              | Bar {                           -- bar, mrest (multi-measure rest) or mrep (measure repeat)
                  type :: Int,
                  repeat_bar :: Char,
                  len :: Char,                  -- ^ len if mrest or mrep
                  dotted :: Char,
                  dc :: Deco                    -- ^ decorations
                }
              | Clef {                          -- clef (and staff!)
                  name :: String,               -- ^ PS drawing function
                  staffscale :: Float,          -- ^ != 0 when change; sets the scale of the associated staff. Default is 1, maximum value is 3, minimum is 0.5
                  stafflines :: SignedChar,     -- ^ >= 0 when change; sets the number of lines of the associated staff
                  type :: SignedChar,           -- ^ no clef if < 0; 0: 'treble' | 1: 'alto' | 2: 'bass' | 3: 'perc'
                  line :: Char,                 -- ^ staff line the clef starts
                  octave :: SignedChar,         -- ^ n octaves above(positive)/below(negative)
                  transpose :: SignedChar,      -- ^ currently does nothing, but is supported by abcMIDI
                  invis :: Char,
                  check_pitch :: Char           -- ^ check if old abc2ps transposition
                }
              | Note {                          -- note, rest
                  note :: NoteDef
                }
              | User {                          -- user defined accent
                  symbol :: UnsignedChar,
                  value :: UnsignedChar
                }
              | Eoln {                          -- end of line
                  type :: Char                  -- ^ 0: end of line; 1: continuation ('\'); 2: line break ('!')
                }
              | VOver {                         -- voice overlay
                  type :: Char,                 -- ^ 0: '&' | 1: '(&' | 2: '&)'
                  voice :: UnsignedChar
                }
              | Tuplet {                        -- tuplet (<n>:<t>:<x>
                  p_plet :: Char,               -- ^ number printed over the tuplet; put <n> notes
                  q_plet :: Char,               -- ^ time; into the time of <t>
                  r_plet :: Char                -- ^ number of notes; for the next <x> notes
                }

-- | Metere Definition
data MeterDef = MeterDef {
                  top :: String,                -- ^ top value  
                  bot :: String                 -- ^ bottom value  
                }

-- | Note Definition
data NoteDef  = NoteDef {                       -- note or rest
                  pits :: [SignedChar],         -- ^ pitches
                  lens :: [Short],              -- ^ note lengths (# pts in [1] if space)
                  accs :: [UnsignedChar],       -- ^ code for accidentals & index in micro_tb
                  sl1 :: [UnsignedChar],        -- ^ slur start per head
                  sl2 :: [Char],                -- ^ number of slur end per head
                  ti1 :: [Char],                -- ^ flag to start tie here
                  decs :: [UnsignedChar],       -- ^ head decorations (index: 5 bits, len: 3 bits)
                  chlen :: Short,               -- ^ chord length
                  nhd :: Char,                  -- ^ number of notes in chord - 1
                  slur_st :: UnsignedChar,      -- ^ slurs starting here (2 bits array)
                  slur_end :: Char,             -- ^ number of slurs ending here
                  brhythm :: SignedChar,        -- ^ broken rhythm
                  dc :: Deco                    -- ^ decorations
                }

-- | Decoration Definition
data Deco     = Deco {                          -- decorations
                  n :: Char,                    -- ^ whole number of decorations
                  h :: Char,                    -- ^ start of head decorations
                  s :: Char,                    -- ^ start of decorations from s: (d:)
                  t :: [UnsignedChar]           -- ^ decoration type
                }
