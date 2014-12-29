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
 *	Utility geometry methods for display systems like
 * 	- ray > pointer conversion
 *  - pointer > ray conversion
 ******************************************************************************/
#ifndef __DISPLAY_UTILS_H__
#define __DISPLAY_UTILS_H__

#include "DisplayConfig.h"

namespace omega
{
    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API DisplayUtils
    {
    public:
        //! Returns a view ray given a global (canvas) pointer position in pixel coordinates
        static Ray getViewRay(Vector2i position, const DisplayConfig& cfg);
        //! Returns a view ray given a local pointer positon and a tile index.
        static Ray	getViewRay(Vector2i position, DisplayTileConfig* dtc);
        //! Computes a view ray from a pointer or wand event. Returns true if the ray has been generated succesfully, 
        //! false otherwise (i.e. because the event is not a wand or pointer event)
        static bool getViewRayFromEvent(const Event& evt, Ray& ray, const DisplayConfig& cfg, bool normalizedPointerCoords = false, Camera* = NULL);
    private:
        DisplayUtils();
    };

}; // namespace omega

#endif