#include <omega.h>

using namespace omega;

void hello()
{
    omsg("Hello");
}

///////////////////////////////////////////////////////////////////////////////
// Python wrapper code.
#ifdef OMEGA_USE_PYTHON
#include "omega/PythonInterpreterWrapper.h"

BOOST_PYTHON_MODULE(signac)
{
    def("hello", hello);
}
#endif