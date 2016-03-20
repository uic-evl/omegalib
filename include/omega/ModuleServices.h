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
 *	Engine module spport. Engine modules are classes that can be attached to 
 *	the engine and receive update, event and command calls.
 ******************************************************************************/
#ifndef __MODULE_SERVICES_H__
#define __MODULE_SERVICES_H__

#include "osystem.h"
#include "Engine.h"
#include "omega/SharedDataServices.h"

namespace omega {
    class RenderPass;
    class Renderer;
    
    ///////////////////////////////////////////////////////////////////////////
    //! Base class for engine modules
    //! Engine modules are classes that can be attached to the engine and receive
    //! update, event and command calls.
    class OMEGA_API EngineModule: public SharedObject, public IEventListener
    {
    friend class ModuleServices;
    public:
        enum Priority 
        { 
			PriorityLowest = 0, 
			PriorityLow = 1, 
			PriorityNormal = 2, 
			PriorityHigh = 3, 
			PriorityHighest = 4 
        };

		enum OpenGLProfile
		{
		   CoreProfile,
		   CompatibilityProfile,
		   UnspecifiedProfile
		};
		
    public:
        EngineModule(const String& name): 
          myInitialized(false), myEngine(NULL), myName(name), 
              myPriority(PriorityNormal), mySharedDataEnabled(false),
              myEventTimeStat(NULL),  myUpdateTimeStat(NULL) 
          {
          }

        EngineModule(): 
          myInitialized(false), myEngine(NULL), myName(mysNameGenerator.generate()), 
              myPriority(PriorityNormal), mySharedDataEnabled(false),
              myEventTimeStat(NULL),  myUpdateTimeStat(NULL) 
          {
          }

        virtual ~EngineModule();

		//! This method can be used within an engine module ctor or initialization to 
		//! test for module compatibility. After initialization, the module will test 
		//! requested profile against what the display system is running.
		void requestOpenGLProfile(OpenGLProfile profile);
		
        void enableSharedData();
        void disableSharedData();

        virtual void initialize() {}
        virtual void dispose() {}
        virtual void update(const UpdateContext& context) {}
        virtual void handleEvent(const Event& evt) {}
        virtual bool handleCommand(const String& cmd) { return false; }
        virtual void commitSharedData(SharedOStream& out) {}
        virtual void updateSharedData(SharedIStream& in) {}

        virtual void initializeRenderer(Renderer*) {}

        void doInitialize(Engine* server);
        void doDispose();

        virtual bool isInitialized() { return myInitialized; }

        Engine* getEngine() { return myEngine; }

        Priority getPriority() { return myPriority; }
        void setPriority(Priority value) { myPriority = value; }
        
        const String& getName() { return myName; }

    private:
        Engine* myEngine;

        String myName;
        Priority myPriority;
        bool myInitialized;
        bool mySharedDataEnabled;

        static NameGenerator mysNameGenerator;

        // Statistics
        Ref<Stat> myEventTimeStat;
        Ref<Stat> myUpdateTimeStat;
    };

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API ModuleServices
    {
    public:
        static void addModule(EngineModule* module);
        static void removeModule(EngineModule* module);
        static void update(Engine* srv, const UpdateContext& context);
        //! Dispatches an event to all modules with the specified priority.
        static void handleEvent(const Event& evt, EngineModule::Priority p);
        static bool handleCommand(const String& cmd);
        //static void initializeRenderer(Engine* srv, Renderer* r);
        static void disposeAll();
        static void disposeNonCoreModules();
        static void setNonCoreMode() { mysCoreMode = false; }
        static void setCoreMode() { mysCoreMode = true; }
        static bool isCoreMode() { return mysCoreMode; }
        
        static Vector<EngineModule*> getModules();

    private:
        static List< Ref<EngineModule> > mysModules;
        static List< Ref<EngineModule> > mysModulesToRemove;
        static List< EngineModule* > mysNonCoreModules;
        static bool mysCoreMode;
        static Timer mysTimer;
    };
}; // namespace omega

#endif
