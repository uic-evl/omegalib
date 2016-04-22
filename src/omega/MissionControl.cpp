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
 * Classes that implement the mission control client and server.
 * Mission control is a protocol used to communicate to running omegalib 
 * instances, or let different instances to connect with each other and exchange
 * data or python commands.
 ******************************************************************************/
#include "omega/MissionControl.h"
#include "omega/PythonInterpreter.h"

// Needed for EventData
#include "connector/omicronConnectorClient.h"

using namespace omega;

#ifdef OMEGA_OS_LINUX
const int MissionControlServer::DefaultPort;
#endif

// Message id definitions
const char* MissionControlMessageIds::MyNameIs = "mnis";
const char* MissionControlMessageIds::Bye = "bye!";
const char* MissionControlMessageIds::ScriptCommand = "scmd";
const char* MissionControlMessageIds::StatRequest = "strq";
const char* MissionControlMessageIds::StatEnable = "sten";
const char* MissionControlMessageIds::StatUpdate = "stup";
const char* MissionControlMessageIds::LogMessage = "smsg";
const char* MissionControlMessageIds::ClientConnected = "ccon";
const char* MissionControlMessageIds::ClientDisconnected = "dcon";
const char* MissionControlMessageIds::ClientList = "clls";
const char* MissionControlMessageIds::Event = "ievt";
const char* MissionControlMessageIds::LogEnabled = "log1";
const char* MissionControlMessageIds::LogDisabled = "log0";


///////////////////////////////////////////////////////////////////////////////
MissionControlConnection::MissionControlConnection(ConnectionInfo ci, IMissionControlMessageHandler* msgHandler, MissionControlServer* server): 
    TcpConnection(ci),
    myServer(server),
    myMessageHandler(msgHandler),
    myRecipient(NULL),
    myLogEnabled(false)
{
    // Set initial buffer size to 1k
    myBufferSize = 1024;
    myBuffer = (char*)malloc(myBufferSize);
}

///////////////////////////////////////////////////////////////////////////////
MissionControlConnection::~MissionControlConnection()
{
    free(myBuffer);
    myBuffer = NULL;
}        

///////////////////////////////////////////////////////////////////////////////
void MissionControlConnection::setName(const String& name)
{
    myName = name;
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlConnection::setLogForwardingEnabled(bool value)
{
    myLogEnabled = value;
    // If we are not a server-side connection and we are connected, send a message
    // to tell the server of our updated log forwarding state.
    if(myServer == NULL && this->getState() == ConnectionOpen)
    {
        if(myLogEnabled) sendMessage(MissionControlMessageIds::LogEnabled, NULL, 0);
        else sendMessage(MissionControlMessageIds::LogDisabled, NULL, 0);
    }
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlConnection::handleData()
{
    // Read message header.
    char header[4];
    read(myBuffer, 4);
    memcpy(header, myBuffer, 4);

    // Read data length.
    int dataSize;
    read(myBuffer, 4);
    memcpy(&dataSize, myBuffer, 4);

    // Do we need to resize the incoming data buffer?
    if(myBufferSize < dataSize + 1)
    {
        myBufferSize = dataSize + 1;
        oflog(Verbose, "[MissionControlConnection] %1%: resizing buffer to %2%", 
            %myName %myBufferSize);
        free(myBuffer);
        myBuffer = (char*)malloc(myBufferSize);
    }
    
    // Read data.
    read(myBuffer, dataSize);
    myBuffer[dataSize] = '\0';

    // 'bye!' message closes the connection
    if(!strncmp(header, MissionControlMessageIds::Bye, 4)) 
    {
        close();
        return;
    }
    else if(!strncmp(header, MissionControlMessageIds::LogEnabled, 4))
    {
        myLogEnabled = true;
    }
    else if(!strncmp(header, MissionControlMessageIds::LogDisabled, 4))
    {
        myLogEnabled = false;
    }

    // Handle message locally, if a message handler is available.
    if(myMessageHandler != NULL) myMessageHandler->handleMessage(this, header, myBuffer, dataSize);

    // On a server, send the message to the server to be handled.
    if(myServer != NULL) myServer->handleMessage(header, myBuffer, dataSize, this);
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlConnection::handleClosed()
{
    oflog(Verbose, "Mission control connection %1% closed", %getConnectionInfo().id);
    if(myServer != NULL) myServer->closeConnection(this);
}
        
///////////////////////////////////////////////////////////////////////////////
void MissionControlConnection::handleConnected()
{
    TcpConnection::handleConnected();
    oflog(Verbose, "Mission control connection %1% open", %getConnectionInfo().id);
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlConnection::handleError(const ConnectionError& err)
{
    TcpConnection::handleError(err);
    //if(myServer != NULL) myServer->closeConnection(this);
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlConnection::sendMessage(const char* header, void* data, size_t size)
{
    write((void*)header, 4);
    uint32_t size32 = (int)size;
    write(&size32, sizeof(uint32_t));
    write(data, size);
}

///////////////////////////////////////////////////////////////////////////////////////////
void MissionControlConnection::goodbyeServer()
{
    sendMessage(MissionControlMessageIds::Bye, NULL, 0);
    waitClose();
}

///////////////////////////////////////////////////////////////////////////////////////////
void MissionControlServer::initialize() 
{
    TcpServer::initialize();
}

///////////////////////////////////////////////////////////////////////////////////////////
void MissionControlServer::dispose() 
{
    List< Ref<MissionControlConnection> > tmp = myConnections;

    foreach(MissionControlConnection* c, tmp)
    {
        c->close();
    }
    TcpServer::dispose();
}

///////////////////////////////////////////////////////////////////////////////////////////
TcpConnection* MissionControlServer::createConnection(const ConnectionInfo& ci)
{
    MissionControlConnection* conn = new MissionControlConnection(ci, myMessageHandler, this);
    myConnections.push_back(conn);

    // NOTE: the connection has no name (yet) so we wait notifying clients 
    // about the new connection until we get its name.
    // (see MissionControlServer::handleMessage)

    return conn;
}

///////////////////////////////////////////////////////////////////////////////////////////
void MissionControlServer::closeConnection(MissionControlConnection* conn)
{
    // Notify listener
    if(myListener != NULL) myListener->onClientDisconnected(conn->getName());

    myConnections.remove(conn);

    // Tell clients about the closed connection
    handleMessage(
        MissionControlMessageIds::ClientDisconnected, 
        (void*)conn->getName().c_str(), conn->getName().size());

    // Send an updated client name list to all connected clients.
    String namelist = "";
    foreach(MissionControlConnection* conn, myConnections)
    {
        namelist += conn->getName() + " ";
    }

    // Tell connected clients about the updated client list.
    handleMessage(
        MissionControlMessageIds::ClientList, 
        (void*)namelist.c_str(), namelist.size());
}

///////////////////////////////////////////////////////////////////////////////
MissionControlConnection* MissionControlServer::findConnection(const String& name)
{
    foreach(MissionControlConnection* conn, myConnections)
    {
        if(conn->getName() == name) return conn;
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlServer::handleMessage(const char* header, void* data, size_t size, MissionControlConnection* sender)
{
    if(!strncmp(header, MissionControlMessageIds::MyNameIs, 4)) 
    {
        // If this connection currently has no name, it is a new connection:
        // handle it differently.
        bool isNewConnection = sender->getName() == "";

        //name string
        String name((char*)data);
        sender->setName(name);
        oflog(Verbose, "Mission Control: client %1% name changed to %2%", 
            %sender->getConnectionInfo().id
            %name);

        // Send an updated client name list to all connected clients.
        String namelist = "";
        foreach(MissionControlConnection* conn, myConnections)
        {
            namelist += conn->getName() + " ";
        }

        // If this is a new connection, notify all clients of it.
        if(isNewConnection)
        {
            handleMessage(
                MissionControlMessageIds::ClientConnected, 
                (void*)name.c_str(), name.size());

            // Notify listener
            if(myListener != NULL) myListener->onClientConnected(name);
        }


        // Tell connected clients about the updated client list.
        handleMessage(
            MissionControlMessageIds::ClientList, 
            (void*)namelist.c_str(), namelist.size());
    }
    else
    {
        // If the message is a script command and the command begins with '@',
        // the message first word is a client id (in the form @client:): send a message only to that
        // client.
        if(!strncmp(header, MissionControlMessageIds::ScriptCommand, 4) 
            && ((char*)data)[0] == '@')
        {
            char* str = (char*)data;
            char* sp = strchr(&str[1], ':');
            if(sp != NULL)
            {
                *sp = '\0';
                String clientId = &str[1];
                // If clientId ends with '*' we use clientId as a prefix to
                // send a message to multiple clients.
                bool prefix = false;
                if(StringUtils::endsWith(clientId, "*")) 
                {
                    prefix = true;
                    *(sp - 1) = '\0';
                    clientId = &str[1];
                }
                String cmd = (sp + 1);
                foreach(MissionControlConnection* conn, myConnections)
                {
                    if(conn->getState() == TcpConnection::ConnectionOpen 
                        && conn != sender
                        && ((conn->getName() == clientId) ||
                           (prefix && StringUtils::startsWith(conn->getName(), clientId))))
                    {
                        conn->sendMessage(
                            header, 
                            (void*)cmd.c_str(), cmd.size());
                    }
                }
            }
        }
        else
        {
            // By default, broadcast the message to all other connected clients.
            foreach(MissionControlConnection* conn, myConnections)
            {
                if(conn->getState() == TcpConnection::ConnectionOpen && conn != sender)
                {
                    conn->sendMessage(header, data, size);
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlServer::addLine(const String& line)
{
    foreach(MissionControlConnection* conn, myConnections)
    {
        if(conn->getState() == TcpConnection::ConnectionOpen && conn->isLogForwardingEnabled())
        {
            conn->sendMessage(MissionControlMessageIds::LogMessage, (void*)line.c_str(), line.size());
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlServer::broadcastEvent(const Event& evt, MissionControlConnection* sender)
{
    omicronConnector::EventData ed;
    size_t sz = evt.serialize(&ed);
    foreach(MissionControlConnection* conn, myConnections)
    {
        if(conn->getState() == TcpConnection::ConnectionOpen && conn != sender)
        {
            conn->sendMessage(MissionControlMessageIds::Event, &ed, sz);
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlServer::sendEventTo(const Event& evt, MissionControlConnection* target)
{
    oassert(target != NULL);
    omicronConnector::EventData ed;
    size_t sz = evt.serialize(&ed);
    target->sendMessage(MissionControlMessageIds::Event, &ed, sz);
}

///////////////////////////////////////////////////////////////////////////////
MissionControlClient* MissionControlClient::create()
{
    MissionControlClient* missionControlClient = new MissionControlClient();
    ModuleServices::addModule(missionControlClient);
    return missionControlClient;
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlClient::initialize()
{
    if(myConnection == NULL)
    {
        myConnection = new MissionControlConnection(
            ConnectionInfo(myIoService), this, NULL);
        myConnection->setName(myName);
    }
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlClient::dispose()
{
    if(myConnection != NULL)
    {
        if(myConnection->getState() == TcpConnection::ConnectionOpen)
        {
            // SInce we are disposing this connection, just send a goodbye
            // message without waiting for the actual connection to close.
            myConnection->sendMessage(MissionControlMessageIds::Bye, NULL, 0);
        }
        myConnection = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlClient::update(const UpdateContext& context)
{
    myConnection->poll();
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlClient::connect(const String& host, int port)
{
    if(myConnection == NULL)
    {
        initialize();
    }
    myHost = host;
    myPort = port;
    myConnection->open(host, port);
    if(isConnected())
    {
        myConnection->sendMessage(
            MissionControlMessageIds::MyNameIs, 
            (void*)myName.c_str(), myName.size());

        // Force-send an update on the log forwarding status for this client.
        if(myConnection->isLogForwardingEnabled())
        {
            myConnection->setLogForwardingEnabled(true);
        }
    }
}


///////////////////////////////////////////////////////////////////////////////
void MissionControlClient::postCommand(const String& cmd)
{
    if(myConnection->getState() == TcpConnection::ConnectionOpen)
    {
        String trimmed = cmd;
        StringUtils::trim(trimmed);
        myConnection->sendMessage(
            MissionControlMessageIds::ScriptCommand, 
            (void*)trimmed.c_str(), trimmed.size());
    }
}

///////////////////////////////////////////////////////////////////////////////
bool MissionControlClient::isConnected()
{
    return myConnection != NULL && 
        myConnection->getState() == TcpConnection::ConnectionOpen;
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlClient::closeConnection()
{
    if(isConnected())
    {
        myConnection->goodbyeServer();
        myConnection = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlClient::setName(const String& name)
{
    myName = name;
    if(myConnection != NULL)
    {
        myConnection->setName(name);
    }
    if(isConnected())
    {
        myConnection->sendMessage(
            MissionControlMessageIds::MyNameIs, 
            (void*)name.c_str(), name.size());
    }
}

///////////////////////////////////////////////////////////////////////////////
String MissionControlClient::getName()
{
    myConnection->getName();
    return myName;
}

///////////////////////////////////////////////////////////////////////////////
vector<String>& MissionControlClient::listConnectedClients()
{
    return myConnectedClients;
}

///////////////////////////////////////////////////////////////////////////////
bool MissionControlClient::handleMessage(
    MissionControlConnection* sender, const char* header, char* data, int size)
{
    bool handled = true;

    PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();

    if(!strncmp(header, MissionControlMessageIds::ScriptCommand, 4)) 
    {
        //script command message
        String command(data);
        if(interp != NULL)
        {
            interp->queueCommand(command);
        }
    }
    else if(!strncmp(header, MissionControlMessageIds::ClientList, 4)) 
    {
        String list(data);
        // Split string into client names.
        myConnectedClients = StringUtils::split(list, " ");
        if(interp != NULL && !myClientListUpdatedCommand.empty())
        {
            interp->queueCommand(myClientListUpdatedCommand);
        }
    }
    else if(!strncmp(header, MissionControlMessageIds::ClientConnected, 4))
    {
        String clid(data);

        // If WE are the client that just connected, avoid notifications.
        if(clid != myName)
        {
            ofmsg("Mission control client connected: %1%", %clid);
        }
        if(myListener != NULL) myListener->onClientConnected(clid);
        if(interp != NULL && !myClientConnectedCommand.empty())
        {
            String cmd = StringUtils::replaceAll(
                myClientConnectedCommand,
                "%clientId%",
                clid);
            interp->queueCommand(cmd);
        }
    }
    else if(!strncmp(header, MissionControlMessageIds::ClientDisconnected, 4))
    {
        String clid(data);
        
        ofmsg("Mission control client disconnected: %1%", %clid);
        if(myListener != NULL) myListener->onClientDisconnected(clid);
        if(interp != NULL && !myClientDisconnectedCommand.empty())
        {
            String cmd = StringUtils::replaceAll(
                myClientDisconnectedCommand,
                "%clientId%",
                clid);
            interp->queueCommand(cmd);
        }
    }
    else if(!strncmp(header, MissionControlMessageIds::Event, 4))
    {
        omicronConnector::EventData ed;
        memcpy(&ed, data, size);
        
        ServiceManager* sm = SystemManager::instance()->getServiceManager();
        sm->lockEvents();

        // Post event to service manager
        Event* e = sm->writeHead();
        e->deserialize(&ed);
        sm->unlockEvents();
    }

    //if(!strncmp(header, "help", 4)) 
    //{
    //	// Request for help string.
    //	String command(myBuffer);
    //	PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
    //	if(interp != NULL)
    //	{
    //		String helpString = interp->getHelpString("");
    //		sendMessage("help", (void*)helpString.c_str(), helpString.size());
    //	}
    //}
    if(!strncmp(header, MissionControlMessageIds::StatRequest, 4)) 
    {
        // Request for stats names.
        String statIds = "";
        StatsManager* sm = SystemManager::instance()->getStatsManager();
        if(sm != NULL)
        {
            foreach(Stat* s, sm->getStats())
            {
                statIds.append(s->getName());
                statIds.append("|");
            }
            sender->sendMessage(MissionControlMessageIds::StatRequest, (void*)statIds.c_str(), statIds.size());
        }
    }
    if(!strncmp(header, MissionControlMessageIds::StatEnable, 4)) 
    {
        StatsManager* sm = SystemManager::instance()->getStatsManager();
        if(sm != NULL)
        {
            // Set enabled stats.
            String stats(data);
            myEnabledStats.clear();
            std::vector<String> statVector = StringUtils::tokenise(stats, " ");
            foreach(String statId, statVector)
            {
                Stat* s = sm->findStat(statId);
                if(s != NULL)
                {
                    myEnabledStats.push_back(s);
                }
            }
        }
    }
    if(!strncmp(header, MissionControlMessageIds::StatUpdate, 4)) 
    {
        if(myEnabledStats.size() > 0)
        {
            // Request for stats update.
            String statIds = "";
            foreach(Stat* s, myEnabledStats)
            {
                statIds.append(ostr("%1% %2% %3% %4% %5% ", %s->getName() %(int)s->getCur() %(int)s->getMin() %(int)s->getMax() %(int)s->getAvg()));
            }
            sender->sendMessage(MissionControlMessageIds::StatUpdate, (void*)statIds.c_str(), statIds.size());
        }
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
void MissionControlClient::setLogForwardingEnabled(bool value)
{
    myConnection->setLogForwardingEnabled(true);
}

///////////////////////////////////////////////////////////////////////////////
bool MissionControlClient::isLogForwardingEnabled()
{
    return myConnection->isLogForwardingEnabled();
}

///////////////////////////////////////////////////////////////////////////////
bool MissionControlClient::spawn(const String& id, int slot, const String& script, const String& config)
{
    String fullPath;
    if(DataManager::findFile(script, fullPath))
    {
        // get an app name based on the script
        String appname;
        String path;
        String ext;
        StringUtils::splitFullFilename(script, appname, ext, path);

        String cmd = ostr("%1% -c %2% -s %3% -N %4% -I %5% --mc @%6%:%7% --interactive-off",
            %ogetexecpath()
            %config
            %fullPath
            %id
            %slot
            %myHost
            %myPort);

        ofmsg("[MissionControlClient::spawn] running %1%:%2%", %id %script);
        olaunch(cmd);

        return true;
    }
    return false;
}

