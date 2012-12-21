/**
 * @title Working with Missing Values
 * @author Hadley Wickham
 * @license GPL (>= 2)
 * @tags basics
 * @summary Techniques for dealing with missing values in both vectors
 *   and scalars.
 */
 
/**
 * If you're working with missing values, you need to know two things: - What
 * happens when you put missing values in scalars (e.g. `double1`) - How to get
 * and set missing values in vectors (e.g. `NumericVector`)
 * 
 * ## Scalars
 * 
 * The following code explores what happens when you take one of R's missing
 * values, coerce it into a scalar, and then coerce back to an R vector.
 */

#include <Rcpp.h>
using namespace Rcpp;

// [[Rcpp::export]]
List scalarMissings() {
  int int_s = NA_INTEGER;
  String chr_s = NA_STRING;
  bool lgl_s = NA_LOGICAL;
  double num_s = NA_REAL;

  return List::create(int_s, chr_s, lgl_s, num_s);
}

/**
 * Which as expected yields missing values in R:
 */

/*** R
str(scalarMissings())
*/

/**
 * So:
 *  
 *  - `IntegerVector` -> `int`: stored as the smallest integer 
 *  - `CharacterVector` -> `String`: the string "NA" 
 *  - `LogicalVector` -> `bool`: TRUE. To work with missing values in logical
 *     vectors, use an int instead of a bool.
 *  - `NumericVector` -> `double`: stored as an NaN, and preserved. Most 
 *     numerical operations will behave as you expect, but as discussed 
 *     below logical comparison will not. 
 * 
 * If you're working with doubles, you may be able to get away with ignoring
 * missing values and working with NaN (not a number). R's missing values are
 * a special type of the IEEE 754 floating point number NaN. That means if you
 * coerce them to double in your C++ code, they will behave like regular NaN's.
 * That means, in a logical context they always evaluate to FALSE.
 */

/**
 * ## Vectors
 * 
 * To set a missing value in a vector, you need to use a missing value 
 * specific to the type of vector:
 */

// [[Rcpp::export]]
List missingSampler() {
  return(List::create(
    NumericVector::create(NA_REAL), 
    IntegerVector::create(NA_INTEGER),
    LogicalVector::create(NA_LOGICAL), 
    CharacterVector::create(NA_STRING)));
}

/**
 * Now let's confirm that these values do in fact appear missing in R:
 */

/*** R
str(missingSampler())
*/

/**
 * To check if a value in a vector is missing, use the class method `is_na`:
 */

// [[Rcpp::export]]
LogicalVector isNA(NumericVector x) {
  int n = x.size();
  LogicalVector out(n);
  
  for (int i = 0; i < n; ++i) {
    out[i] = NumericVector::is_na(x[i]);
  }
  return out;
}

/**
 * Here we test with some missing and non-missing values:
 */

/*** R
isNA(c(NA, 5.4, 3.2, NA))
*/

/**
 * Equivalent behavior to the `isNA` function can be obtained by calling the
 * `is_na` sugar function, which takes a vector and returns a logical vector.
 */

