/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2015		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2015, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#include "eqinternal.h"
#include "omega/EqualizerDisplaySystem.h"

using namespace omega;
using namespace co::base;
using namespace std;

///////////////////////////////////////////////////////////////////////////////////////////////////
NodeImpl::NodeImpl( eq::Config* parent ):
	Node(parent),
	myServer(NULL)
{
	//omsg("[EQ] NodeImpl::NodeImpl");

	SystemManager* sys = SystemManager::instance();

	ApplicationBase* app = sys->getApplication();
	if(!sys->isMaster())
	{
		// This is the not master node. Create a standard server instance.
		myServer = new Engine(app);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool NodeImpl::configInit( const eq::uint128_t& initID )
{
	//ofmsg("[EQ] NodeImpl::configInit %1%", %initID);

	SystemManager* sys = SystemManager::instance();
	if(!sys->isMaster())
	{
		ConfigImpl* config = static_cast<ConfigImpl*>( getConfig());
		config->mapSharedData(initID);

		myServer->initialize();
		
		EqualizerDisplaySystem* eqds = (EqualizerDisplaySystem*)SystemManager::instance()->getDisplaySystem();
		eqds->finishInitialize(config, myServer);
	}

	return Node::configInit(initID);
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool NodeImpl::configExit()
{
	// If this is not a master node, call dispose on the engine object. 
	// Otherwise, ConfigImpl will take care of it.
	SystemManager* sys = SystemManager::instance();
	if(!sys->isMaster())
	{
		myServer->dispose();
	}
	return Node::configExit();
}

////////////////////////////////////////////////////////////////////////////////
void NodeImpl::frameStart( const eq::uint128_t& frameID, const uint32_t frameNumber )
{
	// If server is not NULL (only on slave nodes) call update here
	// on the master node, update is invoked in ConfigImpl.
	if(myServer != NULL)
	{
		ConfigImpl* config = (ConfigImpl*)getConfig();
		config->updateSharedData();

		const UpdateContext& uc = config->getUpdateContext();
		myServer->update(uc);
	}

	if(!getClient()->isConnected()) getClient()->exitLocal();
	
	// NOTE: This call NEEDS to stay after Engine::update, or frames will not update / display correctly.
	Node::frameStart(frameID, frameNumber);
}

////////////////////////////////////////////////////////////////////////////////
void NodeImpl::frameFinish( const eq::uint128_t& frameID, const uint32_t frameNumber )
{
    EqualizerDisplaySystem* eqds = (EqualizerDisplaySystem*)SystemManager::instance()->getDisplaySystem();
    if(eqds != NULL) eqds->frameFinished();
	Node::frameFinish(frameID, frameNumber);
}
