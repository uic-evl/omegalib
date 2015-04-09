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
#ifndef __PLATFORM_H__
#define __PLATFORM_H__

#include "otypes.h"

namespace omega
{
    //! Contains generic information about the platform (software, hardware and
    //! runtime), obtained by parsing the 'config/platform' section in the system
    //! configuration file. Platform settings are globally accessible and are
    //! hints that user code can use to customize its execution on different platforms.
    class OMEGA_API Platform
    {
    public:
        static void setup(const Setting& s);
        //! returns true if the flag with the specified name is set.
        static bool is(const String& name);

        //! The user interface / 2D graphics scale that should be used on this
        //! platform. Read from config/platform/scale, defaults to 1.
        static float scale;

        //! When set to true, print warning about deprecated functionality being 
        //! used.
        static bool deprecationWarnings;

    private:
        static Dictionary<String, bool> myFlags;
        Platform() {}
    };
};

#endif