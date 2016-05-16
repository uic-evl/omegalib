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
 *	The core class for the omegalib scene tree.
 ******************************************************************************/
#ifndef __SCENE_NODE_H__
#define __SCENE_NODE_H__

#include "osystem.h"
#include "omega/NodeComponent.h"
#include "omega/Node.h"
#include "omega/Color.h"

namespace omega {
    class Engine;
    class Renderable;
    class SceneNode;
    class Camera;
    class TrackedObject;
    class NodeComponent;
    struct RenderState;

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API SceneNodeListener
    {
    public:
        virtual void onVisibleChanged(SceneNode* source, bool value) {}
        virtual void onSelectedChanged(SceneNode* source, bool value) {}
        virtual void onParentChanged(SceneNode* source, SceneNode* newParent) {}
        //! Called when this node becomes part of the scene tree.
        virtual void onAttachedToScene(SceneNode* source) {}
        //! Called when this node is removed from the scene tree either directly 
        //! or due to a parent node beging removed.
        virtual void onDetachedFromScene(SceneNode* source) {}
    };

    ///////////////////////////////////////////////////////////////////////////
    //! Represents a node in the omegalib scene graph.
    //! @remarks
    //!		SceneNode instances add some functionality over the Node base class:
    //!			- renderable objects can be attached to a scene node;
    //!			- a scene node has a bounding box;
    //!			- scene nodes have selection and visibility flags
    //!			- it is possible to attach listeners to scene nodes, to handle 
    //!				visibility change, selection change and other events.
    class OMEGA_API SceneNode: public Node
    {
    public:
//		typedef ChildNode<SceneNode> Child;
        enum HitType { 
            //! Perform hit tests on object bounding sphere
            HitBoundingSphere, 
            //! Perform hit tests using best available intersector for the node.
            HitBest };
        static SceneNode* create(const String& name);

    public:
        SceneNode(Engine* server):
            myServer(server),
            mySelectable(false),
            myChanged(false),
            myVisible(true),
            mySelected(false),
            myFacingCamera(NULL),
            myTracker(NULL),
            myNeedsBoundingBoxUpdate(false),
            myFacingCameraFixedY(false),
            myFlags(0),
            myCurrentUpdateContext(NULL)
            {}

        SceneNode(Engine* server, const String& name):
            Node(name),
            myServer(server),
            mySelectable(false),
            myChanged(false),
            myVisible(true),
            mySelected(false),
            myFacingCamera(NULL),
            myTracker(NULL),
            myNeedsBoundingBoxUpdate(false),
            myFacingCameraFixedY(false),
            myFlags(0),
            myCurrentUpdateContext(NULL)
            {}

        Engine* getEngine();

        // Object
        //@{
        void addComponent(NodeComponent* o);
        int getNumComponents();
        void removeComponent(NodeComponent* o);
        //@}

        // Options
        //@{
        bool isSelectable();
        void setSelectable(bool value);
        //! Sets this node visibility. A node visibility does not influence 
        //! children of the node, but only scene objects attached to the node
        //! itself. To change children visibility use setChildrenVisible instead.
        void setVisible(bool value);
        bool isVisible();
        void setChildrenVisible(bool value);
        void setSelected(bool value);
        bool isSelected();
        // Returns true if this node is attached to the scene (that is, if 
        // there is a path from the scene root to this node)
        bool isAttachedToScene();
        //@}

        // Bounding box handling
        //@{
        const AlignedBox3& getBoundingBox();
        const Sphere& getBoundingSphere();
        const Vector3f& getBoundMinimum();
        const Vector3f& getBoundMaximum();
        Vector3f getBoundCenter();
        float getBoundRadius();
        //! Force an update of the bounding box for this node. Usually called by 
        //! NodeComponent objects attached to this node.
        void requestBoundingBoxUpdate();
        //@}

        // Listeners
        //@{
        void addListener(SceneNodeListener* listener);
        void removeListener(SceneNodeListener* listener);
        //@}

        //! Hit test.
        bool hit(const Ray& ray, Vector3f* hitPoint, HitType type);

        //! Invoked the update function for all node components on this node and down in the hierarchy.
        void update(const UpdateContext& context);
        //! @internal Updates all transforms from this node down in the hierarchy.
        virtual void update(bool updateChildren, bool parentHasChanged);
        virtual void needUpdate(bool forceParentUpdate = true);

        void setTag(const String& value) { myTag = value; }
        const String& getTag() { return myTag; } 

        //! Billboard mode
        //@{
        void setFacingCamera(Camera* cam);
        Camera* getFacingCamera();
        //! When set to true, Y axis for nodes facing camera will be fixed to the
        //! world Y axis. When set to false, the Y axis will follow the camera
        //! Y axis.
        void setFacingCameraFixedY(bool value);
        bool isFacingCameraFixedY();
        //@}

        //! Trackable object
        //@{
        void followTrackable(int trackableId);
        void setFollowOffset(const Vector3f& offset, const Quaternion& ooffset);
        TrackedObject* getTracker();
        void unfollow();
        //@}

        //! Node flags
        //@{
        void setFlag(uint bit);
        void unsetFlag(uint bit);
        bool isFlagSet(uint bit);
        //@}

    protected:
        virtual void updateTraversal(const UpdateContext& context);
        /// Only available internally - notification of parent.
        virtual void setParent(Node* parent);
        void onAttachedToScene();
        void onDetachedFromScene();
    
    private:
        void updateBoundingBox(bool force = false);
        bool needsBoundingBoxUpdate();

    private:
        Engine* myServer;

        List<SceneNodeListener*> myListeners;

        List< Ref<NodeComponent> > myObjects;

        bool mySelectable;
        bool mySelected;
        bool myVisible;

        bool myChanged;
        AlignedBox3 myBBox;
        Sphere myBSphere;

        String myTag;
        uint myFlags;

        bool myNeedsBoundingBoxUpdate;

        // Target camera for billboard mode. Can't use Ref due to circular dependency.
        Camera* myFacingCamera;
        // When set to true, Y axis for nodes facing camera will be fixed to the
        // world Y axis. 
        bool myFacingCameraFixedY;
        // Tracked object.
        TrackedObject* myTracker;
        
        const UpdateContext* myCurrentUpdateContext;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline void SceneNode::requestBoundingBoxUpdate() 
    { 
        // If we already requested a bounding box update, we are done.
        if(!myNeedsBoundingBoxUpdate)
        {
            myNeedsBoundingBoxUpdate = true;
            SceneNode* parent = dynamic_cast<SceneNode*>(getParent());
            if(parent != NULL) parent->requestBoundingBoxUpdate();
        }
    }

    ///////////////////////////////////////////////////////////////////////////
    inline bool SceneNode::needsBoundingBoxUpdate() 
    { return myNeedsBoundingBoxUpdate; }

    ///////////////////////////////////////////////////////////////////////////
    inline Engine* SceneNode::getEngine()
    { return myServer; }

    ///////////////////////////////////////////////////////////////////////////
    inline bool SceneNode::isSelectable() 
    { return mySelectable; }

    ///////////////////////////////////////////////////////////////////////////
    inline void SceneNode::setSelectable(bool value) 
    { mySelectable = value; }

    ///////////////////////////////////////////////////////////////////////////
    inline void SceneNode::setFacingCamera(Camera* cam)
    { myFacingCamera = cam; }

    ///////////////////////////////////////////////////////////////////////////
    inline Camera* SceneNode::getFacingCamera()
    { return myFacingCamera; }

    ///////////////////////////////////////////////////////////////////////////
    inline void SceneNode::setFacingCameraFixedY(bool value)
    { myFacingCameraFixedY = value; }

    ///////////////////////////////////////////////////////////////////////////
    inline bool SceneNode::isFacingCameraFixedY()
    { return myFacingCameraFixedY; }

    ///////////////////////////////////////////////////////////////////////////
    inline TrackedObject* SceneNode::getTracker()
    { return myTracker; }

    ///////////////////////////////////////////////////////////////////////////
    inline void SceneNode::setFlag(uint bit)
    { myFlags |= bit; }

    ///////////////////////////////////////////////////////////////////////////
    inline void SceneNode::unsetFlag(uint bit)
    { myFlags &= ~bit; }

    ///////////////////////////////////////////////////////////////////////////
    inline bool SceneNode::isFlagSet(uint bit)
    { return (myFlags & bit) != 0; }

    // This is a definition from NodeComponent. Doing it here because we need
    // SceneNode.
    ///////////////////////////////////////////////////////////////////////////
    inline void NodeComponent::requestBoundingBoxUpdate() 
    { 
        myNeedBoundingBoxUpdate = true; 
        if(myOwner) 
        {
            myOwner->requestBoundingBoxUpdate();
        }
    }
}; // namespace omega

#endif
