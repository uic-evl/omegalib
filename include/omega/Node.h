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
 * Original code taken from OGRE
 * Copyright (c) 2000-2009 Torus Knot Software Ltd
 *  For the latest info, see http://www.ogre3d.org/
 *-----------------------------------------------------------------------------
 * What's in this file
 *	A generic node in a transformation hierarchy
 ******************************************************************************/
#ifndef _Node_H__
#define _Node_H__

#include<set>
#include "osystem.h"


namespace omega {
	///////////////////////////////////////////////////////////////////////////
	/** Class representing a general-purpose node in an articulated scene graph.
        @remarks
            A node in the scene graph is a node in a structured tree. A node contains
            information about the transformation which will apply to it and all 
			of it's children. Child nodes can have transforms of their own, which
            are combined with their parent's transformations.
    */
    class OMEGA_API Node: public ReferenceType 
    {
    public:
        /** Enumeration denoting the spaces which a transform can be relative to.
        */
        enum TransformSpace
        {
            /// Transform is relative to the local space
            TransformLocal,
            /// Transform is relative to the space of the parent node
            TransformParent,	
            /// Transform is relative to world space
            TransformWorld
        };

        typedef Dictionary<String, Ref<Node> > ChildNodeMap;

    public:
        /** Constructor, should only be called by parent, not directly.
        @remarks
            Generates a name.
        */
        Node();
        /** Constructor, should only be called by parent, not directly.
        @remarks
            Assigned a name.
        */
        Node(const String& name);

        virtual ~Node();  

        /** Returns the name of the node. */
        const String& getName(void) const;

        /** Gets this node's parent (NULL if this is the root).
        */
        virtual Node* getParent(void) const;

        /** Returns a quaternion representing the nodes orientation.
        */
        virtual const Quaternion & getOrientation() const;

        /** Sets the orientation of this node via a quaternion.
        @remarks
            Orientations, unlike other transforms, are not always inherited by child nodes.
            Whether or not orientations affect the orientation of the child nodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent node to apply to a child node (e.g. where the child node is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child node is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
        @par
            Note that rotations are oriented around the node's origin.
        */
        virtual void setOrientation( const Quaternion& q );

        /** Sets the orientation of this node via quaternion parameters.
        @remarks
            Orientations, unlike other transforms, are not always inherited by child nodes.
            Whether or not orientations affect the orientation of the child nodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent node to apply to a child node (e.g. where the child node is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child node is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
        @par
            Note that rotations are oriented around the node's origin.
        */
        virtual void setOrientation( float w, float x, float y, float z);

        /** Resets the nodes orientation (local axes as world axes, no rotation).
        @remarks
            Orientations, unlike other transforms, are not always inherited by child nodes.
            Whether or not orientations affect the orientation of the child nodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent node to apply to a child node (e.g. where the child node is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child node is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
        @par
            Note that rotations are oriented around the node's origin.
        */
        virtual void resetOrientation(void);

        /** Sets the position of the node relative to it's parent.
        */
        virtual void setPosition(const Vector3f& pos);

        /** Sets the position of the node relative to it's parent.
        */
        virtual void setPosition(float x, float y, float z);

        /** Gets the position of the node relative to it's parent.
        */
        virtual const Vector3f & getPosition(void) const;

        /** Sets the scaling factor applied to this node.
        @remarks
            Scaling factors, unlike other transforms, are not always inherited by child nodes.
            Whether or not scalings affect the size of the child nodes depends on the setInheritScale
            option of the child. In some cases you want a scaling factor of a parent node to apply to
            a child node (e.g. where the child node is a part of the same object, so you want it to be
            the same relative size based on the parent's size), but not in other cases (e.g. where the
            child node is just for positioning another object, you want it to maintain it's own size).
            The default is to inherit as with other transforms.
        @par
            Note that like rotations, scalings are oriented around the node's origin.
        */
        virtual void setScale(const Vector3f& scale);

        /** Sets the scaling factor applied to this node.
        @remarks
            Scaling factors, unlike other transforms, are not always inherited by child nodes.
            Whether or not scalings affect the size of the child nodes depends on the setInheritScale
            option of the child. In some cases you want a scaling factor of a parent node to apply to
            a child node (e.g. where the child node is a part of the same object, so you want it to be
            the same relative size based on the parent's size), but not in other cases (e.g. where the
            child node is just for positioning another object, you want it to maintain it's own size).
            The default is to inherit as with other transforms.
        @par
            Note that like rotations, scalings are oriented around the node's origin.
        */
        virtual void setScale(float x, float y, float z);

        /** Gets the scaling factor of this node.
        */
        virtual const Vector3f & getScale(void) const;

        /** Tells the node whether it should inherit orientation from it's parent node.
        @remarks
            Orientations, unlike other transforms, are not always inherited by child nodes.
            Whether or not orientations affect the orientation of the child nodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent node to apply to a child node (e.g. where the child node is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child node is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
        @param inherit If true, this node's orientation will be affected by its parent's orientation.
            If false, it will not be affected.
        */
        virtual void setInheritOrientation(bool inherit);

        /** Returns true if this node is affected by orientation applied to the parent node. 
        @remarks
            Orientations, unlike other transforms, are not always inherited by child nodes.
            Whether or not orientations affect the orientation of the child nodes depends on
            the setInheritOrientation option of the child. In some cases you want a orientating
            of a parent node to apply to a child node (e.g. where the child node is a part of
            the same object, so you want it to be the same relative orientation based on the
            parent's orientation), but not in other cases (e.g. where the child node is just
            for positioning another object, you want it to maintain it's own orientation).
            The default is to inherit as with other transforms.
        @remarks
            See setInheritOrientation for more info.
        */
        virtual bool getInheritOrientation(void) const;

        /** Tells the node whether it should inherit scaling factors from it's parent node.
        @remarks
            Scaling factors, unlike other transforms, are not always inherited by child nodes.
            Whether or not scalings affect the size of the child nodes depends on the setInheritScale
            option of the child. In some cases you want a scaling factor of a parent node to apply to
            a child node (e.g. where the child node is a part of the same object, so you want it to be
            the same relative size based on the parent's size), but not in other cases (e.g. where the
            child node is just for positioning another object, you want it to maintain it's own size).
            The default is to inherit as with other transforms.
        @param inherit If true, this node's scale will be affected by its parent's scale. If false,
            it will not be affected.
        */
        virtual void setInheritScale(bool inherit);

        /** Returns true if this node is affected by scaling factors applied to the parent node. 
        @remarks
            See setInheritScale for more info.
        */
        virtual bool getInheritScale(void) const;

        /** Scales the node, combining it's current scale with the passed in scaling factor. 
        @remarks
            This method applies an extra scaling factor to the node's existing scale, (unlike setScale
            which overwrites it) combining it's current scale with the new one. E.g. calling this 
            method twice with Vector3f(2,2,2) would have the same effect as setScale(Vector3f(4,4,4)) if
            the existing scale was 1.
        @par
            Note that like rotations, scalings are oriented around the node's origin.
        */
        virtual void scale(const Vector3f& scale);

        /** Scales the node, combining it's current scale with the passed in scaling factor. 
        @remarks
            This method applies an extra scaling factor to the node's existing scale, (unlike setScale
            which overwrites it) combining it's current scale with the new one. E.g. calling this 
            method twice with Vector3f(2,2,2) would have the same effect as setScale(Vector3f(4,4,4)) if
            the existing scale was 1.
        @par
            Note that like rotations, scalings are oriented around the node's origin.
        */
        virtual void scale(float x, float y, float z);

        /** Moves the node along the Cartesian axes.
            @par
                This method moves the node by the supplied vector along the
                world Cartesian axes, i.e. along world x,y,z
            @param 
                d Vector with x,y,z values representing the translation.
            @param
                relativeTo The space which this transform is relative to.
        */
        virtual void translate(const Vector3f& d, TransformSpace relativeTo = TransformParent);
        /** Moves the node along the Cartesian axes.
            @par
                This method moves the node by the supplied vector along the
                world Cartesian axes, i.e. along world x,y,z
            @param 
                x
            @param
                y
            @param
                z float x, y and z values representing the translation.
            @param
            relativeTo The space which this transform is relative to.
        */
        virtual void translate(float x, float y, float z, TransformSpace relativeTo = TransformParent);
        /** Moves the node along arbitrary axes.
            @remarks
                This method translates the node by a vector which is relative to
                a custom set of axes.
            @param 
                axes A 3x3 Matrix containg 3 column vectors each representing the
                axes X, Y and Z respectively. In this format the standard cartesian
                axes would be expressed as:
                <pre>
                1 0 0
                0 1 0
                0 0 1
                </pre>
                i.e. the identity matrix.
            @param 
                move Vector relative to the axes above.
            @param
            relativeTo The space which this transform is relative to.
        */
        virtual void translate(const Matrix3f& axes, const Vector3f& move, TransformSpace relativeTo = TransformParent);
        /** Moves the node along arbitrary axes.
            @remarks
            This method translates the node by a vector which is relative to
            a custom set of axes.
            @param 
                axes A 3x3 Matrix containg 3 column vectors each representing the
                axes X, Y and Z respectively. In this format the standard cartesian
                axes would be expressed as
                <pre>
                1 0 0
                0 1 0
                0 0 1
                </pre>
                i.e. the identity matrix.
            @param 
                x,y,z Translation components relative to the axes above.
            @param
                relativeTo The space which this transform is relative to.
        */
        virtual void translate(const Matrix3f& axes, float x, float y, float z, TransformSpace relativeTo = TransformParent);

        //! Rotate the node around the Z-axis.
        virtual void roll(const float& angle, TransformSpace relativeTo = TransformLocal);
        //! Rotate the node around the Z-axis, The angle is specified in degrees.
        void rollDeg(const float& angle, TransformSpace relativeTo = TransformLocal)
		{ roll(angle * Math::DegToRad, relativeTo); }

        //! Rotate the node around the X-axis.
        virtual void pitch(const float& angle, TransformSpace relativeTo = TransformLocal);
        //! Rotate the node around the X-axis, The angle is specified in degrees.
        virtual void pitchDeg(const float& angle, TransformSpace relativeTo = TransformLocal)
		{ pitch(angle * Math::DegToRad, relativeTo); }

        //! Rotate the node around the Y-axis.
        virtual void yaw(const float& angle, TransformSpace relativeTo = TransformLocal);
        //! Rotate the node around the Y-axis, The angle is specified in degrees.
        virtual void yawDeg(const float& angle, TransformSpace relativeTo = TransformLocal)
		{ yaw(angle * Math::DegToRad, relativeTo); }

        /** Rotate the node around an arbitrary axis.
        */
        virtual void rotate(const Vector3f& axis, const float& angle, TransformSpace relativeTo = TransformLocal);

        /** Rotate the node around an aritrary axis using a Quarternion.
        */
        virtual void rotate(const Quaternion& q, TransformSpace relativeTo = TransformLocal);

		//! Rotate the node to make its positive Z axis point toward the specified position.
		virtual void lookAt(const Vector3f& position, const Vector3f& upVector);

        /** Gets a matrix whose columns are the local axes based on
            the nodes orientation relative to it's parent. */
        virtual Matrix3f getLocalAxes(void) const;

        /** Adds a (precreated) child scene node to this node. If it is attached to another node,
            it must be detached first.
        @param child The Node which is to become a child node of this one
        */
        virtual void addChild(Node* child);

        /** Reports the number of child nodes under this one.
        */
        virtual unsigned short numChildren(void) const;

        /** Gets a pointer to a child node.
        @remarks
            There is an alternate getChild method which returns a named child.
        */
        virtual Node* getChild(unsigned short index) const;    

        /** Gets a pointer to a named child node.
        */
        virtual Node* getChild(const String& name) const;

        /** Drops the specified child from this node. 
        @remarks
            Does not delete the node, just detaches it from
            this parent, potentially to be reattached elsewhere. 
            There is also an alternate version which drops a named
            child from this node.
        */
        virtual void removeChild(unsigned short index);
        /** Drops the specified child from this node. 
        @remarks
        Does not delete the node, just detaches it from
        this parent, potentially to be reattached elsewhere. 
        There is also an alternate version which drops a named
        child from this node.
        */
        virtual void removeChild(Node* child);

        /** Drops the named child from this node. 
        @remarks
            Does not delete the node, just detaches it from
            this parent, potentially to be reattached elsewhere.
        */
        virtual void removeChild(const String& name);
        /** Removes all child Nodes attached to this node. Does not delete the nodes, just detaches them from
            this parent, potentially to be reattached elsewhere.
        */
        virtual void removeAllChildren(void);
		
		//! #PYPI Returns the list of children of this node
		const List<Node*>& getChildren() const { return mChildrenList; }

		/** Sets the final world position of the node directly.
		@remarks 
			It's advisable to use the local setPosition if possible
		*/
		virtual void _setDerivedPosition(const Vector3f& pos);

		/** Sets the final world orientation of the node directly.
		@remarks 
		It's advisable to use the local setOrientation if possible, this simply does
		the conversion for you.
		*/
		virtual void _setDerivedOrientation(const Quaternion& q);

		/** Gets the orientation of the node as derived from all parents.
        */
        virtual const Quaternion & getDerivedOrientation(void) const;

        /** Gets the position of the node as derived from all parents.
        */
        virtual const Vector3f & getDerivedPosition(void) const;

        /** Gets the scaling factor of the node as derived from all parents.
        */
        virtual const Vector3f & getDerivedScale(void) const;

        /** Gets the full transformation matrix for this node.
            @remarks
                This method returns the full transformation matrix
                for this node, including the effect of any parent node
                transformations, provided they have been updated using the Node::_update method.
                This should only be called by a SceneManager which knows the
                derived transforms have been updated before calling this method.
                Applications using Ogre should just use the relative transforms.
        */
        virtual const AffineTransform3& getFullTransform(void) const;

        /** Internal method to update the Node.
            @note
                Updates this node and any relevant children to incorporate transforms etc.
                Don't call this yourself unless you are writing a SceneManager implementation.
            @param
                updateChildren If true, the update cascades down to all children. Specify false if you wish to
                update children separately, e.g. because of a more selective SceneManager implementation.
            @param
                parentHasChanged This flag indicates that the parent xform has changed,
                    so the child should retrieve the parent's xform and combine it with its own
                    even if it hasn't changed itself.
        */
        virtual void update(bool updateChildren, bool parentHasChanged);

		/** Gets the local position, relative to this node, of the given world-space position */
		virtual Vector3f convertWorldToLocalPosition( const Vector3f &worldPos );

		/** Gets the world position of a point in the node local space
			useful for simple transforms that don't require a child node.*/
		virtual Vector3f convertLocalToWorldPosition( const Vector3f &localPos );

		/** Gets the local orientation, relative to this node, of the given world-space orientation */
		virtual Quaternion convertWorldToLocalOrientation( const Quaternion &worldOrientation );

		/** Gets the world orientation of an orientation in the node local space
			useful for simple transforms that don't require a child node.*/
		virtual Quaternion convertLocalToWorldOrientation( const Quaternion &localOrientation );

        /** To be called in the event of transform changes to this node that require it's recalculation.
        @remarks
            This not only tags the node state as being 'dirty', it also requests it's parent to 
            know about it's dirtiness so it will get an update next time.
		@param forceParentUpdate Even if the node thinks it has already told it's
			parent, tell it anyway
        */
        virtual void needUpdate(bool forceParentUpdate = true);
        /** Called by children to notify their parent that they need an update. 
		@param forceParentUpdate Even if the node thinks it has already told it's
			parent, tell it anyway
		*/
        virtual void requestUpdate(Node* child, bool forceParentUpdate = false);
        /** Called by children to notify their parent that they no longer need an update. */
        virtual void cancelUpdate(Node* child);

		void* getUserData() { return myUserData; }
		void setUserData(void* data) { myUserData = data; }

		void setName(const String& name);
		bool isUpdateNeeded() { return mNeedParentUpdate; }
		//! Children begin iterator
		//List<Node*>::iterator begin() { return mChildrenList.begin(); }
		//List<Node*>::const_iterator begin() const { return mChildrenList.begin(); }
		////! Children end iterator
		//List<Node*>::const_iterator end() const { return mChildrenList.end(); }

    protected:
        /// Pointer to parent node
        Node* mParent;
        /// Collection of pointers to direct children; hashmap for efficiency
        ChildNodeMap mChildren;
		// Children list, used to simplify iteration.
		List<Node*> mChildrenList;

		typedef std::set<Node*> ChildUpdateSet;
        /// List of children which need updating, used if self is not out of date but children are
        mutable ChildUpdateSet mChildrenToUpdate;
        /// Flag to indicate own transform from parent is out of date
        mutable bool mNeedParentUpdate;
		/// Flag indicating that all children need to be updated
		mutable bool mNeedChildUpdate;
		/// Flag indicating that parent has been notified about update request
	    mutable bool mParentNotified ;
        /// Flag indicating that the node has been queued for update
        mutable bool mQueuedForUpdate;

        /// Friendly name of this node, can be automatically generated if you don't care
        String mName;

        /// Incremented count for next name extension
        static NameGenerator msNameGenerator;

        /// Stores the orientation of the node relative to it's parent.
        Quaternion mOrientation;

        /// Stores the position/translation of the node relative to its parent.
        Vector3f mPosition;

        /// Stores the scaling factor applied to this node
        Vector3f mScale;

        /// Stores whether this node inherits orientation from it's parent
        bool mInheritOrientation;

        /// Stores whether this node inherits scale from it's parent
        bool mInheritScale;

		void* myUserData;

        /// Only available internally - notification of parent.
        virtual void setParent(Node* parent);

        /** Cached combined orientation.
            @par
                This member is the orientation derived by combining the
                local transformations and those of it's parents.
                This is updated when updateFromParent is called by the
                SceneManager or the nodes parent.
        */
        mutable Quaternion mDerivedOrientation;

        /** Cached combined position.
            @par
                This member is the position derived by combining the
                local transformations and those of it's parents.
                This is updated when updateFromParent is called by the
                SceneManager or the nodes parent.
        */
        mutable Vector3f mDerivedPosition;

        /** Cached combined scale.
            @par
                This member is the position derived by combining the
                local transformations and those of it's parents.
                This is updated when updateFromParent is called by the
                SceneManager or the nodes parent.
        */
        mutable Vector3f mDerivedScale;

		/** Class-specific implementation of updateFromParent.
		@remarks
			Splitting the implementation of the update away from the update call
			itself allows the detail to be overridden without disrupting the 
			general sequence of updateFromParent (e.g. raising events)
		*/
		virtual void updateFromParent(void) const;

        /// Cached derived transform as a 4x4 matrix
        mutable AffineTransform3 mCachedTransform;
        mutable bool mCachedTransformOutOfDate;

		//typedef std::vector<Node*> QueuedUpdates;
		//static QueuedUpdates msQueuedUpdates;
    };
}; //namespace

#endif
