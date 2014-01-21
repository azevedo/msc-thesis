%include polycode.fmt

\begin{code}

-- * Type definitions

data AbcTunes =  AbcTunes [AbcTune]  


--  Tune definition
data AbcTune =  AbcTune {  
                  abc_vers :: Int,              
                  client_data :: string,        
                  micro_tb :: [Int],            
                  symbols :: [AbcSym]           
                }


--  Symbol Definition
data AbcSym =   AbcSym {
                  type :: Int,                  
                  state :: Int,                 
                  colnum :: Int,                
                  flags :: Int,                 
                  linenum :: Int,               
                  text :: String,               
                  comment :: String,            
                  u :: SymInfo                  
                }


--  Tune Information Definition
data SymInfo  = Key {                           -- K: info
                  sf :: Int,                    
                  empty :: Int,                 
                  exp :: Int,                   
                  mode :: Int,                  
                  nacc :: Int,                  
                  octave :: Int,                
                  pits :: [Int],
                  accs :: [Int]
                }
              | Length {                        -- L: info
                  base_length :: Int
                }
              | Meter {                         -- M: info
                  wmeasure :: Int, 
                  nmeter :: Int,  
                  expdur :: Int, 
                  meter :: [MeterDef]
                }
              | Tempo {                         -- Q: info
                  str1 :: String,
                  length :: [Int],
                  value :: String,
                  str2 :: String 
                }
              | Voice {                         -- V: info
                  id :: String, 
                  fname :: String,
                  nname :: String,
                  scale :: Float,
                  voice :: Int, 
                  octave :: Int, 
                  merge :: Int, 
                  stem :: Int, 
                  gstem :: Int,
                  dyn :: Int,  
                  lyrics :: Int,
                  gchord :: Int
                }
              | Bar {                           -- bar, mrest (multi-measure rest) or mrep (measure repeat)
                  type :: Int,
                  repeat_bar :: Int,
                  len :: Int,  
                  dotted :: Int,
                  dc :: Deco  
                }
              | Clef {                          -- clef (and staff!)
                  name :: String,               
                  staffscale :: Float,          
                  stafflines :: Int,            
                  type :: Int,                  
                  line :: Int,                  
                  octave :: Int,                
                  transpose :: Int,             
                  invis :: Int,
                  check_pitch :: Int            
                }
              | Note {                          -- note, rest
                  note :: NoteDef
                }
              | User {                          -- user defined accent
                  symbol :: Int,
                  value :: Int
                }
              | Eoln {                          -- end of line
                  type :: Int
                }
              | VOver {                         -- voice overlay
                  type :: Int,
                  voice :: Int
                }
              | Tuplet {                        -- tuplet (n:t:x
                  p_plet :: Int,
                  q_plet :: Int,
                  r_plet :: Int 
                }



--  Meter Definition
data MeterDef = MeterDef {
                  top :: String, 
                  bot :: String 
                }


--  Note Definition
data NoteDef  = NoteDef {                       -- note or rest
                  pits :: [Int], 
                  lens :: [Int],
                  accs :: [Int],
                  sl1 :: [Int], 
                  sl2 :: [Int],
                  ti1 :: [Int],  
                  decs :: [Int],
                  chlen :: Int,   
                  nhd :: Int,    
                  slur_st :: Int,  
                  slur_end :: Int, 
                  brhythm :: Int, 
                  dc :: Deco     
                }


-- Decoration Definition
data Deco     = Deco {                          -- decorations
                  n :: Int,     
                  h :: Int,    
                  s :: Int,   
                  t :: [Int] 
                }
\end{code}
