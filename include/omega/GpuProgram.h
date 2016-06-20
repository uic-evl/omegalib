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
#ifndef __GPU_PROGRAM__
#define __GPU_PROGRAM__

#include "GpuResource.h"
#include "GpuBuffer.h"
#include "Texture.h"

namespace omega
{
    class GpuProgram;

    ///////////////////////////////////////////////////////////////////////////
    //! A shader uniform
    class OMEGA_API Uniform : public ReferenceType
    {
    public:
        enum Type
        {
            Double1, Float1, Int1,
            Double2, Float2, Int2,
            Double3, Float3, Int3,
            Double4, Float4, Int4,
            FloatMat4x4
        };

    public:
        Uniform(const String& name, GpuProgram* prog);
        void update();
        void set(float x);
        void set(int x);
        void set(double x);
        void set(const Transform3& t);
        void set(const AffineTransform3& t);

    private:
        GpuProgram* myProgram;
        GLint myId;
        bool myDirty;
        Type myType;
        String myName;
        uint myStamp;

        union {
            double myDoubleData[16];
            float myFloatData[16];
            int myIntData[16];
        };
    };

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API GpuProgram : public GpuResource
    {
        friend class GpuContext;
        static const int MaxShaderFragments = 64;
    public:
        enum ShaderType
        {
            VertexShader,
            FragmentShader,
            GeometryShader,
            ShaderTypes
        };

    public:
        virtual void dispose();

        bool build();

        bool setShader(ShaderType type, const String& filename, int index = 0);
        void setShaderSource(ShaderType type, const String& source, int index = 0);

        int getUniformLocation(const String& name);
        int getAttributeLocation(const String& name);

        GLuint getId() { return myId; }

        Uniform* addUniform(const String& name);
        void clearUniforms();
        bool use();

        uint getStamp() { return myStamp; }

    protected:
        // Only Renderer can create GpuPrograms.
        GpuProgram(GpuContext* context);

    private:
        uint myStamp;
        GLint myId;

        String myShaderFilename[ShaderTypes][MaxShaderFragments];
        String myShaderSource[ShaderTypes][MaxShaderFragments];
        bool myShaderDirty[ShaderTypes];
        bool myDirty;
        GLuint myShader[ShaderTypes];
        List< Ref<Uniform> > myUniforms;
    };

    ///////////////////////////////////////////////////////////////////////////
    class OMEGA_API GpuDrawCall : public ReferenceType
    {
    public:
        static const int MaxParams = 64;
        enum PrimType { PrimNone, PrimPoints, PrimLines, PrimTriangles, PrimTriangleStrip };

        struct TextureBinding
        {
            Ref<Texture> texture;
            String name;
            GLuint location;
        };

    public:
        GpuDrawCall(GpuProgram* program):
            myProgram(program) { }


        void setVertexArray(GpuArray* va);
        void addTexture(const String& name, Texture* tx);
        void clearTextures();

        //! Add a uniform to the draw call.
        //! @remarks Uniforms specified through the draw call take priority over
        //! uniforms specified by the gpu program.
        Uniform* addUniform(const String& name);
        void clearUniforms();

        void run();

        PrimType primType;
        unsigned int items;
    private:
        Ref<GpuArray> myVertexArray;
        Ref<GpuProgram> myProgram;
        List<TextureBinding*> myTextureBindings;
        List< Ref<Uniform> > myUniforms;
    };
}; // namespace omega

#endif
