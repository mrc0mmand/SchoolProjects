module SymbolDB (readDB) where

import Control.Monad
import Data.Function (on)
import Data.List (sortBy)
import System.Exit
import System.Directory
import System.IO

strToCharTuple :: (String, Float) -> (Char, Float)
strToCharTuple (x, y) = (head x, y)

tupleToFreqTuple :: [String] -> (String, Float)
tupleToFreqTuple (x:y:_) = (x, read y :: Float)

lineToTuple :: String -> (String, Float)
lineToTuple line = tupleToFreqTuple $ words line

readDB :: FilePath -> IO ([(Char, Float)], [(String, Float)], [(String, Float)])
readDB db = do
    fileExists <- doesFileExist db

    unless fileExists $
        die ("Database file '" ++ db ++ "' does not exist")

    fh <- openFile db ReadMode
    contents <- hGetContents fh

    let rows = lines contents
        freqs_ = map lineToTuple rows
        letterFreqs = sortBy (flip compare `on` snd) .
                      map strToCharTuple .
                      filter (\(x,y) -> length x == 1) $ freqs_
        digramFreqs= sortBy (flip compare `on` snd) .
                     filter (\(x,y) -> length x == 2) $ freqs_
        trigramFreqs = sortBy (flip compare `on` snd) .
                       filter (\(x,y) -> length x == 3) $ freqs_

    -- Force the lazy IO to read the entire file
    seq (length contents) (return ())

    hClose fh
    return (letterFreqs, digramFreqs, trigramFreqs)
