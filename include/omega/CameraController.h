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
#ifndef __CAMERA_CONTROLLER_H__
#define __CAMERA_CONTROLLER_H__

#include "osystem.h"
#include "ApplicationBase.h"
#include "ModuleServices.h"

namespace omega {
	class Camera;

	///////////////////////////////////////////////////////////////////////////////////////////////
	class OMEGA_API CameraController: public EngineModule
	{
	public:
		enum MoveFlags { 
			// Map move flags to some standard event button flags for convenience
			MoveLeft = Event::ButtonLeft,
			MoveRight = Event::ButtonRight,
			MoveUp = Event::Button5,
			MoveDown = Event::Button6,
			MoveForward = Event::ButtonUp,
			MoveBackward = Event::ButtonDown,
			};

	public:
		CameraController(const String& name = "CameraController");

		bool isEnabled();

		virtual void setup(Setting& s) {}
		virtual void update(const UpdateContext& context) {}
		virtual void handleEvent(const Event& evt) {}
		virtual void reset();
		void setCamera(Camera* value) { myCamera = value; reset(); }
		Camera* getCamera() { return myCamera; }
		
		float getSpeed() { return mySpeed; }
		void setSpeed(float value) { mySpeed = value; }

		//! Utility method: updates the camera position using speed, yaw pich roll, and a time step.
		//void updateCamera(const Vector3f& speed, float yaw, float pitch, float roll, float dt);
		//void updateCamera(const Vector3f& speed, const Quaternion& orientation, float dt);

		//! Utility method: return a speed vector depeding on move flags and speed multipliers.
		//! Useful for turning digital input from keyboards / gamepads into speed information.
		Vector3f computeSpeedVector(uint moveFlags, float speed, float strafeMultiplier = 1.0f);

	private:
		Camera* myCamera;
		//Quaternion myOriginalOrientation;
		
	protected:
		float mySpeed;
	};

}; // namespace omega

#endif