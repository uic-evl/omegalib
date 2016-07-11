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
* Wraps a python shell to redirect input-output through omegalib
* Original code Copyright (c) Kitware, Inc.
* All rights reserved.
* See Copyright.txt or http://www.paraview.org/HTML/Copyright.html for details.
******************************************************************************/

#include "omega/PythonInterpreterWrapper.h"
#include "omega/PythonInterpreter.h"
#include "omega/SystemManager.h"
#ifdef OMEGA_USE_PYTHON

using namespace omega;

///////////////////////////////////////////////////////////////////////////
class PythonInterpreterWrapper
{
public:
    PyObject_HEAD
        int softspace;  // Used by print to keep track of its state.
    PythonInterpreter* Interpretor;
    bool DumpToError;

    void Write(const char* string)
    {
        ologaddnewline(false);
        if(!strncmp(string, "Traceback", 9))
        {
            omsg("");
            ofmsg("[%1% :: PYTHON ERROR] ", %SystemManager::instance()->getApplication()->getName());
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
    { const_cast<char*>("write"), vtkWrite, METH_VARARGS, const_cast<char*>("Dump message") },
    { const_cast<char*>("readline"), vtkRead, METH_VARARGS, const_cast<char*>("Read input line") },
    { 0, 0, 0, 0 }
};

static PyObject* PythonInterpreterWrapperNew(
    PyTypeObject* type, PyObject* /*args*/, PyObject* /*kwds*/)
{
    return type->tp_alloc(type, 0);
}

///////////////////////////////////////////////////////////////////////////
static PyMemberDef PythonInterpreterWrapperMembers[] = {
    { const_cast<char*>("softspace"),
    T_INT, offsetof(PythonInterpreterWrapper, softspace), 0,
    const_cast<char *>("Placeholder so print can keep state.") },
    { 0, 0, 0, 0, 0 }
};

///////////////////////////////////////////////////////////////////////////
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

///////////////////////////////////////////////////////////////////////////
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
    if(wrapper && PyArg_ParseTuple(args, const_cast<char*>("s"), &string))
    {
        wrapper->Write(string);
    }

    return Py_BuildValue(const_cast<char*>(""));
}

///////////////////////////////////////////////////////////////////////////
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
    if(wrapper)
    {
        ret = wrapper->Read();
    }
    return Py_BuildValue(const_cast<char*>("s"), const_cast<char*>(ret.c_str()));
}

///////////////////////////////////////////////////////////////////////////
PythonInterpreterWrapper* wrapPythonShell(PythonInterpreter* interpretor, bool dumpToError)
{
    if(PyType_Ready(&PythonInterpreterWrapperType) < 0)
    {
        return 0;
    }

    PythonInterpreterWrapper* wrapper =
        PyObject_New(PythonInterpreterWrapper, &PythonInterpreterWrapperType);
    if(wrapper)
    {
        wrapper->Interpretor = interpretor;
        wrapper->DumpToError = dumpToError;
    }

    return wrapper;
}
#endif