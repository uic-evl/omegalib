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
#ifndef __MOUSE_MANIPULATOR_H__
#define __MOUSE_MANIPULATOR_H__

#include "omegaToolkitConfig.h"
#include "omega/Actor.h"

namespace omegaToolkit {
	///////////////////////////////////////////////////////////////////////////////////////////////
	class OTK_API MouseManipulator: public Actor
	{
	public:
		MouseManipulator();

		virtual void handleEvent(const Event& evt);
		virtual void update(const UpdateContext& context);

		Event::Flags getMoveButtonFlag() { return myMoveButtonFlag; }
		void setMoveButtonFlag(Event::Flags value) { myMoveButtonFlag = value; }

		Event::Flags getRotateButtonFlag() { return myRotateButtonFlag; }
		void setRotateButtonFlag(Event::Flags value) { myRotateButtonFlag = value; }

	private:
		Event::Flags myMoveButtonFlag;
		Event::Flags myRotateButtonFlag;

		float myPointerAxisMultiplier;
		float myMoveSpeed;
		float myRotateSpeed;
		float myResizeSpeed;
		Vector2f myLastPointerPosition;

		// pointer event data.
		bool myPointerEventReceived;
		Vector2f myPointerPosition;
		Ray myPointerRay;
		Event::Type myPointerEventType;
		int myPointerEventData;
		bool myPointerButton1Pressed;
		bool myPointerButton2Pressed;
	};
}; // namespace omegaToolkit

#endif