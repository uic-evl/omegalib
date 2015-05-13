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
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer. Redistributions in binary 
 * form must reproduce the above copyright notice, this list of conditions and 
 * the following disclaimer in the documentation and/or other materials 
 * provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE  GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * What's in this file
 *	The code for the system manager: the kernel of the omegalib runtime.
 *****************************************************************************/
#ifndef __SYSTEM_MANAGER_H__
#define __SYSTEM_MANAGER_H__

#include "osystem.h"
#include "StatsManager.h"

namespace omega
{
    ///////////////////////////////////////////////////////////////////////////
    // Forward declarations
    class ApplicationBase;
    class DisplaySystem;
    class PythonInterpreter;
    // Forward decl of Mission Control classes to break header circular dependency
    class MissionControlClient;
    class MissionControlServer;

    ///////////////////////////////////////////////////////////////////////////
    //! Contains data about multi-instance mode.
    struct MultiInstanceConfig
    {
        MultiInstanceConfig():
            enabled(false),
            tilex(0),tiley(0),tilew(0),tileh(0),id(0),portPool(100) {}

        //! application instance Id. On systems running multiple omegalib 
        //! instances concurrently, each application should have a different 
        //! index set. The index is used, among other things, to apply offsets 
        //! to communication ports, to avoid port conflicts. This number is set 
        //! automatically during initialization by the omain() call. It is 
        //! derived by command line parameters passed with the '-I' switch.
        int id;

        bool enabled;
        int tilex;
        int tiley;
        int tilew;
        int tileh;
        int portPool;
    };

    ///////////////////////////////////////////////////////////////////////////
    //! SystemManager is the kernel of the omegalib runtime. It takes care of 
    //! setting up the system, starting up and shutting down services and 
    //! displays, and it manages the application execution.
    class OMEGA_API SystemManager
    {
    public:
        //! Utility method: searches for setting in application and system 
        //! configuration file, returns true if it exists
        static bool settingExists(const String& name);
        //! Utility method: searches for setting in application and system 
        //! configuration file, returns a reference to it if it exists.
        static Setting& settingLookup(const String& name);

    public:
        // Get the singleton instance of the system manager.
        static SystemManager* instance();
        
        //! sets up the system configuration
        void setupConfig(Config* cfg);

        //! Initializes the system manager
        void setup(Config* cfg);
        void setupRemote(Config* cfg, const String& masterHostname);

        void initialize();

        //! Starts running the current application.
        //! This method does not return until the application is done running.
        void	run();

        //! Sends an exit request to the system manager.
        void postExitRequest(const String& reason = "Undefined reason");

        //! Cleans up runtime resources, performs final debug checks etc.
        void cleanup();

        //! Gets the application configuration.
        Config* getAppConfig();
        //! Sets the application configuration.
        //! @remarks Changing the application configuration will not influence 
        //! already loaded modules and services.
        void setAppConfig(Config* cfg);
        //! Loads an application configuration file
        //! @remarks Changing the application configuration will not influence 
        //! already loaded modules and services.
        void loadAppConfig(const String& filename);

        //! Gets the system configuration.
        Config* getSystemConfig();

        DataManager* getDataManager();

        //! Gets the stats manager instance. (on master node only)
        StatsManager* getStatsManager();

        //! Gets the ServiceManager object
        ServiceManager* getServiceManager();

        //! Gets the DisplaySystem object
        DisplaySystem* getDisplaySystem();
        //! Sets the DisplaySystem object
        void setDisplaySystem(DisplaySystem* value);

        //! Gets the ApplicationBase object
        ApplicationBase* getApplication();

        //! Sets the ApplicationBase object
        void setApplication(ApplicationBase* value);

        bool isExitRequested();

        const String& getExitReason();

        bool isInitialized();
        
        bool isMaster(); 

        String getHostname();
        const String& getHostnameAndPort();

        PythonInterpreter* getScriptInterpreter(); 

        MultiInstanceConfig& getMultiInstanceConfig(); 

        //! Mission Control
        //@{
        //! Returns a mission control server instance. Returns NULL if no 
        //! mission control server is runnning.
        MissionControlServer* getMissionControlServer();
        //! Returns a mission control client instance. Returns NULL if no 
        //! mission control client is runnning.
        MissionControlClient* getMissionControlClient();
        void setupMissionControl(const String& mode);
        //@}

        void setupServiceManager();
        void setupDisplaySystem();

    private:
        SystemManager();
        ~SystemManager();

        //! Utility function, offsets the netservice port using the system 
        //! manager application instance id, to avoid port conflicts.
        void adjustNetServicePort(Setting& stnetsvc);

        void initModules();


    private:
        // Singleton instance.
        static SystemManager* mysInstance;

        bool myIsInitialized;

        // NOTE; We can't use Ref<> for a few types because they are 
        // forward-declared.
        Ref<Config>			myAppConfig;
        Ref<Config>			mySystemConfig;
        DataManager*    myDataManager;
        DisplaySystem*	myDisplaySystem;
		// External display system library, used by custom 
		// display system implementations
		Ref<Library> myDisplaySystemPlugin;
        ServiceManager*	myServiceManager;
        ApplicationBase*	myApplication;
        bool			myExitRequested;
        String			myExitReason;
        PythonInterpreter* myInterpreter;
        
        bool myIsMaster;

        String myHostname;
        String myProgramName;

        //! The application instance id.
        MultiInstanceConfig myMultiInstanceConfig;

        // Stats manager.
        Ref<StatsManager> myStatsManager;

        // Mission Contol
        MissionControlServer* myMissionControlServer;
        MissionControlClient* myMissionControlClient;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline DataManager* SystemManager::getDataManager()
    { return myDataManager; }

    ///////////////////////////////////////////////////////////////////////////
    inline ServiceManager* SystemManager::getServiceManager() 
    { return myServiceManager; }
    
    ///////////////////////////////////////////////////////////////////////////
    inline DisplaySystem* SystemManager::getDisplaySystem() 
    { return myDisplaySystem; }
    
    ///////////////////////////////////////////////////////////////////////////
    inline void SystemManager::setDisplaySystem(DisplaySystem* value) 
    { myDisplaySystem = value; }
    
    ///////////////////////////////////////////////////////////////////////////
    inline ApplicationBase* SystemManager::getApplication() 
    { return myApplication; }
    
    ///////////////////////////////////////////////////////////////////////////
    inline void SystemManager::setApplication(ApplicationBase* value) 
    { myApplication = value; }
    
    ///////////////////////////////////////////////////////////////////////////
    inline bool SystemManager::isExitRequested() 
    { return myExitRequested; }
    
    ///////////////////////////////////////////////////////////////////////////
    inline const String& SystemManager::getExitReason() 
    { return myExitReason; }
    
    ///////////////////////////////////////////////////////////////////////////
    inline bool SystemManager::isInitialized() 
    { return myIsInitialized; }

    ///////////////////////////////////////////////////////////////////////////
    inline Config* SystemManager::getAppConfig() 
    { return myAppConfig; }

    ///////////////////////////////////////////////////////////////////////////
    inline void SystemManager::setAppConfig(Config* cfg)
    { myAppConfig = cfg; }

    ///////////////////////////////////////////////////////////////////////////
    inline Config* SystemManager::getSystemConfig() 
    { return mySystemConfig; }

    ///////////////////////////////////////////////////////////////////////////
    inline bool SystemManager::isMaster() 
    { return myIsMaster; }

    ///////////////////////////////////////////////////////////////////////////
    inline PythonInterpreter* SystemManager::getScriptInterpreter() 
    { return myInterpreter; }

    ///////////////////////////////////////////////////////////////////////////
    inline MultiInstanceConfig& SystemManager::getMultiInstanceConfig() 
    { return myMultiInstanceConfig; }

    ///////////////////////////////////////////////////////////////////////////
    inline StatsManager* SystemManager::getStatsManager() 
    { return myStatsManager; }

    ///////////////////////////////////////////////////////////////////////////
    inline MissionControlServer* SystemManager::getMissionControlServer()
    { return myMissionControlServer; }

    ///////////////////////////////////////////////////////////////////////////
    inline MissionControlClient* SystemManager::getMissionControlClient()
    { return myMissionControlClient; }

}; // namespace omega

#endif