/**
 * @title Computing an Inner Product with RcppParallel
 * @author JJ Allaire
 * @license GPL (>= 2)
 * @tags parallel
 * @summary Demonstrates computing the inner product of two vectors in
 *   parallel using the RcppParallel package.
 *
 * The [RcppParallel](https://rcppcore.github.com/RcppParallel) package includes
 * high level functions for doing parallel programming with Rcpp. For example,
 * the `parallelReduce` function can be used aggreggate values from a set of
 * inputs in parallel. This article describes using RcppParallel to parallelize
 * the [inner-product](https://gallery.rcpp.org/articles/stl-inner-product/)
 * example previously posted to the Rcpp Gallery.
 */

/**
 * ### Serial Version
 *
 * First the serial version of computing the inner product. For this we use
 * a simple call to the STL `std::inner_product` function:
 */

#include <Rcpp.h>
using namespace Rcpp;

#include <algorithm>

// [[Rcpp::export]]
double innerProduct(NumericVector x, NumericVector y) {
   return std::inner_product(x.begin(), x.end(), y.begin(), 0.0);
}

/**
 * ### Parallel Version
 *
 * Now we adapt our code to run in parallel. We'll use the `parallelReduce`
 * function to do this. This function requires a "worker" function object
 * (defined below as `InnerProduct`). For details on worker objects see the
 * [parallel-vector-sum](https://gallery.rcpp.org/articles/parallel-vector-sum/)
 * article on the Rcpp Gallery.
 */

// [[Rcpp::depends(RcppParallel)]]
#include <RcppParallel.h>
using namespace RcppParallel;

struct InnerProduct : public Worker
{
   // source vectors
   const RVector<double> x;
   const RVector<double> y;

   // product that I have accumulated
   double product;

   // constructors
   InnerProduct(const NumericVector x, const NumericVector y)
      : x(x), y(y), product(0) {}
   InnerProduct(const InnerProduct& innerProduct, Split)
      : x(innerProduct.x), y(innerProduct.y), product(0) {}

   // process just the elements of the range I've been asked to
   void operator()(std::size_t begin, std::size_t end) {
      product += std::inner_product(x.begin() + begin,
                                    x.begin() + end,
                                    y.begin() + begin,
                                    0.0);
   }

   // join my value with that of another InnerProduct
   void join(const InnerProduct& rhs) {
     product += rhs.product;
   }
};

/**
 * Note that `InnerProduct` derives from the `RcppParallel::Worker` class. This
 * is required for function objects passed to `parallelReduce`.
 *
 * Note also that we use the `RVector<double>` type for accessing the vector.
 * This is because this code will execute on a background thread where it's not
 * safe to call R or Rcpp APIs. The `RVector` class is included in the
 * RcppParallel package and provides a lightweight, thread-safe wrapper around R
 * vectors.
 */

/**
 * Now that we've defined the function object, implementing the parallel inner
 * product function is straightforward. Just initialize an instance of
 * `InnerProduct` with the input vectors and call `parallelReduce`:
 */

// [[Rcpp::export]]
double parallelInnerProduct(NumericVector x, NumericVector y) {

   // declare the InnerProduct instance that takes a pointer to the vector data
   InnerProduct innerProduct(x, y);

   // call paralleReduce to start the work
   parallelReduce(0, x.length(), innerProduct);

   // return the computed product
   return innerProduct.product;
}

/**
 * ### Benchmarks
 *
 * A comparison of the performance of the two functions shows the parallel
 * version performing about 2.5 times as fast on a machine with 4 cores:
 */

/*** R
x <- runif(1000000)
y <- runif(1000000)

library(rbenchmark)
res <- benchmark(sum(x*y),
                 innerProduct(x, y),
                 parallelInnerProduct(x, y),
                 order="relative")
res[,1:4]
*/

/**
 * You can learn more about using RcppParallel at
 * [https://rcppcore.github.com/RcppParallel](https://rcppcore.github.com/RcppParallel).
 */
