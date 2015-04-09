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
 *	The menu manager: keeps track of application menus, active menus, etc.
 ******************************************************************************/
#include "omegaToolkit/ui/MenuManager.h"
#include "omega/DisplaySystem.h"

using namespace omegaToolkit::ui;

MenuManager* mysInstance = NULL;

///////////////////////////////////////////////////////////////////////////////
MenuManager* MenuManager::instance() 
{ 
    if(mysInstance == NULL)
    {
        createAndInitialize();
    }
    return mysInstance; 
}

///////////////////////////////////////////////////////////////////////////////
MenuManager* MenuManager::createAndInitialize()
{
    if(mysInstance == NULL)
    {
        mysInstance = new MenuManager();
        ModuleServices::addModule(mysInstance);
        mysInstance->doInitialize(Engine::instance());
    }
    return mysInstance;
}

///////////////////////////////////////////////////////////////////////////////
MenuManager::MenuManager():
    EngineModule("MenuManager"),
    myMainMenu(NULL),
    myDefaultMenuPosition(-1.0, -0.5, -2.0),
    myDefaultMenuScale(1.0f),
    myNavigationSuspended(false),
    myRayPlaceEnabled(true),
    myMenuToggleButton(Event::User),
    myUseMenuToggleButton(false),
    myHideSoundMenu(NULL),
    myShowMenuSound(NULL)
{
    setPriority(EngineModule::PriorityLow);
    mysInstance = this;
}

///////////////////////////////////////////////////////////////////////////////
MenuManager::~MenuManager()
{
    //omsg("~MenuManager");
    mysInstance = NULL;
}

///////////////////////////////////////////////////////////////////////////////
void MenuManager::initialize()
{
    // Create the ui module if one is not available already
    myUiModule = UiModule::createAndInitialize();

    if(SystemManager::settingExists("config/ui"))
    {
        Setting& sUi = SystemManager::settingLookup("config/ui");
        myRayPlaceEnabled = Config::getBoolValue("menuRayPlaceEnabled", sUi, myRayPlaceEnabled);
        myDefaultMenuPosition = Config::getVector3fValue("menuDefaultPosition", sUi, myDefaultMenuPosition);
        myDefaultMenuScale = Config::getFloatValue("menuDefaultScale", sUi, myDefaultMenuScale);
        my3dMenuEnabled = Config::getBoolValue("menu3dEnabled", sUi, true);
        myNavigationSuspended = Config::getBoolValue("menuSuspendNavigation", sUi, myNavigationSuspended);


        myMenuInteractorId = Config::getIntValue("menuWandId", sUi, -1);

        // Parse menu toggle button name (if present)
        String toggleButtonName = Config::getStringValue("menuToggleButton", sUi, "");
        if(toggleButtonName != "")
        {
            myMenuToggleButton = Event::parseButtonName(toggleButtonName);
            myUseMenuToggleButton = true;
        }

        myDefaultMenuScale *= 0.001f;
    }

    // See if we have a sound environment available.
    // If we don't it means sounds are disabled OR we are not the master node.
    SoundEnvironment* se = getEngine()->getSoundEnvironment();
    if(se != NULL)
    {
        if(SystemManager::settingExists("config/sound"))
        {
            Setting& sUi = SystemManager::settingLookup("config/sound");

            float volume = Config::getFloatValue("menuSoundVolume", sUi, 0.2f);
            float width = Config::getFloatValue("menuSoundWidth", sUi, 2.0);
            float mix = Config::getFloatValue("menuSoundMix", sUi, 0.0);
            float reverb = Config::getFloatValue("menuSoundReverb", sUi, 0.0);

            if( sUi.exists("showMenuSound") ){
                //myShowMenuSound = se->createSound("showMenuSound");
                //myShowMenuSound->loadFromFile( Config::getStringValue("showMenuSound", sUi ) );
                myShowMenuSound = se->loadSoundFromFile("showMenuSound", Config::getStringValue("showMenuSound", sUi ));
                myShowMenuSound->setDefaultParameters(volume, width, mix, reverb, false, false);
                // Played from Menu class
            }

            if( sUi.exists("hideMenuSound") ){
                //myHideSoundMenu = se->createSound("hideMenuSound");
                //myHideSoundMenu->loadFromFile( Config::getStringValue("hideMenuSound", sUi ) );
                myHideSoundMenu = se->loadSoundFromFile("hideMenuSound", Config::getStringValue("hideMenuSound", sUi ));
                myHideSoundMenu->setDefaultParameters(volume, width, mix, reverb, false, false);
                // Played from Menu class
            }

            if( sUi.exists("selectMenuSound") ){
                //selectMenuSound = se->createSound("selectMenuSound");
                //selectMenuSound->loadFromFile( Config::getStringValue("selectMenuSound", sUi ) );
                selectMenuSound = se->loadSoundFromFile("selectMenuSound", Config::getStringValue("selectMenuSound", sUi ));
                selectMenuSound->setDefaultParameters(volume, width, mix, reverb, false, false);
                // Played from Button class
            }

            if( sUi.exists("scrollMenuSound") ){
                //scrollMenuSound = se->createSound("scrollMenuSound");
                //scrollMenuSound->loadFromFile( Config::getStringValue("scrollMenuSound", sUi ) );
                scrollMenuSound = se->loadSoundFromFile("scrollMenuSound", Config::getStringValue("scrollMenuSound", sUi ));
                scrollMenuSound->setDefaultParameters(volume, width, mix, reverb, false, false);
                // Played from Widget class
            }
            
        }
    }

    // Read configuration parameters from system config
    //Setting& sSysCfg = SystemManager::instance()->getSystemConfig()->lookup("config");
    //myMenu3dEnabled = Config::getBoolValue("menu3dEnabled", sSysCfg, false);nMenu
    //myMenu3dEnabled = true;
    //if(myMenu3dEnabled)
    //{
    //	myAutoPlaceEnabled = true;
    //	myAutoPlaceDistance = Config::getFloatValue("menu3dDistance", sSysCfg, myAutoPlaceDistance);
    //	myMenu3dScale = Config::getFloatValue("menu3dScale", sSysCfg, myMenu3dScale);
    //}
}

///////////////////////////////////////////////////////////////////////////////
void MenuManager::dispose()
{
    //ofmsg("MenuManager::dispose: cleaning up %1% menus", %myMenuList.size());
    myMainMenu = NULL;
    myMenuList.clear();
}

///////////////////////////////////////////////////////////////////////////////
void MenuManager::update(const UpdateContext& context)
{
    foreach(Menu* m, myMenuList)
    {
        m->update(context);
    }
}

///////////////////////////////////////////////////////////////////////////////
void MenuManager::handleEvent(const Event& evt)
{
    if(!evt.isProcessed() && (evt.getSourceId() == myMenuInteractorId || myMenuInteractorId == -1 ) )
    {
        if(myMainMenu != NULL)
        {
            if(!myMainMenu->isVisible())
            {
                bool showMenuButtonPressed = 
                    (myUseMenuToggleButton && evt.isButtonDown(myMenuToggleButton)) ||
                    (!myUseMenuToggleButton && evt.isButtonDown(UiModule::getConfirmButton()));

                if(showMenuButtonPressed)
                {
                    autoPlaceMenu(myMainMenu, evt);
                    myMainMenu->show();
                    if(myNavigationSuspended)
                    {
                        Camera* cam = getEngine()->getDefaultCamera();
                        myNavigationState = cam->isControllerEnabled();
                        cam->setControllerEnabled(false);
                    }
                    evt.setProcessed();
                }
            }
            else
            {
                bool hideMenuButtonPressed = 
                    (myUseMenuToggleButton && evt.isButtonDown(myMenuToggleButton)) ||
                    evt.isButtonDown(UiModule::getCancelButton());

                if(hideMenuButtonPressed)
                {
                    Menu* topMostMenu = myMainMenu->getTopActiveSubMenu();
                    if(topMostMenu == myMainMenu) 
                    {
                        myMainMenu->hide();
                        if(myNavigationSuspended)
                        {
                            Camera* cam = getEngine()->getDefaultCamera();
                            cam->setControllerEnabled(myNavigationState);
                        }
                    }
                    else
                    {
                        Menu* parent = topMostMenu->getParent();
                        parent->setActiveSubMenu(NULL);
                        parent->focus();
                    }
                    evt.setProcessed();
                }
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void MenuManager::autoPlaceMenu(Menu* menu, const Event& evt)
{
    if(myRayPlaceEnabled)
    {
        menu->placeOnWand(evt);
    }
    else
    {
        DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
        Camera* cam = Engine::instance()->getDefaultCamera();
        
        //ofmsg("MENU World head position: %1%", %obs->getWorldHeadPosition());

        Vector3f obsForward = cam->getOrientation() * Vector3f(0, 0, -1);
        Vector3f headWorldPos = cam->getPosition() + cam->getHeadOffset();
        Vector3f menuPosition = headWorldPos + cam->getHeadOrientation() * myDefaultMenuPosition;
        //menuPosition = Vector3f(0, 1, -3);

        //ofmsg("MENU Menu position, offset: %1% %2%", %menuPosition %myDefaultMenuPosition);
        
        Container3dSettings& c3ds = menu->get3dSettings();
        Widget* menuWidget = menu->getContainer();

        c3ds.scale = myDefaultMenuScale;
        Vector3f offset = Vector3f(0, menuWidget->getHeight() * c3ds.scale * 0.5f, 0);
        c3ds.position = myDefaultMenuPosition + offset;

        //c3ds.normal = -obsForward;
        c3ds.scale = myDefaultMenuScale;
    }
}

///////////////////////////////////////////////////////////////////////////////
Menu* MenuManager::createMenu(const String& name)
{
    Menu* menu = new Menu(name, this);
    myMenuList.push_back(menu);
    
    // Set not visible by default.
    menu->hide();
    
    return menu;
}

///////////////////////////////////////////////////////////////////////////////
void MenuManager::deleteMenu(Menu* m)
{
    myMenuList.remove(m);
}

///////////////////////////////////////////////////////////////////////////////
Sound* MenuManager::getShowMenuSound() 
{ 
    if( getEngine()->getSoundEnvironment() != NULL ) 
    {
        return getEngine()->getSoundEnvironment()->getSound("showMenuSound"); 
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
Sound* MenuManager::getHideMenuSound() 
{ 
    if( getEngine()->getSoundEnvironment() != NULL )
    {
        return getEngine()->getSoundEnvironment()->getSound("hideMenuSound"); 
    }
    return NULL;
}
