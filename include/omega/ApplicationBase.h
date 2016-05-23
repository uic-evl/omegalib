/******************************************************************************
* THE OMEGA LIB PROJECT
*-----------------------------------------------------------------------------
* Copyright 2010-2015		Electronic Visualization Laboratory,
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
* What's in this file
*	The basic class containing an application entry point.
******************************************************************************/
#ifndef __APPLICATION_H__
#define __APPLICATION_H__

#include "osystem.h"
//#include "SystemManager.h"
//#include "DisplayConfig.h"
//#include "GpuResource.h"
//#include "DrawContext.h"

namespace omega
{
    ///////////////////////////////////////////////////////////////////////////
    // Forward declarations
    class SystemManager;
    class DisplaySystem;
    class RenderTarget;
    class ApplicationBase;
    class ChannelImpl;
    class Renderer;
    class Camera;

    ///////////////////////////////////////////////////////////////////////////
    //! Contains information about the context in which an update operation is taking place
    struct UpdateContext
    {
        uint64 frameNum;
        float time;
        float dt;
    };

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API ApplicationBase
    {
    public:
        // Some useful properties parsed from the command line or settable by
        // an application before calling omain

        //!! Application interactive mode
        enum InteractiveMode
        {
            //!! Use the interactive mode specified in the configuration filesyn s
            FromConfig,
            //!! Force interactive mode
            Interactive,
            //! Force non-interactive mode
            NonInteractive
        };

        //! Runs the program in interactive mode, even if the script console 
        //! is not enabled in the system configuration
        InteractiveMode interactive;

    public:
        ApplicationBase() : interactive(FromConfig) {}

        virtual const char* getName() { return "OmegaLib"; }
        virtual void setName(const String& name) = 0;
        // Implemented in Application<T>
        virtual const char* getExecutableName() = 0;
        virtual void setExecutableName(const String& name) = 0;

        //! Called once for entire application initialization tasks.
        virtual void initialize() {}
    };
}; // namespace omega

#endif
