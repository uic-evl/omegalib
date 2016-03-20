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
#ifndef __MISSION_CONTROL_H__
#define __MISSION_CONTROL_H__

#include "omega/osystem.h"
#include "omega/StatsManager.h"
#include "omega/ModuleServices.h"
#include "omega/PythonInterpreter.h"
#include "omicron/Tcp.h"

namespace omega {
    
    class MissionControlServer;
    class MissionControlConnection;

    ///////////////////////////////////////////////////////////////////////////
    //! Stores the 4-character ids of messages used by the Mission Control protocol.
    class OMEGA_API MissionControlMessageIds
    {
    public:
        //! bye! - tells the server this connection is over and will be closed 
        //!after this message.
        static const char* Bye;
        //! mnis <name> - updates the name the server will use to identify 
        //!this connection.
        static const char* MyNameIs;
        //! smsg <string> - sent by the server to a client: contains a log or 
        //!response message that can be printed to the local console.
        static const char* LogMessage;
        //! ccon <name> - Sent by the server to all clients when a new client 
        //! connects.
        static const char* ClientConnected;
        //! dcon <name> - Sent by the server to all clients when a client 
        //! disconnects.
        static const char* ClientDisconnected;
        //! clls <client list> - Sent by the server to clients, contains a list
        //! of space-separated client names for all the clients currently 
        //! connected to the server.
        static const char* ClientList;
        //! log1 <log enabled> - Sent by clients to server, tells server to forward
        //! log messages to this client.
        static const char* LogEnabled;
        //! log0 <log disabled> - Sent by clients to server, tells server to disable
        //! forwarding of log messages to this client.
        static const char* LogDisabled;


        //! scmd <command> - default behavior (see MissionControlMessageHandler): 
        //! the receiver will dispatch <command> to the script interpreter.
        static const char* ScriptCommand;
        //! strq [statname][|statname]* - default behavior 
        //! (see MissionControlMessageHandler): the receiver will send back a 
        //! strq message with a list of pipe | separated stat names
        static const char* StatRequest;
        //! sted [statname]+ - default behavior 
        //! (see MissionControlMessageHandler): the receiver will enable a set 
        //! of statistics whose data will be returned back for each stat 
        //! update message
        static const char* StatEnable;
        //! stup [name cur min max avg]* - default behavior 
        //! (see MissionControlMessageHandler): the receiver will send back a 
        //! stup message with current data (name, min, max, average 
        //! times / values) about statistics enabled by a sten message.
        static const char* StatUpdate;
        //! ievt [binary event data] - default behavior
        //! (see MissionControlMessageHandler): the receiver will deserialize
        //! and forward an Event to the ServiceManager.
        static const char* Event;

    private:
        //! Can't be instantiated.
        MissionControlMessageIds() {}
    };

    ///////////////////////////////////////////////////////////////////////////
    //! Interface for classes that handle mission control messages. Useful
    //! for extending the deault mission control protocol message set with new
    //! message formats.
    class OMEGA_API IMissionControlMessageHandler
    {
    public:
        virtual bool handleMessage(
            MissionControlConnection* sender, 
            const char* header, char* data, int size) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////
    //! Interface for classes that listen to mission control events like client
    //! connections, disconnections and name changes.
    class OMEGA_API IMissionControlListener
    {
    public:
        virtual void onClientConnected(const String& clientId) = 0;
        virtual void onClientDisconnected(const String& clientId) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API MissionControlConnection: public TcpConnection
    {
    public:
        MissionControlConnection(
            ConnectionInfo ci, IMissionControlMessageHandler* msgHandler, 
            MissionControlServer* server);
        virtual ~MissionControlConnection();

        virtual void handleData();
        virtual void handleClosed();
        virtual void handleConnected();
        virtual void handleError(const ConnectionError& err);

        void sendMessage(const char* header, void* data, size_t size);
        //! Client side: tells the server we are done talking and waits for graceful close.
        void goodbyeServer();

        String getName() { return myName; }
        virtual void setName(const String& name);

        bool isLogForwardingEnabled() { return myLogEnabled; }
        void setLogForwardingEnabled(bool value);

    private:
        char* myBuffer;
        uint myBufferSize;
        MissionControlServer* myServer;
        MissionControlConnection* myRecipient; // Message destination when private-message mode is enabled.
        IMissionControlMessageHandler* myMessageHandler;
        String myName;
        bool myLogEnabled;
    };

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API MissionControlServer: public TcpServer, public ILogListener
    {
    public:
        static const int DefaultPort = 22500;
    public:
        MissionControlServer():
            myMessageHandler(NULL), myListener(NULL)
          {}

        virtual void initialize();
        virtual void dispose();

        virtual TcpConnection* createConnection(const ConnectionInfo& ci);
        void closeConnection(MissionControlConnection* conn);
        MissionControlConnection* findConnection(const String& name);
        void handleMessage(const char* header, void* data, size_t size, MissionControlConnection* sender = NULL);
        void setMessageHandler(IMissionControlMessageHandler* msgHandler) { myMessageHandler = msgHandler; }

        void setListener(IMissionControlListener* l) { myListener = l; }
        IMissionControlListener* getListener() { return myListener; }
    
        // from ILogListener
        virtual void addLine(const String& line);

        //! Broadcasts an event. If sender is not null, the event not be sent
        //! to the specified client. 
        void broadcastEvent(const Event& evt, MissionControlConnection* sender = NULL);
        //! Send an event to the specified client.
        void sendEventTo(const Event& evt, MissionControlConnection* target);

    private:
        List< Ref<MissionControlConnection> > myConnections;
        IMissionControlMessageHandler* myMessageHandler;
        IMissionControlListener* myListener;
    };

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API MissionControlClient: public EngineModule,
        public IMissionControlMessageHandler
    {
    public:
        //! Utility method: creates a new client and registers it as a module.
        static MissionControlClient* create();

    public:
        MissionControlClient(): 
          EngineModule("MissionControlClient"), myName("client"), myListener(NULL) {}
        virtual ~MissionControlClient() 
        { 
            // We make sure the connection object is destroyed here. This is
            // important because if we let default destruction take place, the 
            // io_service object may get destroyed before the connection that 
            // uses it, leading to a crash. io_service is declared before the 
            // connection object in the class members so things would work 
            // anyways, but it's better to be explicit. In the future it would
            // be good to keep this required behavior hidden in a base 
            // TcpClient class.
            dispose(); 
        }

        virtual void dispose();
        virtual void initialize();
        virtual void update(const UpdateContext& context);
        void connect(const String& host, int port);
        void postCommand(const String& command);
        bool isConnected();
        void closeConnection();
        void setName(const String& name);
        String getName();
        vector<String>& listConnectedClients();

        void setClientConnectedCommand(const String& cmd);
        void setClientDisconnectedCommand(const String& cmd);
        void setClientListUpdatedCommand(const String& cmd);

        void setListener(IMissionControlListener* l) { myListener = l; }
        IMissionControlListener* getListener() { return myListener; }

        //! Enable or disable log forwarding. When log forwarding is enabled and
        //! this client is connected, server log messages will be received and
        //! printed on this client's console.
        void setLogForwardingEnabled(bool value);
        bool isLogForwardingEnabled();

        // IMissionControlMessageHandler override
        virtual bool handleMessage(
            MissionControlConnection* sender, 
            const char* header, char* data, int size);

        //! Starts a new instance of omegalib using the specified script and
        //! configuration file. The instance will use mission control to connect
        //! to the same server this instance is connected to.
        //! @returns true if the script has been launched successfully. Note: this
        //! does not mean the instance is already running and connected back. You 
        //! will need to use the clientConnected method of IMissionControlMessageHandler for that.
        bool spawn(const String& id, int slot, const String& script, const String& config);

    private:
        String myName;
        vector<String> myConnectedClients;

        // Script callbacks
        String myClientConnectedCommand;
        String myClientDisconnectedCommand;
        String myClientListUpdatedCommand;

        asio::io_service myIoService;
        Ref<MissionControlConnection> myConnection;
        List<Stat*> myEnabledStats;
        IMissionControlListener* myListener;

        String myHost;
        int myPort;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline void MissionControlClient::setClientConnectedCommand(const String& cmd)
    { myClientConnectedCommand = cmd; }

    ///////////////////////////////////////////////////////////////////////////
    inline void MissionControlClient::setClientDisconnectedCommand(const String& cmd)
    { myClientDisconnectedCommand = cmd; }

    ///////////////////////////////////////////////////////////////////////////
    inline void MissionControlClient::setClientListUpdatedCommand(const String& cmd)
    { myClientListUpdatedCommand = cmd; }
}; // namespace omicron

#endif