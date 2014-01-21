-- * Type definitions

data AbcTunes =  AbcTunes [AbcTune]  

-- | Tune definition
data AbcTune =  AbcTune {  
                  abc_vers :: Int,              -- ^ ABC version = (H << 16) + (M << 8) + L
                  client_data :: String,        -- ^ client data
                  micro_tb :: [Int],            -- ^ microtone values [ (n-1) | (d-1) ]
                  symbols :: [AbcSym]           -- ^ list of symbols
                }


-- | Symbol Definition
data AbcSym =   AbcSym {
                  type :: Int,                  -- ^ symbol type; 0: null | 1: info | 2: pscom | 3: clef | 4: note | 5: rest | 6: bar | 7: eoln | 8: mrest | 9: mrep | 10: v_over | 11: tuplet
                  state :: Int,                 -- ^ symbol state in file/tune; 0: global | 1: in header (after X:) | 2: in tune (after K:)
                  colnum :: Int,                -- ^ ABC source column number
                  flags :: Int,                 -- ^ 1: error around this symbol | 2: invisible symbol | 4: space before a note | 8: note with no stem | 16: may start a lyric here | 32: grace note | 64: end of grace note sequence | 128: short appoggiatura
                  linenum :: Int,               -- ^ ABC source line number
                  text :: String,               -- ^ main text (INFO, PSCOM), guitar chord (NOTE, REST, BAR)
                  comment :: String,            -- ^ comment part (when keep_comment)
                  u :: SymInfo                  -- ^ information on the tune; content dependent on type
                }


-- | Tune Information Definition
data SymInfo  = Key {                           -- K: info
                  sf :: Int,                    -- ^ sharp (> 0) flats (< 0)
                  empty :: Int,                 -- ^ clef alone if 1, 'none' if 2
                  exp :: Int,                   -- ^ exp (1) or mod (0)
                  mode :: Int,                  -- ^ mode; 0: Ionian, 1: Dorian, 2: Phrygian, 3: Lydian, 4: Mixolydian, 5: Aeolian, 6: Locrian, 7: major, 8:minor, 9: HP, 10: Hp
                  nacc :: Int,                  -- ^ number of explicit accidentals, (-1) if no accidental
                  octave :: Int,                -- ^ 'octave='
                  pits :: [Int],
                  accs :: [Int]
                }
              | Length {                        -- L: info
                  base_length :: Int            -- ^ basic note length
                }
              | Meter {                         -- M: info
                  wmeasure :: Int,              -- ^ duration of a measure
                  nmeter :: Int,                -- ^ number of meter elements
                  expdur :: Int,                -- ^ explicit measure duration
                  meter :: [MeterDef]
                }
              | Tempo {                         -- Q: info
                  str1 :: String,               -- ^ string before
                  length :: [Int],              -- ^ up to 4 note lengths
                  value :: String,              -- ^ tempo value
                  str2 :: String                -- ^ string after
                }
              | Voice {                         -- V: info
                  id :: String,                 -- ^ voice ID 
                  fname :: String,              -- ^ full name
                  nname :: String,              -- ^ nick name
                  scale :: Float,               -- ^ != 0 when change
                  voice :: Int,                 -- ^ voice number
                  octave :: Int,                -- ^ 'octave=' - same as in K:
                  merge :: Int,                 -- ^ merge with previous voice
                  stem :: Int,                  -- ^ have stems up or down (2 = auto)
                  gstem :: Int,                 -- ^ have grace stems up or down (2 = auto)
                  dyn :: Int,                   -- ^ have dynamic marks above or below the staff
                  lyrics :: Int,                -- ^ have lyrics above or below the staff
                  gchord :: Int                 -- ^ have gchord above or below the staff
                }
              | Bar {                           -- bar, mrest (multi-measure rest) or mrep (measure repeat)
                  type :: Int,
                  repeat_bar :: Int,
                  len :: Int,                   -- ^ len if mrest or mrep
                  dotted :: Int,
                  dc :: Deco                    -- ^ decorations
                }
              | Clef {                          -- clef (and staff!)
                  name :: String,               -- ^ PS drawing function
                  staffscale :: Float,          -- ^ != 0 when change; sets the scale of the associated staff. Default is 1, maximum value is 3, minimum is 0.5
                  stafflines :: Int,            -- ^ >= 0 when change; sets the number of lines of the associated staff
                  type :: Int,                  -- ^ no clef if < 0; 0: 'treble' | 1: 'alto' | 2: 'bass' | 3: 'perc'
                  line :: Int,                  -- ^ staff line the clef starts
                  octave :: Int,                -- ^ n octaves above(positive)/below(negative)
                  transpose :: Int,             -- ^ currently does nothing, but is supported by abcMIDI
                  invis :: Int,
                  check_pitch :: Int            -- ^ check if old abc2ps transposition
                }
              | Note {                          -- note, rest
                  note :: NoteDef
                }
              | User {                          -- user defined accent
                  symbol :: Int,
                  value :: Int
                }
              | Eoln {                          -- end of line
                  type :: Int                   -- ^ 0: end of line; 1: continuation ('\'); 2: line break ('!')
                }
              | VOver {                         -- voice overlay
                  type :: Int,                  -- ^ 0: '&' | 1: '(&' | 2: '&)'
                  voice :: Int
                }
              | Tuplet {                        -- tuplet (<n>:<t>:<x>
                  p_plet :: Int,                -- ^ number printed over the tuplet; put <n> notes
                  q_plet :: Int,                -- ^ time; into the time of <t>
                  r_plet :: Int                 -- ^ number of notes; for the next <x> notes
                }

-- | Metere Definition
data MeterDef = MeterDef {
                  top :: String,                -- ^ top value  
                  bot :: String                 -- ^ bottom value  
                }

-- | Note Definition
data NoteDef  = NoteDef {                       -- note or rest
                  pits :: [Int],                -- ^ pitches
                  lens :: [Int],                -- ^ note lengths (# pts in [1] if space); has the
                                                  -- absolute duration including when in
                                                  -- broken_rhythm and excluding when in tuplet (in
                                                  -- the latter it has the duration of the note
                                                  -- written and not its actual duration)
                  accs :: [Int],                -- ^ code for accidentals & index in micro_tb
                  sl1 :: [Int],                 -- ^ slur start per head
                  sl2 :: [Int],                 -- ^ number of slur end per head
                  ti1 :: [Int],                 -- ^ flag to start tie here
                  decs :: [Int],                -- ^ head decorations (index: 5 bits, len: 3 bits)
                  chlen :: Int,                 -- ^ chord length
                  nhd :: Int,                   -- ^ number of notes in chord - 1
                  slur_st :: Int,               -- ^ slurs starting here (2 bits array)
                  slur_end :: Int,              -- ^ number of slurs ending here
                  brhythm :: Int,               -- ^ broken rhythm
                  dc :: Deco                    -- ^ decorations
                }

-- | Decoration Definition
data Deco     = Deco {                          -- decorations
                  n :: Int,                     -- ^ whole number of decorations
                  h :: Int,                     -- ^ start of head decorations
                  s :: Int,                     -- ^ start of decorations from s: (d:)
                  t :: [Int]                    -- ^ decoration type
                }
