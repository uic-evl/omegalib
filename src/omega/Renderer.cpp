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
 *	The omegalib renderer is the entry point for all of omegalib rendering code.
 *	The renderer does not draw anything: it just manages rendering resources, 
 *	cameras and render passes.
 ******************************************************************************/
#include "omega/Renderer.h"
#include "omega/Engine.h"
#include "omega/DisplaySystem.h"
#include "omega/Texture.h"
#include "omega/PythonInterpreter.h"
#include "omega/glheaders.h"

using namespace omega;

GLEWContext* sGlewContext;

///////////////////////////////////////////////////////////////////////////
GLEWContext* glewGetContext()
{
    return sGlewContext;
}

///////////////////////////////////////////////////////////////////////////
void glewSetContext(const GLEWContext* context)
{
    sGlewContext = (GLEWContext*)context;
}

///////////////////////////////////////////////////////////////////////////////
Renderer::Renderer(Engine* engine)
{
    myRenderer = new DrawInterface();
    myServer = engine;
    myServer->addRenderer(this);
}

///////////////////////////////////////////////////////////////////////////////
Texture* Renderer::createTexture()
{
    if(Platform::deprecationWarnings)
    {
        static bool firstTime = true;
        if(firstTime)
        {
            owarn("[v10.1 DEPRECATION WARNING] Renderer::createTexture - use GpuContext::createTexture instead");
            firstTime = false;
        }
    }

    return myGpuContext->createTexture();
}

///////////////////////////////////////////////////////////////////////////////
RenderTarget* Renderer::createRenderTarget(RenderTarget::Type type)
{
    if(Platform::deprecationWarnings)
    {
        static bool firstTime = true;
        if(firstTime)
        {
            owarn("[v10.1 DEPRECATION WARNING] Renderer::createRenderTarget - use GpuContext::createRenderTarget instead");
            firstTime = false;
        }
    }
    return myGpuContext->createRenderTarget(type);
}

///////////////////////////////////////////////////////////////////////////////
bool RenderPassSortOp(RenderPass* p1, RenderPass* r2)
{ return p1->getPriority() < r2->getPriority(); }

///////////////////////////////////////////////////////////////////////////////
void Renderer::addRenderPass(RenderPass* pass)
{
    myRenderPassLock.lock();
    //ofmsg("Renderer(%1%): adding render pass %2%", %getGpuContext()->getId() %pass->getName());
    myRenderPassList.push_back(pass);
    // Re-sort the render pass list. Render passes implement a comparison 
    // operator to perform the sorting based on their priority.
    myRenderPassList.sort(RenderPassSortOp);
    myRenderPassLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::removeRenderPass(RenderPass* pass)
{
    myRenderPassLock.lock();
    pass->requestDispose();
    myRenderPassLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::removeAllRenderPasses()
{
    myRenderPassLock.lock();
    foreach(RenderPass* rp, myRenderPassList) rp->requestDispose();
    myRenderPassLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
RenderPass* Renderer::getRenderPass(const String& name)
{
    foreach(RenderPass* rp, myRenderPassList)
    {
        if(rp->getName() == name) return rp;
    }
    return NULL;
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::initialize()
{
    oflog(Verbose, "[Renderer::initialize] id=<%1%>", %getGpuContext()->getId());


	DisplayConfig& dcfg = SystemManager::instance()->getDisplaySystem()->getDisplayConfig();
	
	if(!dcfg.openGLCoreProfile)
	{
		// Create the default font.
		const FontInfo& fi = myServer->getDefaultFont();
		if(fi.size != 0)
		{
			Font* fnt = myRenderer->createFont(fi.name, fi.filename, fi.size);
			myRenderer->setDefaultFont(fnt);
		}
    }

    initializeCompositor();

    printf("stereo compositor initialized! %d\n", glGetError());

    StatsManager* sm = getEngine()->getSystemManager()->getStatsManager();
    myFrameTimeStat = sm->createStat(ostr("ctx%1% frame", %getGpuContext()->getId()), StatsManager::Time);
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::initializeCompositor()
{
    loadCompositorShaders();

    // vertices
    glGenBuffers(1, &compositeVertexPositionBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, compositeVertexPositionBuffer);
    float compositeVertices[] = {
        -1.0, -1.0, -1.0,
        -1.0,  1.0, -1.0,
         1.0, -1.0, -1.0,
         1.0,  1.0, -1.0
    };
    glBufferData(GL_ARRAY_BUFFER, 12 * sizeof(float), compositeVertices, GL_STATIC_DRAW);

    // texture coords
    glGenBuffers(1, &compositeVertexTexCoordBuffer);
    glBindBuffer(GL_ARRAY_BUFFER, compositeVertexTexCoordBuffer);
    float compositeTexCoords[] = {
        0.0, 0.0,
        0.0, 1.0,
        1.0, 0.0,
        1.0, 1.0
    };
    glBufferData(GL_ARRAY_BUFFER, 8 * sizeof(float), compositeTexCoords, GL_STATIC_DRAW);

    // faces of triangles
    glGenBuffers(1, &compositeVertexIndexBuffer);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, compositeVertexIndexBuffer);
    unsigned short compositeIndices[] = {
        0, 3, 1,
        3, 0, 2
    };
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6 * sizeof(unsigned short), compositeIndices, GL_STATIC_DRAW);
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::loadCompositorShaders()
{
    const std::string shaders[5] = {"LineInterleaved", "ColumnInterleaved", "PixelInterleaved", "AnaglyphRedCyan", "AnaglyphGreenMagenta"};
    int i;
    FILE *vf = fopen("shaders/composit.vert", "rb");
    if (vf == NULL) fprintf(stderr, "error: could not open composit.vert\n");
    fseek(vf, 0, SEEK_END);
    long vfsize = ftell(vf);
    fseek(vf, 0, SEEK_SET);

    char *vertSource = (char*)malloc(vfsize);
    fread(vertSource, vfsize, 1, vf);
    fclose(vf);

    int vertexShader = compileShader(vertSource, vfsize, GL_VERTEX_SHADER);

    FILE *ff;
    for (i=0; i<5; i++) {
        ff = fopen(("shaders/"+shaders[i]+".frag").c_str(), "rb");
        if (vf == NULL) fprintf(stderr, "error: could not open %s.frag\n", shaders[i].c_str());
        fseek(ff, 0, SEEK_END);
        long ffsize = ftell(ff);
        fseek(ff, 0, SEEK_SET);

        char *fragSource = (char*)malloc(ffsize);
        fread(fragSource, ffsize, 1, ff);
        fclose(ff);

        int fragmentShader = compileShader(fragSource, ffsize, GL_FRAGMENT_SHADER);

        createCompositShaderProgram(i, vertexShader, fragmentShader);
    }

    printf("stereo shader programs loaded! %d\n", glGetError());
}

///////////////////////////////////////////////////////////////////////////////
int Renderer::compileShader(char* source, int length, int type)
{
    int status;
    int shader = glCreateShader(type);

    const char *srcBytes = (const char *)source;
    glShaderSource(shader, 1, &srcBytes, &length);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == 0) {
        GLint ilength;
        char *info;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &ilength);
        info = (char*)malloc(ilength+1);
        glGetShaderInfoLog(shader, length, NULL, info);
        fprintf(stderr, "Failed to compile shader:\n%s\n", info);
        free(info);

        return -1;
    }
    else {
        return shader;
    }
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::createCompositShaderProgram(int idx, int vertexShader, int fragmentShader)
{
    GLint status;
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);

    glBindAttribLocation(shaderProgram, 0, "aVertexPosition");
    glBindAttribLocation(shaderProgram, 1, "aVertexTexCoord");

    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &status);
    if(status == 0) {
        fprintf(stderr, "Unable to initialize the shader program\n");
    }

    // set vertex array
    compositeVertexPositionAttribute[idx] = glGetAttribLocation(shaderProgram, "aVertexPosition");
    glEnableVertexAttribArray(compositeVertexPositionAttribute[idx]);

    // set texture coord array
    compositeVertexTexCoordAttribute[idx] = glGetAttribLocation(shaderProgram, "aVertexTexCoord");
    glEnableVertexAttribArray(compositeVertexTexCoordAttribute[idx]);

    // set backface image texture
    leftEyeUniform[idx] = glGetUniformLocation(shaderProgram, "leftEyeImage");
    // set transfer function image texture
    rightEyeUniform[idx] = glGetUniformLocation(shaderProgram, "rightEyeImage");
    
    stereoCompositor[idx] = shaderProgram;
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::dispose()
{
    foreach(RenderPass* rp, myRenderPassList) rp->dispose();
    myRenderPassList.clear();

    while(!myRenderableCommands.empty()) myRenderableCommands.pop();

    myRenderer = NULL;
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::queueCommand(IRendererCommand* cmd)
{
    myRenderCommandLock.lock();
    myRenderableCommands.push(cmd);
    myRenderCommandLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::startFrame(const FrameInfo& frame)
{
    //omsg("Renderer::startFrame");

    myFrameTimeStat->startTiming();
    myServer->getDefaultCamera()->startFrame(frame);
    foreach(Ref<Camera> cam, myServer->getCameras())
    {
        if(cam->isEnabled()) cam->startFrame(frame);
    }
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::finishFrame(const FrameInfo& frame)
{
    //omsg("Renderer::finishFrame");

    myServer->getDefaultCamera()->finishFrame(frame);
    foreach(Ref<Camera> cam, myServer->getCameras())
    {
        if(cam->isEnabled()) cam->finishFrame(frame);
    }

    // Release unused gpu resources.
    myGpuContext->garbageCollect();
    
    myFrameTimeStat->stopTiming();
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::clear(DrawContext& context)
{
    //omsg("Renderer::clear");

    myServer->getDefaultCamera()->clear(context);
    foreach(Ref<Camera> cam, myServer->getCameras())
    {
        if(cam->isEnabled()) cam->clear(context);
    }
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::prepare(DrawContext& context)
{
    foreach(RenderPass* rp, myRenderPassList)
    {
        rp->prepare(this, context);
    }
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::draw(DrawContext& context)
{
    //omsg("Renderer::draw");

    myRenderPassLock.lock();
    // First of all make sure all render passes are initialized.
    foreach(RenderPass* rp, myRenderPassList)
    {
        if(!rp->isInitialized()) rp->initialize();
    }
    // Now check if some render passes need to be disposed
    List<RenderPass*> tbdisposed;
    foreach(RenderPass* rp, myRenderPassList)
    {
        if(rp->needsDispose())
        {
            rp->dispose();
            tbdisposed.push_back(rp);
        }
    }
    foreach(RenderPass* rp, tbdisposed) myRenderPassList.remove(rp);
    myRenderPassLock.unlock();

    // Execute renderable commands.
    myRenderCommandLock.lock();
    while(!myRenderableCommands.empty())
    {
        myRenderableCommands.front()->execute(this);
        myRenderableCommands.pop();
    }
    myRenderCommandLock.unlock();

    foreach(Ref<Camera> cam, myServer->getCameras())
    {
        // See if camera is enabled for the current client and draw context.
        if(cam->isEnabledInContext(context))
        {
            // Begin drawing with the camera: get the camera draw context.
            cam->beginDraw(context);
            innerDraw(context, cam);
            cam->endDraw(context);
        }
    }

    // Draw once for the default camera (using the passed main draw context).
    // NOTE: We use the draw context returned by the camera because in principle
    // the camera may adjust the context before drawing. In practice, for the 
    // default camera the context should stay the same as what is passed to this
    // method.
    Camera* cam = myServer->getDefaultCamera();
    if(cam->isEnabledInContext(context))
    {
        cam->beginDraw(context);
        innerDraw(context, myServer->getDefaultCamera());
        cam->endDraw(context);
    }
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::innerDraw(const DrawContext& context, Camera* cam)
{
    //omsg("[DRAW]");
    myRenderPassLock.lock();
    // Execute all render passes in order. 
    foreach(RenderPass* pass, myRenderPassList)
    {
        if(pass->isInitialized())
        {
            // Run the pass if both its mask and the camera mask are 0 (left unspecified)
            // Alternatively, run the pass if at least one of the mask bits is set on both the camera an the pass
            if((cam->getMask() == 0 && pass->getCameraMask() == 0) ||
                ((cam->getMask() & pass->getCameraMask()) != 0))
            {
                pass->render(this, context);
            }
        }
    }
    myRenderPassLock.unlock();

    // Draw the pointers
    // NOTE: Pointer only run for cameras that do not have a mask specified
    if(cam->getMask() == 0 && context.task == DrawContext::OverlayDrawTask && 
        context.eye == DrawContext::EyeCyclop)
    {
        getRenderer()->beginDraw2D(context);

        // Let the interpreter handle scriptable draw callbacks.
        PythonInterpreter* pi = SystemManager::instance()->getScriptInterpreter();
        pi->draw(context, cam);
        
        myServer->drawPointers(this, context);
    
        getRenderer()->endDraw();
    }
}

///////////////////////////////////////////////////////////////////////////////
void Renderer::composite(DrawContext& context)
{
    int program = getStereoCompositeProgram(context.getCurrentStereoMode());
    if (program < 0) return;

    glUseProgram(stereoCompositor[program]);

    glBindBuffer(GL_ARRAY_BUFFER, compositeVertexPositionBuffer);
    glVertexAttribPointer(compositeVertexPositionAttribute[program], 3, GL_FLOAT, false, 0, 0);

    glBindBuffer(GL_ARRAY_BUFFER, compositeVertexTexCoordBuffer);
    glVertexAttribPointer(compositeVertexTexCoordAttribute[program], 2, GL_FLOAT, false, 0, 0);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, context.getLeftEyeTexture());
    glUniform1i(leftEyeUniform[program], 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, context.getRightEyeTexture());
    glUniform1i(rightEyeUniform[program], 1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, compositeVertexIndexBuffer);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);

    glUseProgram(0);
}

///////////////////////////////////////////////////////////////////////////////
int Renderer::getStereoCompositeProgram(DisplayTileConfig::StereoMode sm) {
    int programId;
    
    switch (sm) {
        case DisplayTileConfig::LineInterleaved:
            programId = 0;
            break;
        case DisplayTileConfig::ColumnInterleaved:
            programId = 1;
            break;
        case DisplayTileConfig::PixelInterleaved:
            programId = 2;
            break;
        case DisplayTileConfig::AnaglyphRedCyan:
            programId = 3;
            break;
        case DisplayTileConfig::AnaglyphGreenMagenta:
            programId = 4;
            break;
        default:
            programId = -1;
            break;
    }
    return programId;
}

