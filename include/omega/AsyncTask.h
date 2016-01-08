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
*	Asynchronous task objects and worker pools
******************************************************************************/
#ifndef __ASYNC_TASK__
#define __ASYNC_TASK__

#include "osystem.h"
#include "omega/SystemManager.h"
#include "omega/PythonInterpreter.h"

namespace omega {
    ///////////////////////////////////////////////////////////////////////////
    template<typename T>
    class AsyncTask: public ReferenceType
    {
    public:
        typedef T Data;

        ///////////////////////////////////////////////////////////////////////
        class IAsyncTaskHandler
        {
        public:
            virtual void onTaskCompleted(AsyncTask<T>* task) = 0;
        };

    public:
        AsyncTask(double timestamp = 0): myProgress(0), myComplete(false), myHandler(NULL) 
        {
            if(timestamp > 0) myTimestamp = timestamp;
            else myTimestamp = otimestamp();
        }

        T& getData() { return myData; }
        void setData(const T& data) { myData = data; }

        bool isComplete() { return myComplete; }
        int getProgress() { return myProgress; }
        void setProgress(int value) { myProgress = value; }
        double getTimestamp() { return myTimestamp; }

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

        void setTaskId(const String& value) { myTaskId = value; }
        const String& getTaskId() { return myTaskId; }

        bool hasFailed() { return myFailed; }
        const String& getCompletionMessage() { return myCompletionMessage; }

    private:
        T myData;
        String myTaskId;
        bool myComplete;
        int myProgress;
        bool myFailed;
        double myTimestamp;
        String myCompletionMessage;
        String myCompletionCommand;
        IAsyncTaskHandler* myHandler;
    };

    ///////////////////////////////////////////////////////////////////////////
    class WorkerPool;
    class WorkerTask: public ReferenceType
    {
    public:
        typedef AsyncTask< Ref<WorkerTask> > TaskInfo;
        friend class WorkerPool;
    public:
        virtual void execute(TaskInfo* ti) = 0;
        WorkerPool* getPool() { return myPool; }
    private:
        WorkerPool* myPool;
    };

    ///////////////////////////////////////////////////////////////////////////
    class WorkerPool
    {
    public:
        WorkerPool() :
            myShutdown(false)
        {}

        void start(int numThreads)
        {
            for(int i = 0; i < 4; i++)
            {
                WorkerThread* t = new WorkerThread();
                t->pool = this;
                t->start();
                myPool.push_back(t);
            }
        }

        void stop()
        {
            myShutdown = true;
            foreach(Thread* t, myPool) t->stop();
        }

        void queue(WorkerTask* task)
        {
            WorkerTask::TaskInfo* t = new WorkerTask::TaskInfo();
            task->myPool = this;
            t->setData(task);
            myLock.lock();
            myQueue.push(t);
            myLock.unlock();
        }

        void clearQueue()
        {
            myLock.lock();
            while(!myQueue.empty()) myQueue.pop();
            myLock.unlock();
        }

    private:
        class WorkerThread : public Thread
        {
        public:
            WorkerPool* pool;

            virtual void threadProc()
            {
                while(!pool->myShutdown)
                {
                    if(pool->myQueue.size() > 0)
                    {
                        pool->myLock.lock();

                        if(pool->myQueue.size() > 0)
                        {
                            Ref<WorkerTask::TaskInfo> task = pool->myQueue.front();
                            pool->myQueue.pop();
                            pool->myLock.unlock();
                            if(!pool->myShutdown)
                            {
                                task->getData()->execute(task);
                            }
                        }
                        else
                        {
                            pool->myLock.unlock();
                        }
                    }
                    osleep(100);
                }
            }
        };
    private:
        Lock myLock;
        Queue< Ref<WorkerTask::TaskInfo> > myQueue;
        bool myShutdown;
        List<Thread*> myPool;
    };
}; // namespace omega

#endif
