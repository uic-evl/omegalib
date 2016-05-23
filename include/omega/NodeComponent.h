/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2016		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2016, Electronic Visualization Laboratory,  
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
 *	The base class for objects that can be attached to a scene node.
 ******************************************************************************/
#ifndef __ISCENE_OBJECT_H__
#define __ISCENE_OBJECT_H__

#include "osystem.h"

namespace omega {
    class Engine;
    class SceneNode;
    struct DrawContext;
    struct UpdateContext;
    struct RenderState;

    ///////////////////////////////////////////////////////////////////////////
    //! NodeComponent is the base class for objects that can be attached to a 
    //! scene node
    class OMEGA_API NodeComponent: public ReferenceType
    {
    friend class SceneNode;
    public:
        NodeComponent(): myNeedBoundingBoxUpdate(false), myOwner(NULL) {}
        virtual void update(const UpdateContext& context) = 0;
        virtual const AlignedBox3* getBoundingBox() { return NULL; }
        virtual bool hasBoundingBox() { return false; }
        virtual bool isInitialized() { return true; }
        virtual bool hasCustomRayIntersector() { return false; }
        virtual bool intersectRay(const Ray& ray, Vector3f* hitPoint) { return false; }
        virtual void initialize(Engine* server) { }
        virtual void updateBoundingBox() { myNeedBoundingBoxUpdate = false; }
        virtual void onAttached(SceneNode*) { }
        virtual void onDetached(SceneNode*) { }
        
        void requestBoundingBoxUpdate();
        bool needsBoundingBoxUpdate() { return myNeedBoundingBoxUpdate; }

        SceneNode* getOwner() { return myOwner; }

    private:
        void attach(SceneNode* owner) { myOwner = owner; onAttached(myOwner); }
        void detach(SceneNode* owner) { onDetached(myOwner); myOwner = NULL; }

        bool myNeedBoundingBoxUpdate;
        SceneNode* myOwner;
    };
}; // namespace omega

#endif