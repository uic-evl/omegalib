/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2011		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted 
 * provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions 
 * and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the documentation and/or other 
 * materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *-------------------------------------------------------------------------------------------------
 * Original code Copyright (c) Kitware, Inc.
 * All rights reserved.
 * See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
 *************************************************************************************************/
#ifndef __PythonInterpreterWrapper_h
#define __PythonInterpreterWrapper_h

#ifdef _DEBUG
	#define PYTHON_DEBUG_HACK
	#undef _DEBUG
#endif

#include <Python.h>

#ifdef PYTHON_DEBUG_HACK
	#define _DEBUG
#endif

#include "structmember.h"

#include "osystem.h"

///////////////////////////////////////////////////////////////////////////////////////////////////
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
#define PYAPI_REF_CLASS(className, baseName) class_<className, bases<baseName>, boost::noncopyable, Ref<className> >(#className, no_init)
//! Declare a new class with by-reference semantics and that supports reference counting. 
//! The class has no base for the python API. Its C++ implementation should derive from ReferenceType.
#define PYAPI_REF_BASE_CLASS(className) class_<className, boost::noncopyable, Ref<className> >(#className, no_init)
//! Declare a new class with by-reference semantics and that supports reference counting. The class has an empty constuctor.
#define PYAPI_REF_BASE_CLASS_WITH_CTOR(className) class_<className, boost::noncopyable, Ref<className> >(#className)
//! Declare a new class with by-reference semantics and that supports reference counting. The class has an empty constuctor.
#define PYAPI_REF_CLASS_WITH_CTOR(className, baseName) class_<className, bases<baseName>, boost::noncopyable, Ref<className> >(#className)

//! Declare a method. Can be used for methods returning void, or returning simple plain types like int, float, bool etc.
#define PYAPI_METHOD(className, methodName) .def(#methodName, &className::methodName)
//! Declare a method returning an object by value
#define PYAPI_GETTER(className, methodName) .def(#methodName, &className::methodName, return_value_policy<return_by_value>())
//! Declare a method returning an object by reference
#define PYAPI_REF_GETTER(className, methodName) .def(#methodName, &className::methodName, return_value_policy<return_by_smart_ptr>())
//! Declare a static method returning an object by reference
#define PYAPI_STATIC_REF_GETTER(className, methodName) .def(#methodName, &className::methodName, return_value_policy<return_by_smart_ptr>()).staticmethod(#methodName)
#define PYAPI_PROPERTY(className, propName) .def_readwrite(#propName, &className::propName)

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

namespace omega
{
	///////////////////////////////////////////////////////////////////////////////////////////////
	struct PythonInterpreterWrapper
	{
		PyObject_HEAD
		int softspace;  // Used by print to keep track of its state.
		PythonInterpreter* Interpretor;
		bool DumpToError;

		void Write(const char* string)
		{
			ologaddnewline(false);
			if(!strncmp(string, "Traceback", 9))
			{
				omsg("\n>>>>> PYTHON ERROR");
				owarn(string);
			}
			else
			{
				omsg(string);
			}
			ologaddnewline(true);
		}

		String Read()
		{
			return "";
		}
	};

	static PyObject* vtkWrite(PyObject* self, PyObject* args);
	static PyObject* vtkRead(PyObject* self, PyObject* args);

	// const_cast since older versions of python are not const correct.
	static PyMethodDef PythonInterpreterWrapperMethods[] = {
		{const_cast<char*>("write"), vtkWrite, METH_VARARGS, const_cast<char*>("Dump message")},
		{const_cast<char*>("readline"), vtkRead, METH_VARARGS, const_cast<char*>("Read input line")},
		{0, 0, 0, 0}
	};

	static PyObject* PythonInterpreterWrapperNew(
	  PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
	{
	  return type->tp_alloc(type, 0);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	static PyMemberDef PythonInterpreterWrapperMembers[] = {
	  { const_cast<char*>("softspace"),
		T_INT, offsetof(PythonInterpreterWrapper, softspace), 0,
		const_cast<char *>("Placeholder so print can keep state.") },
	  { 0, 0, 0, 0, 0 }
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	static PyTypeObject PythonInterpreterWrapperType = {
		PyObject_HEAD_INIT(NULL)
		0,                         // ob_size
		const_cast<char*>("PythonInterpreterWrapper"),   // tp_name
		sizeof(PythonInterpreterWrapper), // tp_basicsize
		0,                         // tp_itemsize
		0,                         // tp_dealloc
		0,                         // tp_print
		0,                         // tp_getattr
		0,                         // tp_setattr
		0,                         // tp_compare
		0,                         // tp_repr
		0,                         // tp_as_number
		0,                         // tp_as_sequence
		0,                         // tp_as_mapping
		0,                         // tp_hash 
		0,                         // tp_call
		0,                         // tp_str
		PyObject_GenericGetAttr,   // tp_getattro
		PyObject_GenericSetAttr,   // tp_setattro
		0,                         // tp_as_buffer
		Py_TPFLAGS_DEFAULT | Py_TPFLAGS_BASETYPE, // tp_flags
		const_cast<char*>("PythonInterpreterWrapper"),   //  tp_doc 
		0,                         //  tp_traverse 
		0,                         //  tp_clear 
		0,                         //  tp_richcompare 
		0,                         //  tp_weaklistoffset 
		0,                         //  tp_iter 
		0,                         //  tp_iternext 
		PythonInterpreterWrapperMethods, //  tp_methods 
		PythonInterpreterWrapperMembers, //  tp_members 
		0,                         //  tp_getset 
		0,                         //  tp_base 
		0,                         //  tp_dict 
		0,                         //  tp_descr_get 
		0,                         //  tp_descr_set 
		0,                         //  tp_dictoffset 
		0,                         //  tp_init 
		0,                         //  tp_alloc 
		PythonInterpreterWrapperNew,  //  tp_new
		0, // freefunc tp_free; /* Low-level free-memory routine */
		0, // inquiry tp_is_gc; /* For PyObject_IS_GC */
		0, // PyObject *tp_bases;
		0, // PyObject *tp_mro; /* method resolution order */
		0, // PyObject *tp_cache;
		0, // PyObject *tp_subclasses;
		0, // PyObject *tp_weaklist;
	#if PYTHON_API_VERSION >= 1012
		0  // tp_del
	#endif
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	static PyObject* vtkWrite(PyObject* self, PyObject* args)
	{
	  if(!self || !PyObject_TypeCheck(self, &PythonInterpreterWrapperType))
		{
		return 0;
		}

	  PythonInterpreterWrapper* wrapper = 
		reinterpret_cast<PythonInterpreterWrapper*>(self);
  
	  char *string;
	  // const_cast since older versions of python are not const correct.
	  if (wrapper && PyArg_ParseTuple(args, const_cast<char*>("s"), &string))
	  {
		wrapper->Write(string);
      }

	  return Py_BuildValue(const_cast<char*>(""));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	static PyObject* vtkRead(PyObject* self, PyObject* args)
	{
	  (void)args;
	  if(!self || !PyObject_TypeCheck(self, &PythonInterpreterWrapperType))
		{
		return 0;
		}

	  PythonInterpreterWrapper* wrapper = 
		reinterpret_cast<PythonInterpreterWrapper*>(self);

	  String ret;
	  if (wrapper)
		{
		ret = wrapper->Read();
		}
	  return Py_BuildValue(const_cast<char*>("s"), const_cast<char*>(ret.c_str()));
	}

	///////////////////////////////////////////////////////////////////////////////////////////////
	static PythonInterpreterWrapper* vtkWrapInterpretor(PythonInterpreter* interpretor)
	{
	if(PyType_Ready(&PythonInterpreterWrapperType) < 0)
	{
		return 0;
	}

	PythonInterpreterWrapper* wrapper = 
	PyObject_New(PythonInterpreterWrapper, &PythonInterpreterWrapperType);
	if (wrapper)
	{
		wrapper->Interpretor = interpretor;
	}

	return wrapper;
	}
};
#endif

