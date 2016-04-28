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
VertexBuffer::VertexBuffer(GpuContext* context):
GpuResource(context),
myId(0)
{
    glGenBuffers(1, &myId);
    oassert(!oglError);

    setType(VertexData);
    clearAttributes();
}

///////////////////////////////////////////////////////////////////////////////
void VertexBuffer::dispose()
{
    glDeleteBuffers(1, &myId);
}

///////////////////////////////////////////////////////////////////////////////
void VertexBuffer::setType(BufferType type) 
{ 
    myType = type; 
    if(myType == IndexData) myGLType = GL_ELEMENT_ARRAY_BUFFER;
    else myGLType = GL_ARRAY_BUFFER;
}

///////////////////////////////////////////////////////////////////////////////
void VertexBuffer::bind()
{
    glBindBuffer(myGLType, myId);
}

///////////////////////////////////////////////////////////////////////////////
void VertexBuffer::unbind()
{
    glBindBuffer(myGLType, 0);
}


///////////////////////////////////////////////////////////////////////////////
bool VertexBuffer::setData(size_t size, void* data)
{
    bind();
    if(oglError) ofwarn("ERROR 84 %1% %2% %3%", %size %myGLType %data);
    glBufferData(myGLType, size, data, GL_STATIC_DRAW);
    if(oglError) ofwarn("ERROR 86 %1% %2% %3%", %size %myGLType %data);
    unbind();
    return !oglError;
}

///////////////////////////////////////////////////////////////////////////////
void VertexBuffer::setAttribute(uint index, AttributeType type, uint components, bool normalize, uint offset, uint stride)
{
    myAttributes[index].enabled = true;
    myAttributes[index].type = type;
    myAttributes[index].components = components;
    myAttributes[index].offset = offset;
    myAttributes[index].stride = stride;
    myAttributes[index].normalize = normalize;
}

///////////////////////////////////////////////////////////////////////////////
void VertexBuffer::clearAttributes()
{
    memset(myAttributes, 0, sizeof(VertexAttribute) * MaxAttributes);
}

///////////////////////////////////////////////////////////////////////////////
void VertexBuffer::bindVertexAttribute(uint index, uint loc)
{
    bind();
    VertexAttribute& v = myAttributes[index];
    GLenum type;
    switch(v.type)
    {
    case VertexBuffer::Float: type = GL_FLOAT; break;
    case VertexBuffer::Double: type = GL_DOUBLE; break;
    case VertexBuffer::Int: type = GL_INT; break;
    case VertexBuffer::Byte: type = GL_BYTE; break;
    case VertexBuffer::UnsignedByte: type = GL_UNSIGNED_BYTE; break;
    }
    const unsigned char* ptrOffset = reinterpret_cast< unsigned char* >(0u + v.offset);
    if(v.type == VertexBuffer::Double)
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
VertexArray::VertexArray(GpuContext* context):
GpuResource(context),
myId(0),
myDirty(false),
myLastProgram(NULL)
{
    glGenVertexArrays(1, &myId);
    oassert(!oglError);
}

///////////////////////////////////////////////////////////////////////////////
void VertexArray::dispose()
{
    glDeleteVertexArrays(1, &myId);
}

///////////////////////////////////////////////////////////////////////////////
void VertexArray::bind(GpuProgram* program)
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
            if(myBuffer[i]->getType() == VertexBuffer::IndexData) myHasIndices = true;

            // Loop over attribute bindings for this buffer
            for(int j = 0; j < VertexBuffer::MaxAttributes; j++)
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
void VertexArray::unbind()
{
    glBindVertexArray(0);
}

///////////////////////////////////////////////////////////////////////////////
void VertexArray::setBuffer(uint index, VertexBuffer* buffer)
{
    myBuffer[index] = buffer;
    myDirty = true;
}

///////////////////////////////////////////////////////////////////////////////
void VertexArray::clearBuffers()
{
    foreach(Ref<VertexBuffer>& b, myBuffer) b.reset();
    myDirty = true;
}

///////////////////////////////////////////////////////////////////////////////
void VertexArray::setAttributeBinding(uint buffer, uint attribute, const String& name)
{
    myAttributeBinding[buffer][attribute] = name;
    myDirty = true;
}

///////////////////////////////////////////////////////////////////////////////
VertexBuffer* VertexArray::addBuffer(uint index, VertexBuffer::BufferType type, size_t size, void* data)
{
    VertexBuffer* buf = getContext()->createVertexBuffer();
    buf->setType(type);
    buf->setData(size, data);
    setBuffer(index, buf);
    return buf;
}

///////////////////////////////////////////////////////////////////////////////
void VertexArray::addAttribute(uint buffer, uint index, const String& name,
    VertexBuffer::AttributeType type, bool normalize, uint components, 
    uint offset, uint stride)
{
    VertexBuffer* buf = getBuffer(buffer);
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
