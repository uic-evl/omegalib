/******************************************************************************
* THE OMEGA LIB PROJECT
*-----------------------------------------------------------------------------
* Copyright 2010-2015		Electronic Visualization Laboratory,
*							University of Illinois at Chicago
* Authors:
*  Alessandro Febretti		febret@gmail.com
*-----------------------------------------------------------------------------
* Copyright (c) 2010-2015, Electronic Visualization Laboratory,
* University of Illinois at Chicago
* All rights reserved.
* Redistribution and use in source and binary forms, with or without modification,
* are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer. Redistributions in binary
* form must reproduce the above copyright notice, this list of conditions and
* the following disclaimer in the documentation and/or other materials provided
* with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR
* SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
* CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*-----------------------------------------------------------------------------
* What's in this file
*  All the stuff needed to use python from omegalib
******************************************************************************/
#ifndef __PythonInterpreterWrapper_h
#define __PythonInterpreterWrapper_h

#ifdef _DEBUG
    #define PYTHON_DEBUG_HACK
    #undef _DEBUG
#endif

#ifdef OMEGA_OS_LINUX
    // Remove redefinition warnings in linux
    #undef _POSIX_C_SOURCE
    #undef _XOPEN_SOURCE
#endif

// Visual Studio 2013 and up have round defined
#if _MSC_VER > 1700
    #define HAVE_ROUND 1
#endif

#include <Python.h>

#ifdef PYTHON_DEBUG_HACK
    #define _DEBUG
#endif

#include "structmember.h"

#include "osystem.h"

///////////////////////////////////////////////////////////////////////////////
#ifndef BOOST_PYTHON_SOURCE
    #define BOOST_PYTHON_NO_LIB
#endif
#include <boost/python.hpp>
#include <boost/utility.hpp>
using namespace boost::python;

// INTERNAL USE ONLY >>>>>
#define PYAPI_RETURN_VALUE return_value_policy<return_by_value>()
#define PYAPI_RETURN_REF return_value_policy<return_by_smart_ptr>()
#define PYAPI_RETURN_NEW_INSTANCE return_value_policy<manage_new_object>()
#define PYAPI_RETURN_INTERNAL_REF return_internal_reference<>()
//#define PYAPI_RETURN_REFERENCE return_value_policy<copy_const_reference>()
#define PYAPI_POINTER_LIST(itemName, className) class_< List<itemName*> > (className, no_init).def("__iter__", iterator< List<itemName*>, return_internal_reference<> >());
// <<<<< INTERNAL USE ONLY

//! Start declaring an enumeration
#define PYAPI_ENUM(fullEnumName, enumName) enum_<fullEnumName>(#enumName)
//! Declare an enumeration value
#define PYAPI_ENUM_VALUE(enumName, valueName) .value(#valueName, enumName::valueName)

//! Declare a new class with by-value semantics and no constructor. Useful for objects that can be 
//! created on the C++ side and passed to python, but that should not be created by python code.
#define PYAPI_BASE_CLASS(className) 	class_<className, boost::noncopyable >(#className, no_init)
//! Declare a new class with by value semantics, and with an empty constructor.
#define PYAPI_BASE_CLASS_WITH_CTOR(className) class_<className, boost::noncopyable >(#className)

//! Declare a new class with by-reference semantics and that supports reference counting. 
//! baseName is the id of the base for this class
#define PYAPI_REF_CLASS(className, baseName) class_<className, bases<baseName>, boost::noncopyable, omega::Ref<className> >(#className, no_init)
//! Declare a new class with by-reference semantics and that supports reference counting. 
//! The class has no base for the python API. Its C++ implementation should derive from ReferenceType.
#define PYAPI_REF_BASE_CLASS(className) class_<className, boost::noncopyable, omega::Ref<className> >(#className, no_init)
//! Declare a new class with by-reference semantics and that supports reference counting. The class has an empty constuctor.
#define PYAPI_REF_BASE_CLASS_WITH_CTOR(className) class_<className, boost::noncopyable, omega::Ref<className> >(#className)
//! Declare a new class with by-reference semantics and that supports reference counting. The class has an empty constuctor.
#define PYAPI_REF_CLASS_WITH_CTOR(className, baseName) class_<className, bases<baseName>, boost::noncopyable, omega::Ref<className> >(#className)

//! Declare a method. Can be used for methods returning void, or returning simple plain types like int, float, bool etc.
#define PYAPI_METHOD(className, methodName) .def(#methodName, &className::methodName)
//! Declare a method returning an object by value
#define PYAPI_GETTER(className, methodName) .def(#methodName, &className::methodName, return_value_policy<return_by_value>())
//! Declare a method returning an object by reference
#define PYAPI_REF_GETTER(className, methodName) .def(#methodName, &className::methodName, return_value_policy<return_by_smart_ptr>())
//! Declare a static method. Can be used for static methods returning void, or returning simple plain types like int, float, bool etc.
#define PYAPI_STATIC_METHOD(className, methodName) .def(#methodName, &className::methodName).staticmethod(#methodName)
//! Declare a static method returning an object by reference
#define PYAPI_STATIC_REF_GETTER(className, methodName) .def(#methodName, &className::methodName, return_value_policy<return_by_smart_ptr>()).staticmethod(#methodName)
#define PYAPI_PROPERTY(className, propName) .def_readwrite(#propName, &className::propName)
#define PYAPI_REF_PROPERTY(className, propName) .add_property(#propName, make_getter(&className::propName, PYAPI_RETURN_REF), make_setter(&className::propName, PYAPI_RETURN_REF))
#define PYAPI_VALUE_PROPERTY(className, propName) .add_property(#propName, make_getter(&className::propName, PYAPI_RETURN_VALUE), make_setter(&className::propName, PYAPI_RETURN_VALUE))

bool OMEGA_API isRefPtrForwardingEnabled();
void OMEGA_API disableRefPtrForwarding();

///////////////////////////////////////////////////////////////////////////////////////////////////
// SMART POINTER WRAPPING CODE FROM http://isolation-nation.blogspot.com/2008/09/returnbysmartptr-policy-for-boost.html
// attempting to instantiate this type will result in a compiler error,
// if that happens it means you're trying to use return_by_smart_pointer
// on a function/method that doesn't return a pointer!
namespace boost {
namespace python {
namespace detail {
    template <class R>
    struct return_by_smart_ptr_requires_a_pointer_return_type
    # if defined(__GNUC__) && __GNUC__ >= 3 || defined(__EDG__)
        {}
    # endif
        ;
    // this is where all the work is done, first the plain pointer is
    // converted to a smart pointer, and then the smart pointer is embedded
    // in a Python object
    struct make_owning_smart_ptr_holder
    {
        template <typename T>
        static PyObject* execute(T* p)
        {
            typedef omega::Ref<T> smart_pointer;
            typedef objects::pointer_holder<smart_pointer, T> holder_t;

            smart_pointer ptr(const_cast<T*>(p));

            // If reference pointer forwarding is enabled, the reference count for the object has been incremented
            // by 1, to avoid object deletion during pointer forwarding to python. Not that the object is safe inside a 
            // new smart pointer, decrease the refcount.
            if(isRefPtrForwardingEnabled()) 
            {
                ptr->unref();
                disableRefPtrForwarding();
            }
            return objects::make_ptr_instance<T, holder_t>::execute(ptr);
        }
    };
} // namespace detail

struct return_by_smart_ptr
{
    template <typename T>
    struct apply
    {
        typedef typename boost::mpl::if_c<
            boost::is_pointer<T>::value,
            to_python_indirect<T, detail::make_owning_smart_ptr_holder>,
            detail::return_by_smart_ptr_requires_a_pointer_return_type<T>
        >::type type;
    };
};

}} // namespace boost::python
#endif

