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
*	The base class for classes that define screen aligned geometry for warping post-render.
******************************************************************************/
#ifndef __WARP_MESH_H__
#define __WARP_MESH_H__

#include "osystem.h"
#include "omega/ApplicationBase.h"
#include "DrawInterface.h"

namespace omega {
    class Renderer;
    class DrawCentext;

    ///////////////////////////////////////////////////////////////////////////
    //! The base class for classes that define screen aligned geometry for warping post-render.
    class OMEGA_API WarpMesh: public ReferenceType
    {
    public:
        WarpMesh()
        {
            // EMPTY!
        }

        virtual ~WarpMesh()
        {
            // EMPTY!
        }

        virtual void initialize() { myInitialized = true; }

        virtual void prepare(Renderer* client, const DrawContext& context)
        {
            // EMPTY!
        }

        virtual void render(Renderer* client, const DrawContext& context)
        {
            // EMPTY!
        }

        virtual void dispose()
        {
            // EMPTY!
        }

        bool isInitialized() { return myInitialized; }

    private:
        bool myInitialized;
    };

}; // namespace omega

#endif
