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
*	Interface to buffers and variables on GPUs
******************************************************************************/
#ifndef __GPU_BUFFER__
#define __GPU_BUFFER__

#include "GpuResource.h"

namespace omega
{
    ///////////////////////////////////////////////////////////////////////////
    //! A buffer containing per-vertex data
    class OMEGA_API GpuBuffer : public GpuResource
    {
        friend class GpuContext;

    public:
        enum BufferType
        {
            VertexData, IndexData
        };

        enum AttributeType
        {
            Float, Int, Byte, UnsignedByte, Double
        };

        struct VertexAttribute
        {
            bool enabled;
            uint components;
            AttributeType type;
            uint stride;
            uint offset;
            bool normalize;
        };

        static const int MaxAttributes = 8;

    public:
        virtual void dispose();

        void setType(BufferType type);
        BufferType getType() { return myType; }

        bool setData(size_t size, void* data);
        void bind();
        void unbind();

        void setAttribute(uint index, AttributeType type, uint components = 1, bool normalize = false, uint offset = 0, uint stride = 0);
        const VertexAttribute& getAttribute(uint index) { return myAttributes[index]; }
        void clearAttributes();

        void bindVertexAttribute(uint index, uint location);
        
    protected:
        // Only renderer can create Vertex Buffers
        GpuBuffer(GpuContext* context);

    private:

        GLuint myId;
        size_t mySize;
        BufferType myType;
        GLenum myGLType;

        VertexAttribute myAttributes[MaxAttributes];
    };

    class GpuProgram;

    ///////////////////////////////////////////////////////////////////////////
    //! A collection of vertex buffers forming a complete vertex stream.
    class OMEGA_API GpuArray : public GpuResource
    {
        friend class GpuContext;
    public:
        static const int MaxBuffers = 8;

    public:
        virtual void dispose();

        void bind(GpuProgram* program);
        void unbind();

        void setBuffer(uint index, GpuBuffer* buffer);
        GpuBuffer* getBuffer(uint index) { return myBuffer[index]; }
        void clearBuffers();
        void setAttributeBinding(uint buffer, uint attribute, const String& name);

        //! Returns true if this array has indices attached. Only valid after
        //! a call to bind()
        bool hasIndices() { return myHasIndices;  }

        //! Convenience menthod for creating a vertex buffer and attaching it to
        //! this array.
        GpuBuffer* addBuffer(uint index, GpuBuffer::BufferType type, size_t size, void* data);
        //! Convenience method for creating and binding a named attribute for 
        //! a buffer.
        void addAttribute(uint buffer, uint index, const String& name, GpuBuffer::AttributeType type, bool normalize, uint components, uint offset, uint stride);

    protected:
        // Only renderer can create Vertex Streams
        GpuArray(GpuContext* context);

    private:
        uint myStamp;

        GLuint myId;
        bool myDirty;
        bool myHasIndices;
        GpuProgram* myLastProgram;

        Ref<GpuBuffer> myBuffer[MaxBuffers];
        String myAttributeBinding[MaxBuffers][GpuBuffer::MaxAttributes];
    };

}; // namespace omega

#endif
