// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Using the Rcpp Timer
 * @author Dirk Eddelbuettel
 * @updated Dec 30, 2016
 * @license GPL (>= 2)
 * @tags benchmark rng featured
 * @summary This post shows how to use the Timer class in Rcpp
 *
 *
 * Sine the 0.10.2 release, Rcpp contains an internal class `Timer`
 * which can be used for fine-grained benchmarking.  Romain motivated
 * `Timer` in a 
 * [post to the mailing list](http://article.gmane.org/gmane.comp.lang.r.rcpp/4525) 
 * where `Timer` is used to measure the different components of the costs of
 * random number generation.
 * 
 * A slightly modified version of that example follows below.
 */


#include <Rcpp.h>
#include <Rcpp/Benchmark/Timer.h>

using namespace Rcpp;

// [[Rcpp::export]]
NumericVector useTimer() {
    int n = 1000000;

    // start the timer
    Timer timer;
    timer.step("start");        // record the starting point

    for (int i=0; i<n; i++) {
        GetRNGstate();
        PutRNGstate();
    }
    timer.step("get/put");      // record the first step

    for (int i=0; i<n; i++) {
        GetRNGstate();
        rnorm(10, 0.0, 1.0);
        PutRNGstate();
    }
    timer.step("g/p+rnorm()");  // record the second step

    for (int i=0; i<n; i++) {
        // empty loop
    }
    timer.step("empty loop");   // record the final step

    NumericVector res(timer);   // 
    for (int i=0; i<res.size(); i++) {
        res[i] = res[i] / n;
    }
    return res;
}

/**
 * We get the following result, each expressing the cost per iteration in nanoseconds, 
 * both cumulative (default) and incrementally (by taking differences).
 */

/*** R
res <- useTimer()
res          # normal results: cumulative
diff(res)    # simple difference
*/

/**
 * The interesting revelation is that *repeatedly* calling
 * `GetRNGstate()` and `PutRNGstate()` can amount to about 60% of the
 * cost of RNG draws.  Luckily, we usually only have to call these
 * helper functions once per subroutine called from R (rather than
 * repeatedly as shown here) so this is not really a permanent cost to
 * bear when running simulations with R. 
 *
 * It also show the usefulness of a fine-grained timer at the code level.
 */
