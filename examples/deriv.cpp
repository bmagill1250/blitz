/*****************************************************************************
 * deriv.cpp     Blitz++ Array<T,1> example, illustrating expressions,
 *               ranges, and subvectors.
 *
 * $Id$
 *
 * $Log$
 * Revision 1.2  2001/01/24 23:41:54  tveldhui
 * Widespread changes to reduce compile time.  For backwards
 * compatibility, #include <blitz/array.h> enables BZ_GANG_INCLUDE
 * mode which includes all array and vector functionality (about
 * 120000 lines of code).  #include <blitz/array-only.h> includes
 * a minimal subset of Array funcitonality; other features must
 * be included explicitly.
 *
 * Revision 1.1.1.1  2000/06/19 12:26:17  tveldhui
 * Imported sources
 *
 * Revision 1.2  1997/07/16 19:38:23  tveldhui
 * Update: Alpha release 0.2 (Arrays)
 *
 * Revision 1.1  1997/01/17 17:54:06  tveldhui
 * Initial revision
 */

#include <blitz/array-only.h>
#include <blitz/array/ops.h>
#include <blitz/array/funcs.h>

#ifdef BZ_NAMESPACES
using namespace blitz;
using namespace blitz::tensor;   
#endif

typedef Array<double,1> Array1D;

int main()
{
    // In this example, the function cos(x)^2 and its second derivative
    // 2 (sin(x)^2 - cos(x)^2) are sampled over the range [0,1).
    // The second derivative is approximated numerically using a
    // [ 1 -2  1 ] mask, and the approximation error is computed.

    const int numSamples = 100;              // Number of samples
    double delta = 1. / numSamples;          // Spacing of samples
    Range R(0, numSamples - 1);              // Index set of the vector

    // Sample the function y = cos(x)^2 over [0,1)
    //
    // The initialization for y (below) will be translated via expression
    // templates into something of the flavour
    //
    // for (unsigned i=0; i < 99; ++i)
    // {
    //     double _t1 = cos(i * delta);
    //     y[i] = _t1 * _t1;
    // }
    //
    // The variable i comes from the blitz::tensor namespace, and is
    // an "index placeholder" which represents the array index.
   
    Array1D y(R);
    y = sqr(cos(i * delta));

    // Sample the exact second derivative
    Array1D y2exact(R);
    y2exact = 2.0 * (sqr(sin(i * delta)) - sqr(cos(i * delta)));

    // Approximate the 2nd derivative using a [ 1 -2  1 ] mask
    // We can only apply this mask to the elements 1 .. 98, since
    // we need one element on either side to apply the mask.
    Range I(1,numSamples-2);
    Array1D y2(numSamples);

    y2(I) = (y(I-1) - 2 * y(I) + y(I+1)) / (delta*delta);
  
    // The above difference equation will be transformed into
    // something along the lines of
    //
    // double _t2 = delta*delta;
    // for (int i=1; i < 99; ++i)
    //     y2[i] = (y[i-1] - 2 * y[i] + y[i+1]) / _t2;
 
    // Now calculate the root mean square approximation error:

    double error = sqrt(mean(sqr(y2(I) - y2exact(I))));
 
    // Display a few elements from the vectors.
    // This range constructor means elements 1 to 91 in increments
    // of 15.
    Range displayRange(1, 91, 15);
 
    cout << "Exact derivative:" << y2exact(displayRange) << endl
         << "Approximation:   " << y2(Range(displayRange)) << endl
         << "RMS Error:       " << error << endl;

    return 0;
}

