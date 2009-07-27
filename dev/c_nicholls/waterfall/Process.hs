module Process(
  size,
  bounds,
  output,
  getAdjacencyList,
  arrayToTree,freeze'',
  Adjacency,Point
  	) where

import SkewHeap(Heap,insert,mkHeap,merge,deleteMin,heapToList)
import Data.Array.IO
import Data.Array.Unboxed       (IArray, Ix, UArray, amap, bounds, elems, listArray, (!)	)
import Data.Array.IO
import List(sort,sortBy)
import Data.Word                (Word8, Word16)
import Waterfall(waterfall,Tree(Node),Edge,Mergable(union),ListMergable)
import PGM
  ( pgmToArray,
    pgmsToArrays,
    pgmToArrayWithComments, pgmsToArraysWithComments,
    arrayToPgmWithComment,
    pgmsFromFile, pgmsFromHandle,
    arrayToPgm, arrayToFile, arrayToHandle, arraysToHandle,
    arraysToFile
  )

type Adjacency  = (Int,Voxel,Voxel)
type Region = [Point]
type Point = (Int,Int)
type Voxel = (Int,Point)


neighbours :: UArray Point Int -> Point -> Point -> [Adjacency]
neighbours arr  (a,b) (x,y) = 
  [(abs (arr!(a,b)-arr!(c,d)),
   (arr!(a,b),(a,b)),
   (arr!(c,d),(c,d)))| (c,d) <-  [
      (a,b+1),(a+1,b),(a,b-1),(a-1,b),
      (a+1,b+1),(a-1,b-1),(a+1,b-1),(a-1,b+1)], 0<=c,0<=d,c<x,d<y]	

               
---- IO Array Functions ----            
fillIn :: (MArray a Int IO) => Tree (Heap Voxel) -> a Point Int -> IO (a Point Int)
fillIn (Node rs es) ar = do
  {let rs' = heapToList rs
  ;let x = avg rs'
  ;mapM (\v -> writeArray ar (snd v) x) rs'
  ;ar' <- foldR (fillIn.fst) ar es
  ;return $!  ar
  }

arrayToTree :: (MArray a [(Int, Voxel)] IO ) =>
               a Point [(Int, Voxel)] -> Point -> (Int, Voxel) -> IO (Edge (Heap Voxel))
arrayToTree arr miss (n,(v,p))  = do
  { --print p
  ;ls <-readArray arr p
  ;let ls' = remove miss ls
  ;let ls'' = remove p ls'
  ;es <- mapM  (arrayToTree arr p) ls''
  ;return $!  ((Node (mkHeap [(v,p)]) es),n)
  }

--remove :: Point -> [(Point,Int)] -> [(Point,Int)]
remove p [] = []
remove p ((w,(v,a)):ps) 
  | a ==p = remove p ps
  | otherwise = ((w,(v,a)):remove p ps)

getAdjacencyList :: (IArray UArray Int) =>  UArray Point Int -> IO (IOArray Point [(Int,Voxel)])
getAdjacencyList arr = do 
  {let ((a,b),(c,d)) = bounds arr
  ;brr <- newArray ((a,b),(c,d)) [] :: IO (IOArray Point [(Int,Voxel)]	)
  ;writeArray brr (0,0) [(0,(0,(0,0)))]	
  ;let e = neighbours arr (0,0) (c,d)
  ;print (c*d-1)
  ;pickNAdjacencys (c*d-1) (c,d) (mkHeap e) arr brr []
  }

pickNAdjacencys :: ( MArray IOArray [(Int,Voxel)] IO	) =>
              Int-> Point ->Heap Adjacency-> UArray Point Int -> IOArray Point [(Int,Voxel)] -> [Adjacency] -> IO (IOArray Point [(Int,Voxel)])
pickNAdjacencys 0 is es ar ls end    = do {return $!  ls}
pickNAdjacencys n is h ar ls end = do
  {
  ;let ((w,a,b),h') = deleteMin h
  ;l1 <- readArray ls  (snd a)
  ;l2 <- readArray ls  (snd b)
  ;if((l1==[]||l2==[]))
     then do
       {writeArray ls (snd a) ((w,b):l1)
       ;writeArray ls (snd b) ((w,a):l2)
       ;if l1 ==[]
         then do 
           {l <- filter' ls []$ neighbours ar (snd a) is
           ;pickNAdjacencys (n-1) is (merge h' (mkHeap$ l)) ar ls ((w,a,b):end) }
         else do 
           {l <- filter' ls []$	 neighbours ar (snd b) is
           ;pickNAdjacencys (n-1) is (merge h' (mkHeap$ l)) ar ls ((w,a,b):end) }
       }
     else do {	pickNAdjacencys n is h' ar ls end}
  }

--filter' :: (MArray IOArray [(Int, Voxel)] IO) => IOArray Point [(Int,Voxel)]-> [Adjacency]-> [Adjacency]-> IO ([Adjacency])
filter' br [] ls = do{ return $!  ls}
filter' br ls ((a,b,(c,d)):xs) = do
  {l <- readArray br d
  ;if l == [] 
    then filter' br ((a,b,(c,d)):ls) xs
    else filter' br ls xs
  }

---  Main  -------------  
output :: (Point, Point) -> Tree (Heap Voxel) -> IO (UArray Point Word16)
output bounds tree = do
  {arr <- newArray bounds 0 :: IO (IOUArray Point Int)
  ;arrr <- fillIn tree arr
  ;arrrr <- freeze' arrr
  ;let arrrrr = amap (fromIntegral :: Int -> Word16) arrrr
  ;return $!  arrrrr
  }


--- Other Functions ---------
instance Ord a => Mergable (Heap a) where
  union = SkewHeap.merge

freeze' :: ( MArray a Int IO, IArray UArray Int) => a Point Int -> IO (UArray Point Int)
freeze' = freeze

freeze'' :: ( MArray a b IO, IArray UArray b) => a Point b -> IO (UArray Point b)
freeze'' = freeze

size :: Tree a-> Int
size (Node rs es) = 1 + sum(map (size.fst) es)
    
foldR            :: (Monad m) => (a -> b -> m b) -> b -> [a] -> m b
foldR f a []     = return $!  a
foldR f a (x:xs) = foldR f a xs >>= \y -> f x y

avg [] = 0
avg xs = sum(map fst xs) `div` length xs
