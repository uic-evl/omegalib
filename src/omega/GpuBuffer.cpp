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
#include "omega/GpuBuffer.h"
#include "omega/GpuProgram.h"
#include "omega/glheaders.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
GpuBuffer::GpuBuffer(GpuContext* context):
    GpuResource(context),
    myId(0)
{
    glGenBuffers(1, &myId);
    oassert(!oglError);

    setType(VertexData);
    clearAttributes();
}

///////////////////////////////////////////////////////////////////////////////
void GpuBuffer::dispose()
{
    glDeleteBuffers(1, &myId);
}

///////////////////////////////////////////////////////////////////////////////
void GpuBuffer::setType(BufferType type) 
{ 
    myType = type; 
    if(myType == IndexData) myGLType = GL_ELEMENT_ARRAY_BUFFER;
    else myGLType = GL_ARRAY_BUFFER;
}

///////////////////////////////////////////////////////////////////////////////
void GpuBuffer::bind()
{
    glBindBuffer(myGLType, myId);
}

///////////////////////////////////////////////////////////////////////////////
void GpuBuffer::unbind()
{
    glBindBuffer(myGLType, 0);
}


///////////////////////////////////////////////////////////////////////////////
bool GpuBuffer::setData(size_t size, void* data)
{
    bind();
    if(oglError) ofwarn("ERROR 84 %1% %2% %3%", %size %myGLType %data);
    glBufferData(myGLType, size, data, GL_STATIC_DRAW);
    if(oglError) ofwarn("ERROR 86 %1% %2% %3%", %size %myGLType %data);
    unbind();
    return !oglError;
}

///////////////////////////////////////////////////////////////////////////////
void GpuBuffer::setAttribute(uint index, AttributeType type, uint components, bool normalize, uint offset, uint stride)
{
    myAttributes[index].enabled = true;
    myAttributes[index].type = type;
    myAttributes[index].components = components;
    myAttributes[index].offset = offset;
    myAttributes[index].stride = stride;
    myAttributes[index].normalize = normalize;
}

///////////////////////////////////////////////////////////////////////////////
void GpuBuffer::clearAttributes()
{
    memset(myAttributes, 0, sizeof(VertexAttribute) * MaxAttributes);
}

///////////////////////////////////////////////////////////////////////////////
void GpuBuffer::bindVertexAttribute(uint index, uint loc)
{
    bind();
    VertexAttribute& v = myAttributes[index];
    GLenum type;
    switch(v.type)
    {
    case GpuBuffer::Float: type = GL_FLOAT; break;
    case GpuBuffer::Double: type = GL_DOUBLE; break;
    case GpuBuffer::Int: type = GL_INT; break;
    case GpuBuffer::Byte: type = GL_BYTE; break;
    case GpuBuffer::UnsignedByte: type = GL_UNSIGNED_BYTE; break;
    }
    const unsigned char* ptrOffset = reinterpret_cast< unsigned char* >(0u + v.offset);
    if(v.type == GpuBuffer::Double)
    {
        glVertexAttribLPointer(loc, v.components, type, v.stride, (GLvoid*)ptrOffset);
    }
    else
    {
        glVertexAttribPointer(loc, v.components, type, v.normalize, v.stride, (GLvoid*)ptrOffset);
    }
    unbind();
}

///////////////////////////////////////////////////////////////////////////////
GpuArray::GpuArray(GpuContext* context):
    GpuResource(context),
    myId(0),
    myDirty(false),
    myHasIndices(false),
    myLastProgram(NULL)
{
    glGenVertexArrays(1, &myId);
    oassert(!oglError);
}

///////////////////////////////////////////////////////////////////////////////
void GpuArray::dispose()
{
    glDeleteVertexArrays(1, &myId);
}

///////////////////////////////////////////////////////////////////////////////
void GpuArray::bind(GpuProgram* program)
{
    if(myDirty || program != myLastProgram)
    {
        myHasIndices = false;
        myDirty = false;
        myLastProgram = program;

        glBindVertexArray(myId);

        // Loop over buffers attached to this vertex array
        for(int i = 0; i < MaxBuffers; i++)
        {
            if(myBuffer[i] == NULL) continue;

            myBuffer[i]->bind();
            if(myBuffer[i]->getType() == GpuBuffer::IndexData) myHasIndices = true;

            // Loop over attribute bindings for this buffer
            for(int j = 0; j < GpuBuffer::MaxAttributes; j++)
            {
                // Do we have a named binding for this vertex buffer attribute?
                String& bindingName = myAttributeBinding[i][j];
                if(!bindingName.empty())
                {
                    uint loc = program->getAttributeLocation(bindingName);
                    myBuffer[i]->bindVertexAttribute(j, loc);
                    glEnableVertexAttribArray(loc);
                }
            }
        }
    }
    else
    {
        glBindVertexArray(myId);
    }
}

///////////////////////////////////////////////////////////////////////////////
void GpuArray::unbind()
{
    glBindVertexArray(0);
}

///////////////////////////////////////////////////////////////////////////////
void GpuArray::setBuffer(uint index, GpuBuffer* buffer)
{
    myBuffer[index] = buffer;
    myDirty = true;
}

///////////////////////////////////////////////////////////////////////////////
void GpuArray::clearBuffers()
{
    foreach(Ref<GpuBuffer>& b, myBuffer) b.reset();
    myDirty = true;
}

///////////////////////////////////////////////////////////////////////////////
void GpuArray::setAttributeBinding(uint buffer, uint attribute, const String& name)
{
    myAttributeBinding[buffer][attribute] = name;
    myDirty = true;
}

///////////////////////////////////////////////////////////////////////////////
GpuBuffer* GpuArray::addBuffer(uint index, GpuBuffer::BufferType type, size_t size, void* data)
{
    GpuBuffer* buf = getContext()->createVertexBuffer();
    buf->setType(type);
    buf->setData(size, data);
    setBuffer(index, buf);
    return buf;
}

///////////////////////////////////////////////////////////////////////////////
void GpuArray::addAttribute(uint buffer, uint index, const String& name,
    GpuBuffer::AttributeType type, bool normalize, uint components, 
    uint offset, uint stride)
{
    GpuBuffer* buf = getBuffer(buffer);
    buf->setAttribute(index, type, normalize, components, offset, stride);
    setAttributeBinding(buffer, index, name);
}


///////////////////////////////////////////////////////////////////////////////
Uniform::Uniform(const String& name) :
    myDirty(false),
    myId(0),
    myName(name)
{

}

///////////////////////////////////////////////////////////////////////////////
void Uniform::update(GpuProgram* p)
{
    if(myDirty)
    {
        if(myId == 0) myId = p->getUniformLocation(myName);
        switch(myType)
        {
        case Float1: glUniform1f(myId, myFloatData[0]); break;
        case Int1: glUniform1i(myId, myIntData[0]); break;
        case Double1: glUniform1d(myId, myDoubleData[0]); break;
        case FloatMat4x4: glUniformMatrix4fv(myId, 1, false, myFloatData); break;
        }
        myDirty = false;
    }
}

///////////////////////////////////////////////////////////////////////////////
void Uniform::set(float x)
{
    if(x != myFloatData[0])
    {
        myDirty = true;
        myType = Float1;
        myFloatData[0] = x;
    }
}

///////////////////////////////////////////////////////////////////////////////
void Uniform::set(int x)
{
    myDirty = true;
    myType = Int1;
    myIntData[0] = x;
}

///////////////////////////////////////////////////////////////////////////////
void Uniform::set(double x)
{
    myDirty = true;
    myType = Double1;
    myDoubleData[0] = x;
}

///////////////////////////////////////////////////////////////////////////////
void Uniform::set(const Transform3& t)
{
    myDirty = true;
    myType = FloatMat4x4;
    memcpy(myFloatData, t.cast<float>().data(), 16 * sizeof(float));
}

///////////////////////////////////////////////////////////////////////////////
void Uniform::set(const AffineTransform3& t)
{
    myDirty = true;
    myType = FloatMat4x4;
    memcpy(myFloatData, t.cast<float>().data(), 16 * sizeof(float));
}
