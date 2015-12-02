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
*	Interface to code running on GPUs
******************************************************************************/
#include "omega/GpuProgram.h"

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
void GpuProgram::dispose()
{

}

///////////////////////////////////////////////////////////////////////////////
GpuProgram::GpuProgram(GpuContext* context):
    GpuResource(context),
    myId(0),
    myDirty(false)
{
    memset(myShader, 0, sizeof(GLuint) * ShaderTypes);
    memset(myShaderDirty, 0, sizeof(bool) * ShaderTypes);
}

///////////////////////////////////////////////////////////////////////////////
bool GpuProgram::setShader(ShaderType type, const String& filename)
{
    myShaderFilename[type] = filename;
    String source = DataManager::readTextFile(filename);
    if(source.size() == 0) return false;
    setShaderSource(type, source);
    return true;
}

///////////////////////////////////////////////////////////////////////////////
void GpuProgram::setShaderSource(ShaderType type, const String& source)
{
    myShaderSource[type] = source;
    myShaderDirty[type] = true;
    myDirty = true;
}

///////////////////////////////////////////////////////////////////////////////
bool GpuProgram::build()
{
    // Create program object if needed
    if(myId == 0)
    {
        myId = glCreateProgram();
        oassert(!oglError);
    }

    if(myDirty)
    {
        for(unsigned int i = 0; i < ShaderTypes; i++)
        {
            if(myShaderDirty[i])
            {
                if(myShader[i] == 0)
                {
                    GLuint s = 0;
                    switch(i)
                    {
                    case VertexShader: s = glCreateShader(GL_VERTEX_SHADER); break;
                    case FragmentShader: s = glCreateShader(GL_FRAGMENT_SHADER); break;
                    case GeometryShader: s = glCreateShader(GL_GEOMETRY_SHADER); break;
                    }
                    oassert(!oglError);
                    myShader[i] = s;
                }

                const char* cstr = myShaderSource[i].c_str();
                GLint len = myShaderSource[i].length();
                glShaderSource(myShader[i], 1, &cstr, &len);
                glCompileShader(myShader[i]);

                int infologLength = 0;
                int charsWritten = 0;
                char *infoLog;
                glGetShaderiv(myShader[i], GL_INFO_LOG_LENGTH, &infologLength);
                if(infologLength > 1)
                {
                    infoLog = new char[infologLength];
                    glGetShaderInfoLog(myShader[i], infologLength, &charsWritten, infoLog);
                    // Print log only when it contains error messages.
                    if(strncmp(infoLog, "No errors.", 8))
                    {
                        ofmsg("[%1%] %2%", %myShaderFilename[i] %infoLog);
                        return false;
                    }
                    delete[] infoLog;
                }

                glAttachShader(myId, myShader[i]);

                myShaderDirty[i] = false;

            }
        }

        // Link program.
        glLinkProgram(myId);

        int infologLength = 0;
        int charsWritten = 0;
        char *infoLog;
        glGetProgramiv(myId, GL_INFO_LOG_LENGTH, &infologLength);
        if(infologLength > 1)
        {
            infoLog = new char[infologLength];
            glGetProgramInfoLog(myId, infologLength, &charsWritten, infoLog);
            // Print log only when it contains error messages.
            if(strncmp(infoLog, "No errors.", 8))
            {
                omsg(infoLog);
                return false;
            }
            delete[] infoLog;
        }

        myDirty = false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
bool GpuProgram::use()
{
    if(build())
    {
        glUseProgram(myId);
        return true;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////
unsigned int GpuProgram::getUniformLocation(const String& name)
{
    if(build())
    {
        return glGetUniformLocation(myId, name.c_str());
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
unsigned int GpuProgram::getAttributeLocation(const String& name)
{
    if(build())
    {
        return glGetAttribLocation(myId, name.c_str());
    }
    return 0;
}

///////////////////////////////////////////////////////////////////////////////
void GpuDrawCall::setVertexArray(VertexArray* va)
{
    myVertexArray = va;
}

///////////////////////////////////////////////////////////////////////////////
void GpuDrawCall::addTexture(const String& name, Texture* tx)
{
    TextureBinding* ti = new TextureBinding();
    ti->texture = tx;
    ti->name = name;
    myTextureBindings.push_back(ti);
}

///////////////////////////////////////////////////////////////////////////////
void GpuDrawCall::clearTextures()
{
    foreach(TextureBinding* ti, myTextureBindings) delete ti;
    myTextureBindings.clear();
}

///////////////////////////////////////////////////////////////////////////////
Uniform* GpuDrawCall::addUniform(const String& name)
{
    Uniform* u = new Uniform(name);
    myUniforms.push_back(u);
    return u;
}

///////////////////////////////////////////////////////////////////////////////
void GpuDrawCall::clearUniforms()
{
    myUniforms.clear();
}

///////////////////////////////////////////////////////////////////////////////
void GpuDrawCall::run()
{
    if(myProgram->build())
    {
        GLint lastProg;
        glGetIntegerv(GL_CURRENT_PROGRAM, &lastProg);
        glUseProgram(myProgram->getId());

        // Bind uniforms
        foreach(Uniform* u, myUniforms)
        {
            u->update(myProgram);
        }

        // Bind textures
        uint stage = GpuContext::TextureUnit0;
        foreach(TextureBinding* t, myTextureBindings)
        {
            if(t->location == 0)
            {
                t->location = myProgram->getUniformLocation(t->name);
            }

            t->texture->bind((GpuContext::TextureUnit)stage);
            glUniform1i(t->location, stage);
            stage++;
        }

        // Bind attributes
        myVertexArray->bind(myProgram);

        // Draw
        GLenum mode = GL_POINTS;
        switch(primType)
        {
        case PrimLines: mode = GL_LINES; break;
        case PrimTriangles: mode = GL_TRIANGLES; break;
        case PrimTriangleStrip: mode = GL_TRIANGLE_STRIP; break;
        }
        if(myVertexArray->hasIndices())
        {
            // NOTE: we are assuming the indices are unsigned int. We should
            // get this from the actual index buffer attached to the vertex array.
            glDrawElements(mode, items, GL_UNSIGNED_INT, (void*)0);
        }
        else
        {
            glDrawArrays(mode, 0, items);
        }

        glUseProgram(lastProg);

        myVertexArray->unbind();
    }
}
