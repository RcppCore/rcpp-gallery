/**
 * @title Accessing List Elements
 * @author Hadley Wickham
 * @tags featured lists
 * @summary Demonstrates verifying a list's class then accessing its elements
 *   
 * Linear model objects that lm produces are lists and the components are
 * always of the same type. The following code illustrates how you might
 * component the mean percentage error (mpe) of a linear model:
 */

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
double mpe(List mod) {
  if (!mod.inherits("lm")) stop("Input must be a linear model");

  NumericVector resid = as<NumericVector>(mod["residuals"]);
  NumericVector fitted = as<NumericVector>(mod["fitted.values"]);

  int n = resid.size();
  double err = 0;
  for(int i = 0; i < n; ++i) {
    err += resid[i] / (fitted[i] + resid[i]);
  }
  return err / n;
}

/**
 * Note the use of the inherits() method and the stop() function to check that
 * the object really is a linear model.
 */


/*** R
  mod <- lm(mpg ~ wt, data = mtcars)
  mpe(mod)
*/

