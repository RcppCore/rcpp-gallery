---
title: Implementing an EM Algorithm for Probit Regressions
author: Jonathan Olmsted
license: GPL (>= 2)
tags: openmp armadillo featured
mathjax: true
summary: We illustrate the development process of creating code to estimate the parameters of a Probit regression model using the EM algorithm sequentially and in parallel.

layout: post
src: 2014-10-01-EM-algorithm-example.Rmd
---

Users new to the [Rcpp](http://dirk.eddelbuettel.com/code/rcpp.html)
family of functionality are often impressed with the
performance gains that can be realized, but struggle to see how to approach
their own computational problems. Many of the most impressive performance gains
are demonstrated with seemingly advanced statistical methods, advanced
C++--related constructs, or both. Even when users are able to understand how
various demonstrated features operate in isolation, examples may implement too
many at once to seem accessible.

The point of this Gallery article is to offer an example application that
performs well (thanks to the Rcpp family) but has reduced statistical and
programming overhead for some users. In addition, rather than simply presenting
the final product, the development process is explicitly documented and
narrated.

## Motivating Example: Probit Regression

As an example, we will consider estimating the parameters the standard Probit
regression model given by

$$
y_i^* =  x_i'\beta + \epsilon_i
$$

where $$x_i$$ and $$\beta$$ are length $$K$$ vectors and the presence of an "intercept" term is absorbed into $$x_i$$ if desired.

The analyst only has access to a censored version of $$y_i^*$$, namely $$y_i$$
where the subscript $$i$$ denotes the $$i$$ th observation.

As is common, the censoring is assumed to generate $$y_i = 1$$
if $$y_i^* \geq 0$$
and
$$y_i = 0$$
otherwise. When we assume
$$\epsilon_i \sim N(0, 1)$$,
the problem is just the Probit regression model loved by all.

To make this concrete, consider a model of voter turnout using the dataset
provided by the Zelig R package.


{% highlight r %}
library("Zelig")
data("turnout")
head(turnout)
{% endhighlight %}



<pre class="output">
   race age educate income vote
1 white  60      14 3.3458    1
2 white  51      10 1.8561    0
3 white  24      12 0.6304    0
4 white  38       8 3.4183    1
5 white  25      12 2.7852    1
6 white  67      12 2.3866    1
</pre>



{% highlight r %}
dim(turnout)
{% endhighlight %}



<pre class="output">
[1] 2000    5
</pre>

Our goal will be to estimate the parameters associated with the variables
*income*, *educate*, and *age*. Since there is nothing special about this
dataset, standard methods work perfectly well.


{% highlight r %}
fit0 <- glm(vote ~ income + educate + age,
            data = turnout,
            family = binomial(link = "probit")
            )

fit0
{% endhighlight %}



<pre class="output">

Call:  glm(formula = vote ~ income + educate + age, family = binomial(link = &quot;probit&quot;), 
    data = turnout)

Coefficients:
(Intercept)       income      educate          age  
    -1.6824       0.0994       0.1067       0.0169  

Degrees of Freedom: 1999 Total (i.e. Null);  1996 Residual
Null Deviance:	    2270 
Residual Deviance: 2030 	AIC: 2040
</pre>

Using `fit0` as our baseline, the question is how can we recover these estimates
with an Rcpp-based approach. One answer is implement the EM-algorithm in C++
snippets that can be processed into R-level functions; that's what we will
do. (Think of this as a Probit regression analog to
[the linear regression example](./fast-linear-model-with-armadillo/) --- but with fewer features.)

### EM Algorithm: Intuition

For those unfamiliar with the EM algorithm, consider
[the Wikipedia article](http://en.wikipedia.org/wiki/Expectation%E2%80%93maximization_algorithm)
and a [denser set of Swarthmore lecture notes](http://www.sccs.swarthmore.edu/users/09/btomasi1/em-probit-regression-2008.pdf).

The intuition behind this approach begins by noticing that if mother nature
revealed the $$y_i^*$$ values, we would simply have a linear regression problem
and focus on

$$
\widehat{\beta} = (X'X)^{-1} X'y^*
$$

where the meaning of the matrix notation is assumed.

Because mother nature is not so kind, we have to *impute* the $$y_i^*$$
values. For a given guess of $$\widehat{\beta}$$, due to our distributional
assumptions about $$\epsilon_i$$ we know that

$$
E\left[y_i^* \big\vert y_i=1, \widehat{\beta}\right] = \mu_i + \frac{\phi(-\mu_i)}{1 - \Phi(-\mu_I)}
$$

and

$$
E\left[y_i^* \big\vert y_i=0, \widehat{\beta}\right] = \mu_i - \frac{\phi(-\mu_i)}{\Phi(-\mu_I)}
$$

where $$\mu_i = x_i'\hat{\beta}$$.

By iterating through these two steps we can eventually recover the desired
parameter estimates:

1. impute/augment $$y_i^*$$ values
2. estimate $$\widehat{\beta}$$ given the data augmentation

## Our Implementations

To demonstrate implementation of the EM algorithm for a Probit regression model
using Rcpp-provided functionality we consider a series of steps.

These are:

1. [Attempt 1: Main Structure](#attempt-1)
2. [Attempt 2: EM with Mistaken Augmentation](#attempt-2)
3. [Attempt 3: EM with Correct Augmentation](#attempt-3)
4. [Attempt 4: EM with Correct Augmentation in Parallel](#attempt-4)

These steps are **not** chosen because each produces useful output (from the
perspective of parameters estimation), but because they mirror milestones in a
development process that benefits new users: only small changes are made at a
time.

To begin, we prepare our R-level data for passage to our eventual C++-based
functions.


{% highlight r %}
mY <- matrix(turnout$vote)
mX <- cbind(1,
            turnout$income,
            turnout$educate,
            turnout$age
            )
{% endhighlight %}

### <a name="attempt-1">Attempt 1: Main Structure</a>

The first milestone will be to mock up a function `em1` that is exported to
create an R-level function of the same name. The key features here are that we
have defined the function to
- accept arguments corresponding to likely inputs
- create containers for the to-be-computed values,
- outline the main loop of the code for the EM iterations, and
- return various values of interest in a list

Users new to the Rcpp process will benefit from return `List` objects in the
beginning. They allow you rapidly return new and different values to the R-level
for inspection.



{% highlight cpp %}
# include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp ;

// [[Rcpp::export()]]
List em1 (const arma::mat y,
          const arma::mat X,
          const int maxit = 10
          ) {
    // inputs
    const int N = y.n_rows ;
    const int K = X.n_cols ;

    // containers
    arma::mat beta(K, 1) ;
    beta.fill(0.0) ; // initialize betas to 1
    arma::mat eystar(N, 1) ;
    eystar.fill(0) ;

    // algorithm
    for (int it = 0 ; it < maxit ; it++) { // EM iterations
        // NEXT STEP: implement algorithm
    }

    // returns
    List ret ;
    ret["N"] = N ;
    ret["K"] = K ;
    ret["beta"] = beta ;
    ret["eystar"] = eystar ;
    return(ret) ;
}
{% endhighlight %}

We know that this code does not produce estimates of anything. Indeed, that is
by design. Neither the `beta` nor `eystar` elements of the returned `list` are
ever updated after they are initialized to 0.

However, we can see that much of the administrative work for a working
implementation is complete.


{% highlight r %}
fit1 <- em1(y = mY,
            X = mX,
            maxit = 20
            )

fit1$beta
{% endhighlight %}



<pre class="output">
     [,1]
[1,]    0
[2,]    0
[3,]    0
[4,]    0
</pre>



{% highlight r %}
head(fit1$eystar)
{% endhighlight %}



<pre class="output">
     [,1]
[1,]    0
[2,]    0
[3,]    0
[4,]    0
[5,]    0
[6,]    0
</pre>

Having verified that input data structures and output data structures are
"working" as expected, we turn to updating the $$y_i^*$$ values.

### <a name="attempt-2">Attempt 2: EM with Mistaken Augmentation</a>

Updates to the $$y_i^*$$ values are different depending on whether $$y_i=1$$ or
$$y_i=0$$. Rather than worrying about correctly imputing the unobserved
propensities, we will use dummy values of 1 and -1 as placeholders. Instead, the
focus is on building on the necessary conditional structure of the code and
looping through the update step for every observation.

Additionally, at the end of each imputation step (the *E* in *EM*) we update the
$$\beta$$ estimate with the least squares estimate (the *M* in *EM*).



{% highlight cpp %}
# include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp ;

// [[Rcpp::export()]]
List em2 (const arma::mat y,
          const arma::mat X,
          const int maxit = 10
          ) {
    // inputs
    const int N = y.n_rows ;
    const int K = X.n_cols ;

    // containers
    arma::mat beta(K, 1) ;
    beta.fill(0.0) ; // initialize betas to 0
    arma::mat eystar(N, 1) ;
    eystar.fill(0) ;

    // algorithm
    for (int it = 0 ; it < maxit ; it++) {
        arma::mat mu = X * beta ;
        // augmentation step
        for (int n = 0 ; n < N ; n++) {
            if (y(n, 0) == 1) { // y = 1
                // NEXT STEP: fix augmentation
                eystar(n, 0) = 1 ;
            }
            if (y(n, 0) == 0) { // y = 0
                // NEXT STEP: fix augmentation
                eystar(n, 0) = -1 ;
            }
        }
        // maximization step
        beta = (X.t() * X).i() * X.t() * eystar ;
    }

    // returns
    List ret ;
    ret["N"] = N ;
    ret["K"] = K ;
    ret["beta"] = beta ;
    ret["eystar"] = eystar ;
    return(ret) ;
}
{% endhighlight %}

This code, like that in Attempt 1, is syntactically fine. But, as we know, the
update step is very wrong. However, we can see that the updates are happening as
we'd expect and we see non-zero returns for the `beta` element and the `eystar`
element.


{% highlight r %}
fit2 <- em2(y = mY,
            X = mX,
            maxit = 20
            )

fit2$beta
{% endhighlight %}



<pre class="output">
          [,1]
[1,] -0.816273
[2,]  0.046065
[3,]  0.059481
[4,]  0.009085
</pre>



{% highlight r %}
head(fit2$eystar)
{% endhighlight %}



<pre class="output">
     [,1]
[1,]    1
[2,]   -1
[3,]   -1
[4,]    1
[5,]    1
[6,]    1
</pre>

### <a name="attempt-3">Attempt 3: EM with Correct Augmentation</a>

With the final logical structure of the code built out, we will now correct the
data augmentation. Specifically, we replace the assignment of 1 and -1 with the
expectation of the unobservable values $$y_i^*$$. Rather than muddy our EM
function (`em3()`) with further arithmetic, we sample call the C++ level
functions `f()` and `g()` which were included prior to our definition of
`em3()`.

But, since these are just utility functions needed internally by `em3()`, they
are not tagged to be exported (via `// [[Rcpp::export()]]`) to the R level.

As it stands, this is a correct implementation (although there is room for
improvement).


{% highlight cpp %}
# include <RcppArmadillo.h>
// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp ;

double f (double mu) {
    double val = ((R::dnorm(-mu, 0, 1, false)) /
                  (1 - R::pnorm(-mu, 0, 1, true, false))
                  ) ;
    return(val) ;
}

double g (double mu) {
    double val = ((R::dnorm(-mu, 0, 1, false)) /
                  (R::pnorm(-mu, 0, 1, true, false))
                  ) ;
    return(val) ;
}

// [[Rcpp::export()]]
List em3 (const arma::mat y,
          const arma::mat X,
          const int maxit = 10
          ) {
    // inputs
    const int N = y.n_rows ;
    const int K = X.n_cols ;

    // containers
    arma::mat beta(K, 1) ;
    beta.fill(0.0) ; // initialize betas to 0
    arma::mat eystar(N, 1) ;
    eystar.fill(0) ;

    // algorithm
    for (int it = 0 ; it < maxit ; it++) {
        arma::mat mu = X * beta ;
        // augmentation step
        // NEXT STEP: parallelize augmentation step
        for (int n = 0 ; n < N ; n++) {
            if (y(n, 0) == 1) { // y = 1
                eystar(n, 0) = mu(n, 0) + f(mu(n, 0)) ;
            }
            if (y(n, 0) == 0) { // y = 0
                eystar(n, 0) = mu(n, 0) - g(mu(n, 0)) ;
            }
        }
        // maximization step
        beta = (X.t() * X).i() * X.t() * eystar ;
    }

    // returns
    List ret ;
    ret["N"] = N ;
    ret["K"] = K ;
    ret["beta"] = beta ;
    ret["eystar"] = eystar ;
    return(ret) ;
}
{% endhighlight %}


{% highlight r %}
fit3 <- em3(y = mY,
            X = mX,
            maxit = 100
            )
{% endhighlight %}


{% highlight r %}
head(fit3$eystar)
{% endhighlight %}



<pre class="output">
        [,1]
[1,]  1.3910
[2,] -0.6599
[3,] -0.7743
[4,]  0.8563
[5,]  0.9160
[6,]  1.2677
</pre>


Second, notice that this output is identical to the parameter estimates (the
object `fit0`) from our R level call to the `glm()` function.


{% highlight r %}
fit3$beta
{% endhighlight %}



<pre class="output">
         [,1]
[1,] -1.68241
[2,]  0.09936
[3,]  0.10667
[4,]  0.01692
</pre>



{% highlight r %}
fit0
{% endhighlight %}



<pre class="output">

Call:  glm(formula = vote ~ income + educate + age, family = binomial(link = &quot;probit&quot;), 
    data = turnout)

Coefficients:
(Intercept)       income      educate          age  
    -1.6824       0.0994       0.1067       0.0169  

Degrees of Freedom: 1999 Total (i.e. Null);  1996 Residual
Null Deviance:	    2270 
Residual Deviance: 2030 	AIC: 2040
</pre>

### <a name="attempt-4">Attempt 4: EM with Correct Augmentation in Parallel</a>

With a functional implementation complete as `em3()`, we know turn to the second
order concern: *performance*. The time required to evaluate our function can be
reduced from the perspective of a user sitting at a computer with idle cores.

Although the small size of these data don't necessitate parallelization, the *E*
step is a natural candidate for being parallelized. Here, the parallelization
relies on OpenMP. See [here](./tags/openmp/) for other examples of combining
Rcpp and OpenMP or [here](./tags/parallel/) for a different approach.



{% highlight r %}
Sys.setenv("PKG_CXXFLAGS" = "-fopenmp")
Sys.setenv("PKG_LIBS" = "-fopenmp")
{% endhighlight %}

Aside from some additional compiler flags, the changes to our new implementation
in `em4()` are minimal. They are:

- include the additional header
- mark the `for` loop for parallelization with a `#pragma`



{% highlight cpp %}
# include <RcppArmadillo.h>
# include <omp.h>

// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp ;

double f (double mu) {
    double val = ((R::dnorm(-mu, 0, 1, false)) /
                  (1 - R::pnorm(-mu, 0, 1, true, false))
                  ) ;
    return(val) ;
}

double g (double mu) {
    double val = ((R::dnorm(-mu, 0, 1, false)) /
                  (R::pnorm(-mu, 0, 1, true, false))
                  ) ;
    return(val) ;
}

// [[Rcpp::export()]]
List em4 (const arma::mat y,
          const arma::mat X,
          const int maxit = 10,
          const int nthr = 1
          ) {
    // inputs
    const int N = y.n_rows ;
    const int K = X.n_cols ;
    omp_set_num_threads(nthr) ;

    // containers
    arma::mat beta(K, 1) ;
    beta.fill(0.0) ; // initialize betas to 0
    arma::mat eystar(N, 1) ;
    eystar.fill(0) ;

    // algorithm
    for (int it = 0 ; it < maxit ; it++) {
        arma::mat mu = X * beta ;
        // augmentation step
#pragma omp parallel for
        for (int n = 0 ; n < N ; n++) {
            if (y(n, 0) == 1) { // y = 1
                eystar(n, 0) = mu(n, 0) + f(mu(n, 0)) ;
            }
            if (y(n, 0) == 0) { // y = 0
                eystar(n, 0) = mu(n, 0) - g(mu(n, 0)) ;
            }
        }
        // maximization step
        beta = (X.t() * X).i() * X.t() * eystar ;
    }

    // returns
    List ret ;
    ret["N"] = N ;
    ret["K"] = K ;
    ret["beta"] = beta ;
    ret["eystar"] = eystar ;
    return(ret) ;
}
{% endhighlight %}

This change should not (and does not) result in any change to the calculations
being done. However, if our algorithm involved random number generation, great
care would need to be taken to ensure our results were reproducible.


{% highlight r %}
fit4 <- em4(y = mY,
            X = mX,
            maxit = 100
            )

identical(fit4$beta, fit3$beta)
{% endhighlight %}



<pre class="output">
[1] TRUE
</pre>

Finally, we can confirm that our parallelization was "successful". Again,
because there is really no need to parallelize this code, performance gains are
modest. But, that it indeed runs faster is clear.


{% highlight r %}
library("microbenchmark")

microbenchmark(seq = (em3(y = mY,
                          X = mX,
                          maxit = 100
                          )
                      ),
               par = (em4(y = mY,
                          X = mX,
                          maxit = 100,
                          nthr = 4
                          )
                      ),
               times = 20
               )
{% endhighlight %}



<pre class="output">
Unit: milliseconds
 expr   min    lq  mean median    uq   max neval cld
  seq 32.94 33.01 33.04  33.03 33.07 33.25    20   b
  par 11.16 11.20 11.35  11.26 11.29 13.16    20  a 
</pre>

## Wrap-Up

The purpose of this lengthy gallery post is neither to demonstrate new
functionality nor the computational feasibility of cutting-edge
algorithms. Rather, it is to explicitly walk through a development process
similar that which new users can benefit from using while using a very common
statistical problem, Probit regression.
