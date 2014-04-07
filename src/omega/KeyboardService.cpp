/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2014		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *  Dennis                  koracas@gmail.com
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
 * What's in this file:
 *	A service to receive and process keyboard input
 ******************************************************************************/
#include "omega/SystemManager.h"
#include "omega/DisplaySystem.h"
#include "omega/KeyboardService.h"

#ifdef OMEGA_USE_DISPLAY_GLUT
#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include "GL/freeglut.h"
#endif
#endif

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
KeyboardService* KeyboardService::mysInstance = NULL;
static uint sKeyFlags = 0;

#define HANDLE_KEY_FLAG(keycode, flag) \
    if(key == keycode && type == Event::Down) sKeyFlags |= Event::flag; \
    if(key == keycode && type == Event::Up) sKeyFlags &= ~Event::flag;

///////////////////////////////////////////////////////////////////////////////
void KeyboardService::keyboardButtonCallback( uint key, Event::Type type )
{
    if(mysInstance)
    {
        mysInstance->lockEvents();

        Event* evt = mysInstance->writeHead();
        evt->reset(type, Service::Keyboard, key);

        HANDLE_KEY_FLAG(296, Alt)
        HANDLE_KEY_FLAG(292, Shift)
        HANDLE_KEY_FLAG(294, Ctrl)

        // Convert arrow keys to buttons. This allows user code to do es. 
        // evt.isButtonDown(Event::ButtonLeft) without having to make a 
        // separate call to isKeyDown when using keyboards instead of gamepads.
        HANDLE_KEY_FLAG(KC_LEFT, ButtonLeft);
        HANDLE_KEY_FLAG(KC_RIGHT, ButtonRight);
        HANDLE_KEY_FLAG(KC_DOWN, ButtonDown);
        HANDLE_KEY_FLAG(KC_UP, ButtonUp);

        // Add some special keys as buttons
        HANDLE_KEY_FLAG(KC_RETURN, Button4);
        HANDLE_KEY_FLAG(KC_BACKSPACE, Button5);
        HANDLE_KEY_FLAG(KC_TAB, Button6);
        HANDLE_KEY_FLAG(KC_HOME, Button7);

        evt->setFlags(sKeyFlags);

        mysInstance->unlockEvents();
    }
}

///////////////////////////////////////////////////////////////////////////////
void glutKeyDown(unsigned char key, int x, int y)
{
    KeyboardService::keyboardButtonCallback((uint)key, Event::Down);
}

///////////////////////////////////////////////////////////////////////////////
void glutKeyUp(unsigned char key, int x, int y)
{
    KeyboardService::keyboardButtonCallback((uint)key, Event::Up);
}

///////////////////////////////////////////////////////////////////////////////
void KeyboardService::initialize() 
{
    mysInstance = this;
#ifdef OMEGA_USE_DISPLAY_GLUT
    if(SystemManager::instance()->getDisplaySystem()->getId() == DisplaySystem::Glut)
    {
        glutKeyboardFunc(glutKeyDown); 
        glutKeyboardUpFunc(glutKeyUp); 
    }
#endif
}

///////////////////////////////////////////////////////////////////////////////
void KeyboardService::dispose() 
{
    mysInstance = NULL;
}
