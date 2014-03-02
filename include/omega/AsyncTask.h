/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
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
 *************************************************************************************************/
#ifndef __ASYNC_TASK__
#define __ASYNC_TASK__

#include "osystem.h"
#include "omega/SystemManager.h"
#include "omega/PythonInterpreter.h"

namespace omega {
	///////////////////////////////////////////////////////////////////////////////////////////////
	template<typename T>
	class AsyncTask: public ReferenceType
	{
	public:
		typedef T Data;

		///////////////////////////////////////////////////////////////////////////////////////////////
		class IAsyncTaskHandler
		{
		public:
			virtual void onTaskCompleted(AsyncTask<T>* task) = 0;
		};

	public:
		AsyncTask(): myProgress(0), myComplete(false), myHandler(NULL) {}

		T& getData() { return myData; }
		void setData(const T& data) { myData = data; }

		bool isComplete() { return myComplete; }
		int getProgress() { return myProgress; }
		void setProgress(int value) { myProgress = value; }

		void  notifyComplete(bool failed = false, const String& completionMessage = "")
		{
			myFailed = failed;
			myCompletionMessage = completionMessage;

			myProgress = 100;
			myComplete = true;

			// Call completion handler, if present.
			if(myHandler != NULL) 
			{
				myHandler->onTaskCompleted(this);
			}

			// Run completion script command, if present
			if(myCompletionCommand.size() > 0)
			{
				PythonInterpreter* pi = SystemManager::instance()->getScriptInterpreter();
				// Queue a script command for execution on the main thread.
				pi->queueCommand(myCompletionCommand, true);
			}
		}

		void setCompletionCommand(const String& cmd) { myCompletionCommand = cmd; }
		const String& getCompletionCommand() { return myCompletionCommand; }

		void setCompletionHandler(IAsyncTaskHandler* handler) { myHandler = handler; }
		IAsyncTaskHandler* setCompletionHandler() { return myHandler; }

		void setTaskId(const String& value) {myTaskId = value; }
		const String& getTaskId();

		bool hasFailed() { return myFailed; }
		const String& getCompletionMessage() { return myCompletionMessage; }

	private:
		T myData;
		String myTaskId;
		bool myComplete;
		int myProgress;
		bool myFailed;
		String myCompletionMessage;
		String myCompletionCommand;
		IAsyncTaskHandler* myHandler;
	};
}; // namespace omega

#endif
