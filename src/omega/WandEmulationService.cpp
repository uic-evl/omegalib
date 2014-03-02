/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#include "omega/WandEmulationService.h"
#include "omega/DisplaySystem.h"

using namespace omega;


///////////////////////////////////////////////////////////////////////////////////////////////////
WandEmulationService::WandEmulationService():
	myEventFlags(0)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool WandEmulationService::processKey(const Event* evt, const char key, Event::Flags flag)
{
	if(evt->isKeyDown(key))
	{
		myEventFlags |= flag;
		generateEvent(Event::Down);
		return true;
	}
	if(evt->isKeyUp(key))
	{
		myEventFlags &= ~flag;
		generateEvent(Event::Up);
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool WandEmulationService::processPointer(const Event* evt)
{
	myXAxis = 0;
	myYAxis = 0;

	// Update the wand position and orientation using the ray stored in the pointer event.
	DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
	Ray r;
	if(ds->getViewRayFromEvent(*evt, r))
	{
		myWandPosition = r.getOrigin();
		myWandOrientation = Math::buildRotation(r.getDirection(), Vector3f(0, 0, -1), Vector3f::UnitY());
	}

	if((myEventFlags & Event::Button2) == Event::Button2)
	{
		updateAxes(*evt);
	}

	if(evt->getType() == Event::Down)
	{
		myEventFlags = evt->getFlags();
		generateEvent(Event::Down);
		evt->setProcessed();
		return true;
	}
	if(evt->getType() == Event::Up)
	{
		myEventFlags = evt->getFlags();
		generateEvent(Event::Up);
		evt->setProcessed();
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WandEmulationService::updateAxes(const Event& evt)
{
	float pointerAxisMultiplier = 100.0f;

	// Process mouse axes.
	DisplaySystem* ds = SystemManager::instance()->getDisplaySystem();
	Vector2i resolution = ds->getCanvasSize();

	float curX = evt.getPosition().x() / resolution[0];
	float curY = evt.getPosition().y() / resolution[1];

	float dx = (curX - myLastPointerPosition[0]) * pointerAxisMultiplier;
	float dy = -(curY - myLastPointerPosition[1]) * pointerAxisMultiplier;

	myXAxis = dx;
	myYAxis = dy;

	// Save current (normalized) pointer position.
	myLastPointerPosition[0] = curX;
	myLastPointerPosition[1] = curY;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WandEmulationService::poll()
{
	lockEvents();
	int numEvts = getManager()->getAvailableEvents();
	bool eventWasKeyUpDown = false;
	for(int i = 0; i < numEvts; i++)
	{
		Event* evt = getEvent(i);
		if(evt->getServiceType() == Event::ServiceTypeKeyboard)
		{
			eventWasKeyUpDown |= processKey(evt, 'a', Event::ButtonLeft);
			eventWasKeyUpDown |= processKey(evt, 'd', Event::ButtonRight);
			eventWasKeyUpDown |= processKey(evt, 'w', Event::ButtonUp);
			eventWasKeyUpDown |= processKey(evt, 's', Event::ButtonDown);

			// Buttons 1, 2, 3 Are mapped to mouse buttons. Add some 
			// more buttons using the keyboard

			eventWasKeyUpDown |= processKey(evt, '1', Event::Button3);
			eventWasKeyUpDown |= processKey(evt, '2', Event::Button4);
			eventWasKeyUpDown |= processKey(evt, '3', Event::Button5);
			eventWasKeyUpDown |= processKey(evt, '4', Event::Button6);
			evt->setProcessed();
		}
		
		if(evt->getServiceType() == Event::ServiceTypePointer)
		{
			eventWasKeyUpDown |= processPointer(evt);
			evt->setProcessed();
		}
	}

	if(!eventWasKeyUpDown)
	{
		generateEvent(Event::Update);
	}

	unlockEvents();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void WandEmulationService::generateEvent(Event::Type type)
{
	Event* newEvent = writeHead();
	newEvent->reset(type, Service::Wand, 0);
	newEvent->setPosition(myWandPosition);
	newEvent->setOrientation(myWandOrientation);
	newEvent->setFlags(myEventFlags);

	// Add axis data.
	newEvent->setExtraDataType(Event::ExtraDataFloatArray);
	newEvent->setExtraDataFloat(0, myXAxis);
	newEvent->setExtraDataFloat(1, myYAxis);
}
