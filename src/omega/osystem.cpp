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
 *	The omegalib entry point (omain), initialization and shutdown code, plus a
 *	set of system utility functions.
 ******************************************************************************/
#include "version.h"
#include "omega/osystem.h"
#include "omega/ApplicationBase.h"
#include "omega/SystemManager.h"
#include "omega/DisplaySystem.h"
#include "omega/Engine.h"
#include "omicron/StringUtils.h"
#include "omega/MissionControl.h"
#include "omega/Platform.h"

#include <iostream>

// for getenv()
#include <stdlib.h>

#ifndef WIN32
    #include <unistd.h>
    #include<sys/wait.h>
#endif

#ifdef OMEGA_OS_WIN
    #include <direct.h>
    #define GetCurrentDir _getcwd
    // Needed for GetModuleFileName
    #include <Windows.h>
#else
    #include <unistd.h>
    #define GetCurrentDir getcwd
#endif

#ifdef OMEGA_OS_OSX
    #include <mach-o/dyld.h>
#endif

namespace omega
{
    ///////////////////////////////////////////////////////////////////////////
    libconfig::ArgumentHelper sArgs;
    Vector<String> sOptionalArgs;
    Timer sTimer;
    
    ///////////////////////////////////////////////////////////////////////////
    double otimestamp()
    {
        return sTimer.getElapsedTimeInMilliSec();
    }

    ///////////////////////////////////////////////////////////////////////////
    class LeakPrinter: public ReferenceType
    {
    public:
        void print()
        {
            if(mysObjList.size() > 1)
            {
                omsg("===================== ReferenceType object leaks follow:");
                printObjCounts();
            }
        }
    };

    ///////////////////////////////////////////////////////////////////////////
    OMEGA_API libconfig::ArgumentHelper& oargs()
    {
        return sArgs;
    }

    ///////////////////////////////////////////////////////////////////////////
    OMEGA_API const Vector<String>& oxargv()
    {
        return sOptionalArgs;
    }

    ///////////////////////////////////////////////////////////////////////////
    extern "C" void abortHandler(int signal_number)
    {
        // Just exit.
        exit(-1);
    }

    ///////////////////////////////////////////////////////////////////////////
    extern "C" void sigproc(int signal_number)
    { 		 
        DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();

        // Explicitly kill sound server
        SoundEnvironment* se = Engine::instance()->getSoundEnvironment();
        if(se != NULL)
        {
            se->getSoundManager()->stopAllSounds();
            se->getSoundManager()->cleanupAllSounds();
        }

        ds->killCluster();
        SystemManager::instance()->cleanup();
        
        osleep(2000);
    }

    ///////////////////////////////////////////////////////////////////////////
    void setupMultiInstance(SystemManager* sys, const String& multiAppString)
    {
        Vector<String> args = StringUtils::split(multiAppString, ",");
        if(args.size() == 1)
        {
            // If we have a single argument, that is the application instance id. 
            MultiInstanceConfig& mic = sys->getMultiInstanceConfig();
            mic.enabled = true;
            // Setting all the tile entries to -1 will use the full tile
            // set specified in the system configuration. We are using this
            // here because we are aonly interested in setting the application
            // instance id, not an initial tile set.
            mic.tilex = -1;
            mic.tiley = -1;
            mic.tilew = -1;
            mic.tileh = -1;
            mic.id = boost::lexical_cast<int>(args[0]);
        }
        else if(args.size() < 4)
        {
            ofwarn("Invalid number of arguments for -I option '%1%'. 1,4 or 5 expected: [<tilex>,<tiley>,<tilewidth>,<tileHeight>][,id]", %multiAppString);
        }
        else
        {
            MultiInstanceConfig& mic = sys->getMultiInstanceConfig();

            mic.enabled = true;
            mic.tilex = boost::lexical_cast<int>(args[0]);
            mic.tiley = boost::lexical_cast<int>(args[1]);
            mic.tilew = boost::lexical_cast<int>(args[2]);
            mic.tileh = boost::lexical_cast<int>(args[3]);
            mic.portPool = 100;

            // If we explicitly specify an instance id, use it. Otherwise
            // set it to -1 and let omegalib choose it.
            if(args.size() == 5) mic.id = boost::lexical_cast<int>(args[4]);
            else mic.id = -1;
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    void setupCanvasRect(SystemManager* sys, const String& canvasRect)
    {
        Vector<String> args = StringUtils::split(canvasRect, ",");
        if(args.size() < 4)
        {
            ofwarn("Invalid number of arguments for -w option '%1%'. 4 expected: <x>,<y>,<w>,<h>", %canvasRect);
        }
        else
        {
            int x = boost::lexical_cast<int>(args[0]);
            int y = boost::lexical_cast<int>(args[1]);
            int w = boost::lexical_cast<int>(args[2]);
            int h = boost::lexical_cast<int>(args[3]);

            DisplaySystem* ds = sys->getDisplaySystem();
            if(ds != NULL)
            {
                ds->getDisplayConfig().setCanvasRect(Rect(x, y, w, h));
            }
        }
    }
    
    ///////////////////////////////////////////////////////////////////////////
    int omain(omega::ApplicationBase& app, int argc, char** argv)
    {
        sTimer.start();

        // Always initialze the executable name using the name coming from
        // the command line. NOTE: just using the application name as the
        // executable name does not work on linux (application name misses
        // the initial './')
        app.setExecutableName(argv[0]);

        // register the abort handler.
        signal(SIGABRT, &abortHandler);

#ifdef OMEGA_ENABLE_VLD
        // Mark everything before this point as already reported to avoid 
        // reporting static global objects as leaks. This makes the report less
        // precise but gets rid of a lot of noise.
        VLDMarkAllLeaksAsReported();
#endif
        {
            bool remote = false;
            String masterHostname;
            String configFilename = ostr("%1%.cfg", %app.getName());
            String multiAppString = "";
            String mcmode = "default";
            String appName = app.getName();
            String canvasRect = "";

            // If we have an environment variable OMEGA_HOME, use it as the
            // default data path. The OMEGA_HOME macro is set to the
            // source code directory of omegalib.
            String dataPath = OMEGA_HOME;
            
            // If we can't find default.cfg in the hardcoded dataPath,
            // that means we are not running in a build environment.
            // change the default data path to /usr/share/omegalib in
            // LINUX and /Applications/omegalib on OSX.
            FILE* f = fopen((dataPath + "/default.cfg").c_str(), "r");
            if(f == NULL)
            {
#ifdef OMEGA_OS_LINUX
                dataPath = "/usr/share/omegalib";
#else
                dataPath = "/Applications/omegalib";
#endif
            }
            else
            {
                fclose(f);
            }
            
            char* omegaHome = getenv("OMEGA_HOME");
            if(omegaHome != NULL) dataPath = omegaHome;
            String logFilename; 

            bool kill = false;
            bool help = false;
            bool disableSIGINTHandler = false;
            bool logRemoteNodes = false;

            bool forceInteractiveOn = false;
            bool forceInteractiveOff = false;

            oargs().setStringVector(
                "args", 
                "optional arguments. If the first argument ends with .cfg it will be used as a configuration file",
                sOptionalArgs);

            sArgs.newNamedString(
                'c',
                "config", 
                "<config file>",
                "configuration file to use with this application.",
                configFilename);

            sArgs.newFlag(
                'K',
                "kill",
                "Don't run the application, only run the nodeKiller command on all nodes in a clustered configuration",
                kill);

            sArgs.newFlag(
                '?',
                "help",
                "Prints this application help screen",
                help);

            sArgs.newFlag(
                'r',
                "log-remote",
                "generate log for remote nodes (NOTE: must add to launcherCommand)",
                logRemoteNodes);
                
            sArgs.newNamedString(
                'D',
                "data",
                "Data path for this application", "",
                dataPath);
        
            sArgs.newNamedString(
                'L',
                "log",
                "log file to use with this application, OR [v|d] to enable verbose or debug log level. When log file is specified, log level is always set to debug", "",
                logFilename);

            sArgs.newNamedString(
                'I',
                "instance",
                "[<tilex>,<tiley>,<tilewidth>,<tileHeight>][,<id>]", 
                "Enable multi-instance mode and set global viewport and instance id",
                multiAppString);

            sArgs.newNamedString(
                'm',
                "mc",
                "default|server|disable", "Sets mission control mode.",
                mcmode);

            sArgs.newFlag(
                'd',
                "disable-sigint",
                "disables the Control+C handler (useful when debugging with gdb)",
                disableSIGINTHandler);
                
            sArgs.newNamedString(
                'N',
                "name",
                "<name>", "Application name",
                appName);

            sArgs.newNamedString(
                'w',
                "viewport",
                "<x>,<y>,<w>,<h>]",
                "The initial canvas for this application.",
                canvasRect);

            sArgs.newFlag(
                'i',
                "interactive",
                "Runs the program in interactive mode, overriding configuration file settings",
                forceInteractiveOn);

            sArgs.newFlag(
                0,
                "interactive-off",
                "Runs the program in non interactive mode, , overriding configuration file settings",
                forceInteractiveOff);

            sArgs.setAuthor("The Electronic Visualization Lab, UIC");
            //String appName;
            //String extName;
            //String pathName;
            //StringUtils::splitFullFilename(app.getName(), appName, extName, pathName);
            sArgs.setDescription(appName.c_str());
            sArgs.setName(appName.c_str());
            sArgs.setVersion(OMEGA_VERSION "-" GIT_BRANCH);
            
            // If argument processing fails, exit immediately.
            if(!sArgs.process(argc, argv))
            {
                return -1;
            }

            if(forceInteractiveOff) app.interactive = ApplicationBase::NonInteractive;
            else if(forceInteractiveOn) app.interactive = ApplicationBase::Interactive;

            // Update the application name.
            app.setName(appName);

            // Entering - as the path will force omegalib to use the source 
            // directory as the data directory, even if the OMEGA_HOME 
            // environment variable is present. 
            if(dataPath == "-")
            {
                dataPath = OMEGA_HOME;
            }
            
            if(!disableSIGINTHandler)
            {
                //omsg("Registering Control-C SIGINT handler");
                signal(SIGINT, sigproc);
            }

            if(help)
            {
                sArgs.writeUsage(std::cout);
                return 0;
            }
            
            // Set log level and/or log filename
            if(logFilename == "v")
            {
                StringUtils::logLevel = StringUtils::Verbose;
                logFilename = ostr("%1%.log", %app.getName());        
            } 
            else if(logFilename == "d")
            {
                StringUtils::logLevel = StringUtils::Debug;
                logFilename = ostr("%1%.log", %app.getName());        
            } 
            else if(logFilename == "")
            {
                logFilename = ostr("%1%.log", %app.getName());        
            }
            else
            {
                // A log filename was entered, always set log level to debug.
                StringUtils::logLevel = StringUtils::Debug;
            }

            std::vector<std::string> args = StringUtils::split(configFilename, "@");
            configFilename = args[0];
            if(args.size() == 2)
            {
                remote = true;
                masterHostname = args[1];
            
                // If logging on remote nodes is enabled, set up an output file using the app + node name.
                // otherwise, disable logging.
                if(logRemoteNodes)
                {
                    omsg("Remote node logging enabled");
                    String hostLogFilename = masterHostname + "-" + logFilename;
                    ologopen(hostLogFilename.c_str());
                }
                else
                {
                    ologdisable();
                }
            }
            else
            {
                if(logFilename != "off") ologopen(logFilename.c_str());
                else ologdisable();
            }
        
            SystemManager* sys = SystemManager::instance();
            DataManager* dm = sys->getDataManager();
            
            olog(Verbose, "omegalib data search paths:");
            String cwd = ogetcwd();
            oflog(Verbose, "::: %1%", %cwd);

            // Add some default filesystem search paths: 
            // - an empty search path for absolute paths
            // - the current directory
            // - the default omegalib data path
            // - the modules path
            // - the current executable path (for dynamic lib loading)
            dm->addSource(new FilesystemDataSource(cwd));
            dm->addSource(new FilesystemDataSource(""));
            dm->addSource(new FilesystemDataSource(dataPath));
            dm->addSource(new FilesystemDataSource(dataPath + "/bin"));
            dm->addSource(new FilesystemDataSource(dataPath + "/modules"));
            
            String exePath = ogetexecpath();
            String exeDir;
            String exeName;
            StringUtils::splitFilename(exePath, exeName, exeDir);
            dm->addSource(new FilesystemDataSource(exeDir));
            
            // Set the root data dir as the data prefix. This way, we can 
            // retrieve the root data dir later on (for instance, to pass it to other
            // instances during cluster startup)
            osetdataprefix(dataPath);
            
            oflog(Verbose, "::: %1%", %dataPath);
            olog(Verbose, "omegalib application config lookup:");
            String curCfgFilename = ostr("%1%/%2%", %app.getName() %configFilename);

            // Config files can be specified either through the -c argument or 
            // just as optional arguments. If the first optional argument passed
            // to omegalib ends in /cfg or .oapp, we treat that argument as a 
            // configuration file. The optional-arg configuration file takes
            // priority over anything specified by -c
            if(sOptionalArgs.size() > 0 &&
                (StringUtils::endsWith(sOptionalArgs[0], ".cfg") ||
                StringUtils::endsWith(sOptionalArgs[0], ".oapp")))
            {
                curCfgFilename = sOptionalArgs[0];
            }

            oflog(Verbose, "::: trying %1%", %curCfgFilename);
            String path;
            if(!DataManager::findFile(curCfgFilename, path))
            {
                curCfgFilename = configFilename;
                oflog(Verbose, "::: not found, trying %1%", %curCfgFilename);

                if(!DataManager::findFile(curCfgFilename, path))
                {
                    curCfgFilename = "default.cfg";
                    oflog(Verbose, "::: not found, trying %1%", %curCfgFilename);
                    if(!DataManager::findFile(curCfgFilename, path))
                    {
                        oerror("FATAL: Could not load default.cfg. Application will exit now.");
                        return -1;
                    }
                }
            }

            oflog(Verbose, "::: found config: %1%", %curCfgFilename);

            Config* cfg = new Config(curCfgFilename);
            cfg->load();


            // Set the current working dir to the configuration dir
            // so we can load local files from there during setup if needed
            // this is used for example by the initScript option to load a script
            // in the same dir as the config file.
            String cfgdir;
            String cfgbasename;
            String cfgext;
            StringUtils::splitFullFilename(path, cfgbasename, cfgext, cfgdir);
            dm->addSource(new FilesystemDataSource(cfgdir));
            
            // If multiApp string is set, setup multi-application mode.
            // In multi-app mode, this instance will output to a subset of the available tiles, and will choose a
            // communication port using a port interval starting at the configuration base port plus and dependent on a port pool.
            if(multiAppString != "") setupMultiInstance(sys, multiAppString);

            if(kill)
            {
                sys->setApplication(&app);
                sys->setupConfig(cfg);
                sys->setupDisplaySystem();
                DisplaySystem* ds = sys->getDisplaySystem();
                ds->killCluster();
            }
            else
            {
                //omsg(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> OMEGALIB BOOT");
                sys->setApplication(&app);
                if(remote)
                {
                    sys->setupRemote(cfg, masterHostname);
                }
                else
                {
                    sys->setup(cfg);
                    sys->setupMissionControl(mcmode);
                }

                // If we have an initial viewport specified, set it.
                if(canvasRect != "") setupCanvasRect(sys, canvasRect);

                sys->initialize();

                //omsg("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< OMEGALIB BOOT\n\n");

                sys->run();

                //omsg(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> OMEGALIB SHUTDOWN");
                sys->cleanup();
                //omsg("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< OMEGALIB SHUTDOWN\n\n");

               Ref<LeakPrinter> lp = new LeakPrinter();
                lp->print();
            }

            ologclose();
        }

#ifdef OMEGA_ENABLE_VLD
        _cexit();
        VLDReportLeaks();
#endif

        return 0;
    }

    ///////////////////////////////////////////////////////////////////////////
    bool olaunch(const String& command)
    {
        if( command.empty( )) return false;
        
        olog(Verbose, "[olaunch] " + command);

#ifdef OMEGA_OS_WIN
        STARTUPINFO         startupInfo;
        ZeroMemory(&startupInfo, sizeof(STARTUPINFO));

        PROCESS_INFORMATION procInfo;
        ZeroMemory(&procInfo, sizeof(PROCESS_INFORMATION));

        const char*         cmdLine     = command.c_str();

        startupInfo.cb = sizeof( STARTUPINFO );
        const bool success = 
            CreateProcess( 0, LPSTR( cmdLine ), // program, command line
                           0, 0,                // process, thread attributes
                           FALSE,               // inherit handles
                           0,                   // creation flags
                           0,                   // environment
                           0,                   // current directory
                           &startupInfo,
                           &procInfo );

        //WaitForInputIdle( procInfo.hProcess, 1000 );
        CloseHandle( procInfo.hProcess );
        CloseHandle( procInfo.hThread );

        return true;
#else
        std::vector<std::string> commandLine = StringUtils::split(command, " ");

        // NOTE 28Jul13: This has been changed to SIG_IGN instead of a custom 
        // handler to avoid a weird deadlock with glXCreateContext on SUSE 12.3
        // glXCreateContext invoked (thorugh the nvidia driver) a waitpid that apparently
        // waits for the same child process generated here. Note that the child 
        // handler just waits for the process to exit cleanly to avoid 
        // creating zombies, which can be done just by specifying SIG_IGN as
        // the handler.
        signal( SIGCHLD, SIG_IGN );
        const int result = fork();
        switch( result )
        {
            case 0: // child
                break;

            case -1: // error
                ofwarn("Launching command %1% failed.", %command);
                return false;
            default: // parent
                return true;
        }

        // child
        const size_t  argc         = commandLine.size();
        char*         argv[argc+1];
        std::ostringstream stringStream;

        for( size_t i=0; i<argc; i++ )
        {
            argv[i] = (char*)commandLine[i].c_str();
            stringStream << commandLine[i] << " ";
        }

        argv[argc] = 0;

        //ofmsg("Executing: %1%", %command);
        int nTries = 10;
        while( nTries-- )
        {
            execvp( argv[0], argv );
            ofwarn("Error executing %1%", %command);
            // EQWARN << "Error executing '" << argv[0] << "': " << sysError
                   // << std::endl;
            if( errno != ETXTBSY )
                break;
        }

        // Launch failed. Crash and burn.
        exit(-1);
        return false;
#endif
    }

    ///////////////////////////////////////////////////////////////////////////
    String ogetcwd()
    {
        char cCurrentPath[FILENAME_MAX];
        GetCurrentDir(cCurrentPath, sizeof(cCurrentPath));
        return cCurrentPath;
    }

    ///////////////////////////////////////////////////////////////////////////
    String ogetexecpath()
    {
        char path[2048];
        path[0] = '\0';
#ifdef OMEGA_OS_LINUX
        ssize_t l = readlink("/proc/self/exe", path, 2048);
        path[l] = '\0';
#elif defined OMEGA_OS_WIN
        GetModuleFileName(NULL, path, 2048);
#else
        uint32_t bufsize = 2048;
        _NSGetExecutablePath(path, &bufsize);
        //owarn("OSX NOT IMPLEMENTED: (osystem.cpp) ogetexecpath");
        //owarn("Imlement using _NSGetExecutablePath()");
#endif	
        return path;
    }
    
    String _dataPrefix;

    ///////////////////////////////////////////////////////////////////////////
    void osetdataprefix(const String& data)
    {
        _dataPrefix = data;
    }

    ///////////////////////////////////////////////////////////////////////////
    String ogetdataprefix()
    {
        return _dataPrefix;
    }
}
