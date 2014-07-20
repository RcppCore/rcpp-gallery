/**
 * @title Summing a Vector in Parallel with RcppParallel
 * @author JJ Allaire
 * @license GPL (>= 2)
 * @tags parallel featured
 * @summary Demonstrates computing the sum of a vector in parallel using 
 *   the RcppParallel package. 
 *
 * The [RcppParallel](https://github.com/RcppCore/RcppParallel) package includes
 * high level functions for doing parallel programming with Rcpp. For example,
 * the `parallelReduce` function can be used aggreggate values from a set of
 * inputs in parallel. This article describes using RcppParallel to sum an R
 * vector.
 */

/**
 * ### Serial Version
 * 
 * First a serial version of computing the sum of a vector. For this we use
 * a simple call to the STL `std::accumulate` function:
 */

#include <Rcpp.h>
using namespace Rcpp;

#include <algorithm>

// [[Rcpp::export]]
double vectorSum(NumericVector x) {
   return std::accumulate(x.begin(), x.end(), 0.0);
}

/**
 * ### Parallel Version
 * 
 * Now we adapt our code to run in parallel. We'll use the `parallelReduce` 
 * function to do this. As with the [previous 
 * article](2014-06-29-parallel-matrix-transform.cpp) describing `parallelFor`, 
 * we implement a "Worker" function object with our logic and RcppParallel takes
 * care of scheduling work on threads and calling our function when required. For
 * parallelReduce the function object has three jobs:
 * 
 * 1. Implement a standard and "splitting" constructor. The standard constructor
 * takes a pointer to the array that will be traversed and sets it's sum 
 * variable to 0. The splitting constructor is called when work needs to be 
 * split onto other threads---it takes a reference to the instance it is being 
 * split from and simply copies the pointer to the input array and sets it's 
 * internal sum to 0.
 * 
 * 2. Implement `operator()` to perform the summing. Here we just call 
 * `std::accumulate` as we did in the serial version, but limit the accumulation
 * to the items specified by the `begin` and `end` arguments (note that other 
 * threads will have been given the task of processing other items in the input 
 * array). We save the accumulated value in our `value` member variable.
 * 
 * 3. Finally, we implement a `join` method which composes the operations of two
 * Sum instances that were previously split. Here we simply add the accumulated 
 * sum of the instance we are being joined with to our own.
 *
 * Here's the definition of the `Sum` function object:
 * 
 */

// [[Rcpp::depends(RcppParallel)]]
#include <RcppParallel.h>
using namespace RcppParallel;

struct Sum : public Worker
{   
   // source vector
   const RVector<double> input;
   
   // accumulated value
   double value;
   
   // constructors
   Sum(const NumericVector input) : input(input), value(0) {}
   Sum(const Sum& sum, Split) : input(sum.input), value(0) {}
   
   // accumulate just the element of the range I've been asked to
   void operator()(std::size_t begin, std::size_t end) {
      value += std::accumulate(input.begin() + begin, input.begin() + end, 0.0);
   }
     
   // join my value with that of another Sum
   void join(const Sum& rhs) { 
      value += rhs.value; 
   }
};

/**
 * Note that `Sum` derives from the `RcppParallel::Worker` class. This is
 * required for function objects passed to `parallelReduce`.
 * 
 * Note also that we use the `RVector<double>` type for accessing the vector. 
 * This is because this code will execute on a background thread where it's not 
 * safe to call R or Rcpp APIs. The `RVector` class is included in the 
 * RcppParallel package and provides a lightweight, thread-safe wrapper around R
 * vectors.
 */

/**
 * Now that we've defined the functor, implementing the parallel sum 
 * function is straightforward. Just initialize an instance of `Sum`
 * with the input vector and call `parallelReduce`:
 */

// [[Rcpp::export]]
double parallelVectorSum(NumericVector x) {
   
   // declare the SumBody instance 
   Sum sum(x);
   
   // call parallel_reduce to start the work
   parallelReduce(0, x.length(), sum);
   
   // return the computed sum
   return sum.value;
}

/**
 * ### Benchmarks
 * 
 * A comparison of the performance of the two functions shows the parallel
 * version performing about 4 times as fast on a machine with 4 cores:
 */

/*** R
# allocate a vector
v <- as.numeric(c(1:10000000))

# ensure that serial and parallel versions give the same result
stopifnot(identical(vectorSum(v), parallelVectorSum(v)))

# compare performance of serial and parallel
library(rbenchmark)
res <- benchmark(vectorSum(v),
                 parallelVectorSum(v),
                 order="relative")
res[,1:4]
*/

/**
 * Note that performance gains will typically be 30-50% less on Windows systems 
 * as a result of less sophisticated thread scheduling (RcppParallel does not 
 * currently use [TBB](https://www.threadingbuildingblocks.org/) on Windows 
 * whereas it does on the Mac and Linux).
 * 
 * You can learn more about using RcppParallel at 
 * [https://github.com/RcppCore/RcppParallel](https://github.com/RcppCore/RcppParallel).
 */ 
 