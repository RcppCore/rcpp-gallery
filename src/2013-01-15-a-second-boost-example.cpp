// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title A second example of using Boost
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags basics boost
 * @summary This post shows how to use some string functionality from Boost
 *
 * We introduced [Boost](http://www.boost.org) in a 
 * [first post doing some integer math](../a-first-boost-example). In this post we want
 * to look at the very versatile 
 * [Boost.Lexical_Cast](http://www.boost.org/doc/libs/1_52_0/doc/html/boost_lexical_cast.html)
 * library to convert text to numbers -- see the 
 * [Motivation](http://www.boost.org/doc/libs/1_51_0/doc/html/boost_lexical_cast.html#boost_lexical_cast.motivation)
 * for more.
 *
 * As before, I should note that I initially wrote this post on a machine with [Boost](http://www.boost.org) 
 * in a standard system location. <em>So stuff just works.</em> Others may have had to install Boost from source,
 * and into a non-standard location, which may have required an <code>-I</code> flag, 
 * not unlike how we initially added 
 * the C++11 flag in [this post](../first-steps-with-C++11) before the corresponding plugin was added. 
 *
 * This is now automated thanks to the
 * [BH package](http://dirk.eddelbuettel.com/code/bh.html) which, if installed, provides Boost headers 
 * for use by R in compilations just like this one.
 *
 */

// We can now use the BH package
// [[Rcpp::depends(BH)]]

#include <Rcpp.h>
#include <boost/lexical_cast.hpp>  	// one file, automatically found for me

using namespace Rcpp;

using boost::lexical_cast;
using boost::bad_lexical_cast;
 
// [[Rcpp::export]]
std::vector<double> lexicalCast(std::vector<std::string> v) {

    std::vector<double> res(v.size());

    for (int i=0; i<v.size(); i++) {
        try {
            res[i] = lexical_cast<double>(v[i]);
        } catch(bad_lexical_cast &) {
            res[i] = NA_REAL;
        }
    }

    return res;
}


/**
 * This simple program uses the [exceptions idiom we
 * discussed](../intro-to-exceptions) to branch: when a value cannot
 * be converted, a `NA` value is inserted.  
 *
 * We can test the example:
 *
 */

/*** R
v <- c("1.23", ".4", "1000", "foo", "42", "pi/4")
lexicalCast(v)
*/

