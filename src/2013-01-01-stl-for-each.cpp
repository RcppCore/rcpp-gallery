// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title STL for_each and generalized iteration
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags stl featured
 * @summary Using the STL's for_each 
 *
 * The STL contains a very general looping or sweeping construct in
 * the `for_each` algorith.  It can be used with function objects
 * (such as the simple `square` function used [here](../stl-transform))
 * but also with custom class which can be used to keep to keep state.
 * 
 */

#include <Rcpp.h>

using namespace Rcpp;

// somewhat silly little class derived from unary_function<T, void> to
// illustrate keeping state -- we interpret the vector x as containing
// growth rates (or returns), and we compute cumulative as well as
// relative gains.
template<class T> class cumProd : public std::unary_function<T, void> {
public:
    cumProd() : cp(1.0), cnt(1) {}      // constructor
    void operator() (T x) {             // default operator()
        cp *= 1.0 + x;
        Rcout << "Iteration "   << cnt++
              << " Growth "     << x
              << " Compounded " << cp 
              << " Proportion " << x/(cp - 1.0)
              << std::endl;
    }
private:  
    double cp;
    int cnt;
};

// [[Rcpp::export]]
void forEach(Rcpp::NumericVector x) {
    std::for_each(x.begin(), x.end(), cumProd<double>());
}

/** 
 * We can illustrate this on a simple example:
 */

/*** R
set.seed(42)
x <- rnorm(6, 0, 0.01)
x
forEach(x)
*/

