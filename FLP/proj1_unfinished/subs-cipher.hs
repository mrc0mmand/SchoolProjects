module Main (main) where

import NLP
import Control.Monad
import Data.Char
import Data.Function
import Data.List
import Data.Maybe
import Data.Set (fromList, toList)
import SymbolDB
import System.Directory
import System.Exit
import System.Console.GetOpt
import System.Environment
import System.IO
import System.IO.Error

-- Commandline arguments handling
data Options = Options {
    optPlaintext :: Bool,
    optPrintKey  :: Bool
}

startOptions = Options {
    optPlaintext = False,
    optPrintKey  = False
}

options :: [OptDescr (Options -> IO Options)]
options =
    [ Option "t" []
        (NoArg
            (\opt -> return opt { optPlaintext = True }))
        "Print plaintext to stdout"
    , Option "k" []
        (NoArg
            (\opt -> return opt { optPrintKey = True}))
        "Print key used to encrypt the plaintext"
    , Option "h" []
        (NoArg
            (\_ -> do
                prg <- getProgName
                let header = prg ++ " [args] db [input]"
                hPutStrLn stderr (usageInfo header options)
                exitSuccess))
        "Show help"
    ]

-- IO functions
readFileContents :: Handle -> IO String
readFileContents handle = do
    contents <- hGetContents handle
    -- Force the lazy IO to read the entire file
    seq (length contents) (return ())
    return contents

readStdInput :: IO String
readStdInput = readFileContents stdin

readInputFile :: FilePath -> IO String
readInputFile inFile = do
    fileExists <- doesFileExist inFile
    if not fileExists
    then do
        hPutStrLn stderr ("File '" ++ inFile ++ "' does not exist")
        exitWith (ExitFailure 1)
    else do
        fh <- openFile inFile ReadMode
        contents <- readFileContents fh
        hClose fh
        return contents

decideReadInput args
    | length args >= 2 = readInputFile $ args !! 1
    | otherwise = readStdInput

-- String processing functions
letterOccurence :: String -> Char -> Int
letterOccurence str c = length $ filter (==c) str

getFreq :: Int -> Int -> Float
getFreq = (/) `on` fromIntegral

letterCount :: String -> Int
letterCount str = length $ filter isAlpha str

letterFreq :: String -> Char -> (Char, Float)
letterFreq str c = (c, 100 * getFreq (letterOccurence str c) (letterCount str))

countMonoFreqs :: String -> [(Char, Float)]
countMonoFreqs input = sortBy (compare `on` snd) .
                       map (letterFreq input) $ ['a'..'z']

getLetterIdx :: [(Char, Float)] -> Char -> Maybe Int
getLetterIdx lst c = elemIndex c (map fst lst)

getLetter :: (Char, Float) -> Char
getLetter (x, _) = x

-- Map DB letter frequency to input letter frequency
zipFreqs (x1, _) (x2, _) = (x1, x2)
mergeFreqs [] [] = []
mergeFreqs (_:_) [] = []
mergeFreqs [] (_:_) = []
mergeFreqs [x][y] = [zipFreqs x y]
mergeFreqs (x:db)(y:freqs) = zipFreqs x y : mergeFreqs db freqs

sortedMergeFreqs :: [(Char, Float)] -> [(Char, Float)] -> [(Char, Char)]
sortedMergeFreqs db freqs = sortBy (compare `on` fst) (mergeFreqs db freqs)

freqsToKey :: [(Char, Float)] -> [(Char, Float)] -> String
freqsToKey db freqs = map snd (sortedMergeFreqs db freqs)

-- Digram/Trigram operations
toDigrams [] = []
toDigrams [_] = []
toDigrams (x:y:xs) = [x, y] : toDigrams xs

toTrigrams [] = []
toTrigrams [_] = []
toTrigrams [x, y] = []
toTrigrams (x:y:z:xs) = [x, y, z] : toTrigrams xs

splitInput [] = []
splitInput [x] = [[x, ' ', ' ']]
splitInput [x, y] = [[x, y, ' ']]
splitInput (x:y:z:xs) = [x, y, z] : toTrigrams xs

unNgram [] = []
unNgram [x] = x
unNgram (x:xs) = x ++ unNgram xs

ngramOccurence :: [String] -> String -> Int
ngramOccurence diList di = length . mapMaybe (stripPrefix [di]) $ tails diList

ngramFreq :: [String] -> String -> (String, Float)
ngramFreq diList di = (di, 100 * getFreq (ngramOccurence diList di) (length diList))

countNgramFreqs :: [String] -> [(String, Float)]
countNgramFreqs diIn = sortBy (compare `on` snd) .
                    map (ngramFreq diIn) $ nub diIn

lookupLetter :: [(Char, Char)] -> Char -> Char
lookupLetter monoM c
    | isJust cn = fromJust cn
    | otherwise = c
    where cn = lookup c monoM

-- lookupTriInDi :: String -> [(Char, Char)] -> [(String, String)]
--               -> Maybe String
-- lookupTriInDi (x:y:z:xs) monoM diM
--     | isJust first = Just $ fromJust first ++ [lookupLetter monoM z]
--     | isJust second = Just $ [lookupLetter monoM x] ++ fromJust second
--     | otherwise = Nothing
--     where first = lookup [x, y] diM
--           second = lookup [y, z] diM
--
-- mapNgrams :: [(Char, Char)] -> [(String, String)] -> [(String, String)]
--           -> String -> String
-- mapNgrams monoM diM triM nGram
--     | isJust triLookup = fromJust triLookup
--     | isJust diLookup = fromJust diLookup
--     | otherwise = monoLookup
--     where triLookup = lookup nGram triM
--           diLookup = lookupTriInDi nGram monoM diM
--           monoLookup = map (lookupLetter monoM) nGram
--
-- applyNgrams :: [(Char, Char)] -> [(String, String)] -> [(String, String)]
--             -> [String] -> [String]
-- applyNgrams monoM diM triM = map (mapNgrams monoM diM triM)
--

applyNgrams monoM diM triM [] = []
applyNgrams monoM diM triM [w] = applyNgrams monoM diM triM [w,' ',' ',' ']
applyNgrams monoM diM triM [w,x] = applyNgrams monoM diM triM [w,x,' ',' ']
applyNgrams monoM diM triM [w,x,y] = applyNgrams monoM diM triM [w,x,y,' ']
applyNgrams monoM diM triM (w:x:y:z:xs)
    | isJust t1  = fromJust t1 ++ applyNgrams monoM diM triM (z : xs)
    | isJust t2 = c1 : (fromJust t2 ++ applyNgrams monoM diM triM xs)
    | isJust d1 = fromJust d1 ++ applyNgrams monoM diM triM ([y,z] ++ xs)
    | isJust d2 = c1 : (fromJust d2 ++ applyNgrams monoM diM triM (z : xs))
    | isJust d3 = [c1, c2] ++ fromJust d3 ++ applyNgrams monoM diM triM xs
    | otherwise = [c1, c2, c3, c4] ++ applyNgrams monoM diM triM xs
    where t1 = lookup [w,x,y] triM
          t2 = lookup [x,y,z] triM
          d1 = lookup [w,x] diM
          d2 = lookup [x,y] diM
          d3 = lookup [y,z] diM
          c1 = lookupLetter monoM w
          c2 = lookupLetter monoM x
          c3 = lookupLetter monoM y
          c4 = lookupLetter monoM z

swapFreqs [] = []
swapFreqs [x] = [x]
swapFreqs (x:y:xs)
    | diff <= 0.5 = (fst x, snd y) : (fst y, snd x) : swapFreqs xs
    | otherwise = x : y : swapFreqs xs
    where diff = abs $ snd x - snd y

main :: IO ()
main = do
    -- Parse options
    args <- getArgs
    let(actions, nonOptions, errors) = getOpt RequireOrder options args
    opts <- foldl (>>=) (return startOptions) actions

    let Options {
        optPlaintext = printPlaintext,
        optPrintKey  = printKey
    } = opts

    when (null nonOptions) $
        die "Missing arguments (use -h to show help)"

    -- Load DB with letter/digram/trigram frequencies
    let dbName = nonOptions !! 0
    (letterFreqs, digramFreqs, trigramFreqs) <- readDB dbName

    -- Load input file/stdin
    input <- decideReadInput nonOptions
    let inputTrigrams = splitInput input

    -- Calculate trigram mappings
    let triInput = toTrigrams . filter isAlpha $ input
    let inTriFreqs = reverse (countNgramFreqs triInput)
    let trigramMap = mergeFreqs inTriFreqs trigramFreqs

    -- Calculate digram mappings
    let diInput = toDigrams . filter isAlpha $ input
    let inDiFreqs = reverse (countNgramFreqs diInput)
    let digramMap = mergeFreqs inDiFreqs digramFreqs

    -- Calculate monogram mappings
    let inMonoFreqs = reverse (countMonoFreqs input)
    let letterMap = mergeFreqs inMonoFreqs letterFreqs

    --- Apply mappings
    let output = applyNgrams letterMap digramMap trigramMap input

    when printPlaintext $
        putStr output

    when printKey $
        putStrLn $ freqsToKey letterFreqs inMonoFreqs

    exitSuccess
