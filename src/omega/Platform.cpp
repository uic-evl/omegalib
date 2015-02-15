/******************************************************************************
* THE OMEGA LIB PROJECT
*-----------------------------------------------------------------------------
* Copyright 2010-2015	Electronic Visualization Laboratory,
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
* What's in this file:
* Contains generic information about the platform (software, hardware and
* runtime), obtained by parsing the 'config/platform' section in the system
* configuration file. Platform settings are globally accessible and are
* hints that user code can use to customize its execution on different platforms.
******************************************************************************/
#include"omega/Platform.h"

using namespace omega;

float Platform::scale = 1.0f;
bool Platform::deprecationWarnings = true;
Dictionary<String, bool> Platform::myFlags;

///////////////////////////////////////////////////////////////////////////////
void Platform::setup(const Setting& s)
{
    for(int i = 0; i < s.getLength(); i++)
    {
        Setting& sc = s[i];
        if(sc.getType() == Setting::TypeBoolean)
        {
            myFlags[sc.getName()] = (bool)sc;
        }
    }

    scale = Config::getFloatValue("scale", s, 1.0f);
    deprecationWarnings = Config::getBoolValue("deprecationWarnings", s, true);
}

///////////////////////////////////////////////////////////////////////////////
bool Platform::is(const String& name)
{
    return myFlags[name];
}

