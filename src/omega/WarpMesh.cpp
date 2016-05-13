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
*	Support for offscreen rendering to various targets
******************************************************************************/
#include "omega/WarpMesh.h"
#include "omega/DisplaySystem.h"
#include "omega/SystemManager.h"
#include "omega/GpuBuffer.h"
#include "omega/glheaders.h"

#include <iostream>     // cout, endl
#include <fstream>      // fstream
#include <algorithm>    // copy
#include <iterator>     // ostream_operator
#include <vector>
#include <string>

#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>

using namespace omega;

static Lock sMeshQueueLock;
static Queue< Ref<WarpMeshUtils::LoadWarpMeshGridAsyncTask> > sMeshQueue;
static bool sShutdownLoaderThread = false;
bool WarpMeshUtils::sVerbose = false;
int WarpMeshUtils::sNumLoaderThreads = 1;
List<Thread*> WarpMeshUtils::sMeshLoaderThread;

///////////////////////////////////////////////////////////////////////////////////////////////////
class MeshLoaderThread: public Thread
{
public:
    MeshLoaderThread()
    {}

    virtual void threadProc()
    {
        omsg("MeshLoaderThread: start");

        while(!sShutdownLoaderThread)
        {
            if(sMeshQueue.size() > 0)
            {
                sMeshQueueLock.lock();

                if(sMeshQueue.size() > 0)
                {
                    Ref<WarpMeshUtils::LoadWarpMeshGridAsyncTask> task = sMeshQueue.front();
                    sMeshQueue.pop();

                    sMeshQueueLock.unlock();

                    Ref<WarpMeshGrid> mesh = WarpMeshUtils::loadWarpMeshGrid(task->getData().path, task->getData().isFullPath);

                    if(!sShutdownLoaderThread)
                    {
                        task->getData().mesh = mesh;
                        task->notifyComplete();
                    }
                }
                else
                {
                    sMeshQueueLock.unlock();
                }
            }
            osleep(100);
        }

        omsg("MeshLoaderThread: shutdown");
    }

private:
};

////////////////////////////////////////////////////////////////////////////////

static bool readWarpMeshCSV(const String& filename, std::vector<WarpMeshUtils::WarpMeshGridRecord>& records)
{
    using namespace std;
    using namespace boost;

    ifstream in(filename.c_str());
    if (!in.is_open()) return false;

    typedef tokenizer< char_separator<char> > Tokenizer;
    char_separator<char> sep(" ,", "+");
    vector< string > vec;
    string line;

    static const char* sExpectedFields[] = {
        "GridX", "GridY", "PosX", "PosY", "U", "V", 0
    };


    // read header
    if(getline(in, line))
    {
        int i = 0;
        int j = 0;
        Tokenizer tok(line, sep);
        for(Tokenizer::const_iterator it = tok.begin(); it != tok.end() && sExpectedFields[i]; ++it, ++i)
        {
            String str = boost::algorithm::trim_copy(*it);
            if(str != string(sExpectedFields[i]))
            {
                ofwarn("readWarpMeshCSV: unexpected CSV header field '%1%'!", %*it);
                ofwarn("readWarpMeshCSV: Required data fields are: '%1%' '%2%' '%3%' '%4%' '%5%' '%6%'",
                       %sExpectedFields[0] %sExpectedFields[1] %sExpectedFields[2]
                       %sExpectedFields[3] %sExpectedFields[4] %sExpectedFields[5] );

                return false;
            }
        }
    }

    while (getline(in,line))
    {
        stringstream ss;
        Tokenizer tok(line, sep);
        WarpMeshUtils::WarpMeshGridRecord data;
        Tokenizer::const_iterator it = tok.begin();

        ss << boost::algorithm::trim_copy(*it);
        ss >> data.GridX;
        ss.clear();
        if(++it == tok.end()){ return false; }

        ss << boost::algorithm::trim_copy(*it);
        ss >> data.GridY;
        ss.clear();
        if(++it == tok.end()){ return false; }

        ss << boost::algorithm::trim_copy(*it);
        ss >> data.PosX;
        ss.clear();
        if(++it == tok.end()){ return false; }

        ss << boost::algorithm::trim_copy(*it);
        ss >> data.PosY;
        ss.clear();
        if(++it == tok.end()){ return false; }

        ss << boost::algorithm::trim_copy(*it);
        ss >> data.U;
        ss.clear();
        if(++it == tok.end()){ return false; }

        ss << boost::algorithm::trim_copy(*it);
        ss >> data.V;
        ss.clear();

        records.push_back(data);
    }

    return true;
}



///////////////////////////////////////////////////////////////////////////////////////////////////
void WarpMeshUtils::internalDispose()
{
    sShutdownLoaderThread = true;
    foreach(Thread* t, sMeshLoaderThread) t->stop();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
WarpMeshUtils::LoadWarpMeshGridAsyncTask* WarpMeshUtils::loadWarpMeshGridAsync(const String& filename, bool hasFullPath)
{
    if(sMeshLoaderThread.size() == 0)
    {
        for(int i = 0; i < sNumLoaderThreads; i++)
        {
            Thread* t = new MeshLoaderThread();
            t->start();
            sMeshLoaderThread.push_back(t);;
        }
    }

    LoadWarpMeshGridAsyncTask* task = new LoadWarpMeshGridAsyncTask();
    task->setData( LoadWarpMeshGridAsyncTask::Data(filename, hasFullPath) );
    task->setTaskId(filename);

    sMeshQueueLock.lock();
    sMeshQueue.push(task);
    sMeshQueueLock.unlock();
    return task;
}

////////////////////////////////////////////////////////////////////////////////
WarpMeshGeometry::WarpMeshGeometry() :
    ReferenceType(),
    displayList(0),
    vertexBuffer(NULL),
    indexBuffer(NULL),
    vertexArray(NULL),
    indexCount(0)
{
    // EMPTY!
}

WarpMeshGeometry::~WarpMeshGeometry()
{
    // EMPTY!
}

////////////////////////////////////////////////////////////////////////////////

void WarpMeshGeometry::initialize(const DrawContext& context, WarpMeshGrid& grid)
{
    // store the dimensions instread of largest index
    uint cols = grid.columns + 1;
    uint rows = grid.rows + 1;

    // copy vertices
    uint vertexCount = cols * rows;
    if(vertexCount < 1) return;

    std::vector<uint> indices;
    uint i0, i1, i2, i3;
    for (uint c = 0; c < (cols -1); c++)
    {
        for (uint r = 0; r < (rows-1); r++)
        {
            i0 = r * cols + c;
            i1 = r * cols + (c + 1);
            i2 = (r + 1) * cols + (c + 1);
            i3 = (r + 1) * cols + c;

            /*

            3      2
             x____x
             |t2 /|
             |  / |
             | /  |
             |/ t1|
             x----x
            0      1

            */

            // triangle 1
            indices.push_back(i0);
            indices.push_back(i1);
            indices.push_back(i2);

            // triangle 2
            indices.push_back(i0);
            indices.push_back(i2);
            indices.push_back(i3);
        }
    }

    Vector2f pos = context.tile->activeCanvasRect.min.cast<omicron::real>();
    Vector2f size = context.tile->activeCanvasRect.size().cast<omicron::real>();

    if(context.tile->flipWarpMesh)
    {
        // rescale to screen space coordinates and invert vertical axis and texcoords
        for(std::vector<WarpMeshVertex>::iterator it = grid.vertices.begin(); it != grid.vertices.end(); ++it)
        {
            it->x = (it->x + 0.5f);
            it->y = 1.0f - (it->y + 0.5f);

            it->v = (1.0f-it->v); // flip
        }
    }
    else
    {
        // rescale to screen space coordinates
        for(std::vector<WarpMeshVertex>::iterator it = grid.vertices.begin(); it != grid.vertices.end(); ++it)
        {
            it->x = (it->x + 0.5f);
            it->y = (it->y + 0.5f);
        }
    }

#if 0
    displayList = glGenLists(1);
    glNewList(displayList, GL_COMPILE);
    glBegin(GL_TRIANGLES);
    for (std::vector<uint>::const_iterator it = indices.begin(); it != indices.end(); ++it)
    {
        WarpMeshVertex vertex = grid.vertices[*it];
        glTexCoord2f(vertex.u, vertex.v);
        glVertex2f(vertex.x, vertex.y);
    }
    glEnd();
    glEndList();
#endif

    vertexBuffer = context.gpuContext->createVertexBuffer();
    vertexBuffer->setType(VertexBuffer::VertexData);
    vertexBuffer->setData(sizeof(WarpMeshVertex) * grid.vertices.size(), grid.vertices.data());
    vertexBuffer->setAttribute(0, VertexBuffer::Float, 2, false, 0, sizeof(WarpMeshVertex));
    vertexBuffer->setAttribute(1, VertexBuffer::Float, 2, false, 2 * sizeof(float), sizeof(WarpMeshVertex));

    indexBuffer = context.gpuContext->createVertexBuffer();
    indexBuffer->setType(VertexBuffer::IndexData);
    indexBuffer->setData(sizeof(uint) * indices.size(), indices.data());

    bool coreProfile = context.tile->displayConfig.openGLCoreProfile;
    if(coreProfile)
    {
        vertexArray = context.gpuContext->createVertexArray();
        vertexArray->setBuffer(0, vertexBuffer);
        vertexArray->setBuffer(1, indexBuffer);
    }
    indexCount = indices.size();
}

void WarpMeshGeometry::prepare(Renderer *client, const DrawContext &context)
{

}

void WarpMeshGeometry::render(Renderer *client, const DrawContext &context)
{
#if 0
    if(displayList < 1) return;
    glCallList(displayList);
#endif
	if (indexCount < 1) return;

    Vector2f tilePos = tileRect.min.cast<omicron::real>();
    Vector2f tileSize = tileRect.size().cast<omicron::real>();

	float tx = tilePos.x() / context.tile->pixelSize[0];
	float ty = tilePos.y() / context.tile->pixelSize[1];
	float sx = tileSize.x() / context.tile->pixelSize[0];
	float sy = tileSize.y() / context.tile->pixelSize[1];

    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();

    glTranslatef(tilePos.x(), tilePos.y(), 0.0f);
    glScalef(tileSize.x(), tileSize.y(), 1.0f);

	glMatrixMode(GL_TEXTURE);
	glPushMatrix();

	glTranslatef(tx, ty, 0.0f);
	glScalef(sx, sy, 1.0f);

	glMatrixMode(GL_MODELVIEW);

    // Bind attributes
    bool coreProfile = context.tile->displayConfig.openGLCoreProfile;
    if(coreProfile)
    {
        vertexArray->bind(NULL);
        vertexBuffer->bindVertexAttribute(0, 0);
        vertexBuffer->bindVertexAttribute(1, 1);
    }
    else
    {
        glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT);

        glClientActiveTexture(GL_TEXTURE0);
        glEnableClientState(GL_VERTEX_ARRAY);
        glEnableClientState(GL_TEXTURE_COORD_ARRAY);

        vertexBuffer->bind();
        indexBuffer->bind();

        glVertexPointer(2, GL_FLOAT, sizeof(WarpMeshVertex), reinterpret_cast<void*>(0));
        glTexCoordPointer(2, GL_FLOAT, sizeof(WarpMeshVertex), reinterpret_cast<void*>(8));
    }

    glDrawElements(GL_TRIANGLES, (GLsizei)indexCount, GL_UNSIGNED_INT, (void*)0);

    if(coreProfile)
    {
        vertexArray->unbind();
    }
    else
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);

        glPopClientAttrib();
    }

    glMatrixMode(GL_TEXTURE);
    glPopMatrix();

    glMatrixMode(GL_MODELVIEW);
    glPopMatrix();

}

void WarpMeshGeometry::setTileRect(const Rect& tr)
{
    tileRect = tr;
}

void WarpMeshGeometry::dispose()
{
	if (indexBuffer) { indexBuffer->dispose(); }
	if (vertexBuffer) { vertexBuffer->dispose(); }
	if (vertexArray) { vertexArray->dispose(); }
    indexCount = 0;
}

////////////////////////////////////////////////////////////////////////////////
Ref<WarpMeshGrid> WarpMeshUtils::loadWarpMeshGrid(const String& filename, bool hasFullPath)
{
    String path;
    if(!hasFullPath)
    {
        if(!DataManager::findFile(filename, path))
        {
            //ofmsg("LOOKUP: %1%%2%", %ogetdataprefix() %filename);
            if(!DataManager::findFile(ogetdataprefix() + filename, path))
            {
                // Try adding the
                ofwarn("WarpMeshUtils::loadWarpMesh: could not load %1%: file not found.", %filename);
                return NULL;
            }
        }
    }
    else
    {
        path = filename;
    }

    std::vector<WarpMeshUtils::WarpMeshGridRecord> records;
    if(readWarpMeshCSV(path, records) == false)
    {
        ofwarn("WarpMeshUtils::loadWarpMesh: failed to parse data from file %1%: invalid file.", %filename);
        return NULL;
    }

    int startRow = -1;
    int startCol = -1;
    int maxRows = -1;
    int maxCols = -1;
    int missingRows = 0;
    int missingCols = 0;


    Ref<WarpMeshGrid> grid = new WarpMeshGrid();
    for(std::vector<WarpMeshUtils::WarpMeshGridRecord>::const_iterator it = records.begin(); it != records.end(); ++it)
    {
        if(startRow < 0)
        {
            startRow = it->GridY;
        }

        if(startCol < 0)
        {
            startCol = it->GridX;
        }

        maxRows = std::max(maxRows, it->GridY);
        maxCols = std::max(maxCols, it->GridX);

        WarpMeshVertex vertex;
        vertex.x = it->PosX;
        vertex.y = it->PosY;
        vertex.u = it->U;
        vertex.v = it->V;
        grid->vertices.push_back(vertex);

//        ofmsg("readWarpMeshCSV: %1% %2% %3% %4% %5% %6%", %it->GridX %it->GridY %it->PosX %it->PosY %it->U %it->V);

    }
    grid->rows = maxRows;
    grid->columns = maxCols;

//    ofmsg("readWarpMeshCSV: %1% x %2% -> %3% x %4% ", %startCol %startRow %maxCols %maxRows);

    return grid;
}

////////////////////////////////////////////////////////////////////////////////

WarpMesh::WarpMesh()
{
    // EMPTY!
}

WarpMesh::~WarpMesh()
{
    // EMPTY!
}

void WarpMesh::initialize() { myInitialized = true; }

void WarpMesh::prepare(Renderer* client, const DrawContext& context)
{
    // EMPTY!
}

void WarpMesh::render(Renderer* client, const DrawContext& context)
{
    // EMPTY!
}

void WarpMesh::dispose()
{
    // EMPTY!
}

///////////////////////////////////////////////////////////////////////////
