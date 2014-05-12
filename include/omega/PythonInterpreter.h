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
#ifndef __PYTHON_INTERPRETER_H__
#define __PYTHON_INTERPRETER_H__

#include "omega/osystem.h"
#include "omega/ApplicationBase.h"
#include "omega/IRendererCommand.h"
#include "omega/SharedDataServices.h"
#include "omega/Camera.h"

struct PyMethodDef;
class PythonInteractiveThread;

namespace omega
{
	///////////////////////////////////////////////////////////////////////////
	//struct CommandHelpEntry
	//{
	//	String syntax;
	//	String info;
	//};

	///////////////////////////////////////////////////////////////////////////
	class OMEGA_API PythonInterpreter: public SharedObject
	{
		friend struct PythonInterpreterWrapper;
	public:
		enum CallbackType
		{
			CallbackUpdate, CallbackEvent, CallbackDraw
		};

		//! The flags that can be applied to the runFile method
		enum RunFlags
		{
			NoRunFlags = 0,
			//! Set the current working directory to the script path
			SetCwdToScriptPath = 1 << 1,
			//! Adds the script path to the search path for python modules (loaded using the import command)
			AddScriptPathToModuleSearchPath = 1 << 2,
			DefaultRunFlags = SetCwdToScriptPath
		};

		//! @internal returns the last event received by the interpreter. Used for script interoperability
		static const Event* getLastEvent() { return mysLastEvent; }

	public:
		PythonInterpreter();
		virtual ~PythonInterpreter();

		void setup(const Setting& setting);
		void initialize(const char* programName);
		void addModule(const char* name, PyMethodDef* methods);
		void addModule(const char* name, PyMethodDef* methods, const Dictionary<String, int> intConstants, const Dictionary<String, String> stringConstants);

		//! Immediately executes a script statement on the local node.
		void eval(const String& script, const char* format = NULL, ...);
		//! Execute a script file.
		//! The script path accepts two macros:
		//! OMEGA_DATA_ROOT will be substituted with the default data directory for the omegalib installation
		//! OMEGA_APP_ROOT will be substituted with the application directory for the installation (set throguh CMake at build configuration time)
		void runFile(const String& filename, uint flags = DefaultRunFlags);
		//! Cleans the application state.
		void clean();
		//! Cleans the application state and runs a new script.
		void cleanRun(const String& filename);

		//! Executes an event command statement.
		//! @remarks Event command statements are commonly used as event handlers. 
		//! The event passed to this call can be accessed from the script side using the
		//! getEvent() global function
		void evalEventCommand(const String& command, const Event& evt);

		//! Queues a command for execution. If the local flag is set, the command will be executed only on
		//! the local node.
		void queueCommand(const String& command, bool local = false);

		void registerCallback(void* callback, CallbackType type);
		void unregisterAllCallbacks();

		void addPythonPath(const char*);

		bool isEnabled();
		bool isShellEnabled() { return myShellEnabled; }

		// invoke python callbacks.
		void update(const UpdateContext& context);
		void handleEvent(const Event& evt);
		void draw(const DrawContext& context, Camera* cam);

		// Shared data
		virtual void commitSharedData(SharedOStream& out);
		virtual void updateSharedData(SharedIStream& in);

		//String getHelpString(const String& filter);
        void lockInterpreter();
        void unlockInterpreter();

    protected:
		struct QueuedCommand
		{
			QueuedCommand(const String& cmd, bool bneedsExecute, bool bneedsSend):
				command(cmd), needsExecute(bneedsExecute), needsSend(bneedsSend)
			{}
			String command;
			bool needsExecute;
			bool needsSend;
		};

	protected:
		bool myEnabled;
		bool myShellEnabled;
		bool myDebugShell;
		// Command read from a configuration file and executed during 
		// initialization. Helpful to load or setup optional modules.
		String myInitCommand;
        String myInitScript;

        PythonInteractiveThread* myInteractiveThread;

		Lock myInteractiveCommandLock;
		List<QueuedCommand*> myCommandQueue;

		List<void*> myUpdateCallbacks;
		List<void*> myEventCallbacks;
		List<void*> myDrawCallbacks;

		//char* myExecutablePath;

		//List<CommandHelpEntry*> myHelpData;

		Lock myLock;
		
		// Stats
		Ref<Stat> myUpdateTimeStat;


	private:
		static const Event* mysLastEvent;
	};

	///////////////////////////////////////////////////////////////////////////
	//! Implements a renderer command that runs a python statement when executed.
	//class ScriptRendererCommand: public IRendererCommand
	//{
	//public:
	//	ScriptRendererCommand()
	//	{
	//		myInterp = SystemManager::instance()->getScriptInterpreter();
	//		myStatement = "";
	//	}

	//	void setStatement(const String& value) { myStatement = value; }

	//	void execute(Renderer* r)
	//	{
	//		myInterp->eval(myStatement);
	//	}

	//private:
	//	String myStatement;
	//	PythonInterpreter* myInterp;
	//};
};
#endif

