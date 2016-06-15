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
*	The base class for classes that define screen aligned geometry for warping post-render.
******************************************************************************/
#ifndef __WARP_MESH_H__
#define __WARP_MESH_H__

#include "osystem.h"
#include "omega/ApplicationBase.h"
#include "omega/AsyncTask.h"
#include "DrawInterface.h"

namespace omega {
    class Renderer;
    class DrawCentext;

    struct OMEGA_API WarpMeshVertex
    {
        float x;
        float y;
        float u;
        float v;

        WarpMeshVertex() : x(0.0f), y(0.0f), u(0.0f), v(0.0f) {}
    };
    typedef std::vector<WarpMeshVertex> WarpMeshVertexData;

    class OMEGA_API WarpMeshGrid : public ReferenceType
    {
    public:
        uint rows;
        uint columns;
        WarpMeshVertexData vertices;

        WarpMeshGrid() : rows(0), columns(0), vertices() {}
    };

    class OMEGA_API WarpMeshGeometry: public ReferenceType
    {
    public:

        WarpMeshGeometry();
        virtual ~WarpMeshGeometry();

        virtual void initialize(const DrawContext& context, WarpMeshGrid& grid);
        virtual void prepare(Renderer* client, const DrawContext& context);
        virtual void render(Renderer* client, const DrawContext& context);
        virtual void updateViewport(const Rect& vp);
        virtual void dispose();

        bool isInitialized() { return indexCount > 0; }

    private:
        GLuint displayList;
        Ref<GpuBuffer> vertexBuffer;
        Ref<GpuBuffer> indexBuffer;
        Ref<GpuArray> vertexArray;
        size_t indexCount;
        Rect viewport;
    };


    ///////////////////////////////////////////////////////////////////////////////////////////////
    //! WarpMeshUtils is a container of functions for synchronous and asyncronous warp geometry loading.
    class OMEGA_API WarpMeshUtils
    {
    public:
        struct WarpMeshGridRecord
        {
            int GridX;
            int GridY;
            float PosX;
            float PosY;
            float U;
            float V;

            WarpMeshGridRecord() : GridX(0), GridY(0), PosX(0.0f), PosY(0.0f), U(0.0f), V(0.0f) {}
        };

        struct LoadWarpMeshGridAsyncTaskData
        {
            LoadWarpMeshGridAsyncTaskData() {}
            LoadWarpMeshGridAsyncTaskData(const String& _path, bool _isFullPath):
                path(_path), isFullPath(_isFullPath) {}

            Ref<WarpMeshGrid> grid;
            String path;
            bool isFullPath;
        };

        typedef AsyncTask<LoadWarpMeshGridAsyncTaskData> LoadWarpMeshGridAsyncTask;

        //! Load the warp mesh grid geometry from a file.
        static Ref<WarpMeshGrid> loadWarpMeshGrid(const String& filename, bool hasFullPath = false);
    };

    ///////////////////////////////////////////////////////////////////////////
    //! The base class for classes that define screen aligned geometry for warping post-render.
    class OMEGA_API WarpMesh: public ReferenceType
    {
    public:
        WarpMesh();
        virtual ~WarpMesh();

        virtual void initialize();
        virtual void prepare(Renderer* client, const DrawContext& context);
        virtual void render(Renderer* client, const DrawContext& context);
        virtual void dispose();

        bool isInitialized() { return myInitialized; }

    private:
        bool myInitialized;
    };

} // namespace omega

#endif
