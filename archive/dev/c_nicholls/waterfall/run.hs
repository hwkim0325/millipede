{-# OPTIONS_GHC -Wall#-}
import Process(bounds,output,getAdjacencyList,arrayToNode,save,Voxel(Voxel) )

import Waterfall(waterfalls,Edge(Edge))
import Data.Time.Clock(getCurrentTime,diffUTCTime,NominalDiffTime,UTCTime)
import PGM( pgmsFromFile)
--import TreeScorer(kidneyScorer)
import System.Environment(getArgs)

main :: IO()
main = do
  t0 <- getCurrentTime
  putStrLn "Reading file"
  args <- getArgs
  Right (ar:_) <-   pgmsFromFile (head args)

  t1 <- timeSince t0
  putStrLn "Generating MST"
  es <- getAdjacencyList  ar

  t2 <- timeSince t1
  putStrLn "Making Tree"
  (Edge 1000000000 t) <- arrayToNode es (-1,-1) (1000000000,(Voxel 0 (0,0)))

  t3 <- timeSince t2
  putStrLn "Doing waterfall, converting output"
  let bs = bounds ar
  let trees = take 5 (waterfalls t)
  --let kidneyScores = map kidneyScorer trees

  ars <- mapM (output bs) ( map (Edge 1000000000) trees)

  t5 <- timeSince t3
  save path 1 (ars)
  _ <- timeSince t5
  putStrLn "Done"
  _ <- timeSince t0
  return ()

path :: String
path="./output/output"




timeSince ::UTCTime -> IO UTCTime
timeSince t = do
  {t' <- getCurrentTime
  ;(putStrLn.show. (\x ->  (fromInteger (round (x*10000)) :: NominalDiffTime)/10000)) (diffUTCTime t' t)
  ;return t'
  }
