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
 *	The python interface to the omegaToolkit API
 ******************************************************************************/
#include "omega/PythonInterpreter.h"
#include "omega/SystemManager.h"
#include "omega/Engine.h"
#include "omegaToolkit/SceneEditorModule.h"
#include "omegaToolkit/UiModule.h"
#include "omegaToolkit/ui/MenuManager.h"
#include "omegaToolkit/ToolkitUtils.h"
#include "omegaToolkit/ImageBroadcastModule.h"

#ifdef OMEGA_USE_PYTHON

#include "omega/PythonInterpreterWrapper.h"

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

#define BOOST_PYTHON_NO_LIB
#include <boost/python.hpp>
using namespace boost::python;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(ImageBroadcastModule_addChannel, addChannel, 2, 4) 
///////////////////////////////////////////////////////////////////////////////
BOOST_PYTHON_MODULE(omegaToolkit)
{
	PYAPI_REF_BASE_CLASS(ImageBroadcastModule)
		PYAPI_STATIC_REF_GETTER(ImageBroadcastModule, instance)
		.def("addChannel", &ImageBroadcastModule::addChannel, ImageBroadcastModule_addChannel())
		PYAPI_METHOD(ImageBroadcastModule, removeChannel)
		;

	// Container
	PYAPI_ENUM(Container::Layout, ContainerLayout)
		PYAPI_ENUM_VALUE(Container, LayoutFree)
		PYAPI_ENUM_VALUE(Container, LayoutHorizontal)
		PYAPI_ENUM_VALUE(Container, LayoutVertical)
		//PYAPI_ENUM_VALUE(Container, LayoutGrid)
		;

	// MenuItem
	PYAPI_ENUM(MenuItem::Type, MenuItemType)
		PYAPI_ENUM_VALUE(MenuItem, Button)
		PYAPI_ENUM_VALUE(MenuItem, Checkbox)
		PYAPI_ENUM_VALUE(MenuItem, Label)
		PYAPI_ENUM_VALUE(MenuItem, Slider)
		PYAPI_ENUM_VALUE(MenuItem, SubMenu)
		PYAPI_ENUM_VALUE(MenuItem, Container)
		;

	// Widget Layer
	PYAPI_ENUM(Widget::Layer, WidgetLayer)
		PYAPI_ENUM_VALUE(Widget, Front)
		PYAPI_ENUM_VALUE(Widget, Middle)
		PYAPI_ENUM_VALUE(Widget, Back)
		;

	// Widget Blend Mode
	PYAPI_ENUM(Widget::BlendMode, WidgetBlendMode)
		PYAPI_ENUM_VALUE(Widget, BlendInherit)
		PYAPI_ENUM_VALUE(Widget, BlendNormal)
		PYAPI_ENUM_VALUE(Widget, BlendAdditive)
		;

	// Vertical Align
	PYAPI_ENUM(Container::VerticalAlign, VAlign)
		PYAPI_ENUM_VALUE(Container, AlignTop)
		PYAPI_ENUM_VALUE(Container, AlignMiddle)
		PYAPI_ENUM_VALUE(Container, AlignBottom)
		;

	// Horizontal Align
	PYAPI_ENUM(Container::HorizontalAlign, HAlign)
		PYAPI_ENUM_VALUE(Container, AlignLeft)
		PYAPI_ENUM_VALUE(Container, AlignCenter)
		PYAPI_ENUM_VALUE(Container, AlignRight)
		;

	PYAPI_BASE_CLASS(ToolkitUtils)
		PYAPI_STATIC_REF_GETTER(ToolkitUtils, createInteractor)
		PYAPI_STATIC_REF_GETTER(ToolkitUtils, setupInteractor)
		PYAPI_STATIC_REF_GETTER(ToolkitUtils, getKinectDepthCameraImage)
		;

	// MenuManager
	PYAPI_REF_BASE_CLASS(MenuManager)
		PYAPI_STATIC_REF_GETTER(MenuManager, createAndInitialize)
		PYAPI_REF_GETTER(MenuManager, createMenu)
		PYAPI_METHOD(MenuManager, setMainMenu)
		PYAPI_REF_GETTER(MenuManager, getMainMenu)
		;

	// Menu
	PYAPI_REF_BASE_CLASS(Menu)
		PYAPI_REF_GETTER(Menu, addItem)
		PYAPI_REF_GETTER(Menu, addButton)
		PYAPI_REF_GETTER(Menu, addSubMenu)
		PYAPI_REF_GETTER(Menu, addLabel)
		PYAPI_REF_GETTER(Menu, addSlider)
		PYAPI_REF_GETTER(Menu, addImage)
		PYAPI_REF_GETTER(Menu, addContainer)
		PYAPI_REF_GETTER(Menu, getContainer)
		.def("get3dSettings", &Menu::get3dSettings, PYAPI_RETURN_INTERNAL_REF)
		PYAPI_METHOD(Menu, show)
		PYAPI_METHOD(Menu, hide)
		PYAPI_METHOD(Menu, isVisible)
		//PYAPI_METHOD(Menu, setLabel)
		//PYAPI_METHOD(Menu, getLabel)
		PYAPI_METHOD(Menu, placeOnWand)
		;

	PYAPI_REF_BASE_CLASS(MenuItem)
		//PYAPI_REF_GETTER(MenuItem, addItem)
		PYAPI_REF_GETTER(MenuItem, getSubMenu)
		PYAPI_REF_GETTER(MenuItem, getWidget)
		PYAPI_REF_GETTER(MenuItem, getButton)
		PYAPI_REF_GETTER(MenuItem, getLabel)
		PYAPI_REF_GETTER(MenuItem, getSlider)
		PYAPI_REF_GETTER(MenuItem, getImage)
		PYAPI_REF_GETTER(MenuItem, getContainer)
		PYAPI_GETTER(MenuItem, getText)
		PYAPI_METHOD(MenuItem, setText)
		PYAPI_GETTER(MenuItem, getDescription)
		PYAPI_METHOD(MenuItem, setDescription)
		PYAPI_METHOD(MenuItem, setCommand)
		PYAPI_GETTER(MenuItem, getCommand)
		PYAPI_METHOD(MenuItem, setChecked)
		PYAPI_METHOD(MenuItem, isChecked)
		PYAPI_METHOD(MenuItem, setUserTag)
		PYAPI_GETTER(MenuItem, getUserTag)
		PYAPI_GETTER(MenuItem, setImage)
		;

	// Container3dSettings
	PYAPI_BASE_CLASS(Container3dSettings)
		PYAPI_PROPERTY(Container3dSettings, enable3d)
		PYAPI_PROPERTY(Container3dSettings, position)
		PYAPI_PROPERTY(Container3dSettings, normal)
		PYAPI_PROPERTY(Container3dSettings, scale)
		PYAPI_PROPERTY(Container3dSettings, up)
		PYAPI_PROPERTY(Container3dSettings, alpha)
		PYAPI_PROPERTY(Container3dSettings, node)
		;

	// UiModule
	PYAPI_REF_BASE_CLASS(UiModule)
		PYAPI_STATIC_REF_GETTER(UiModule, instance)
		PYAPI_STATIC_REF_GETTER(UiModule, createAndInitialize)
		PYAPI_REF_GETTER(UiModule, getWidgetFactory)
		PYAPI_REF_GETTER(UiModule, getUi)
		PYAPI_REF_GETTER(UiModule, createExtendedUi)
		PYAPI_REF_GETTER(UiModule, getExtendedUi)
		PYAPI_REF_GETTER(UiModule, destroyExtendedUi)
		PYAPI_METHOD(UiModule, setCullingEnabled)
		PYAPI_METHOD(UiModule, isCullingEnabled)
		;

	// WidgetFactory
	PYAPI_REF_BASE_CLASS(WidgetFactory)
		PYAPI_REF_GETTER(WidgetFactory, createButton)
		PYAPI_REF_GETTER(WidgetFactory, createSlider)
		PYAPI_REF_GETTER(WidgetFactory, createCheckButton)
		PYAPI_REF_GETTER(WidgetFactory, createImage)
		PYAPI_REF_GETTER(WidgetFactory, createLabel)
		PYAPI_REF_GETTER(WidgetFactory, createContainer)
		;

	// Widget
	void (Widget::*setPosition1)(const Vector2f&) = &Widget::setPosition;
	void (Widget::*setSize1)(const Vector2f&) = &Widget::setSize;
	PYAPI_REF_BASE_CLASS(Widget)
		PYAPI_METHOD(Widget, setVisible)
		PYAPI_METHOD(Widget, isVisible)
		.def("setPosition", setPosition1)
		PYAPI_METHOD(Widget, setCenter)
		PYAPI_GETTER(Widget, getCenter)
		PYAPI_GETTER(Widget, getPosition)
		.def("setSize", setSize1)
		PYAPI_METHOD(Widget, setWidth)
		PYAPI_METHOD(Widget, getWidth)
		PYAPI_METHOD(Widget, setHeight)
		PYAPI_METHOD(Widget, getHeight)
		PYAPI_GETTER(Widget, getSize)
		PYAPI_METHOD(Widget, setName)
		PYAPI_GETTER(Widget, getName)
		PYAPI_METHOD(Widget, setStyle)
		PYAPI_METHOD(Widget, setStyleValue)
		PYAPI_METHOD(Widget, setFillColor)
		PYAPI_METHOD(Widget, setFillEnabled)
		PYAPI_METHOD(Widget, refresh)
		PYAPI_METHOD(Widget, hitTest)
		PYAPI_METHOD(Widget, setUIEventCommand)
		PYAPI_METHOD(Widget, setLayer)
		PYAPI_METHOD(Widget, getLayer)
		PYAPI_METHOD(Widget, getAutosize)
		PYAPI_METHOD(Widget, setAutosize)
		PYAPI_GETTER(Widget, transformPoint)
		PYAPI_METHOD(Widget, setAlpha)
		PYAPI_METHOD(Widget, getAlpha)
		PYAPI_METHOD(Widget, setBlendMode)
		PYAPI_METHOD(Widget, getBlendMode)
		PYAPI_METHOD(Widget, setScale)
		PYAPI_METHOD(Widget, getScale)
		PYAPI_METHOD(Widget, setUpdateCommand)
		PYAPI_GETTER(Widget, getUpdateCommand)
		PYAPI_METHOD(Widget, isStereo)
		PYAPI_METHOD(Widget, setStereo)
		PYAPI_METHOD(Widget, setDraggable)
		PYAPI_METHOD(Widget, isDraggable)
		PYAPI_METHOD(Widget, setPinned)
		PYAPI_METHOD(Widget, isPinned)
		PYAPI_METHOD(Widget, requestLayoutRefresh)
		PYAPI_METHOD(Widget, setRotation)
		PYAPI_METHOD(Widget, getRotation)
        PYAPI_METHOD(Widget, setSizeAnchorEnabled)
        PYAPI_METHOD(Widget, isSizeAnchorEnabled)
        PYAPI_METHOD(Widget, setSizeAnchor)
        PYAPI_GETTER(Widget, getSizeAnchor)
        // Navigation
		PYAPI_METHOD(Widget, isNavigationEnabled)
		PYAPI_METHOD(Widget, setNavigationEnabled)
		PYAPI_METHOD(Widget, setHorizontalNextWidget)
		PYAPI_METHOD(Widget, setHorizontalPrevWidget)
		PYAPI_METHOD(Widget, setVerticalNextWidget)
		PYAPI_METHOD(Widget, setVerticalPrevWidget)
		PYAPI_REF_GETTER(Widget, getHorizontalNextWidget)
		PYAPI_REF_GETTER(Widget, getHorizontalPrevWidget)
		PYAPI_REF_GETTER(Widget, getVerticalNextWidget)
		PYAPI_REF_GETTER(Widget, getVerticalPrevWidget)
		;

	// Container
	PYAPI_REF_CLASS(Container, Widget)
		.def("get3dSettings", &Container::get3dSettings, PYAPI_RETURN_INTERNAL_REF)
		PYAPI_STATIC_REF_GETTER(Container, create)
		PYAPI_METHOD(Container, isPixelOutputEnabled)
		PYAPI_METHOD(Container, setPixelOutputEnabled)
		PYAPI_REF_GETTER(Container, getPixels)
		PYAPI_METHOD(Container, addChild)
		PYAPI_METHOD(Container, removeChild)
		PYAPI_REF_GETTER(Container, getChildByIndex)
		PYAPI_REF_GETTER(Container, getChildByName)
		PYAPI_METHOD(Container, getNumChildren)
		PYAPI_METHOD(Container, isClippingEnabled)
		PYAPI_METHOD(Container, setClippingEnabled)
		// Layout
		PYAPI_METHOD(Container, setLayout)
		PYAPI_GETTER(Container, getLayout)
		PYAPI_METHOD(Container, getPadding)
		PYAPI_METHOD(Container, setPadding)
		PYAPI_METHOD(Container, getMargin)
		PYAPI_METHOD(Container, setMargin)
		PYAPI_METHOD(Container, setHorizontalAlign)
		PYAPI_METHOD(Container, getHorizontalAlign)
		PYAPI_METHOD(Container, setVerticalAlign)
		PYAPI_METHOD(Container, getVerticalAlign)

		// Interaction
		PYAPI_METHOD(Container, isEventInside)
		;

	// Button
	PYAPI_REF_CLASS(Button, Widget)
		PYAPI_STATIC_REF_GETTER(Button, create)
		PYAPI_METHOD(Button, getText)
		PYAPI_METHOD(Button, setText)
		PYAPI_METHOD(Button, isCheckable)
		PYAPI_METHOD(Button, setCheckable)
		PYAPI_METHOD(Button, setChecked)
		PYAPI_METHOD(Button, isChecked)
		PYAPI_METHOD(Button, setRadio)
		PYAPI_METHOD(Button, isRadio)
		PYAPI_METHOD(Button, setIcon)
		PYAPI_METHOD(Button, setImageEnabled)
		PYAPI_METHOD(Button, isImageEnabled)
		PYAPI_REF_GETTER(Button, getIcon)
		PYAPI_REF_GETTER(Button, getImage)
		PYAPI_REF_GETTER(Button, getLabel)
		;

	// Image
	PYAPI_REF_CLASS(Image, Widget)
		PYAPI_STATIC_REF_GETTER(Image, create)
		PYAPI_REF_GETTER(Image, getData)
		PYAPI_METHOD(Image, setData)
        PYAPI_METHOD(Image, setSourceRect)
        PYAPI_METHOD(Image, setDestRect)
        ;

	// Slider
	PYAPI_REF_CLASS(Slider, Widget)
		PYAPI_STATIC_REF_GETTER(Slider, create)
		PYAPI_METHOD(Slider, getTicks)
		PYAPI_METHOD(Slider, setTicks)
		PYAPI_METHOD(Slider, setValue)
		PYAPI_METHOD(Slider, getValue)
		;

	// Label
	PYAPI_REF_CLASS(Label, Widget)
		PYAPI_STATIC_REF_GETTER(Label, create)
		PYAPI_METHOD(Label, getText)
		PYAPI_METHOD(Label, setText)
		PYAPI_METHOD(Label, setColor)
		PYAPI_METHOD(Label, setFont)
		PYAPI_METHOD(Label, getFont)
		;

    // TextBox
    PYAPI_REF_CLASS(TextBox, Label)
        PYAPI_STATIC_REF_GETTER(TextBox, create)
        ;
}

///////////////////////////////////////////////////////////////////////////////
void OTK_API omegaToolkitPythonApiInit()
{
	static bool sApiInitialized = false;

	if(!sApiInitialized)
	{
		sApiInitialized = true;
		omsg("omegaToolkitPythonApiInit()");
		initomegaToolkit();

		// import the module by default
		omega::PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
		interp->eval("from omegaToolkit import *");
	}
}

#endif