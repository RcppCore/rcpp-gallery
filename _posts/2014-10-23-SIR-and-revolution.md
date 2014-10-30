---
title: Sampling Importance Resampling (SIR) and social revolution.
author: Jonathan Olmsted
license: GPL (>= 2)
mathjax: true
tags: armadillo modeling featured
summary: We use SIR to characterize the posterior distribution of parameters associated with the probability of social revolution.
layout: post
src: 2014-10-23-SIR-and-revolution.Rmd
---

## Motivation

The purpose of this gallery post is several fold:

- to demonstrate the use of the new and improved C++-level
  implementation of **R**'s `sample()` function ([see here](https://github.com/RcppCore/RcppArmadillo/commit/3d772e38651ae5af71a55fe9b8b70854db93f4b3))
- to demonstrate the Gallery's new support for images in contributed
  posts
- to demonstrate the usefulness of SIR for updating posterior beliefs
  given a sample from an arbitrary prior distribution


## Application: Foreign Threats and Social Revolution

The application in this post uses an example from Jackman's _Bayesian
Analysis for the Social Sciences_ (page 72) which now has a 30-year
history in the Political Science (See Jackman for more
references). The focus is on the extent to which the probability of
revolution varies with facing a foreign threat or not. Facing a
foreign threat is measured by "defeated ..." or "not defeated ..."
over a span of 20 years. The countries come from in Latin
America. During this period of time, there are only three revolutions:
Bolivia (1952), Mexico (1910), and Nicaragua (1979).

<style>
table {margin-left : auto ; margin-right : auto ;}
table, th, td {
padding : 5px ;
background-color : lightgrey ;
border: 1px solid white ;}
</style>


|                                            | Revolution  | No Revolution  |
| --------------------------------------:    | ----------: | -------------: |
| **Defeated and invaded or lost territory** | 1           | 7              |
| **Not defeated for 20 years**              | 2           | 74             |


The goal is to learn about the true, unobservable probabilities of
revolution given a recent defeat or the absence of one. That is, we
care about

$$
\theta_1 = \Pr (\text{revolution} ~ \vert ~ \text{defeat})
$$

and

$$
\theta_2 = \Pr (\text{revolution} ~ \vert ~ \text{no defeat}).
$$

And, beyond that, we care about whether $$\theta_1$$ and $$\theta_2$$
differ.

These data are assumed to arise from a Binomial process, where the
likelihood of the probability parameter value, $$\theta$$, is

$$
L(\theta; n, k) = {n \choose k} \theta^k (1 - \theta)^{n - k},
$$

where $$n$$ is the total number of revolutions and non-revolutions and
$$k$$ is the number of revolutions. The MLE for this model is just the
sample proportion, so a Frequentist statistician would be wondering
whether
$$\hat{\theta}_1 = \frac{1}{1 + 7} = .125 $$
was sufficiently larger than
$$\hat{\theta}_2 = \frac{2}{2 + 74} = .026$$
to be unlikely to have happened by chance alone (given the null
hypothesis that the two proportions were identical).

A Bayesian statistician could approach the question a bit more
directly and compute the probability that $$\theta_1 - \theta_2 > 0.$$
To do this, we first need samples from the posterior distribution of
$$\theta_1$$ and $$\theta_2$$. In this post, we will get these samples
via Sampling Importance Resampling.

## Sampling Importance Resampling

Sampling Importance Resampling allows us to sample from the posterior
distribution, $$p(\theta | \text{data})$$ where

$$
p(\theta | \text{data}) \propto L(\theta; \text{data}) \times p(\theta)
$$

by resampling from a series of draws from the prior,
$$p(\theta)$$. Denote one of those $$n$$ draws from the prior
distribution, $$p(\theta)$$, as $$\theta_i$$. Then draw $$i$$ from the
prior sample is drawn with replacement into the posterior sample with
probability

$$
q_i = \frac{L(\theta_i; \text{data})}{ \sum_{j=1}^{n} L(\theta_j; \text{data})}
$$

### Generating Samples from the Prior Distributions

We begin by drawing many samples from a series of prior
distributions. Although using a prior Beta prior distribution on the
$$\theta$$ parameter admits a closed-form solution, the point here is
to demonstrate a simulation based approach. On the other hand, a Gamma
prior distribution over $$\theta$$ is very much **not conjugate** and
simulation is the best approach.

In particular, we will consider our posterior beliefs about the
different in probabilities under five different prior distributions.


{% highlight r %}
dfPriorInfo <- data.frame(id = 1:5,
                          dist = c("beta", "beta", "gamma", "beta", "beta"),
                          par1 = c(1, 1, 3, 10, .5),
                          par2 = c(1, 5, 20, 10, .5),
                          stringsAsFactors = FALSE)
dfPriorInfo
{% endhighlight %}



<pre class="output">
  id  dist par1 par2
1  1  beta  1.0  1.0
2  2  beta  1.0  5.0
3  3 gamma  3.0 20.0
4  4  beta 10.0 10.0
5  5  beta  0.5  0.5
</pre>

Using the data frame `dfPriorInfo` and the `plyr` package, we will
draw a total of 20,000 values from *each* of the prior
distributions. This can be done in any number of ways and is
completely independent of using **Rcpp** for the SIR magic.


{% highlight r %}
library("plyr")
MC1 <- 20000
dfPriors <- ddply(dfPriorInfo, "id",
                  .fun = (function(X) data.frame(draws = (do.call(paste("r", X$dist, sep = ""),
                                                                  list(MC1, X$par1, X$par2))))))
{% endhighlight %}

However, we can confirm that our draws are as we expect and that we
have the right number of them (5 * 20k = 100k).


{% highlight r %}
head(dfPriors)
{% endhighlight %}



<pre class="output">
  id     draws
1  1 0.7124225
2  1 0.5910231
3  1 0.0595327
4  1 0.4718945
5  1 0.4485650
6  1 0.0431667
</pre>



{% highlight r %}
dim(dfPriors)
{% endhighlight %}



<pre class="output">
[1] 100000      2
</pre>

### Re-Sampling from the Prior

Now, we write a C++ snippet that will create our R-level function to
generate a sample of `D` values from the prior draws (`prdraws`) given
their likelihood after the data (i.e., number of success -- `nsucc`,
number of failures -- `nfail`).

The most important feature to mention here is the use of some new and
improved extensions which effectively provide an equivalent,
performant mirror of **R**'s `sample()` function at the
C++-level. **Important: as of the time of the writing of this post
these features were not on CRAN, only on github.**

The return value of this function is a length `D` vector of draws from
the posterior distribution given the draws from the prior distribution
where the likelihood is used as a filtering weight.


{% highlight cpp %}
# include <RcppArmadilloExtensions/sample.h>
# include <RcppArmadilloExtensions/fixprob.h>

// [[Rcpp::depends(RcppArmadillo)]]

using namespace Rcpp ;

// [[Rcpp::export()]]
NumericVector samplePost (const NumericVector prdraws,
                          const int D,
                          const int nsucc,
                          const int nfail) {
    int N = prdraws.size();
    NumericVector wts(N);
    for (int n = 0 ; n < N ; n++) {
        wts(n) = pow(prdraws(n), nsucc) * pow(1 - prdraws(n), nfail);
    }
    RcppArmadillo::FixProb(wts, N, true);

    NumericVector podraws = RcppArmadillo::sample(prdraws, D, true, wts);
    return(podraws);
}
{% endhighlight %}

To use the `samplePost()` function, we create the **R** representation
of the data as follows.


{% highlight r %}
nS <- c(1, 2) # successes
nF <- c(7, 74) # failures
{% endhighlight %}

As a simple example, consider drawing a posterior sample of size 30
for the "defeated case" from discrete prior distribution with equal
weight on the $$\theta$$ values of .125 (the MLE), .127, and .8. We
see there is a mixture of .125 and .127 values, but no .8
values. $$\theta$$ values of .8 were simply to unlikely (given the
likelihood) to be resampled from the prior.



{% highlight r %}
table(samplePost(c(.125, .127, .8), 30, nS[1], nF[1]))
{% endhighlight %}



<pre class="output">

0.125 0.127 
    9    21 
</pre>

Again making use of the **plyr** package, we construct samples of size
20,000 for both $$\theta_1$$ and $$\theta_2$$ under each of the 5
prior distribution samples. These posterior draws are stored in the
data frame `dfPost`.



{% highlight r %}
MC2 <- 20000
f1 <- function(X) {
    draws <- X$draws
    t1 <- samplePost(draws, MC2, nS[1], nF[1])
    t2 <- samplePost(draws, MC2, nS[2], nF[2])
    return(data.frame(theta1 = t1, theta2 = t2))
}

dfPost <- ddply(dfPriors, "id", f1)
{% endhighlight %}



{% highlight r %}
head(dfPost)
{% endhighlight %}



<pre class="output">
  id    theta1    theta2
1  1 0.3067334 0.0130865
2  1 0.1421879 0.0420830
3  1 0.3218130 0.0634511
4  1 0.0739756 0.0363466
5  1 0.1065267 0.0460336
6  1 0.0961749 0.0440790
</pre>



{% highlight r %}
dim(dfPost)
{% endhighlight %}



<pre class="output">
[1] 100000      3
</pre>

## Summarizing Posterior Inferences

Here, we are visualizing the posterior draws for the quantity of
interest $$(\theta_1 - \theta_2)$$ --- the difference in probabilities
of revolution. These posterior draws are grouped according to the
prior distribution used. A test of whether revolution is more likely
given a foreign threat is operationalized by the probability that
$$\theta_1 - \theta_2$$ is positive. This probability for each
distribution is shown in white. For all choices of the prior here, the
probability that "foreign threat matters" exceeds .90.

The full posterior distribution of $$\theta_1 - \theta_2$$ is shown
for each of the five priors in blue. A solid, white vertical band
indicates "no effect". In all cases. the majority of the mass is
clearly to the right of this band.

Recall that the priors are, themselves, over the individual
revolution probabilities, $$\theta_1$$ and $$\theta_2$$. The general
shape of each of these prior distributions of the $$\theta$$ parameter
is shown in a grey box by the white line. For example, $$Beta(1, 1)$$
is actually a uniform distribution over the parameter space,
$$[0, 1]$$. On the other hand, $$Beta(.5,.5)$$ has most of its mass at
the two tails.



<img src="../figure/2014-10-23-SIR-and-revolution-unnamed-chunk-9-1.png" title="plot of chunk unnamed-chunk-9" alt="plot of chunk unnamed-chunk-9" width="800px" height="800px" style="display: block; margin: auto;" />

At least across these specifications of the prior distributions on
$$\theta$$, the conclusion that "foreign threats matter" finds a good
deal of support. What is interesting about this application is that
despite these distributions over the difference in $$\theta$$
probabilities, the p-value associated with Fisher's Exact Test for 2 x
2 tables is just .262.
