/**
 * @title Transforming a Matrix in Parallel using RcppParallel
 * @author JJ Allaire
 * @license GPL (>= 2)
 * @tags matrix parallel featured
 * @summary Demonstrates transforming a matrix in parallel using 
 *   the RcppParallel package.
 *
 * The [RcppParallel](https://rcppcore.github.com/RcppParallel) package includes
 * high level functions for doing parallel programming with Rcpp. For example,
 * the `parallelFor` function can be used to convert the work of a standard
 * serial "for" loop into a parallel one. This article describes using
 * RcppParallel to transform an R matrix in parallel.
 */

/**
 * ### Serial Version
 * 
 * First a serial version of the matrix transformation. We take the square root 
 * of each item of a matrix and return a new matrix with the tranformed values. 
 * We do this by using `std::transform` to call the `sqrt` function on each
 * element of the matrix:
 */

#include <Rcpp.h>
using namespace Rcpp;

#include <cmath>
#include <algorithm>

// [[Rcpp::export]]
NumericMatrix matrixSqrt(NumericMatrix orig) {

  // allocate the matrix we will return
  NumericMatrix mat(orig.nrow(), orig.ncol());

  // transform it
  std::transform(orig.begin(), orig.end(), mat.begin(), ::sqrt);

  // return the new matrix
  return mat;
}

/**
 * ### Parallel Version
 * 
 * Now we'll adapt our code to run in parallel using the `parallelFor` function.
 * RcppParallel takes care of dividing up work between threads, our job is to 
 * implement a "Worker" function object that is called by the RcppParallel 
 * scheduler.
 * 
 * The `SquareRoot` function object below includes pointers to the input matrix
 * as well as the output matrix. Within it's `operator()` method it performs a 
 * `std::transform` with the `sqrt` function on the array elements specified by 
 * the `begin` and `end` arguments:
 */

// [[Rcpp::depends(RcppParallel)]]
#include <RcppParallel.h>
using namespace RcppParallel;

struct SquareRoot : public Worker
{
   // source matrix
   const RMatrix<double> input;
   
   // destination matrix
   RMatrix<double> output;
   
   // initialize with source and destination
   SquareRoot(const NumericMatrix input, NumericMatrix output) 
      : input(input), output(output) {}
   
   // take the square root of the range of elements requested
   void operator()(std::size_t begin, std::size_t end) {
      std::transform(input.begin() + begin, 
                     input.begin() + end, 
                     output.begin() + begin, 
                     ::sqrt);
   }
};

/**
 * Note that `SquareRoot` derives from `RcppParallel::Worker`. This is required
 * for function objects passed to `parallelFor`.
 * 
 * Note also that we use the `RMatrix<double>` type for accessing the matrix. 
 * This is because this code will execute on a background thread where it's not 
 * safe to call R or Rcpp APIs. The `RMatrix` class is included in the 
 * RcppParallel package and provides a lightweight, thread-safe wrapper around R
 * matrixes.
 */

/**
 * Here's the parallel version of our matrix transformation function that makes 
 * uses of the `SquareRoot` function object. The main difference is that rather 
 * than calling `std::transform` directly, the `parallelFor` function is called 
 * with the range to operate on (in this case based on the length of the input
 * matrix) and an instance of `SquareRoot`:
 */

// [[Rcpp::export]]
NumericMatrix parallelMatrixSqrt(NumericMatrix x) {
  
  // allocate the output matrix
  NumericMatrix output(x.nrow(), x.ncol());
  
  // SquareRoot functor (pass input and output matrixes)
  SquareRoot squareRoot(x, output);
  
  // call parallelFor to do the work
  parallelFor(0, x.length(), squareRoot);
  
  // return the output matrix
  return output;
}

/**
 * ### Benchmarks
 * 
 * A comparison of the performance of the two functions shows the parallel
 * version performing about 2.5 times as fast on a machine with 4 cores:
 */

/*** R

# allocate a matrix
m <- matrix(as.numeric(c(1:1000000)), nrow = 1000, ncol = 1000)

# ensure that serial and parallel versions give the same result
stopifnot(identical(matrixSqrt(m), parallelMatrixSqrt(m)))

# compare performance of serial and parallel
library(rbenchmark)
res <- benchmark(matrixSqrt(m),
                 parallelMatrixSqrt(m),
                 order="relative")
res[,1:4]
*/

/**
 * You can learn more about using RcppParallel at 
 * [https://rcppcore.github.com/RcppParallel](https://rcppcore.github.com/RcppParallel).
 */ 
 