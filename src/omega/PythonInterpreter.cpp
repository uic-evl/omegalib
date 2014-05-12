/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory,  
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
 *	The omegalib python interpreter core (excluding the wrapping code to the 
 *	omegalib API)
 ******************************************************************************/
#include "omega/PythonInterpreter.h"
#include "omega/SystemManager.h"
#include "omega/ModuleServices.h"
#include "omega/SystemManager.h"
#include "omega/DisplaySystem.h"

using namespace omega;

const Event* PythonInterpreter::mysLastEvent = NULL;

#ifdef OMEGA_USE_PYTHON

#ifdef OMEGA_READLINE_FOUND
	#include<readline/readline.h>
	#include<readline/history.h>
#endif
#include <signal.h>  // for signal
#include "omega/PythonInterpreterWrapper.h"

#include<iostream>

//struct vtkPythonMessage
//{
//  vtkStdString Message;
//  bool IsError;
//};

void omegaPythonApiInit();

//PyThreadState* sMainThreadState;

///////////////////////////////////////////////////////////////////////////////
class PythonInteractiveThread: public Thread
{
public:
	virtual void threadProc()
	{
		PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();

		while(!SystemManager::instance()->isExitRequested())	
		{
			osleep(100);
			String line;
#ifdef OMEGA_READLINE_FOUND
			String prompt = ostr("%1%>>", %SystemManager::instance()->getApplication()->getName());
			char *inp_c = readline(prompt.c_str()); //Instead of getline()
			
			// THE COMMAND OF DEATH
			if(inp_c[0] == 'D' &&
				inp_c[1] == 'I' &&
				inp_c[2] == 'E') exit(-1);
			
			line = (const char *)(inp_c); //Because C strings stink
			if(strlen(inp_c) != 0)
			{
				add_history(inp_c);
			}
			free(inp_c);
#else
			getline(std::cin, line);
#endif
			
			//ofmsg("line read: %1%", %line);

			interp->queueCommand(line);
		}
		omsg("Ending console interactive thread");
	}
};

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::lockInterpreter()
{
	if(myDebugShell) omsg("PythonInterpreter::lockInterpreter()");
	myLock.lock();
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::unlockInterpreter()
{
	if(myDebugShell) omsg("PythonInterpreter::unlockInterpreter()");
	myLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
bool PythonInterpreter::isEnabled()
{
	return true;
}

///////////////////////////////////////////////////////////////////////////////
PythonInterpreter::PythonInterpreter()
{
	myShellEnabled = false;
	myDebugShell = false;
	myInteractiveThread = new PythonInteractiveThread();
}

///////////////////////////////////////////////////////////////////////////////
PythonInterpreter::~PythonInterpreter()
{
	omsg("~PythonInterpreter");
	//myInteractiveThread->stop();
	delete myInteractiveThread;
	myInteractiveThread = NULL;

	Py_Finalize();
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::addPythonPath(const char* dir)
{
	ofmsg("PythonInterpreter::addPythonPath: %1%", %dir);
	
	// Convert slashes for this platform.
	String out_dir = dir ? dir : "";
#ifdef OMEGA_OS_WIN
	out_dir = StringUtils::replaceAll(out_dir, "/", "\\");
#endif

	// Append the path to the python sys.path object.
	PyObject* opath = PySys_GetObject("path");
	PyObject* newpath = PyString_FromString(out_dir.c_str());
	PyList_Insert(opath, 0, newpath);
	Py_DECREF(newpath);
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::setup(const Setting& setting)
{
	myShellEnabled = Config::getBoolValue("pythonShellEnabled", setting, myShellEnabled);
	myDebugShell = Config::getBoolValue("pythonShellDebug", setting, myDebugShell);

	// Command read from a configuration file and executed during 
	// initialization. Helpful to load or setup optional modules.
	myInitCommand = Config::getStringValue("initCommand", setting, myInitCommand);
    myInitScript = Config::getStringValue("initScript", setting, myInitScript);
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::initialize(const char* programName)
{
	// Register self as shared object
	SharedDataServices::registerObject(this, "interp");

	// Set the program name, so that we can ask python to provide us
	// full path.
	Py_SetProgramName((char*)programName);

	// Use the datamanager to lookup for a local copy of the python library:
	String pythonModulePath;
	if(DataManager::findFile("python/Lib/site.py", pythonModulePath))
	{
		// Remove the final part of the path to get the python library root.
		String pythonLibPath = StringUtils::replaceAll(pythonModulePath, "/Lib/site.py", "");
		ofmsg("PythonInterpreter::initialize: found local python library in %1%", %pythonLibPath);

		static char pythonHome[1024];
		strcpy(pythonHome, pythonLibPath.c_str());
		Py_SetPythonHome(pythonHome);
	}
	else
	{
		owarn("WARNING: could not find a local omegalib python installation. \n"
			  "Omegalib will attempt to use the system python interpreter (if available)");
	}

	// initialize the statically linked modules
	//CMakeLoadAllPythonModules();

	// Initialize interpreter.
	Py_Initialize();

	// HACK: Calling PyRun_SimpleString for the first time for some reason results in
	// a "\n" message being generated which is causing the error dialog to
	// popup. So we flush that message out of the system before setting up the
	// callbacks.
	// The cast is necessary because PyRun_SimpleString() hasn't always been
	// const-correct.
	PyRun_SimpleString(const_cast<char*>(""));
	PythonInterpreterWrapper* wrapperOut = vtkWrapInterpretor(this);
	wrapperOut->DumpToError = false;

	PythonInterpreterWrapper* wrapperErr = vtkWrapInterpretor(this);
	wrapperErr->DumpToError = false;

	// Redirect Python's stdout and stderr and stdin
	PySys_SetObject(const_cast<char*>("stdout"), reinterpret_cast<PyObject*>(wrapperOut));
	PySys_SetObject(const_cast<char*>("stderr"), reinterpret_cast<PyObject*>(wrapperErr));
	//PySys_SetObject(const_cast<char*>("stdin"), reinterpret_cast<PyObject*>(wrapperErr));

	Py_DECREF(wrapperOut);
	Py_DECREF(wrapperErr);
	
	// Add a generic 'current working dir' to the module search paths, so modules
	// in the current working directory will always load, regardless of where the
	// current working dir is.
	// NOTE: This is needed because when we run a script we switch to it's containing
	// directory (so local files can be opened using their relative path). If we
	// dont't add this to the module search paths we will not be able to import local
	// modules.
	addPythonPath("./");
	
	// Add the launching executable's directory to the module search path.  This will
	// allow omegalib to find binary modules that are part of the distribution
	// regardless of the directory from which the application has been launched. 
	// Useful for system installs of omegalib, so users do not to be inside the bin
	// folder to be able to launch and improt modules correclty.
	String exePath = ogetexecpath();
	String exeDir;
	String exeName;
	StringUtils::splitFilename(exePath, exeName, exeDir);
	addPythonPath(exeDir.c_str());

	// The data prexif string is set by omain to point to the data directory 
	// used by this omegalib instance.
	String modulePath = ogetdataprefix() + "/modules";
	addPythonPath(modulePath.c_str());

	if(myShellEnabled && SystemManager::instance()->isMaster())
	{
		omsg("PythonInterpreter: starting interactive shell thread.");
		myInteractiveThread->start();
	}

	// Initialize internal Apis
	omegaPythonApiInit();

	// Run initialization commands
	PyRun_SimpleString("from omega import *");
	PyRun_SimpleString("from euclid import *");

    if(myInitScript != "") runFile(myInitScript, 0);
	if(myInitCommand != "") PyRun_SimpleString(myInitCommand.c_str());
	
	// Setup stats
	StatsManager* sm = SystemManager::instance()->getStatsManager();
	myUpdateTimeStat = sm->createStat("Script update", StatsManager::Time);
	omsg("Python Interpreter initialized.");
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::addModule(const char* name, PyMethodDef* methods)
{
	Dictionary<String, int> intConstants;
	Dictionary<String, String> stringConstants;
	addModule(name, methods, intConstants, stringConstants);
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::addModule(
	const char* name, PyMethodDef* methods, 
	const Dictionary<String, int> intConstants, 
	const Dictionary<String, String> stringConstants)
{
	PyObject* module = Py_InitModule(name, methods);

	typedef Dictionary<String, int>::Item IntConstantItem;
	foreach(IntConstantItem item, intConstants)
	{
		PyModule_AddIntConstant(module, item.getKey().c_str(), item.getValue());
	}

	typedef Dictionary<String, String>::Item StringConstantItem;
	foreach(StringConstantItem item, stringConstants)
	{
		PyModule_AddStringConstant(module, item.getKey().c_str(), item.getValue().c_str());
	}

	PyMethodDef* cur = methods;
	//while(cur->ml_name != NULL)
	//{
	//	CommandHelpEntry* help = new CommandHelpEntry();
	//	String syntax = StringUtils::split(cur->ml_doc, "\n")[0];
	//	help->syntax = syntax;
	//	help->info = cur->ml_doc;

	//	cur++;
	//	myHelpData.push_back(help);
	//}

#ifdef OMEGA_READLINE_FOUND
	cur = methods;
	while(cur->ml_name != NULL)
	{
		add_history(cur->ml_name);
		cur++;
	}
#endif
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::eval(const String& cscript, const char* format, ...)
{
	String script = cscript;
	StringUtils::trim(script);
	char* str = const_cast<char*>(script.c_str());
	if(format == NULL)
	{
		bool handled = false;
		// Handle special 'shortcut' commands. Commands starting with a ':' 
		// won't be interpreted using the python interpreter. Instead, we will 
		// dispatch them to EngineModule::handleCommand 
		// methods.
		if(script.length() > 0 && script[0] == ':')
		{
			// Remove colon.
			String sscript = script.substr(1, script.length() - 1);
			handled = ModuleServices::handleCommand(sscript);

			// Enable / disable debug mode.
			if(sscript == "debug on") myDebugShell = true;
			else if(sscript == "debug off") myDebugShell = false;
		}
		else		
		{
			if(myDebugShell) ofmsg("PythonInterpreter::eval() >>>> %1%", %str);
			lockInterpreter();
			PyRun_SimpleString(str);
			unlockInterpreter();
		}
	}
	else
	{
		lockInterpreter();
		PyObject * module = PyImport_AddModule("__main__");
		PyObject* dict = PyModule_GetDict(module);
		PyObject* result = PyRun_String(str, Py_eval_input, dict, dict);
	
		if(format != NULL && result != NULL)
		{
			va_list args;
			va_start(args, format);

			const char* fmt = const_cast<char*>(format);
			if(!PyArg_Parse(result, format, va_arg(args, void*)))
			{
				ofwarn("PythonInterpreter: result of statement '%1%' cannot be parsed by format string '%2%'", %str %fmt);
			}

			va_end(args);
		}
		unlockInterpreter();
	}
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::evalEventCommand(const String& command, const Event& evt) 
{
	//! Save the last 'current' event to a local variable
	const Event* tempEvt = mysLastEvent;
	mysLastEvent = &evt;

	eval(command);

	//! Reset the current event to the saved one.
	mysLastEvent = tempEvt;
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::runFile(const String& filename, uint flags)
{
	ofmsg("PythonInterpreter::runFile: running %1%", %filename);
	// Substitute the OMEGA_DATA_ROOT and OMEGA_APP_ROOT macros in the path.
	String path = filename;
	
	String fullPath;
	if(DataManager::findFile(path, fullPath))
	{
		SystemManager* sys = SystemManager::instance();
		PythonInterpreter* interp = sys->getScriptInterpreter();

		String scriptPath;
		String baseScriptFilename;
		StringUtils::splitFilename(fullPath, baseScriptFilename, scriptPath);

		// NOTE: we need to read the file before (possibly) resetting the current
		// working dir, otherwise we will not be able to find the file.
		PyObject* PyFileObject = PyFile_FromString((char*)fullPath.c_str(), "r");
		if(PyFileObject == NULL)
		{
			ofwarn("PythonInterpreter:runFile: failed to open script file %1%", %fullPath);
		}

		if(flags & SetCwdToScriptPath)
		{
			DataManager* dm = SystemManager::instance()->getDataManager();
			dm->setCurrentPath(scriptPath);

			// change the current working directory to be the script root, so 
			// we can load files using the open command.
			// NOTE: we use a straight PyRun_SimpleString instead of calling
			// the eval method because we may be inside an eval call already
			// (for instance, evaluating an orun command).
			// Since the interpreter locks in eval, we would get into a 
			// deadlock.
			String cdcmd = "import os; os.chdir('" + scriptPath + "')";
			PyRun_SimpleString(cdcmd.c_str());
		}

		if(flags & AddScriptPathToModuleSearchPath)
		{
			addPythonPath(scriptPath.c_str());
		}

		PyRun_SimpleFile(PyFile_AsFile(PyFileObject), filename.c_str());
	}
	else
	{
		ofwarn("PythonInterpreter: script not found: %1%", %filename);
	}
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::clean()
{
	Engine::instance()->reset();

	// destroy all global variables
	if(myDebugShell)
	{
		// Use this line instead of the previous to get debugging info on 
		// variable deletion. Useful in  case of crashes to know which variable
		// is currently being deleted.
		eval("for uniquevar in [var for var in globals().copy() if var[0] != \"_\" and var != 'clearall']: print(\"deleting \" + uniquevar); del globals()[uniquevar]");
	}
	else
	{
		eval("for uniquevar in [var for var in globals().copy() if var[0] != \"_\" and var != 'clearall']: del globals()[uniquevar]");
	}

	// Import omega and euclid modules by default
	eval("from omega import *");
	eval("from euclid import *");

	// unregister callbacks
	unregisterAllCallbacks();

	// Clear all queued commands.
	//myInteractiveCommandLock.lock();
	//myCommandQueue.clear();
	//myInteractiveCommandLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::cleanRun(const String& filename)
{
	clean();
	// NOTE: Instead of running the script immediately through 
	// PythonInterpreter::runFile, we queue a local orun command. We do this to 
	// give the system a chance to finish reset, if this script is loading 
	// through a :r! command. Also note how we explicitly import module omega, 
	// since all global symbols have been unloaded by the previously mentioned 
	// reset command.
	queueCommand(ostr("from omega import *; from euclid import *; orun(\"%1%\")", %filename), true);
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::registerCallback(void* callback, CallbackType type)
{
	// BLAGH cast
	PyObject* pyCallback =(PyObject*)callback;
	if(callback != NULL)
	{
		Py_INCREF(pyCallback);
		switch(type)
		{
		case CallbackUpdate:
			myUpdateCallbacks.push_back(callback);
			return;
		case CallbackEvent:
			myEventCallbacks.push_back(callback);
			return;
		case CallbackDraw:
			myDrawCallbacks.push_back(callback);
			return;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::unregisterAllCallbacks()
{
	myUpdateCallbacks.clear();
	myEventCallbacks.clear();
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::update(const UpdateContext& context) 
{
	myUpdateTimeStat->startTiming();
	// Execute queued interactive commands first
	if(myCommandQueue.size() != 0)
	{
		// List of commands to be removed from queue
		List<QueuedCommand*> cmdsToRemove;
		foreach(QueuedCommand* qc, myCommandQueue)
		{
			if(qc->needsExecute) 
			{
				if(myDebugShell)
				{
					ofmsg("running %1%", %qc->command);
				}
				// Execute the command
				eval(qc->command);
				qc->needsExecute = false;
			}
			// Purge commands from list
			if(!qc->needsExecute && !qc->needsSend) cmdsToRemove.push_back(qc);
		}
		myInteractiveCommandLock.lock();
		foreach(QueuedCommand* qc, cmdsToRemove)
		{
			myCommandQueue.remove(qc);
			delete qc;
		}
		myInteractiveCommandLock.unlock();
	}
	
	PyObject *arglist;
	arglist = Py_BuildValue("(lff)", (long int)context.frameNum, context.time, context.dt);

	foreach(void* cb, myUpdateCallbacks)
	{
		// BLAGH cast
		PyObject* pyCallback =(PyObject*)cb;
		PyObject_CallObject(pyCallback, arglist);
	}

	Py_DECREF(arglist);
	myUpdateTimeStat->stopTiming();
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::queueCommand(const String& command, bool local)
{
	//oassert(!myInteractiveCommandNeedsExecute && 
	//	!myInteractiveCommandNeedsSend);
	
	myInteractiveCommandLock.lock();
	myCommandQueue.push_back(new QueuedCommand(command, true, !local));
	myInteractiveCommandLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::commitSharedData(SharedOStream& out)
{
	// Count number of commands that need sending
	int i = 0;
	foreach(const QueuedCommand* qc, myCommandQueue) if(qc->needsSend) i++;

	// Send commands
	out << i;
	foreach(QueuedCommand* qc, myCommandQueue) 
	{
		if(qc->needsSend)
		{
			out << qc->command;
			qc->needsSend = false;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::updateSharedData(SharedIStream& in)
{
	int cmdCount;
	
	in >> cmdCount;
	for(int i = 0; i < cmdCount; i++)
	{
		String cmd;
		in >> cmd;
		// Add command to the local command queue.
		queueCommand(cmd, true);
	}
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::handleEvent(const Event& evt) 
{
	// Save the received event to the static variable mysLastEvent.
	// Script code will be able to retrieve it using getEvent()
	mysLastEvent = &evt;

	foreach(void* cb, myEventCallbacks)
	{
		// BLAGH cast
		PyObject* pyCallback =(PyObject*)cb;
		PyObject_CallObject(pyCallback, NULL);
	}

	// We can't guarantee the event will live outside of this call tree, so 
	// clean up the static variable. getEvent() will return None when called 
	// outside the event callback.
	mysLastEvent = NULL;
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::draw(const DrawContext& context, Camera* cam)
{
	if(myDrawCallbacks.size() > 0)
	{
		PyObject *arglist;
		Vector2i displayRez = SystemManager::instance()->getDisplaySystem()->getCanvasSize();
		int width = displayRez[0];
		int height = displayRez[1];
		int tileWidth = context.tile->pixelSize[0];
		int tileHeight = context.tile->pixelSize[1];

		DrawInterface* di = context.renderer->getRenderer();

		lockInterpreter();

		boost::python::object ocam(boost::python::ptr(cam));
		boost::python::object odi(boost::python::ptr(di));

		arglist = Py_BuildValue("((ii)(ii)OO)", width, height, tileWidth, tileHeight, ocam.ptr(), odi.ptr());
		foreach(void* cb, myDrawCallbacks)
		{
			// BLAGH cast
			PyObject* pyCallback =(PyObject*)cb;
			PyObject_CallObject(pyCallback, arglist);

		}
		Py_DECREF(arglist);
		unlockInterpreter();
	}
}

///////////////////////////////////////////////////////////////////////////////
//String PythonInterpreter::getHelpString(const String& filter)
//{
//	String result = "";
//	foreach(CommandHelpEntry* item, myHelpData)
//	{
//		if(filter == "" || StringUtils::startsWith(item->syntax, filter))
//		{
//			result.append(item->syntax);
//			result.append("|");
//			result.append(item->info);
//			result.append("|");
//		}
//	}
//	return result;
//}

#else

///////////////////////////////////////////////////////////////////////////////
bool PythonInterpreter::isEnabled() { return false; }

///////////////////////////////////////////////////////////////////////////////
PythonInterpreter::PythonInterpreter() 
{ 	
	myShellEnabled = false;
}

///////////////////////////////////////////////////////////////////////////////
PythonInterpreter::~PythonInterpreter() { }

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::setup(const Setting& setting) { }

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::addPythonPath(const char* dir) { }

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::initialize(const char* programName) { }

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::addModule(const char* name, PyMethodDef* methods) { }

void PythonInterpreter::addModule(const char* name, PyMethodDef* methods, const Dictionary<String, int> intConstants, const Dictionary<String, String> stringConstants) {}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::eval(const String& script, const char* format, ...) 
{ 
	ofwarn("PythonInterpreter::eval: Python interpreter not available on this system. (%1%)", %script);
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::runFile(const String& filename, uint flags) 
{ 
	ofwarn("PythonInterpreter::runFile: Python interpreter not available on this system. (%1%)", %filename);
}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::registerCallback(void* callback, CallbackType type) { }

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::update(const UpdateContext& context) { }

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::handleEvent(const Event& evt) { }

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::commitSharedData(SharedOStream& out) {}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::updateSharedData(SharedIStream& in) {}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::queueCommand(const String& command, bool local) {}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::unregisterAllCallbacks() {}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::draw(const DrawContext& context, Camera* cam) {}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::evalEventCommand(const String& command, const Event& evt) {}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::clean() {}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::cleanRun(const String& filename) {}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::lockInterpreter() {}

///////////////////////////////////////////////////////////////////////////////
void PythonInterpreter::unlockInterpreter() {}
#endif
