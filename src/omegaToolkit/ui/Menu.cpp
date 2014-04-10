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
 *	A 2D or 3D menu, containing buttons, sliders, etc.
 ******************************************************************************/
#include "omegaToolkit/ui/Menu.h"
#include "omegaToolkit/ui/MenuManager.h"
#include "omega/DisplaySystem.h"

using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////
MenuItem::MenuItem(Type type, Menu* owner):
	myMenu(owner),
	myType(type),
	myListener(NULL),
	myButton(NULL),
	myCommand(NULL),
	myWidget(NULL),
	myUserData(NULL),
	mySubMenu(NULL)
{
	UiModule* ui = owner->getManager()->getUiModule();
	WidgetFactory* wf = ui->getWidgetFactory();

	if(type == MenuItem::SubMenu)
	{
		myButton = wf->createButton("subMenu_button", myMenu->myContainer);
		myButton->setText("Button");
		myWidget = myButton;
		myWidget->setUIEventHandler(this);

		mySubMenu = myMenu->getManager()->createMenu("Submenu");
	}
	else if(type == MenuItem::Button)
	{
		myButton = wf->createButton("button", myMenu->myContainer);
		myButton->setText("Button");
		myWidget = myButton;
	}
	else if(type == MenuItem::Slider)
	{
		mySlider = wf->createSlider("slider", myMenu->myContainer);
		myWidget = mySlider;
	}
	else if(type == MenuItem::Checkbox)
	{
		myButton = wf->createButton("button", myMenu->myContainer);
		myButton->setText("Checkbox");
		myButton->setCheckable(true);
		myWidget = myButton;
	}
	else if(type == MenuItem::Label)
	{
		myLabel = wf->createLabel("label", myMenu->myContainer);
		myLabel->setText("Label");
		myWidget = myLabel;
	}
	else if(type == MenuItem::Image)
	{
		myImage = wf->createImage("img", myMenu->myContainer);
		myWidget = myImage;
	}
	else if(type == MenuItem::Container)
	{
		myContainer = wf->createContainer("container", myMenu->myContainer);
		myWidget = myContainer;
	}

	//myWidget->setStyleValue("fill", "#00000090");

	myWidget->setAutosize(true);
}

///////////////////////////////////////////////////////////////////////////////
void MenuItem::setText(const String& value) 
{ 
	myText = value; 
	switch(myType)
	{
	case MenuItem::Button:
	case MenuItem::Checkbox:
		myButton->setText(myText);
		break;
	case MenuItem::SubMenu:
		myButton->setText(myText + "  >");
		break;
	case MenuItem::Label:
		myLabel->setText(myText);
	}
}

///////////////////////////////////////////////////////////////////////////////
void MenuItem::setDescription(const String& value) 
{ 
	myDescription = value; 
	switch(myType)
	{
	case MenuItem::Button:
	case MenuItem::Checkbox:
		// Do nothing.
		break;
	}
}
		
///////////////////////////////////////////////////////////////////////////////
void MenuItem::setImage(PixelData* image)
{
	if(myButton != NULL) myButton->setIcon(image);
	else if(myImage != NULL) myImage->setData(image);
}

///////////////////////////////////////////////////////////////////////////////
void MenuItem::setChecked(bool value)
{
	if(myType == MenuItem::Checkbox)
	{
		myButton->setChecked(value);
	}
}

///////////////////////////////////////////////////////////////////////////////
bool MenuItem::isChecked()
{
	if(myType == MenuItem::Checkbox)
	{
		return myButton->isChecked();
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////
void MenuItem::setCommand(const String& command)
{
	if(myCommand == NULL)
	{
		myCommand = new UiScriptCommand(command);
		myCommand->setUiEventsOnly(true);
		myWidget->setUIEventHandler(myCommand.get());
	}
	else
	{
		myCommand->setCommand(command);
	}
}

///////////////////////////////////////////////////////////////////////////////
const String& MenuItem::getCommand()
{
	static String emptyString("");

	if(myCommand != NULL)
	{
		return myCommand->getCommand();
	}
	return emptyString;
}

///////////////////////////////////////////////////////////////////////////////
void MenuItem::setListener(IMenuItemListener* value) 
{
	myListener = value; 
	if(myListener != NULL)
	{
		myWidget->setUIEventHandler(this);
	}
}

///////////////////////////////////////////////////////////////////////////////
void MenuItem::handleEvent(const Event& evt)
{
	if(evt.isFrom(Service::Ui, myWidget->getId()))
	{
		if(myListener != NULL) myListener->onMenuItemEvent(this);
		if(myType == SubMenu && evt.getType() == Event::Click)
		{
			myMenu->setActiveSubMenu(mySubMenu);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
Menu::Menu(const String& name, MenuManager* manager): 
	myName(name),
	myManager(manager),
	myActiveSubMenu(NULL),
	myParent(NULL)
{
	UiModule* ui = UiModule::instance();
	WidgetFactory* wf = ui->getWidgetFactory();
	myContainer = wf->createContainer("container", ui->getUi(), Container::LayoutVertical);
	myContainer->setPosition(Vector2f(10, 10));
	myContainer->setStyleValue("fill", "#000000d0");
	//myContainer->setLayout(Container::LayoutHorizontal);

	my3dSettings.enable3d = MenuManager::instance()->is3dMenuEnabled();
	myContainer->setAutosize(true);
	myContainer->setHorizontalAlign(Container::AlignLeft);

	// By default menus are attached to the default camera.
	my3dSettings.node = manager->getEngine()->getDefaultCamera();
	firstHide = true;
}

///////////////////////////////////////////////////////////////////////////////
Menu::~Menu()
{
	ofmsg("~Menu %1%", %myName);
	Container* parent = myContainer->getContainer();
	if(parent != NULL)
	{
		myContainer->getContainer()->removeChild(myContainer);
	}
}

///////////////////////////////////////////////////////////////////////////////
MenuItem* Menu::addItem(MenuItem::Type type)
{
	MenuItem* item = new MenuItem(type, this);
	myMenuItems.push_back(item);
	return item;
}

///////////////////////////////////////////////////////////////////////////////
MenuItem* Menu::addButton(const String& label, const String& command)
{
	MenuItem* item = addItem(MenuItem::Button);
	item->setText(label);
	item->setCommand(command);
	return item;
}

///////////////////////////////////////////////////////////////////////////////
MenuItem* Menu::addLabel(const String& text)
{
	MenuItem* item = addItem(MenuItem::Label);
	item->setText(text);
	return item;
}

///////////////////////////////////////////////////////////////////////////////
MenuItem* Menu::addSlider(int ticks, const String& command)
{
	MenuItem* item = addItem(MenuItem::Slider);
	item->getSlider()->setTicks(ticks);
	item->setCommand(command);
	return item;
}

///////////////////////////////////////////////////////////////////////////////
MenuItem* Menu::addImage(PixelData* image)
{
	MenuItem* item = addItem(MenuItem::Image);
	item->setImage(image);
	return item;
}


///////////////////////////////////////////////////////////////////////////////
MenuItem* Menu::addContainer()
{
	MenuItem* item = addItem(MenuItem::Container);
	return item;
}

///////////////////////////////////////////////////////////////////////////////
Menu* Menu::addSubMenu(const String& label)
{
	MenuItem* item = addItem(MenuItem::SubMenu);
	item->setText(label);
	return item->getSubMenu();
}

///////////////////////////////////////////////////////////////////////////////
void Menu::update(const UpdateContext& context)
{
	float speed = context.dt * 10;
	if(speed > 1.0f) speed = 1.0f; 

	ui::Container3dSettings& c3ds = myContainer->get3dSettings();
	c3ds.enable3d = my3dSettings.enable3d;
	c3ds.normal = my3dSettings.normal;
	c3ds.up = my3dSettings.up;
	c3ds.position += (my3dSettings.position - c3ds.position) * speed;
	c3ds.scale += (my3dSettings.scale - c3ds.scale) * speed;
	//c3ds.position = my3dSettings.position;
	//c3ds.scale = my3dSettings.scale;
	c3ds.alpha += (my3dSettings.alpha - c3ds.alpha) * speed;
	c3ds.node = my3dSettings.node;
	//c3ds.alpha = my3dSettings.alpha;

	if(c3ds.enable3d == false)
	{
		myContainer->setAlpha(c3ds.alpha);
		myContainer->setBlendMode(Widget::BlendNormal);
		myContainer->setScale(c3ds.alpha);
	}

	if(myContainer->isVisible())
	{
		if(c3ds.alpha <= 0.1f)
		{
			myContainer->setVisible(false);
		}
	}
	else
	{
		if(c3ds.alpha > 0.1f)
		{
			myContainer->setVisible(true);
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void Menu::setActiveSubMenu(Menu* submenu)
{
	if(submenu == NULL)
	{
		// Hide current active submenu
		if(myActiveSubMenu != NULL)
		{
			onPopMenuStack();
			myActiveSubMenu->hide();
			myActiveSubMenu->myParent = NULL;
		}
		myActiveSubMenu = NULL;
		myContainer->setEnabled(true);
	}
	else
	{
		submenu->get3dSettings() = my3dSettings;
		submenu->show();

		// Hide current active submenu
		if(myActiveSubMenu != NULL)
		{
			myActiveSubMenu->hide();
			myActiveSubMenu->myParent = NULL;
			myActiveSubMenu = submenu;
			myActiveSubMenu->myParent = this;
		}
		else
		{
			myActiveSubMenu = submenu;
			myActiveSubMenu->myParent = this;
			onPushMenuStack();
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
void Menu::onPopMenuStack()
{
	if(myActiveSubMenu != NULL)
	{
		if(myParent != NULL)
		{
			myParent->onPopMenuStack();
		}
		my3dSettings = myActiveSubMenu->get3dSettings();
	}
}

///////////////////////////////////////////////////////////////////////////////
void Menu::onPushMenuStack()
{
	if(myActiveSubMenu != NULL)
	{
		// I have an active submenu: I am disabled.
		myContainer->setEnabled(false);

		if(myParent != NULL)
		{
			myParent->onPushMenuStack();
		}

		const Container3dSettings& subc3ds = myActiveSubMenu->get3dSettings();

		Vector3f dir = subc3ds.normal.cross(subc3ds.up);
		my3dSettings.position += subc3ds.normal * (-0.3f) + dir * (subc3ds.scale * myContainer->getWidth());
		my3dSettings.alpha = myActiveSubMenu->get3dSettings().alpha / 3;

		if(!my3dSettings.enable3d)
		{
			myActiveSubMenu->getContainer()->setPosition(myContainer->getPosition());
		}
	}
}

///////////////////////////////////////////////////////////////////////////////
Menu* Menu::getTopActiveSubMenu()
{
	if(myActiveSubMenu != NULL) return myActiveSubMenu->getTopActiveSubMenu();
	return this;
}

///////////////////////////////////////////////////////////////////////////////
void Menu::focus()
{
	UiModule::instance()->activateWidget(myContainer);
}

///////////////////////////////////////////////////////////////////////////////
void Menu::show()
{
	myVisible = true;
	myContainer->setEnabled(true);

	focus();

	myContainer->get3dSettings().alpha = 0.0f;
	my3dSettings.alpha = 1.0f;

	myContainer->get3dSettings().scale = myManager->getDefaultMenuScale() / 2;
	my3dSettings.scale = myManager->getDefaultMenuScale();

	if(SystemManager::settingExists("config/sound") && myManager->getShowMenuSound() != NULL )
	{
		// Play a sound based on menu's position
		Ref<SoundInstance> showSound = new SoundInstance(myManager->getShowMenuSound());
		showSound->setLocalPosition( my3dSettings.position );
		showSound->play();
	}
}

///////////////////////////////////////////////////////////////////////////////
void Menu::hide()
{
	//omsg("Menu hide");

	myVisible = false;
	myContainer->setEnabled(false);

	UiModule::instance()->activateWidget(NULL);

	my3dSettings.alpha = 0.0f;
	my3dSettings.scale = myManager->getDefaultMenuScale() / 2;

	if(myActiveSubMenu != NULL)
	{
		myActiveSubMenu->hide();
		myActiveSubMenu = NULL;
	}

	if( SystemManager::settingExists("config/sound") && myManager->getHideMenuSound() != NULL )
	{
		// Play a sound based on menu's position
		Ref<SoundInstance> hideSound = new SoundInstance(myManager->getHideMenuSound());
		hideSound->setLocalPosition( my3dSettings.position );
		
		if( firstHide )
			firstHide = false;
		else
			hideSound->play();
	}
}

///////////////////////////////////////////////////////////////////////////////
void Menu::placeOnWand(const Event& evt)
{
	MenuManager* mm = MenuManager::instance();

    if(mm->is3dMenuEnabled())
    {
        float distance = mm->getDefaultMenuDistance();
        float scale = mm->getDefaultMenuScale();
        Ray ray;
        if(SystemManager::instance()->getDisplaySystem()->getViewRayFromEvent(evt, ray))
        {
            Vector3f pos = ray.getPoint(distance);
            Vector3f dir = ray.getDirection();

            // If the menu is attached to a node (usually the default camera)
            // use untransformed values for its position and direction, since the node transform applied during draw will
            // take care of transformations.
            if(my3dSettings.node != NULL)
            {
                pos = my3dSettings.node->convertWorldToLocalPosition(pos);
                // This is a bit of trickery: If the event is a pointer event we use the orientation of the node
                // to determine the menu orientation.
                // If the event is a Wand event (with 6DOF tracking) we use the actual wand orientation.
                if(evt.getServiceType() == Event::ServiceTypeWand)
                {
                    dir = evt.getOrientation() * -Vector3f::UnitZ();
                }
                else
                {
                    dir = -Vector3f::UnitZ(); //my3dSettings.node->getOrientation() * dir;
                }
            }

            //ofmsg("menu position: %1%", %pos);
            Container3dSettings& c3ds = get3dSettings();
            Widget* menuWidget = myContainer;
            Vector3f offset = Vector3f(0, 0, 0); //-menuWidget->getHeight() * c3ds.scale, 0);
            c3ds.position = pos - offset;
            c3ds.normal = -dir;

            // If the menu widget is not attached to a node, set the up vector using the default camera orientation.
            if(my3dSettings.node == NULL)
            {
                DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
                Camera* cam = Engine::instance()->getDefaultCamera();
                Vector3f up = Vector3f::UnitY();
                c3ds.up = cam->getOrientation() * up;
            }
            else
            {
                // If the menu is attached to a node, the up vector is just positive Y (the node transform applied later will do the rest)
                c3ds.up = Vector3f::UnitY();
            }

            //c3ds.normal = Vector3f(0, 0, 1);
            c3ds.scale = scale;
        }
    }
    else
    {
        // Place 2D menu
        if(evt.getServiceType() == Event::ServiceTypePointer)
        {
            myContainer->setPosition(
                Vector2f(evt.getPosition()[0], evt.getPosition()[1]));
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void Menu::toggle()
{
	if(myContainer->isVisible()) hide();
	else show();
}

///////////////////////////////////////////////////////////////////////////////
bool Menu::isVisible()
{
	return myVisible;
}

