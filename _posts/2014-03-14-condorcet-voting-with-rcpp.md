---
title: Condorcet Voting with Rcpp
author: Avraham Adler
license: GPL (>= 2)
tags: basics modeling benchmark
summary: This post demonstrates the speed increase in loop-heavy Condorcet voting calculations by using Rcpp together with R
layout: post
src: 2014-03-14-CondorcetVotingWithRcpp.Rmd
---

There is a lot of literature and debate on how to rank candidates under
[preferential voting](https://en.wikipedia.org/wiki/Ranked_voting_systems)
systems. Two of the methods used to determine winners are those based on some
form of [Borda count](https://en.wikipedia.org/wiki/Borda_count) and those
based on some form of [Condorcet
method](https://en.wikipedia.org/wiki/Condorcet_winner). Many students of
politics and voting systems prefer Condorcet methods to Borda ones for its
stronger theoretical qualities. However, while Borda counts, especially in
their most recognizable form—average rank, is easy to calculate, Condorecet
winners are more difficult, as it requires pairwise comparisons between all
candidates. Moreover, the possibility of having a [Condorcet
cycle](https://en.wikipedia.org/wiki/Condorcet_method#Circular_ambiguities)
grows as the number of candidates grows. 

One of the more common methods of solving a Condorcet paradox is the [Schulze
method](https://en.wikipedia.org/wiki/Schulze_method). This not only has some
strong theoretical qualities, but it also has a relatively [simple
implementation](https://en.wikipedia.org/wiki/Schulze_method#Implementation). However,
it is slow. Pairwise ranking, in its straightforward form, is of O(n²) and
the Schulze method is O(n³) where n is the number of candidates.

In this post, Rcpp is used to significantly speed up the vote ranking process. 



Here is a sample ballot with eight voters ranking five candiates:

<pre class="output">
  Candidates Vote A Vote B Vote C Vote D Vote E Vote F Vote G Vote H
1     Albert      1      2      1      3      2      1      3      4
2      Bruce      2      4      5      4      5      4      1      2
3    Charles      3      1      3      1      1      2      4      5
4      David      4      5      2      5      4      5      2      1
5     Edward      5      3      4      2      3      3      5      3
</pre>


Here is some simple code to calculate the average rank of the candidates:

{% highlight r %}
AvgRank <- function(BallotMatrix){
    Ballots <- as.matrix(BallotMatrix[, -1], mode = "numeric")
    Num_Candidates <- dim(Ballots)[1]
    Names <- BallotMatrix[, 1]
    Ballots[is.na(Ballots)] <- Num_Candidates + 1 #Treat blanks as one worse than min
    MeanRanks <- rowMeans(Ballots)
    Rankings <- data.frame(Names, MeanRanks)
    Rankings <- Rankings[order(rank(Rankings[, 2], ties.method = "random")), ] #Ties handled through random draw
    Rankings <- data.frame(Rankings, seq_along(Rankings[, 1]))
    names(Rankings) <- c("Names", "Average Rank", "Position")
    return(Rankings)  
}
{% endhighlight %}

The above ballot would result in the following Borda-based ranking:

<pre class="output">
    Names Average Rank Position
1  Albert        2.125        1
3 Charles        2.500        2
2   Bruce        3.375        3
4   David        3.500        4
5  Edward        3.500        5
</pre>


Here is some simplified code to calculate Condorcet and Schulze winners. The ballots have been created to ensure that there is always a unique Schulze winner. In reality, there often is not, and some further form of tiebreaking routine will be necessary:

{% highlight r %}
#This function extracts the matrix of votes from the ballot
VoteExtract <- function(BallotMatrix){
    Votes <- as.matrix(BallotMatrix[, -1], mode = "numeric")
    Num_Candidates <- dim(Votes)[1]
    Votes[is.na(Votes)] <- Num_Candidates + 1 #Treat blanks as one worse than min
    return(Votes)
}

#This function performs the pairwise comparison between candidates and results in a square matrix representing the number of wins the candidate in row i has beaten the candidate in column j.
PairCount <- function(Votes) {
    Num_Candidates <- dim(Votes)[1]
    Pairwise <- matrix(nrow = Num_Candidates, ncol = Num_Candidates)
    for (CurCand in 1:Num_Candidates) {
        CandRank <- as.vector(as.matrix(Votes[CurCand, ]))
        Pref_Cur_Cand <- t(Votes) - CandRank
        for (Pairs in 1:Num_Candidates) {
            Pairwise[CurCand, Pairs] <- sum(Pref_Cur_Cand[, Pairs] > 0)
        }
    }
    return(Pairwise)
}

#This function calculates the beatpaths and members of the Schwarz set. A unique member is the Schulze Condorcet winner.
Schulze <- function(PairsMatrix){
    size <- dim(PairsMatrix)[1]
    p <- matrix(nrow = size, ncol = size)
    for (i in 1:size) {
        for (j in 1:size){
            if (i != j) {
                if (PairsMatrix[i, j] > PairsMatrix[j, i]) {
                    p[i, j] <- PairsMatrix[i, j]
                } else {
                    p[i, j] <- 0
                }
            }
        }
    }
    for (i in 1:size) {
        for (j in 1:size) {
            if (i != j) {
                for (k in 1:size) {
                    if (i != k && j != k) {
                        p[j, k] <- max(p[j, k], min(p[j, i], p[i, k]))
                    }
                }
            }
        }
    }
    diag(p) <- 0
    return(p)
}

#This function performs the ranking, starting with the full ballot, finding a pure Condorcet or Schulze winner, removing him or her from the ballot, and repeating the process until all candidates are ranked.
CondorcetRank <- function(BallotMatrix)  {
    Num_Candidates <- dim(BallotMatrix)[1]
    Rankings <- matrix(nrow = Num_Candidates, ncol = 3)
    CurrentBallot <- BallotMatrix
    CurrentRank <- 1
    while (CurrentRank <= Num_Candidates) {
        CurrentNames <- as.vector(CurrentBallot[, 1])
        CurrentSize <- length(CurrentNames)
        CurrentVotes <- VoteExtract(CurrentBallot)
        Pairwise <- matrix(nrow = CurrentSize, ncol = CurrentSize)
        Pairwise <- PairCount(CurrentVotes)
        Winner <- vector(length = CurrentSize)
    
        # Check for Condorcet Winner    
    
        for (i in 1:CurrentSize) {
            Winner[i] <- sum(Pairwise[i, ] > Pairwise[, i]) == (CurrentSize - 1)
        }
        if (sum(Winner == TRUE) == 1) { #Condorcet Winner Exists
            CurrentWinner <- which(Winner == TRUE)
            Rankings[CurrentRank, ] <- c(CurrentNames[CurrentWinner], CurrentRank, "Condorcet")
        } else {
      
            # Condorcet Winner does not exist, calculate Schulze beatpaths
      
            Pairwise <- Schulze(Pairwise)
            for (i in 1:CurrentSize) {
                 Winner[i] <- sum(Pairwise[i, ] > Pairwise[, i]) == (CurrentSize - 1)
            }
            if (sum(Winner == TRUE) == 1) { #Schwartz set has unique member
                CurrentWinner <- which(Winner == TRUE) 
                Rankings[CurrentRank, ] <- c(CurrentNames[CurrentWinner], CurrentRank, "Schulze")
            }
        }
        CurrentBallot <- CurrentBallot[-CurrentWinner, ]
        CurrentRank = CurrentRank + 1
    }
    Rankings <- data.frame(Rankings)
    names(Rankings) <- c("Name", "Rank", "Method")
    return(Rankings)
}
{% endhighlight %}


Using the sample ballot, the pairwise matrix is:

<pre class="output">
     [,1] [,2] [,3] [,4] [,5]
[1,]    0    6    5    6    6
[2,]    2    0    3    5    3
[3,]    3    5    0    5    7
[4,]    2    3    3    0    4
[5,]    2    5    1    4    0
</pre>

the beatpath matrix for the top ranked candidate (using all ballots) is:

<pre class="output">
     [,1] [,2] [,3] [,4] [,5]
[1,]    0    6    5    6    6
[2,]    0    0    0    5    0
[3,]    0    5    0    5    7
[4,]    0    0    0    0    0
[5,]    0    5    0    5    0
</pre>

and the full Condorcet ranking is:

<pre class="output">
     Name Rank    Method
1  Albert    1 Condorcet
2 Charles    2 Condorcet
3  Edward    3   Schulze
4   Bruce    4 Condorcet
5   David    5 Condorcet
</pre>


When profiling this code using the actual ballot of 30+ people with multiple Condorcet paradoxes, 81% of the time was spent in the Schulze algorithm, another 12% was spent in the PairCount algorithm, and the remaining 7% was spent on everything else (the actual ranking had multiple steps to handle cases when there was no Schulze winner). To speed up the procedure, I ported the Schulze and PairCount functions to C++:

{% highlight cpp %}
#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
IntegerMatrix PairCount_C(IntegerMatrix Votes) {
    int Num_Candidates = Votes.nrow();
    int Num_Ballots = Votes.ncol();
    IntegerMatrix Pairwise(Num_Candidates, Num_Candidates);
    for (int CurCand = 0; CurCand < Num_Candidates; CurCand++) {
        IntegerVector CandRank = Votes(CurCand, _);
        IntegerMatrix Pref_Cur_Cand(Num_Candidates, Num_Ballots);
        for (int i = 0; i < Num_Candidates; i++) {
            for (int j = 0; j < Num_Ballots; j++) {
                Pref_Cur_Cand(i, j) = Votes(i, j) - CandRank(j);
            }
        }
        for (int i = 0; i < Num_Candidates; i++) {
            int G0 = 0;
            for (int j = 0; j < Num_Ballots; j++) {
                if (Pref_Cur_Cand(i, j) > 0) G0 += 1;
            }
        Pairwise(CurCand, i) = G0;
        }
    }
    return(Pairwise);
}

// [[Rcpp::export]]
IntegerMatrix Schulze_C(IntegerMatrix Pairs) {
    int nrow = Pairs.nrow();
    IntegerMatrix Schulze(nrow, nrow);
    for (int i = 0; i < nrow; i++) {
        for (int j = 0; j < nrow; j++) {
            if (i != j) {
                if (Pairs(i, j) > Pairs(j, i)) {
                    Schulze(i, j) = Pairs(i, j);
                } else {
                    Schulze(i, j) = 0;
                }
            }
        }
    }
    for (int i = 0; i < nrow; i++) {
        for (int j = 0; j < nrow; j++) {
            if (i != j) {
                for (int k = 0; k < nrow; k++) {
                    if ((i != k) && (j != k)) {
                        Schulze(j, k) = (std::max)(Schulze(j, k), (std::min)(Schulze(j, i), Schulze(i, k)));
                    }
                }
            } else {
                if ((i = j)) {
                    Schulze(i, j) = 0;
                }
            }
        }
    }
    return(Schulze);
}
{% endhighlight %}

It is also interesting to compare these results with those obtained from byte-compiling the functions:

{% highlight r %}
library(compiler)
PairCount_cmp <- cmpfun(PairCount)
PairCount_cmp3 <- cmpfun(PairCount, options=list(optimize = 3))
Schulze_cmp <- cmpfun(Schulze)
Schulze_cmp3 <- cmpfun(Schulze, options=list(optimize = 3))
{% endhighlight %}

First, we need to check that the functions return the same values:

{% highlight r %}
all.equal(PairCount(VoteExtract(Ballot)), 
          PairCount_cmp(VoteExtract(Ballot)),
          PairCount_cmp3(VoteExtract(Ballot)), 
          PairCount_C(VoteExtract(Ballot))) 
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>



{% highlight r %}
all.equal(Schulze(PairCount(VoteExtract(Ballot))),
          Schulze_cmp(PairCount(VoteExtract(Ballot))),
          Schulze_cmp3(PairCount(VoteExtract(Ballot))),
          Schulze_C(PairCount_C(VoteExtract(Ballot)))) 
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>

Now let's compare the speed:

{% highlight r %}
library(microbenchmark)
V <- VoteExtract(Ballot)
P <- PairCount(V)
microbenchmark(PairCount(V), PairCount_cmp(V), PairCount_cmp3(V), PairCount_C(V), Schulze(P), Schulze_cmp(P), Schulze_cmp3(P), Schulze_C(P), times = 100L)
{% endhighlight %}



<pre class="output">
Unit: microseconds
              expr     min      lq  median      uq     max neval
      PairCount(V) 375.617 384.634 389.130 400.919 1280.33   100
  PairCount_cmp(V) 255.012 263.320 267.079 272.044  361.06   100
 PairCount_cmp3(V) 247.473 254.373 259.001 265.401 1120.02   100
    PairCount_C(V)   7.334  11.793  12.558  13.214   32.37   100
        Schulze(P) 786.678 805.497 812.786 842.798 1677.24   100
    Schulze_cmp(P) 230.732 237.673 241.639 251.459 1062.25   100
   Schulze_cmp3(P) 195.612 200.953 205.156 214.832 1037.79   100
      Schulze_C(P)   3.496   5.712   7.485   8.086   14.26   100
</pre>

While byte-compiling the PairCount function gives an impressive speedup of
ariund 40%, porting it to C++ makes it over 25 times faster (an over 2400%
speedup). Results with the Schulze algorithm is even more
striking. Byte-compilation gives an increase in speed of between 3 to 3.5
times without any change to the R code, but porting it to C++ is **around 100
times** as fast (and was over 120 times as fast on a different machine)!
Moreover, the PairCount algorithm reads more logically in C++, as the way R
handles vectors and matrices, when subtracting the current rank, the
resulting matrix ended up transposed with the candidates across the columns. 

So with easy-to-read code that results in speed gains of multiple orders of
magnitude, what's not to like?! 
