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
 * The omegalib Engine is the core runtime component of omegalib. It runs on 
 * each node of a cluster system and handles the abstract scene graph, cameras, 
 * distribution of events and frame updates. 
 ******************************************************************************/
#include "omega/Engine.h"
#include "omega/Application.h"
#include "omega/Renderable.h"
#include "omega/SystemManager.h"
#include "omega/DisplaySystem.h"
#include "omega/ImageUtils.h"
#include "omega/SystemManager.h"
#include "omega/PythonInterpreter.h"
#include "omega/CameraController.h"
#include "omega/Console.h"

using namespace omega;

Engine* Engine::mysInstance = NULL;

// Variables used by the death switch thread.
Thread* sDeathSwitchThread = NULL;
bool sUpdateReceived = true;
int sDeathSwitchTimeout = 240; // Auto-kill after 4 minutes

///////////////////////////////////////////////////////////////////////////////
// Thsi thread runs on slave nodes and monitors the update loop. If no updates 
// have been received within a specified interval, it kills the process. This
// is useful for auto-killing stuck slave nodes.
class DeathSwitchThread: public Thread
{
public:
	virtual void threadProc()
	{
		while(!SystemManager::instance()->isExitRequested())	
		{
			// Check every second
			osleep(sDeathSwitchTimeout * 1000);
			if(!sUpdateReceived)
			{
				ofmsg("DeathSwitchThread: no update after %1% seconds. Killing process.", %sDeathSwitchTimeout);
				exit(-1);
			}
			sUpdateReceived = false;
		}
	}
};

///////////////////////////////////////////////////////////////////////////////
Engine::Engine(ApplicationBase* app):
    //myActivePointerTimeout(2.0f),
    myApplication(app),
    myDefaultCamera(NULL),
	//myPointerMode(PointerModeWand)
	myDrawPointers(false),
	myPrimaryButton(Event::Button3),
	myEventDispatchEnabled(true),
	soundEnv(NULL)
{
    mysInstance = this;
}

///////////////////////////////////////////////////////////////////////////////
Engine::~Engine()
{
	omsg("~Engine");
}

///////////////////////////////////////////////////////////////////////////////
void Engine::initialize()
{
    myLock.lock();
    ImageUtils::internalInitialize();

	ModuleServices::addModule(new EventSharingModule());

    myScene = new SceneNode(this, "root");

    // Create console.
	myConsole = Console::createAndInitialize();

	Config* syscfg = getSystemManager()->getSystemConfig();
    Config* cfg = getSystemManager()->getAppConfig();

	Setting& syscfgroot = syscfg->lookup("config");

    // Setup the system font.
	// Look in the app config first
    if(cfg->exists("config/defaultFont"))
    {
        Setting& fontSetting = cfg->lookup("config/defaultFont");
        setDefaultFont(FontInfo("default", fontSetting["filename"], fontSetting["size"]));
    }
	else
	{
		if(syscfg->exists("config/defaultFont"))
		{
			Setting& fontSetting = syscfg->lookup("config/defaultFont");
			setDefaultFont(FontInfo("default", fontSetting["filename"], fontSetting["size"]));
		}
		else
		{
			// If all else fails, set a default fallback font.
			setDefaultFont(FontInfo("console", "fonts/arial.ttf", 12));
		}
	}

	// Read draw pointers option.
	myDrawPointers = syscfg->getBoolValue("config/drawPointers", myDrawPointers);
	myPointerSize = Config::getIntValue("pointerSize", syscfgroot, 32);

    myDefaultCamera = new Camera(this);
	myDefaultCamera->setName("DefaultCamera");
	// By default attach camera to scene root.
	myScene->addChild(myDefaultCamera);

	// Load camera config form system config file
	// camera section = default camera only
	if(syscfg->exists("config/camera"))
	{
        Setting& s = syscfg->lookup("config/camera");
		myDefaultCamera->setup(s);
	}

	// Load camera config form system config file
	// cameras section = multiple camera specifications
	if(syscfg->exists("config/cameras"))
	{
        Setting& s = syscfg->lookup("config/cameras");
		for(int i = 0; i < s.getLength(); i++)
		{
			Setting& sc = s[i];
			String camName = sc.getName();
			if(camName == "default") 
			{
				myDefaultCamera->setup(sc);
			}
			else
			{
				Camera* cam = getCamera(camName);
				if(cam == NULL) cam = createCamera(camName);
				cam->setup(sc);
			}
		}
	}

	// Load sound config from system config file
	// On distributed systems, this is executed only on the master node
	soundEnabled = false;
	if(SystemManager::instance()->isMaster())
	{
		if(syscfg->exists("config/sound"))
		{
			soundEnabled = true;
			Setting& s = syscfg->lookup("config/sound");

			String soundServerIP = Config::getStringValue("soundServerIP", s, "localhost");
			int soundServerPort = Config::getIntValue("soundServerPort", s, 57120);

			float volumeScale = Config::getFloatValue("volumeScale", s, 0.5);

			// Config in seconds, function below in milliseconds
			soundServerCheckDelay = Config::getFloatValue("soundServerReconnectDelay", s, 5) * 1000;

			soundManager = new SoundManager(soundServerIP,soundServerPort);
			soundEnv = soundManager->getSoundEnvironment();
			soundManager->startSoundServer();

			ofmsg("Engine: Checking if sound server is ready at %1% on port %2%... (Waiting for %3% seconds)", %soundServerIP %soundServerPort %(soundServerCheckDelay/1000));

			bool serverReady = true;
			timeb tb;
			ftime( &tb );
			int curTime = tb.millitm + (tb.time & 0xfffff) * 1000;
			lastSoundServerCheck = curTime;

			while( !soundManager->isSoundServerRunning() )
			{
				timeb tb;
				ftime( &tb );
				curTime = tb.millitm + (tb.time & 0xfffff) * 1000;
				int timeSinceLastCheck = curTime-lastSoundServerCheck;

				//soundManager->startSoundServer();
				if( timeSinceLastCheck > soundServerCheckDelay )
				{
					omsg("Engine: Failed to start sound server. Sound disabled.");
					serverReady = false;
					break;
				}
			}

			if( serverReady )
			{
				initializeSound();
			}
		}
		else
		{
			// Still create the SoundManager and SoundEnvironment to allow
			// sound apps to run with sound disabled.
			soundManager = new SoundManager();
			soundEnv = soundManager->getSoundEnvironment();
			omsg("Engine: Running with sound disabled.");
			if(syscfg->exists("config/sound"))
			{
				soundEnabled = true;
			}
		}
	}
	else
	{
		// Still create the SoundManager and SoundEnvironment to allow
		// sound apps to run with sound disabled.
		soundManager = new SoundManager();
		soundEnv = soundManager->getSoundEnvironment();
		omsg("Engine: Running with sound disabled.");
		if(syscfg->exists("config/sound"))
		{
			soundEnabled = true;
		}
	}
	soundManager->setup( syscfg->getRootSetting()["config"] );

	// Load input mapping
	if(syscfg->exists("config/inputMap"))
	{
        Setting& s = syscfg->lookup("config/inputMap");
		myPrimaryButton = Event::parseButtonName(Config::getStringValue("confirmButton", s, "Button3"));
	}


	// Load camera config form application config file (if it is different from system configuration)
    if(cfg != syscfg && cfg->exists("config/camera"))
    {
        Setting& s = cfg->lookup("config/camera");
		myDefaultCamera->setup(s);
    }


	Setting& scfg = cfg->lookup("config");
	myEventSharingEnabled = Config::getBoolValue("enableEventSharing", scfg, true);
	
    sDeathSwitchTimeout = Config::getIntValue("deathSwitchTimeout", syscfgroot, sDeathSwitchTimeout);
	ofmsg("Death switch timeout: %1% seconds", %sDeathSwitchTimeout);

	// Initialize the default camera using the 
	//Observer* obs = getDisplaySystem()->getObserver(0);
	//myDefaultCamera->setPosition(obs->getHeadPosition());
	myDefaultCamera->setPosition(Vector3f::Zero());

	// All the engine modules loaded after this point are marked non-core:
	// This means that they will be unloaded during an application reset
	// (see Engine::reset)
	ModuleServices::setNonCoreMode();

	// Setup stats
	StatsManager* sm = getSystemManager()->getStatsManager();
    myHandleEventTimeStat = sm->createStat("Engine handleEvent", StatsManager::Time);
    myUpdateTimeStat = sm->createStat("Engine update", StatsManager::Time);
    mySceneUpdateTimeStat = sm->createStat("Scene transform update", StatsManager::Time);
    myModuleUpdateTimeStat = sm->createStat("Modules update", StatsManager::Time);

	myLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void Engine::dispose()
{
	omsg("Engine::dispose");
	
	if(sDeathSwitchThread != NULL)
	{
		delete sDeathSwitchThread;
		sDeathSwitchThread = NULL;
	}

    ImageUtils::internalDispose();
	ModuleServices::disposeAll();

	// Destroy pointers.
	myPointers.clear();

	// Clear renderer list.
	myClients.clear();

	// Clear root scene node.
	myScene = NULL;

	ofmsg("Engine::dispose: cleaning up %1% cameras", %myCameras.size());
	myCameras.clear();
	myDefaultCamera = NULL;
}

///////////////////////////////////////////////////////////////////////////////
void Engine::reset()
{
	// dispose non-core modules
	ModuleServices::disposeNonCoreModules();

	// Remove all children from the scene root.
	myScene->removeAllChildren();
	myDefaultCamera->removeAllChildren();
	// Re-attach the default camera to the scene root.
	myScene->addChild(myDefaultCamera);

	// Load camera config form application config file (if it is different from system configuration)
	// Then in the system config
	Config* syscfg = getSystemManager()->getSystemConfig();
    Config* cfg = getSystemManager()->getAppConfig();
    if(cfg != syscfg && cfg->exists("config/camera"))
    {
        Setting& s = cfg->lookup("config/camera");
		myDefaultCamera->setup(s);
    }
	
	if(soundEnv != NULL)
	{
		soundEnv->getSoundManager()->stopAllSounds();
		soundEnv->getSoundManager()->cleanupAllSounds();
	}
}

///////////////////////////////////////////////////////////////////////////////
void Engine::addRenderer(Renderer* client)
{
    oassert(client != NULL);
    myClients.push_back(client);
}

///////////////////////////////////////////////////////////////////////////////
void Engine::removeRenderPass(const String& renderPassName)
{
	ofmsg("Engine: removing render pass %1%", %renderPassName);
	foreach(Renderer* r, myClients)
	{
		RenderPass* rp = r->getRenderPass(renderPassName);
		if(rp != NULL)
		{
			r->removeRenderPass(rp);
		}
		else
		{
			ofwarn("Engine::removeRenderPass: could not find render pass %1%", %renderPassName);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void Engine::refreshPointer(int pointerId, const Event& evt)
{
	Vector3f pos = evt.getPosition();

	Pointer* ptr = NULL;
	if(myPointers.find(pointerId) == myPointers.end())
	{
		ofmsg("Engine::refreshPointer: creating pointer %1%", %pointerId);
		ptr = new Pointer();
		ptr->setSize(myPointerSize);
		myPointers[pointerId] = ptr;
		ptr->initialize(this);
	}
	else
	{
		ptr = myPointers[pointerId];
}
	ptr->setVisible(true);
	ptr->setPosition(pos[0], pos[1]);
}

///////////////////////////////////////////////////////////////////////////////
SceneNode* Engine::getScene()
{
    return myScene.get();
}

///////////////////////////////////////////////////////////////////////////////
void Engine::setEventDispatchEnabled(bool value)
{
	myEventDispatchEnabled = value;
}

///////////////////////////////////////////////////////////////////////////////
void Engine::handleEvent(const Event& evt)
{
	// If event dispatch is disabled, ignore all events.
	if(!myEventDispatchEnabled) return;

	myHandleEventTimeStat->startTiming();

	// Python events are processed with normal priority. First pass them to modules
	// With higher priority...
	ModuleServices::handleEvent(evt, EngineModule::PriorityHighest);
	if(!evt.isProcessed()) 
		ModuleServices::handleEvent(evt, EngineModule::PriorityHigh);

	// Now to python callbacks...
	if(!evt.isProcessed()) 
		getSystemManager()->getScriptInterpreter()->handleEvent(evt);

	// Now to modules with lower priority.
	if(!evt.isProcessed()) 
		ModuleServices::handleEvent(evt, EngineModule::PriorityNormal);
	if(!evt.isProcessed()) 
		ModuleServices::handleEvent(evt, EngineModule::PriorityLow);
	if(!evt.isProcessed()) 
		ModuleServices::handleEvent(evt, EngineModule::PriorityLowest);

	// Update pointers.
	if(myDrawPointers)
	{
		if(evt.getServiceType() == Service::Pointer) 
		{
			refreshPointer(evt.getSourceId(), evt);
		}
	}
	if(!evt.isProcessed()) 
	{
		myDefaultCamera->handleEvent(evt);
	}

	myHandleEventTimeStat->stopTiming();
}

///////////////////////////////////////////////////////////////////////////////
void Engine::update(const UpdateContext& context)
{
	myUpdateTimeStat->startTiming();

	// Create the death switch thread if it does not exist yet
	if(sDeathSwitchThread == NULL)
	{
		omsg("Creating death switch thread");
		sDeathSwitchThread = new DeathSwitchThread();
		sDeathSwitchThread->start();
	}
	
	// Set the update received flag to true, so the death switch thread will
	// not kill us.
	sUpdateReceived = true;
	
	// First update the script
	getSystemManager()->getScriptInterpreter()->update(context);

	// Then run update on modules
	myModuleUpdateTimeStat->startTiming();
    ModuleServices::update(this, context);
	myUpdateTimeStat->stopTiming();
	
	// Run update on the scene graph.
	mySceneUpdateTimeStat->startTiming();
	myScene->update(context);
	mySceneUpdateTimeStat->stopTiming();

	// Process sound / reconnect to sound server (if sound is enabled in config and failed on init)
	if( soundEnv != NULL && soundManager->isSoundServerRunning() )
	{
		// Processing messages from sound server
		soundManager->poll();

		// Update the listener position with the camera's 'navigative' position
		soundEnv->setListenerPosition( getDefaultCamera()->getPosition() );
		soundEnv->setListenerOrientation( getDefaultCamera()->getOrientation() );

		// Update the user position with the head tracker's position
		soundEnv->setUserPosition( getDefaultCamera()->getHeadOffset() );
	}

	myUpdateTimeStat->stopTiming();
}

///////////////////////////////////////////////////////////////////////////////
void Engine::initializeSound()
{
	if( soundManager->isSoundServerRunning() )
	{
		Config* syscfg = getSystemManager()->getSystemConfig();
		Setting& s = syscfg->lookup("config/sound");

		String soundServerIP = Config::getStringValue("soundServerIP", s, "localhost");
		int soundServerPort = Config::getIntValue("soundServerPort", s, 57120);

		float volumeScale = Config::getFloatValue("volumeScale", s, 0.5);

		omsg("Engine: Sound server reports ready.");

		soundEnv->setVolumeScale( volumeScale );
		soundEnv->setListenerPosition( getDefaultCamera()->getPosition() );
		soundEnv->setListenerOrientation( getDefaultCamera()->getOrientation() );
		soundEnv->setUserPosition( Vector3f(0.0,1.8f,0.0) );

		bool assetCacheEnabled = Config::getBoolValue("assetCacheEnabled", s, true);
		int assetCachePort = Config::getIntValue("assetCachePort", s, 22500);

		soundManager->setAssetCacheEnabled(assetCacheEnabled);
		soundManager->getAssetCacheManager()->addCacheHost(soundServerIP);
		soundManager->getAssetCacheManager()->setCachePort(assetCachePort);

		soundManager->setServerVolume( Config::getIntValue("soundServerVolume", s, -16) );

		soundReady = true;
	}
}

///////////////////////////////////////////////////////////////////////////////
const SceneQueryResultList& Engine::querySceneRay(const Ray& ray, uint flags)
{
    myRaySceneQuery.clearResults();
	myRaySceneQuery.setSceneNode(myScene.get());
    myRaySceneQuery.setRay(ray);
    return myRaySceneQuery.execute(flags);
}

///////////////////////////////////////////////////////////////////////////////
void Engine::drawPointers(Renderer* client, const DrawContext& context)
{
	if(myDrawPointers)
	{
		typedef pair<int, Ref<Pointer> > PointerItem;
		foreach(PointerItem i, myPointers)
		{
			Pointer* p = i.second;
			if(p->getVisible())
			{
				Renderable* r = p->getRenderable(client);
				r->draw(context);
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
Camera* Engine::createCamera(uint flags)
{
    Camera* cam = new Camera(this, flags);
	// By default attach camera to scene root.
	myScene->addChild(cam);
    myCameras.push_back(cam);
    return cam;
}

///////////////////////////////////////////////////////////////////////////////
void Engine::destroyCamera(Camera* cam)
{
    oassert(cam != NULL);
    myCameras.remove(cam);
    //delete cam;
}

///////////////////////////////////////////////////////////////////////////////
Camera* Engine::createCamera(const String& name, uint flags)
{
	Camera* cam = createCamera(flags);
	cam->setName(name);
	return cam;
}

///////////////////////////////////////////////////////////////////////////////
Camera* Engine::getCamera(const String& name)
{
	foreach(Camera* cam, myCameras)
	{
		if(cam->getName() == name) return cam;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
Camera* Engine::getCameraById(int id)
{
	foreach(Camera* cam, myCameras)
	{
		if(cam->getCameraId() == id) return cam;
	}
	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
Engine::CameraCollection Engine::getCameras()
{
    return myCameras;
}

///////////////////////////////////////////////////////////////////////////////
Engine::CameraCollection Engine::getCameras() const
{
    return myCameras;
}

///////////////////////////////////////////////////////////////////////////////
int Engine::getCanvasWidth() 
{
	return getDisplaySystem()->getCanvasSize().x(); 
}

///////////////////////////////////////////////////////////////////////////////
int Engine::getCanvasHeight()
{
	return getDisplaySystem()->getCanvasSize().y(); 
}

///////////////////////////////////////////////////////////////////////////////
Renderer* Engine::getRendererByContextId(int id)
{
	foreach(Renderer* r, myClients)
	{
		if(r->getGpuContext()->getId() == id) return r;
	}
	return NULL;
}

