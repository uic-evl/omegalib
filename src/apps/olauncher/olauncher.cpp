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
 *	odaemon
 *		Implements a service for syncing and launching applications from 
 *      a remote client.
 ******************************************************************************/
#include <omega.h>

using namespace omega;

unsigned int AssetCachePort = 22501;
unsigned int MissionControlPort = 22502;

///////////////////////////////////////////////////////////////////////////////
// Python script used to generate the application file list
const char* listFilesFunction =
    "import os\n" 
    "def listFiles(scriptDir):\n" 
    "   result = ''\n"
    "   for root,dirs,files in os.walk(scriptDir):\n"
    "      for f in files:\n"
    "         result = result + root + '/' + f + ' '\n"
    "   return result\n";

///////////////////////////////////////////////////////////////////////////////
// Launch the specified script on the target machine
void launch(const String& scriptName, const String& host, bool forceAssetRefresh)
{
    // Find the full path of the script
    String fullScriptPath;
    if(DataManager::findFile(scriptName, fullScriptPath))
    {
        // Create and initialize the python interpreter. We will use it to
        // generate a manifest (file list) of the application assets that need
        // to be synchronized.
        Ref<PythonInterpreter> interpreter = new PythonInterpreter();
        interpreter->initialize("olauncher");
        interpreter->eval(listFilesFunction);

        // Find the directory containing the script, so we can enumerate the 
        // contained files.
        String scriptDir;
        String scriptFilename;
        String scriptExtension;
        StringUtils::splitFullFilename(fullScriptPath, scriptFilename, scriptExtension, scriptDir);

        // Run the python script to obtain the list of files in the script 
        // directory.
        char* fileList = NULL;
        interpreter->eval("listFiles('" + scriptDir + "')", "z", &fileList);

        Vector<String> fileVector = StringUtils::split(fileList, " ");

        // Run synchronization
        Ref<AssetCacheManager> acm = new AssetCacheManager();
        acm->addCacheHost(host);
        acm->setCachePort(AssetCachePort);
        acm->setCacheName(scriptFilename);
        acm->setForceOverwrite(forceAssetRefresh);

        foreach(String file, fileVector)
        {
            acm->addFileToCacheList(file);
        }

        acm->setVerbose(true);
        acm->sync();

        // Use MissionControl to signal the launcher daemon to start the script
        asio::io_service ioService;
        Ref<MissionControlConnection> conn = new MissionControlConnection(ConnectionInfo(ioService), NULL, NULL);
        int port = MissionControlPort;
        conn->open(host, port);
        String commandArgs = ostr("%1%/%2%.py", %scriptFilename %scriptFilename);
        if(conn->getState() == TcpConnection::ConnectionOpen)
        {
            conn->sendMessage("orun", (void*)commandArgs.c_str(), commandArgs.size());
            conn->goodbyeServer();
            omsg("Said goodbye");
        }
    }
    else
    {
        ofmsg("Could not find script %1%", %scriptName);
    }
}

//////////////////////////////////////////////////////////////////////////////
// Handler for launch commands on the daemon side of the launcher
class DaemonCommandHandler: public IMissionControlMessageHandler
{
public:
    String cacheRoot;

    DaemonCommandHandler(const String cr): cacheRoot(cr)
    {}

    virtual bool handleMessage(
        MissionControlConnection* sender, 
        const char* header, char* data, int size)
    {
        if(!strncmp(header, "orun", 4))
        {
            // data contains the full arguments for orun.
            olaunch(ostr("./orun -D - -s %1%/%2%", %cacheRoot %data));
        }
        return true;
    }
};
            
//////////////////////////////////////////////////////////////////////////////
// Run the launcher daemon
void launcherDaemon(const String& cacheRoot)
{
    Ref<AssetCacheService> cacheService = new AssetCacheService();
    cacheService->setCacheRoot(cacheRoot);
    cacheService->setPort(AssetCachePort);
    cacheService->initialize();
    cacheService->start();

    Ref<MissionControlServer> server = new MissionControlServer();
    server->setPort(MissionControlPort);
    server->setMessageHandler(new DaemonCommandHandler(cacheRoot));
    server->initialize();
    server->start();

    while(true)
    {
        server->poll(); 
        cacheService->poll();
        osleep(100);
    }

    server->stop();
    server->dispose();

    cacheService->stop();
    cacheService->dispose();
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    bool forceAssetRefresh = false;
    String scriptName;
    String serverHost = "localhost";
    String daemonCacheRoot;

    libconfig::ArgumentHelper ah;
    ah.newNamedString('d', "daemon", "daemon", "runs the launcher daemon, using the specified cache root", daemonCacheRoot);
    ah.newNamedString('s', "script", "script", "the script name", scriptName);
    ah.newNamedString('h', "host", "host", "the address of the application host", serverHost);
    ah.newFlag('r', "refresh", "forces asset refresh on server", forceAssetRefresh);
    ah.process(argc, argv);

    // Initialize the data manager
    DataManager* dataManager = DataManager::getInstance();
    dataManager->addSource(new FilesystemDataSource("./"));
    dataManager->addSource(new FilesystemDataSource(""));
    String modulePath = OMEGA_HOME;
    modulePath = modulePath + "/modules";
    dataManager->addSource(new FilesystemDataSource(modulePath));

    if(daemonCacheRoot != "")
    {
        launcherDaemon(daemonCacheRoot);
    }
    else
    {
        launch(scriptName, serverHost, forceAssetRefresh);
    }
    dataManager->cleanup();
    return 0;
}

