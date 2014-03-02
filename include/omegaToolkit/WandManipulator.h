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
 * Redistribution and use in source and binary forms, with or without 
 * modification, are permitted provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this 
 * list of conditions and the following disclaimer. Redistributions in binary 
 * form must reproduce the above copyright notice, this list of conditions and 
 * the following disclaimer in the documentation and/or other materials 
 * provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" 
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE 
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE 
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
 * CONSEQUENTIAL  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF 
 * SUBSTITUTE  GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS 
 * INTERRUPTION) HOWEVER  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN 
 * CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
 * ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED OF THE 
 * POSSIBILITY OF SUCH DAMAGE.
 *-----------------------------------------------------------------------------
 * What's in this file
 *	A 3D model manipulator using a tracked wand
 *****************************************************************************/
#ifndef __WAND_MANIPULATOR__
#define __WAND_MANIPULATOR__

#include "omegaToolkitConfig.h"
#include "omega/Actor.h"

namespace omegaToolkit {

	///////////////////////////////////////////////////////////////////////////
	//! Implements a wand node manipulator. Uses the wand ray to pick and move 
	//! objects. Assumes the wand has additional analog axes to control object
	//! rotation.
	class OTK_API WandManipulator: public Actor
	{
	public:
		WandManipulator():
		    Actor("WandManipulator"),
			myMoveButtonFlag(Event::Button1),
			myRotateButtonFlag(Event::Button5),
			myButton1Pressed(false),
			myButton2Pressed(false),
			myXAxis(0),
			myYAxis(0),
			myNodeActive(false), 
			myRotateEnabled(false),
			myMoveEnabled(true),
			myScaleEnabled(true)
		{
			// Setting the interactor priority to low: we can set the target object in the 
			// application event handler and start manipulating the objec tright away, without
			// requiring a new 'selection'.
			setPriority(PriorityLow);
		}

		virtual void handleEvent(const Event& evt);
		virtual void update(const UpdateContext& conext);
		virtual void setup(const Setting& s);
		virtual bool handleCommand(const String& cmd);

		Event::Flags getMoveButtonFlag() { return myMoveButtonFlag; }
		void setMoveButtonFlag(Event::Flags value) { myMoveButtonFlag = value; }

		Event::Flags getRotateButtonFlag() { return myRotateButtonFlag; }
		void setRotateButtonFlag(Event::Flags value) { myRotateButtonFlag = value; }

	private:
		Event::Flags myMoveButtonFlag;
		Event::Flags myRotateButtonFlag;

		bool myRotateEnabled;
		bool myMoveEnabled;
		bool myScaleEnabled;

		float myHandleDistance;
		Vector3f myHandlePosition;
		Sphere myStartBSphere;
		
		Quaternion myWandOrientation;
		Quaternion myStartOrientation;
		Quaternion myStartWandOrientationInv;

		// pointer event data.
		bool myPointerEventReceived;
		Ray myPointerRay;
		Event::Type myPointerEventType;

		bool myButton1Pressed;
		bool myButton2Pressed;

		float myXAxis;
		float myYAxis;
		bool myNodeActive;
	};
}; // namespace omegaToolkit

#endif