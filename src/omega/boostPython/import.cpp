// Copyright Stefan Seefeld 2005.
// Distributed under the Boost Software License, Version 1.0. (See
// accompanying file LICENSE_1_0.txt or copy at
// http://www.boost.org/LICENSE_1_0.txt)
// Visual Studio 2013 and up have round defined
#if _MSC_VER > 1700
#define HAVE_ROUND 1
#endif
#include <boost/python/import.hpp>
#include <boost/python/borrowed.hpp>
#include <boost/python/extract.hpp>
#include <boost/python/handle.hpp>

namespace boost 
{ 
namespace python 
{

object BOOST_PYTHON_DECL import(str name)
{
  // should be 'char const *' but older python versions don't use 'const' yet.
  char *n = python::extract<char *>(name);
  python::handle<> module(PyImport_ImportModule(n));
  return python::object(module);
}

}  // namespace boost::python
}  // namespace boost
