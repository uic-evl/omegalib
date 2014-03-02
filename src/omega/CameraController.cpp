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
#include "omega/Camera.h"
#include "omega/CameraController.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////////////////////////
CameraController::CameraController(const String& name): 
	EngineModule(name),
	myCamera(NULL), 
	//myOriginalOrientation( Quaternion::Identity() ), 
	mySpeed(2.0f) 
{ 
	setPriority(PriorityLowest); 
}

///////////////////////////////////////////////////////////////////////////////////////////////////
bool CameraController::isEnabled()
{
	return (myCamera != NULL && myCamera->isControllerEnabled());
}

///////////////////////////////////////////////////////////////////////////////////////////////////
//void CameraController::updateCamera(const Vector3f& speed, const Quaternion& orientation, float dt)
//{
//	if(myCamera != NULL)
//	{
//		Quaternion o = orientation * myOriginalOrientation;
//		Vector3f ns = o * speed;
//		Vector3f position = myCamera->getPosition() + (ns * dt);
//		myCamera->setPosition(position);
//		myCamera->setOrientationAndResetController(orientation);
//	}
//}

///////////////////////////////////////////////////////////////////////////////////////////////////
//void CameraController::updateCamera(const Vector3f& speed, float yaw, float pitch, float roll, float dt)
//{
//	Quaternion orientation =   AngleAxis(pitch, Vector3f::UnitX()) * AngleAxis(yaw, Vector3f::UnitY()) * AngleAxis(roll, Vector3f::UnitZ());
//	updateCamera(speed, orientation, dt);
//}

///////////////////////////////////////////////////////////////////////////////////////////////////
Vector3f CameraController::computeSpeedVector(uint moveFlags, float speed, float strafeMultiplier)
{
	Vector3f vSpeed = Vector3f::Zero();
	// Update the observer position offset using current speed, orientation and dt.
	if(moveFlags & MoveRight)    vSpeed += Vector3f(speed * strafeMultiplier, 0, 0);
	if(moveFlags & MoveLeft)     vSpeed += Vector3f(-speed * strafeMultiplier, 0, 0);
	if(moveFlags & MoveUp)       vSpeed += Vector3f(0, speed, 0);
	if(moveFlags & MoveDown)     vSpeed += Vector3f(0, -speed, 0);
	if(moveFlags & MoveForward)  vSpeed += Vector3f(0, 0, -speed);
	if(moveFlags & MoveBackward) vSpeed += Vector3f(0, 0, speed);
	return vSpeed;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void CameraController::reset()
{
	//if( getCamera() != NULL )
	//{
	//	myOriginalOrientation = getCamera()->getOrientation();
	//}
}

///////////////////////////////////////////////////////////////////////////////////////////////
//void Camera::setOrientationAndResetController(const Quaternion& value)
//{ 
//	SceneNode::setOrientation(value);
//	if(myController != NULL) myController->reset();
//}
