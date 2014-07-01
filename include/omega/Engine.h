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
#ifndef __ENGINE_SERVER_H__
#define __ENGINE_SERVER_H__

#include "osystem.h"
#include "Renderer.h"
#include "SceneNode.h"
#include "Pointer.h"
#include "Renderable.h"
#include "SceneQuery.h"
#include "Camera.h"
#include "Font.h"
#include "omicron/SoundManager.h"

namespace omega {
    typedef List< Ref<Renderer> > RendererList;

    // Forward decl, cannot include COnsole.h to avoid circular dependency.
    class Console;

    ///////////////////////////////////////////////////////////////////////////
    //! The omegalib Engine is the core runtime component of omegalib. It runs on 
    //! each node of a cluster system and handles the abstract scene graph, 
    //! cameras, distribution of events and frame updates. 
    class OMEGA_API Engine: public ReferenceType, public IEventListener
    {
    public:
        typedef List< Ref<Camera> > CameraCollection;
        enum PointerMode { PointerModeMouse, PointerModeWand, PointerModeDynamic };

    friend class Renderer;
    public:
        static const int MaxActivePointers = 128;
        static Engine* instance() { return mysInstance; }

    public:
        Engine(ApplicationBase* app);
        virtual ~Engine();

        ServiceManager* getServiceManager();

        SystemManager*  getSystemManager()  
        { return SystemManager::instance(); }
        
        ApplicationBase* getApplication() 
        { return myApplication; }
        
        DisplaySystem*  getDisplaySystem() 
        { return SystemManager::instance()->getDisplaySystem(); }
        
        int getCanvasWidth(); 
        int getCanvasHeight();

        //! Renderer management
        //@{
        void addRenderer(Renderer* client);
        Renderer* getRendererByContextId(int id);
        RendererList& getRendererList();
        void removeRenderPass(const String& renderPassName);
        //@}

        //! Camera management
        //@{
        Camera* getDefaultCamera();
        Camera* createCamera(uint flags = Camera::DefaultFlags);
        Camera* createCamera(const String& name, uint flags = Camera::DefaultFlags);
        void destroyCamera(Camera* cam);
        Camera* getCamera(const String& name);
        Camera* getCameraById(int id);
        CameraCollection getCameras();
        CameraCollection getCameras() const;
        //@}

        //! Font management
        //@{
        void setDefaultFont(const FontInfo& font);
        const FontInfo& getDefaultFont();
        //@}

        //! Scene query
        //@{
        const SceneQueryResultList& querySceneRay(const Ray& ray, uint flags = 0);
        //@}

        SceneNode* getScene();

        //! Pointer mode management
        //@{
        //PointerMode getPointerMode() { return myPointerMode; }
        //void setPointerMode(PointerMode value) { myPointerMode = value; }
        bool getDrawPointers() { return myDrawPointers; }
        void setDrawPointers(bool value) { myDrawPointers = value; }
        //@}

        //! Console management
        //@{
        Console* getConsole();
        //@}

        //! Sound management
        //@{
        void initializeSound();
        SoundManager* getSoundManager();
        SoundEnvironment* getSoundEnvironment();
        bool isSoundEnabled();
        //@}

        //! Input mappings
        //@{
        Event::Flags getPrimaryButton() { return myPrimaryButton; }
        //@}

        virtual void initialize();
        virtual void dispose();
        //! Resets the omegalib engine to its initial state. Useful for runtime
        //! application switching.
        void reset();

        //! Set to true by default.
        //! When set to false, the engine will ignore all input events.
        void setEventDispatchEnabled(bool value);
        bool isEventDispatchEnabled() 
            { return myEventDispatchEnabled; }

        virtual void handleEvent(const Event& evt);
        virtual void update(const UpdateContext& context);
        bool handleCommand(const String& cmd);

    private:
        //! Pointer Management
        //@{
        //! Draw pointer objects inside a specific client context.
        void drawPointers(Renderer* client, const DrawContext& context);
        void refreshPointer(int pointerId, const Event& evt);
        //@}

    private:
        static Engine* mysInstance;

        ApplicationBase* myApplication;
        List< Ref<Renderer> > myClients;

        // Engine lock, used when client / server thread synchronization is needed.
        Lock myLock;

        Ref<SceneNode> myScene;

        // Pointers
        Dictionary< int, Ref<Pointer> > myPointers;
        bool myDrawPointers;
        int myPointerSize;
        //float myActivePointerTimeout;
        //PointerMode myPointerMode;

        // Resources
        FontInfo myDefaultFont;

        // Scene querying
        RaySceneQuery myRaySceneQuery;

        // Console
        Console* myConsole;

        //! Configuration value, set to true by default.
        //! When set to true, events will be broadcast from the master node 
        //! to all slave instances (unless they are marked as local).
        //! Disable event sharing if your application wants to handle events
        //! on the master node only. You will be in charge of implementing your
        //! own synchronization scheme.
        bool myEventSharingEnabled;

        //! Set to true by default.
        //! When set to false, the engine will ignore all input events.
        bool myEventDispatchEnabled;

        // Cameras.
        Ref<Camera> myDefaultCamera;
        CameraCollection myCameras;

        // Sound
        Ref<SoundManager> soundManager;
        Ref<SoundEnvironment> soundEnv;
        int lastSoundServerCheck;
        int soundServerCheckDelay;
        bool soundEnabled;
        bool soundReady;

        // Input mapping
        Event::Flags myPrimaryButton;

        // Stats
        Ref<Stat> myHandleEventTimeStat;
        Ref<Stat> myUpdateTimeStat;
        Ref<Stat> mySceneUpdateTimeStat;
        Ref<Stat> myModuleUpdateTimeStat;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline Camera* Engine::getDefaultCamera()
    { return myDefaultCamera.get(); }

    ///////////////////////////////////////////////////////////////////////////
    inline ServiceManager* Engine::getServiceManager()
    { return SystemManager::instance()->getServiceManager(); }

    ///////////////////////////////////////////////////////////////////////////
    inline RendererList& Engine::getRendererList()
    { return myClients; }

    ///////////////////////////////////////////////////////////////////////////
    inline void Engine::setDefaultFont(const FontInfo& font)
    { myDefaultFont = font;	}

    ///////////////////////////////////////////////////////////////////////////
    inline const FontInfo& Engine::getDefaultFont()
    { return myDefaultFont; }

    ///////////////////////////////////////////////////////////////////////////
    inline Console* Engine::getConsole()
    { return myConsole;	}

    ///////////////////////////////////////////////////////////////////////////
    inline SoundManager* Engine::getSoundManager()
    { return soundManager;	}

    ///////////////////////////////////////////////////////////////////////////
    inline SoundEnvironment* Engine::getSoundEnvironment()
    { return soundEnv;	}

    ///////////////////////////////////////////////////////////////////////////
    //! This is only used to indicate if sound was enabled via configuration
    //! If this is true, we assume the master node is connected/connecting
    //! to the sound server. Used to enable the sound volume Python menu.
    inline bool Engine::isSoundEnabled()
    { return soundEnabled;	}
}; // namespace omega

#endif
