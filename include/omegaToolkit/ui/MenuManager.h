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
#ifndef __OTK_MENU_MANAGER__
#define __OTK_MENU_MANAGER__

#include "omegaToolkit/omegaToolkitConfig.h"
#include "omegaToolkit/ui/Widget.h"
#include "omegaToolkit/ui/Button.h"
#include "omegaToolkit/ui/Container.h"
#include "omegaToolkit/UiScriptCommand.h"
#include "omegaToolkit/ui/Menu.h"

namespace omegaToolkit { namespace ui {
	using namespace omega;

	///////////////////////////////////////////////////////////////////////////
	class OTK_API MenuManager: public EngineModule
	{
	public:
		static MenuManager* createAndInitialize();
		static MenuManager* instance();

		UiModule* getUiModule() { return myUiModule; }

		virtual void initialize();
		virtual void dispose();
		virtual void update(const UpdateContext& context);
		virtual void handleEvent(const Event& evt);

		Menu* createMenu(const String& name);

		void setMainMenu(Menu* menu) { myMainMenu = menu; }
		Menu* getMainMenu() { return myMainMenu; }

		void autoPlaceMenu(Menu* menu, const Event& evt);

		float getDefaultMenuScale() { return myDefaultMenuScale; }
		float getDefaultMenuDistance() { return myDefaultMenuPosition.z(); }

		// Specifies whether camera navigation should be disabled when inside a menu
		void setNavigationSuspended(bool value) { myNavigationSuspended = value; }
		//! see setNavigationSuspended
		bool getNavigationSuspended() { return myNavigationSuspended; }

		bool is3dMenuEnabled() { return my3dMenuEnabled; }
		
		Sound* getShowMenuSound();
		Sound* getHideMenuSound();
	private:
		MenuManager();
		virtual ~MenuManager();

		//void autoPlaceMenu(Menu* menu, const Event& evt);

	private:
		static Ref<MenuManager> mysInstance;

		Ref<UiModule> myUiModule;
		List< Ref<Menu> > myMenuList;
		Ref<Menu> myMainMenu;

		bool myRayPlaceEnabled;
		Vector3f myDefaultMenuPosition;
		float myDefaultMenuScale;

		// options
		bool myNavigationSuspended;
		bool myNavigationState;
		bool myUseMenuToggleButton;
		Event::Flags myMenuToggleButton;

		bool my3dMenuEnabled;
		//bool myAutoPlaceEnabled;
		//float myAutoPlaceDistance;
		//float myMenu3dScale;

		int myMenuInteractorId;

		// Menu sounds
		Sound* myShowMenuSound;
		Sound* myHideSoundMenu;
		Sound* selectMenuSound;
		Sound* scrollMenuSound;
	};
}; };

#endif
