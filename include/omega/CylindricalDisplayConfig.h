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
 *	A display configuration builder that can be used for cylindrical display
 *  systems like CAVE2.
 ******************************************************************************/
#ifndef __CYLINDRICAL_DISPLAY_CONFIG__
#define __CYLINDRICAL_DISPLAY_CONFIG__

#include "ApplicationBase.h"
#include "DisplayConfig.h"

namespace omega
{
    ///////////////////////////////////////////////////////////////////////////
    class CylindricalDisplayConfig: public DisplayConfigBuilder
    {
    public:
        static Vector3f computeEyePosition(
            const Vector3f headSpaceEyePosition,
            const AffineTransform3& headTransform,
            const DrawContext& dc);
    public:
        virtual bool buildConfig(DisplayConfig& cfg, Setting& scfg);
        virtual void onCanvasChange(DisplayConfig& cfg);

    private:
        std::pair<bool, Vector2f> calculateScreenPosition(float x, float y, float z);

        float myRadius;
        float myHeight;
        // Offset of cylinder from floor plane (i.e. y=0 plane in tracking 
        // system space)
        float myYOffset;
    };
}; // namespace omega

#endif
