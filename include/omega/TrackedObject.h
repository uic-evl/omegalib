/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
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
 * What's in this file
 *	Implements an Actor that sets the position and orientation of a scene node
 *  based on a events generated from a tracking system
 ******************************************************************************/
#ifndef __TRACKED_OBJECT__
#define __TRACKED_OBJECT__

#include "omega/Actor.h"

namespace omega {
    ///////////////////////////////////////////////////////////////////////////
    //!	Implements an Actor that sets the position and orientation of a scene node
    //! based on a events generated from a tracking system
    class OMEGA_API TrackedObject: public Actor
    {
    public:
        TrackedObject();
        virtual ~TrackedObject();

        virtual void handleEvent(const Event& evt);
        virtual void update(const UpdateContext& context);

        void setOffset(const Vector3f& value);
        const Vector3f& getOffset();

        void setOrientationOffset(const Quaternion& value);
        Quaternion getOrientationOffset();
        
        void setPositionTrackingEnabled(bool value);
        bool isPositionTrackingEnabled();

        void setOrientationTrackingEnabled(bool value);
        bool isOrientationTrackingEnabled();

        void setTrackableServiceType(Event::ServiceType value);
        Event::ServiceType getTrackableServiceType();

        void setTrackableSourceId(int value);
        int getTrackableSourceId();

        //! Set a timeout in seconds  to auto-hide the tracked object, if no 
        //! events have been received.
        void setHideTimeout(float timeout);
        float getHideTimeout();
        void setAutoHideEnabled(bool value);
        bool isAutoHideEnabled();

    private:
        Vector3f myTrackedPosition;
        Quaternion myTrackedOrientation;

        bool myPositionTrackingEnabled;
        bool myOrientationTrackingEnabled;
        Vector3f myOffset;
        Quaternion myOrientationOffset;

        Event::ServiceType myTrackableServiceType;
        int myTrackableSourceId;

        bool myAutoHideEnabled;
        float myHideTimeout;
        bool myEventReceivedSinceLastUpdate;
        float myLastEventTime;
    };

    ///////////////////////////////////////////////////////////////////////////
    inline void TrackedObject::setOffset(const Vector3f& value) 
    { myOffset = value; }

    ///////////////////////////////////////////////////////////////////////////
    inline const Vector3f& TrackedObject::getOffset() 
    { return myOffset; }

    ///////////////////////////////////////////////////////////////////////////
    inline void TrackedObject::setOrientationOffset(const Quaternion& value) 
    { myOrientationOffset = value; }

    ///////////////////////////////////////////////////////////////////////////
    inline Quaternion TrackedObject::getOrientationOffset() 
    { return myOrientationOffset; }
        
    ///////////////////////////////////////////////////////////////////////////
    inline void TrackedObject::setPositionTrackingEnabled(bool value) 
    { myPositionTrackingEnabled = value; }

    ///////////////////////////////////////////////////////////////////////////
    inline bool TrackedObject::isPositionTrackingEnabled() 
    { return myPositionTrackingEnabled; }

    ///////////////////////////////////////////////////////////////////////////
    inline void TrackedObject::setOrientationTrackingEnabled(bool value) 
    { myOrientationTrackingEnabled = value; }

    ///////////////////////////////////////////////////////////////////////////
    inline bool TrackedObject::isOrientationTrackingEnabled() 
    { return myOrientationTrackingEnabled; }

    ///////////////////////////////////////////////////////////////////////////
    inline void TrackedObject::setTrackableServiceType(Event::ServiceType value) 
    { myTrackableServiceType = value; }

    ///////////////////////////////////////////////////////////////////////////
    inline Event::ServiceType TrackedObject::getTrackableServiceType() 
    { return myTrackableServiceType; }

    ///////////////////////////////////////////////////////////////////////////
    inline void TrackedObject::setTrackableSourceId(int value) 
    { myTrackableSourceId = value; }

    ///////////////////////////////////////////////////////////////////////////
    inline int TrackedObject::getTrackableSourceId() 
    { return myTrackableSourceId; }

    ///////////////////////////////////////////////////////////////////////////
    inline void TrackedObject::setHideTimeout(float timeout)
    { myHideTimeout = timeout; }

    ///////////////////////////////////////////////////////////////////////////
    inline float TrackedObject::getHideTimeout()
    { return myHideTimeout; }

    ///////////////////////////////////////////////////////////////////////////
    inline void TrackedObject::setAutoHideEnabled(bool value)
    { myAutoHideEnabled = value; myEventReceivedSinceLastUpdate = true;}

    ///////////////////////////////////////////////////////////////////////////
    inline bool TrackedObject::isAutoHideEnabled()
    { return myAutoHideEnabled; }
}; // namespace omegaToolkit

#endif