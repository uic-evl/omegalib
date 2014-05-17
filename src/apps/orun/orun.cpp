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
 *	A python script launcher and interpreter for omegalib applications.
 ******************************************************************************/
#include <omega.h>
#include <omegaToolkit.h>

#include <boost/algorithm/string/join.hpp>

#ifdef omegaVtk_ENABLED
#include <omegaVtk/omegaVtk.h>
#endif

#ifdef cyclops_ENABLED
#include <cyclops/cyclops.h>
using namespace cyclops;
#endif

#ifdef OMEGA_OS_WIN
#ifdef OMEGA_ENABLE_AUTO_UPDATE
#include <winsparkle.h>
#endif
#endif

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

#ifdef omegaVtk_ENABLED
using namespace omegaVtk;
#endif

// The name of the script to launch automatically at startup
String sDefaultScript = "";
Vector<String> sScriptCommand;
// When set to true, add the script containing directory to the data search paths.
bool sAddScriptDirectoryToData = true;


///////////////////////////////////////////////////////////////////////////////
class OmegaViewer: public EngineModule
{
public:
    OmegaViewer();
    ~OmegaViewer()
    {}

    virtual void initialize();
    virtual bool handleCommand(const String& cmd);

    String getAppStartCommand() { return myAppStartFunctionCall; }
    void setAppStartCommand(const String cmd) { myAppStartFunctionCall = cmd; }

private:
    //Ref<UiModule> myUi;
    String myAppStartFunctionCall;
};

///////////////////////////////////////////////////////////////////////////////
OmegaViewer* gViewerInstance = NULL;
OmegaViewer* getViewer() { return gViewerInstance; }

///////////////////////////////////////////////////////////////////////////////
// Python wrapper code.
#include "omega/PythonInterpreterWrapper.h"
BOOST_PYTHON_MODULE(omegaViewer)
{
    // OmegaViewer
    PYAPI_REF_BASE_CLASS(OmegaViewer)
        PYAPI_METHOD(OmegaViewer, getAppStartCommand)
        PYAPI_METHOD(OmegaViewer, setAppStartCommand)
        ;

    def("getViewer", getViewer, PYAPI_RETURN_REF);
}

///////////////////////////////////////////////////////////////////////////////
OmegaViewer::OmegaViewer():
    EngineModule("OmegaViewer")
{
    gViewerInstance = this;

    // If I create t here, UiModule will be registered as a core module and won't be 
    // deallocated between application switches.
    //myUi = new UiModule();
    //ModuleServices::addModule(myUi);
}

///////////////////////////////////////////////////////////////////////////////
void OmegaViewer::initialize()
{
#ifdef omegaVtk_ENABLED
    omegaVtkPythonApiInit();
#endif

    omegaToolkitPythonApiInit();

#ifdef cyclops_ENABLED
    cyclopsPythonApiInit();
#endif

    //
    String orunInitScriptName = "default_init.py";
    myAppStartFunctionCall = "from euclid import *; from omegaToolkit import *; _onAppStart()";

    Config* cfg = SystemManager::instance()->getAppConfig();
    if(cfg->exists("config/orun"))
    {
        Setting& s = cfg->lookup("config/orun");
        orunInitScriptName = Config::getStringValue("initScript", s, orunInitScriptName);
        myAppStartFunctionCall = Config::getStringValue("appStartFunction", s, myAppStartFunctionCall);
    }

    // Initialize the python wrapper module for this class.
    initomegaViewer();

    // Run the init script.
    PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
    if(orunInitScriptName != "")
    {
        interp->runFile(orunInitScriptName, PythonInterpreter::NoRunFlags);
        interp->eval(myAppStartFunctionCall);
    }

    // If a default script has been passed to orun, queue it's execution through the python
    // interpreter. Queuing it will make sure the script is launched on all nodes when running
    // in a cluster environment.
    if(sDefaultScript != "")
    {
        interp->queueCommand(ostr(":r %1%", %sDefaultScript));
    }

    // If an initial command is present (specified through the -x cmdline arg)
    //  queue it.
    if(sScriptCommand.size() > 0)
    {
        String cmd = boost::algorithm::join(sScriptCommand, " ");
        interp->queueCommand(cmd);
    }

    omsg("\n\n\n---------------------------------------------------------------------");
    omsg("Welcome to orun!");
    omsg("\tomegalib version " OMEGA_VERSION);
    omsg("\tTo get a list of quick commands type :?");
    omsg("\tType :? . to list all global symbols");
    omsg("\tType :? C to list all members of class or variable `C`");
    omsg("\t\texample :? SceneNode");
    omsg("\tType :? ./C [prefix] to list global symbols or object members starting with `prefix`");
    omsg("\t\texample :? . si");
    omsg("\t\texample :? SceneNode set");
    omsg("\n\n\n");
}

///////////////////////////////////////////////////////////////////////////////
bool OmegaViewer::handleCommand(const String& cmd)
{
    Vector<String> args = StringUtils::split(cmd);
    SystemManager* sys = SystemManager::instance();
    PythonInterpreter* interp = sys->getScriptInterpreter();
    if(args[0] == "?")
    {
        // ?: command help
        if(args.size() == 2)
        {
            // Print members of specified object
            if(args[1] == ".")	interp->eval("for m in [x for x in dir() if x[0] != \"_\"]: print(m)");
            else interp->eval(ostr("for m in [x for x in dir(%1%) if x[0] != \"_\"]: print(m)", %args[1]));
        }
        else if(args.size() == 3)
        {
            // Print members of specified object starting with a prefix
            if(args[1] == ".") interp->eval(ostr("for m in [x for x in dir() if x[0] != \"_\" and x.startswith('%1%')]: print(m)", %args[2]));
            else interp->eval(ostr("for m in [x for x in dir(%1%) if x[0] != \"_\" and x.startswith('%2%')]: print(m)", %args[1] %args[2]));
        }
        else
        {
            omsg("OmegaViewer");
            omsg("\t r  <appName> - run the specified script application");
            omsg("\t r! [appName] - reset and run the specified script application. If no script is specified, reload the initial one");
            omsg("\t lo           - list live objects");
            omsg("\t ln           - print the scene node tree");
            omsg("\t u            - unload all running applications");
            omsg("\t s		      - print statistics");
            omsg("\t w		      - toggle wand");
            omsg("\t porthole     - (experimental) enable porthole");
            omsg("\t check_update - (windows only) checks for omegalib updates online");
        }
    }
    else if(args[0] == "r" && args.size() > 1)
    {
        // r: run application.
        interp->runFile(args[1]);
        return true;
    }
    else if(args[0] == "r!")
    {
        if(args.size() > 1)
        {
            sDefaultScript = args[1];
        }
        // r!: reset state and run application.
        interp->queueCommand(myAppStartFunctionCall, true);
        interp->cleanRun(sDefaultScript);
        return true;
    }
    else if(args[0] == "u")
    {
        // u: unload all running applications.
        interp->clean();
        interp->queueCommand(myAppStartFunctionCall, true);
        return true;
    }
    else if(args[0] == "lo")
    {
        // lo: list objects
        ReferenceType::printObjCounts();
        return true;
    }
    else if(args[0] == "ln")
    {
        // ln: list nodes

        // ls is really just a shortcut for printChildren(getEngine().getScene(), <tree depth>)
        interp->eval("printChildren(getScene(), 10)");
        return true;
    }
    else if(args[0] == "s")
    {
        // s: print statistics
        SystemManager::instance()->getStatsManager()->printStats();
        return true;
    }
    //else if(args[0] == "porthole")
    //{
    //
    //	// porthole: start the porthole server
    //	String xmlFile = "porthole/porthello.xml";
    //	String cssFile = "porthole/porthello.css";
    //	if(args.size() == 3)
    //	{
    //		xmlFile = args[1];
    //		cssFile = args[2];
    //	}
    //	PortholeService* service = PortholeService::createAndInitialize(4080,xmlFile, cssFile);
    //	return true;
    //}
    else if(args[0] == "q")
    {
        // q: quit
        SystemManager::instance()->postExitRequest();
        return true;
    }
    else if(args[0] == "hint")
    {
        if(args[1] == "displayWand")
        {
            interp->queueCommand("getSceneManager().displayWand(0, 1)", true);
        }
    }
#ifdef OMEGA_ENABLE_AUTO_UPDATE
    else if(args[0] == "check_update")
    {
        win_sparkle_check_update_with_ui();
    }
#endif
    return false;
}

///////////////////////////////////////////////////////////////////////////////
// Application entry point
int main(int argc, char** argv)
{
    String applicationName = "orun";
    
    // Legacy default script (new apps should use launch script instead)
    oargs().newNamedString('s', "script", "script", "script to launch at startup", sDefaultScript);
    oargs().newNamedStringVector('x', "exec", "exec command", "Script command to execute after loading the script", sScriptCommand);

    Application<OmegaViewer> app(applicationName);
    app.setExecutableName(argv[0]);

#ifdef OMEGA_ENABLE_AUTO_UPDATE
// Convert the omegalib version to wide char (two macros needed for the substitution to work)
#define OMEGA_WIDE_VERSION(ver) OMEGA_WIDE_VERSION2(ver)
#define OMEGA_WIDE_VERSION2(ver) L##ver
    win_sparkle_set_appcast_url("https://raw.github.com/febret/omegalib-windows/master/omegalib-appcast.xml");
    win_sparkle_set_app_details(L"EVL", L"omegalib", OMEGA_WIDE_VERSION(OMEGA_VERSION));
    win_sparkle_init();
#endif

    int result = omain(app, argc, argv);

#ifdef OMEGA_ENABLE_AUTO_UPDATE
    win_sparkle_cleanup();
#endif

    return result;
}
