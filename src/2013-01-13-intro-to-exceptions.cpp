// -*- mode: C++; c-indent-level: 4; c-basic-offset: 4; indent-tabs-mode: nil; -*-
/**
 * @title Introduction to exception handling
 * @author Dirk Eddelbuettel
 * @license GPL (>= 2)
 * @tags basics
 * @summary This post illustrates how exception can be use to report error conditons.
 *
 * One of the many features that make C++ different from C is exception handling.  This is
 * a somewhat big topic, and large codebases sometimes eschew exceptions for lack of traceability 
 * in <em>truly large</em> programs (and eg the 
 * [Google in-house C++ style guide](http://google-styleguide.googlecode.com/svn/trunk/cppguide.xml#Exceptions) 
 * is a well-known example of the <em>Just say no</em> school). Opinions are divided; exceptions are generally 
 * seen as a useful tool for smaller-scale projects.
 *
 * We tend to agree. For our purposes, exceptions are just fine.  They allow for a fine-grained way to report errors to R.
 *
 * The basic idea is the that we <strong>must</strong> surround code which could <em>throw an exception</em> 
 * by a block of <code>try</code> and <code>catch</code>.
 *
 * A simple example will help.
 */


#include <Rcpp.h>

using namespace Rcpp;
 
// [[Rcpp::export]]
double takeLog(double val) {
    try {
        if (val <= 0.0) {         	// log() not defined here
            throw std::range_error("Inadmissible value");
        }
        return log(val);
    } catch(std::exception &ex) {	
	forward_exception_to_r(ex);
    } catch(...) { 
	::Rf_error("c++ exception (unknown reason)"); 
    }
    return NA_REAL;             // not reached
}

/** 
 * We can look at this example with a valid, and an invalid argument:
 */

/*** R
  takeLog(exp(1))   # works
  takeLog(-1.0)     # throws exception
*/

/**
 * As we can see, execptions works as expected. By throwing an
 * exception derived from the standard exception call, we arrive in
 * the case first <code>catch</code> branch where the exception text
 * can be captured and turned into a standard R error message.
 *
 * The <em>scaffolding</em> of the <code>try</code> and
 * <code>catch</code> is even automatically added by our common tools
 * <code>cxxfunction()</code> (from the inline package) and
 * <code>sourceCpp()</code>. So this shorter function is equivalent
 * <em>when these tools are used</em>. Otherwise the macros
 * <code>BEGIN_CPP</code> and <code>END_CPP</code> can be used.
 */

#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
double takeLog2(double val) {
    if (val <= 0.0) {         	// log() not defined here
        throw std::range_error("Inadmissible value");
    }
    return log(val);
}

/** 
 * Again, we can look at this example with a valid, and an invalid argument:
 */

/*** R
  takeLog2(exp(1))   # works
  takeLog2(-1.0)     # throws exception
*/

/**
 * This shows that due to the automatic addition of the needed
 * infrastructure, exception handling can add a useful mechanism to
 * signal error conditions back to R.
 *
 * There is even a shortcut defined as Rcpp function <code>stop</code>:
 */

#include <Rcpp.h>

using namespace Rcpp;

// [[Rcpp::export]]
double takeLog3(double val) {
    if (val <= 0.0) {         	// log() not defined here
        stop("Inadmissible value");
    }
    return log(val);
}

/*** R
  takeLog3(exp(1))   # works
  takeLog3(-1.0)     # throws exception
*/
