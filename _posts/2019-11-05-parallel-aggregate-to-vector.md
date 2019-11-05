---
title: Using RcppParallel to aggregate to a vector
author: Mark Padgham
license: GPL (>= 2)
tags: parallel 
summary: Demonstrates RcppParallel aggregation to an output vector.
layout: post
src: 2019-11-05-parallel-aggregate-to-vector.cpp
---
This article demonstrates using the
[RcppParallel](https://rcppcore.github.com/RcppParallel) package to aggregate
to an output vector. It extends directly from previous demonstrations of
[single-valued
aggregation](https://gallery.rcpp.org/articles/parallel-vector-sum), through
providing necessary details to enable aggregation to a vector, or by
extension, to any arbitrary form.


### The General Problem

Many tasks require aggregation to a vector result, and many such tasks can be
made more efficient by performing such aggregation in parallel. The general
problem is that the vector in which results are to be aggregated has to be
shared among the parallel threads. This is a `parallelReduce` task - we need
to split the singular task into effectively independent, parallel tasks,
perform our aggregation operation on each of those tasks, yielding as many
instances of our aggregate result vector as there are parallel tasks, and
then finally join all of those resultant vectors from the parallel tasks into
our desired singular result vector. The general structure of the code
demonstrated here extends from the previous Gallery article on [parallel
vector sums](https://gallery.rcpp.org/articles/parallel-vector-sum), through
extending to summation to a vector result, along with the passing of
additional variables to the parallel worker. The following code demonstrates
aggregation to a vector result that holds the row sums of a matrix, noting at
the output that is not intended to represent efficient code, rather it is
written to explicitly emphasise the principles of using `RcppParallel` to
aggregate over a vector result.

### The parallelReduce Worker

The following code defines our parallel worker, in which the input is
presumed for demonstration purposes to be a matrix stored as a single vector,
and so has of total length `nrow * ncol`.  The demonstration includes a few
notable features:

1. The main `input` simply provides an integer index into the rows of the
matrix, with the parallel job splitting the task among elements of that
index. This explicit specification of an index vector is not necessary, but
serves here to clarify what the worker is actually doing. An alternative
would be for `input` to be `the_matrix`, and subsequently call the parallel
worker only over `[0 ... nrow]` of that vector which has a total length of
`nrow * ncol`.

2. We are passing two additional variables specifying `nrow` and `ncol`.
Although one of these could be inferred at run time, we pass them simply to
demonstrate how this is done. Note in particular the form in the second
constructor, called for each `Split` job, which accepts as input the
variables as defined by the main constructor, and so all variable definitions
are of the form, `nrow(oneJob.nrow)`. The initial constructor also has input
variables explicitly defined with `_in` suffices, to clarify exactly how such
variable passing works.

3. No initial values for the `output` are passed to the constructors. Rather,
`output` must be resized to the desired size by each of those constructors,
and so each repeats the line `output.resize(nrow, 0.0)`, which also
initialises the values.  (This is more readily done using a `std::vector`
than an `Rcpp` vector, with final conversion to an `Rcpp` vector result
achieved through a simple `Rcpp::wrap` call.)


{% highlight cpp %}
#include <Rcpp.h>
// [[Rcpp::depends(RcppParallel)]]
#include <RcppParallel.h>
using namespace Rcpp;
using namespace RcppParallel;

struct OneJob : public Worker
{
    RVector<int> input;

    const NumericVector the_matrix;
    const size_t nrow;
    const size_t ncol;

    std::vector<double> output;

    // Constructor 1: The main constructor
    OneJob (
            const IntegerVector input_in,
            const NumericVector the_matrix_in,
            const size_t nrow_in,
            const size_t ncol_in) :
        input(input_in), the_matrix(the_matrix_in),
        nrow(nrow_in), ncol(ncol_in), output()
    {
        output.resize(nrow, 0.0);
    }

    // Constructor 2: Called for each split job
    OneJob (
            const OneJob &oneJob,
            Split) :
        input(oneJob.input), the_matrix(oneJob.the_matrix),
        nrow(oneJob.nrow), ncol(oneJob.ncol), output()
    {
        output.resize(nrow, 0.0);
    }

    // Parallel function operator
    void operator() (std::size_t begin, std::size_t end)
    {
        for (size_t i = begin; i < end; i++)
        {
            // Very inefficient yet explicit way to calculate row sums:
            for (size_t j = 0; j < ncol; j++) {
                // static_cast becuase (i,j,nrow) are size_t, aka unsigned long,
                // but Rcpp vectors require `R_xlen_t`, aka long.
                output[i] += the_matrix[static_cast<R_xlen_t>(i + j * nrow)];
            }
        }
    } // end parallel function operator

    void join (const OneJob &rhs)
    {
        for (size_t i = 0; i < nrow; i++) {
            output[i] += rhs.output[i];
        }
    }
};
{% endhighlight %}

The worker can then be called via `parallelReduce` with the following code,
in which `static_cast`s are necessary because `.size()` applied to `Rcpp`
objects returns an `R_xlen_t` or `long` value, but we need to pass `unsigned
long` or `size_t` values to the worker to use as indices into standard C++
vectors. The `output` of `oneJob` is a `std::vector<double>`, which is
converted to an `Rcpp::NumericVector` through a simple call to `Rcpp::wrap`.

{% highlight cpp %}
// [[Rcpp::export]]
NumericVector vector_aggregator (IntegerVector index, NumericVector x)
{
    const size_t nrow = static_cast <size_t> (index.size ());
    const size_t ncol = static_cast <size_t> (x.size ()) / nrow;
    OneJob oneJob (index, x, nrow, ncol);
    parallelReduce (0, nrow, oneJob);
    return wrap (oneJob.output);
}
{% endhighlight %}

### Demonstration

Finally, the following code demonstrates that this parallel worker correctly
returns the row sums of the input matrix.

{% highlight r %}
# allocate a vector
nrow <- 1e5
ncol <- 10
x <- runif (nrow * ncol) # input matrix
res <- vector_aggregator (seq(nrow), x)

# confirm that this equals rowsums of the matrix:
xmat <- matrix(x, ncol = ncol)
identical(res, rowSums(xmat))
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>

You can learn more about using RcppParallel at
[https://rcppcore.github.com/RcppParallel](https://rcppcore.github.com/RcppParallel).
