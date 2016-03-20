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
#include "omega/SystemManager.h"

// Display system
#include "omega/DisplaySystem.h"
#include "omega/NullDisplaySystem.h"
#include "omega/PythonInterpreter.h"
#include "omega/MissionControl.h"
#include "omega/Platform.h"


// Input services
#include "omega/ModuleServices.h"
#include "omega/ImageUtils.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
SystemManager* SystemManager::mysInstance = NULL;

Setting sNullSetting(NULL);

///////////////////////////////////////////////////////////////////////////////
bool SystemManager::settingExists(const String& name)
{
    if(mysInstance != NULL)
    {
        Config* appCfg = mysInstance->getAppConfig();
        Config* sysCfg = mysInstance->getSystemConfig();
        if(appCfg->exists(name)) return true;
        if(sysCfg->exists(name)) return true;
        return false;
    }
    owarn("SystemManager::settingExists: cannot search for settings before System Manager initialization");
    return false;
}

///////////////////////////////////////////////////////////////////////////////
Setting& SystemManager::settingLookup(const String& name)
{
    if(mysInstance != NULL)
    {
        Config* appCfg = mysInstance->getAppConfig();
        Config* sysCfg = mysInstance->getSystemConfig();
        if(appCfg->exists(name)) return appCfg->lookup(name);
        if(sysCfg->exists(name)) return sysCfg->lookup(name);;
        oferror("FATAL SystemManager::settingLookup: could not find setting", %name);
    }
    oerror("FATAL SystemManager::settingLookup: cannot search for settings before System Manager initialization");
    return sNullSetting;
}

///////////////////////////////////////////////////////////////////////////////
SystemManager* SystemManager::instance()
{
    if(!mysInstance) mysInstance = new SystemManager();
    return mysInstance;
}

///////////////////////////////////////////////////////////////////////////////
SystemManager::SystemManager():
    mySystemConfig(NULL),
    myDisplaySystem(NULL),
    myApplication(NULL),
    myStatsManager(NULL),
    myExitRequested(false),
    myIsInitialized(false),
    myIsMaster(true),
    myServiceManager(NULL),
    myMissionControlServer(NULL),
    myMissionControlClient(NULL)
{
    myDataManager = DataManager::getInstance();
    myStatsManager = new StatsManager();
    myInterpreter = new PythonInterpreter();

    ImageUtils::internalInitialize();
}

///////////////////////////////////////////////////////////////////////////////
SystemManager::~SystemManager()
{
    myAppConfig = NULL;
    mySystemConfig = NULL;

    if(myServiceManager != NULL) delete myServiceManager;
    if(myDisplaySystem != NULL) delete myDisplaySystem;
    //if(myStatsManager != NULL) delete myStatsManager;

    // We are doing explicit reference counting here because we are 
    // using raw pointers to store the mission control objects. We can't
    // use refs in the SystemManager header or we would get a circular
    // inclusion when adding MissionControl.h
    if(myMissionControlClient != NULL)
    {
        myMissionControlClient->unref();
        myMissionControlClient = NULL;
    }
    if(myMissionControlServer != NULL)
    {
        ologremlistener(myMissionControlServer);
        myMissionControlServer->unref();
        myMissionControlServer = NULL;
    }
    
    myDisplaySystem = NULL;
    myInterpreter = NULL;

    ImageUtils::internalDispose();
}

///////////////////////////////////////////////////////////////////////////////
void SystemManager::setupRemote(Config* cfg, const String& hostname)
{
    myIsMaster = false;
    myHostname = hostname;
    setup(cfg);
}

///////////////////////////////////////////////////////////////////////////////
void SystemManager::setup(Config* appcfg)
{
    //omsg("SystemManager::setup");

    setupConfig(appcfg);
    try
    {
        // Load platform section
        if(mySystemConfig->exists("config/platform"))
        {
            Platform::setup(mySystemConfig->lookup("config/platform"));
        }

        if(myInterpreter->isEnabled())
        {
            if(mySystemConfig->exists("config"))
            {
                const Setting& sConfig = mySystemConfig->lookup("config");
                myInterpreter->setup(sConfig);
            }
            // If we have an application configuration file, read interpreter 
            // setup values from there as well. Setting
            if(myAppConfig != mySystemConfig)
            {
                if(myAppConfig->exists("config"))
                {
                    const Setting& sConfig = myAppConfig->lookup("config");
                    myInterpreter->setup(sConfig);
                }
            }
        }

        // Instantiate the service manager before initializing the interpreter,
        // so the interpreter initialization command can register additional
        // services.
        myServiceManager = new ServiceManager();

        // NOTE: We initialize the interpreter here (instead of the 
        // SystemManager::initialize function) to allow it to load optional modules
        // that may provide services that we then want do setup during
        // setupServiceManager() or setupDisplaySystem()
        myInterpreter->initialize("omegalib");

        // The display system needs to be set up before service manager, because it finishes setting up
        // the multi instance configuration parameters that are used during service configuration.
        setupDisplaySystem();

        setupServiceManager();

        initModules();
    }
    catch(libconfig::SettingTypeException ste)
    {
        oferror("Wrong setting type at %1% (HINT: make floats have a decimal part in your configuration)", %ste.getPath());
    }
}

///////////////////////////////////////////////////////////////////////////////
void SystemManager::setupConfig(Config* appcfg)
{
    if(!appcfg->isLoaded()) appcfg->load();
    oassert(appcfg->isLoaded());

    // If app config has a systemConfig entry, load the system configuration file specified there.
    myAppConfig = appcfg;
    if(appcfg->exists("config/systemConfig"))
    {
        String systemCfgName = (const char*)appcfg->lookup("config/systemConfig");
        // If system config specified 'DEFAULT', open default.cfg and read the system
        // config entry there.
        // LOGIC CHANGE: 13Jul2013
        // RATIONALE: we can have application config files that don't have to
        // specify an exact system configuration, and just say 'use whatever the
        // default is for this platform.
        if(systemCfgName == "DEFAULT")
        {
            Config* defaultCfg = new Config("default.cfg");
            if(defaultCfg->load())
            {
                String systemCfgName = (const char*)defaultCfg->lookup("config/systemConfig");
                oflog(Verbose, "Default system configuration file: %1%", %systemCfgName);
                mySystemConfig = new Config(systemCfgName);
            }
            else
            {
                oerror("SystemManager::setup: FATAL - coult not load default.cfg");
            }
        }
        else
        {
            oflog(Verbose, "SystemManager::setup: systemConfig = %1%", %systemCfgName);
            mySystemConfig = new Config(systemCfgName);
        }
    }
    else
    {
        // LOGIC CHANGE: 13Jul2013
        // If the app config has no systemConfig section, we will use it as 
        // the system configuration file.
        olog(Verbose, "SystemManager::setup: using app config as system config file");
        mySystemConfig = myAppConfig;
    }

    // Make sure the configuration is loaded.
    if(!mySystemConfig->isLoaded()) mySystemConfig->load();

    oassert(mySystemConfig->isLoaded());
}

///////////////////////////////////////////////////////////////////////////////
void SystemManager::adjustNetServicePort(Setting& stnetsvc)
{
    for(int i = 0; i < stnetsvc.getLength(); i++)
    {
        // If the service has a dataPort field, offset it using the instance id.
        Setting& ssvc = stnetsvc[i];
        if(ssvc.exists("dataPort"))
        {
            int curVal = ssvc["dataPort"];
            curVal += myMultiInstanceConfig.id;
            ssvc["dataPort"] = curVal;
            ofmsg("Service %1%: dataPort set to %2%", %ssvc.getName() %curVal);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void SystemManager::setupServiceManager()
{
    // Kinda hack: run application initialize here because for now it is used to register services from
    // external libraries, so it needs to run before setting up services from the config file.
    // Initialize the application object (if present).
    // NOTE: this also gives a change to the main application module to register
    // python APIS for other modules that may be initialized through the config
    // file. In other words: keep this line here or do it earlier!
    if(myApplication) myApplication->initialize();

    // NOTE setup services only on master node.
    if(isMaster())
    {
        // If app config has a services section, read services from there instead
        // of system config.
        Setting& stRoot = myAppConfig->getRootSetting()["config"];
        if(stRoot.exists("services"))
        {
            Setting& stnetsvc = stRoot["services"];
            if(myMultiInstanceConfig.enabled) adjustNetServicePort(stnetsvc);
            myServiceManager->setup(stnetsvc);
        }
        else
        {
            // Instantiate services (for compatibility reasons, look under'input' and 'services' sections
            Setting& stRoot = mySystemConfig->getRootSetting()["config"];
            if(stRoot.exists("input"))
            {
                Setting& stnetsvc = stRoot["input"];
                if(myMultiInstanceConfig.enabled) adjustNetServicePort(stnetsvc);
                myServiceManager->setup(stnetsvc);
            }
            else if(stRoot.exists("services"))
            {
                Setting& stnetsvc = stRoot["services"];
                if(myMultiInstanceConfig.enabled) adjustNetServicePort(stnetsvc);
                myServiceManager->setup(stnetsvc);
            }
            else
            {
                owarn("Config/InputServices section missing from config file: No services created.");
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void SystemManager::setupDisplaySystem()
{
    // If a display system already exists (i.e. set through a configuration 
    // initScript or initCommand) do nothing.
    if(myDisplaySystem != NULL) return;

    if(mySystemConfig->exists("config/display"))
    {
        Setting& stDS = mySystemConfig->lookup("config/display");
        DisplaySystem* ds = NULL;

        String displaySystemType = "Null";
        stDS.lookupValue("type", displaySystemType);
        
        oflog(Verbose, "SystemManager::setupDisplaySystem: type = %1%", %displaySystemType);
        
        if (displaySystemType == "Null")
        {
            // if display is unspecified or specified as 'Null'
            // setup the application in headless node.
            ds = new NullDisplaySystem();
        }
        else
        {
            // Other display system type: try to load it from an external library.
            // If the display system type is X, we will search displaySystem_X.
            oflog(Verbose, "[SystemManager::setupDisplaySystem] finding display system <%1%>", %displaySystemType);

#ifdef OMEGA_OS_WIN
            String libname = "displaySystem_"; libname = libname + displaySystemType + ".dll";
#elif defined(OMEGA_OS_OSX)
            String libname = "libdisplaySystem_"; libname = libname + displaySystemType + ".dylib";
#else
            String libname = "libdisplaySystem_"; libname = libname + displaySystemType + ".so";
#endif
            String libPath;
            if (!DataManager::findFile(libname, libPath))
            {
                ofwarn("[SystemManager::setupDisplaySystem] could not find library %1%", %libname);
                ds = new NullDisplaySystem();
            }
            else
            {
                oflog(Verbose, "[SystemManager::setupDisplaySystem] loading library <%1%>", %libPath);
                myDisplaySystemPlugin = new Library();
                if (myDisplaySystemPlugin->open(libPath))
                {
                    typedef DisplaySystem*(*CreateFunc)();
                    CreateFunc createDisplaySystem = (CreateFunc)myDisplaySystemPlugin->getFunctionPointer("createDisplaySystem");
                    if (createDisplaySystem == NULL)
                    {
                        ofwarn("[SystemManager::setupDisplaySystem] could not find entry point createDisplaySystem in library %1%", %libPath);
                        ds = new NullDisplaySystem();
                    }
                    else
                    {
                        ds = createDisplaySystem();
                    }
                }
            }
        }

        if(ds != NULL)
        {
            // Setup the display system. This call will parse the display configuration and fill
            // the DisplayConfig structure.
            ds->setup(stDS);

            // If multi-instance mode is active, adjust the display settings.
            if(myMultiInstanceConfig.enabled)
            {
                // This call will also set the instance id in myMultiInstanceConfig.
                ds->getDisplayConfig().setupMultiInstance(&myMultiInstanceConfig);
            }

            setDisplaySystem(ds);
        }
    }
    else
    {
        // if display is unspecified incorrect or specified as 'Null'
        // setup the application in headless node.
        setDisplaySystem(new NullDisplaySystem());
    }
}

///////////////////////////////////////////////////////////////////////////////
void SystemManager::setupMissionControl(const String& mode)
{
    int port = MissionControlServer::DefaultPort;
    String host = "127.0.0.1";
    bool serverEnabled = false;
    bool clientEnabled = false;

    // Read config from file.
    if(settingExists("config/missionControl"))
    {
        Setting& s = settingLookup("config/missionControl");
        port = Config::getIntValue("port", s, port);
        host = Config::getStringValue("host", s, host);
        serverEnabled = Config::getBoolValue("serverEnabled", s, serverEnabled);
        clientEnabled = Config::getBoolValue("clientEnabled", s, clientEnabled);
    }

    // If mode is default and server is enabled in the configuration, or
    // if we specify server mode explicitly in the mode string, start a mission
    // control server.
    if((mode == "default" && serverEnabled) || StringUtils::startsWith(mode, "server"))
    {
        // If mode string is in the form 'server@port', parse the port string.
        size_t pos = mode.find('@');
        if(pos != -1)
        {
            port = boost::lexical_cast<int>(mode.substr(pos + 1));
        }

        if(serverEnabled || StringUtils::startsWith(mode, "server"))
        {
            ofmsg("Initializing mission control server on port %1%", %port);

            // We are doing explicit reference counting here because we are 
            // using raw pointers to store the mission control objects. We can't
            // use refs in the SystemManager header or we would get a circular
            // inclusion when adding MissionControl.h

            myMissionControlServer = new MissionControlServer();
            myMissionControlServer->ref();
            //myMissionControlServer->setMessageHandler(myMissionControlClient);
            myMissionControlServer->setPort(port);

            ologaddlistener(myMissionControlServer);

            // Register the mission control server. The service manager will take care of polling the server
            // periodically to check for new connections.
            myServiceManager->addService(myMissionControlServer);
            myMissionControlServer->start();

            myMissionControlClient = MissionControlClient::create();
            myMissionControlClient->ref();
            myMissionControlClient->setName("server");
            myMissionControlClient->connect("127.0.0.1", port);
        }
    }
    // If mode is client, start a client and connect to a server using host and
    // port from the configuration file. By default connects to a local server.
    else if(mode == "client" || clientEnabled)
    {
        omsg("Initializing mission control client...");
        myMissionControlClient = MissionControlClient::create();
        myMissionControlClient->setName(myApplication->getName());
        myMissionControlClient->connect(host, port);
    }
    // If string begins with @, we are going to connect to a server specified 
    // in the string.
    else if(mode[0] == '@')
    {
        size_t pos = mode.find(':');
        if(pos == -1)
        {
            // use default port 
            port = MissionControlServer::DefaultPort;
            host = mode.substr(1);
        }
        else
        {
            port = boost::lexical_cast<int>(mode.substr(pos + 1));
            host = mode.substr(1, pos - 1);
        }

        ofmsg("Mission control client: connecting to %1%:%2%", %host %port);
        myMissionControlClient = MissionControlClient::create();
        myMissionControlClient->setName(myApplication->getName());
        myMissionControlClient->connect(host, port);
    }
}

///////////////////////////////////////////////////////////////////////////////
void SystemManager::initModules()
{
    if(myInterpreter->isEnabled())
    {
        if(mySystemConfig->exists("config/modules"))
        {
            const Setting& sConfig = mySystemConfig->lookup("config/modules");
            for(int i = 0; i < sConfig.getLength(); i++)
            {
                Setting& sm = sConfig[i];
                String modname = sm.getName();
                String modclass = (const char*)sm["class"];

                // If module has a package name, import it.
                Vector<String> args = StringUtils::split(modclass, ".");
                if(args.size() == 2) myInterpreter->eval("import " + args[0]);

                myInterpreter->eval(ostr("_%2% = %1%.create('%2%')", %modclass %modname));
            }
        }
        // If we have an application configuration file, read interpreter 
        // setup values from there as well. Setting
        if(myAppConfig != mySystemConfig)
        {
            if(myAppConfig->exists("config/modules"))
            {
                const Setting& sConfig = myAppConfig->lookup("config/modules");
                for(int i = 0; i < sConfig.getLength(); i++)
                {
                    Setting& sm = sConfig[i];
                    String modname = sm.getName();
                    String modclass = (const char*)sm["class"];

                    // If module has a package name, import it.
                    Vector<String> args = StringUtils::split(modclass, ".");
                    if(args.size() == 2) myInterpreter->eval("import " + args[0]);

                    myInterpreter->eval(ostr("_%2% = %1%.create('%2%')", %modclass %modname));
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void SystemManager::initialize()
{
    // Initialize the service manager before the display system so services get
    // a chance to modify the display configuration before the display is 
    // initialized. This is used by services in external modules like the
    // Oculus Rift service.
    myServiceManager->initialize();

    if(myDisplaySystem) myDisplaySystem->initialize(this);

    initModules();

    myIsInitialized = true;
}

///////////////////////////////////////////////////////////////////////////////
void SystemManager::run()
{
    // If the system manager has not been initialized yet, do it now.
    if(!myIsInitialized) initialize();

    myServiceManager->start();
    if(myDisplaySystem)
    {
        myDisplaySystem->run();
    }
    else
    {
        owarn("SystemManager::run - no display system specified, returning immediately");
    }
}

///////////////////////////////////////////////////////////////////////////////
void SystemManager::cleanup()
{
    // Shutdown and dispose all services
    if(myServiceManager != NULL)
    {
        myServiceManager->stop();
        myServiceManager->dispose();
    }

    // Dispose the interpreter. All interpreter objects will be 
    // deallocated. The engine state will be reset.
    if(myInterpreter != NULL) 
    {
        myInterpreter->clean();
        delete myInterpreter;
        myInterpreter = NULL;        
    }

    // Shutdown the display system. This will also shutdown and dispose the
    // omegalib engine.
    if(myDisplaySystem) 
    {
        myDisplaySystem->cleanup();
        delete myDisplaySystem;
        myDisplaySystem = NULL;
    }

    myDataManager->cleanup();

    // Dispose myself.
    delete mysInstance;
    mysInstance = NULL;
}

///////////////////////////////////////////////////////////////////////////////
void SystemManager::postExitRequest(const String& reason)
{
    myExitRequested = true;
    myExitReason = reason;
}

///////////////////////////////////////////////////////////////////////////////
void SystemManager::loadAppConfig(const String& filename)
{
    String cfgPath;
    if(DataManager::findFile(filename, cfgPath))
    {
        // Load and set the new app config.
        Config* cfg = new Config(filename);
        cfg->load();
        myAppConfig = cfg;
    }
}

///////////////////////////////////////////////////////////////////////////////
String SystemManager::getHostname() 
{ 
    Vector<String> args = StringUtils::split(myHostname, ":");
    return args[0];
}

///////////////////////////////////////////////////////////////////////////////
const String& SystemManager::getHostnameAndPort() 
{ 
    return myHostname;
}
