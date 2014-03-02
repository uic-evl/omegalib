/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted 
 * provided that the following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions 
 * and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
 * notice, this list of conditions and the following disclaimer in the documentation and/or other 
 * materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
 * FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
 * USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
 * ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *************************************************************************************************/
#include "omega/ApplicationBase.h"
#include "omega/SystemManager.h"
#include "omega/Engine.h"
#include "omega/Renderer.h"
#include "omega/GlutDisplaySystem.h"

#define GLEW_MX
#include "GL/glew.h"

#ifdef __APPLE__
#include <GLUT/glut.h>
#else
#include "GL/freeglut.h"
#endif

using namespace omega;

GLEWContext sGLEWContext;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void displayCallback(void) 
{
	static float lt = 0.0f;
	static uint64 frame = 0;

	GlutDisplaySystem* ds = (GlutDisplaySystem*)SystemManager::instance()->getDisplaySystem();
	Engine* as = ds->getApplicationServer();
	Renderer* ac = ds->getApplicationClient();

	// Compute dt.
	float t = (float)((double)clock() / CLOCKS_PER_SEC);
	UpdateContext uc;
	uc.dt = t - lt;
	lt = t;

	as->update(uc);
	//ac->update(uc);

	glClearColor(0, 0, 0, 0);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	// setup the context viewport.
	DrawContext dc;
	DisplayTileConfig dtc;
	dtc.device = 0;
	//dtc.index = Vector2i::Zero();
	dtc.offset = Vector2i::Zero();
	dtc.pixelSize = ds->getCanvasSize();

	dc.frameNum = frame++;
	dc.tile = &dtc;
	dc.gpuContext = ds->getGpuContext();

	ds->updateProjectionMatrix();

	Camera* cam = Engine::instance()->getDefaultCamera();

	// Push observer matrix.
	glPushMatrix();
	AffineTransform3 mat = cam->getViewTransform();
	glLoadIdentity();
	glLoadMatrixd(mat.data());

	dc.viewport = Rect(0, 0, glutGet(GLUT_WINDOW_WIDTH), glutGet(GLUT_WINDOW_HEIGHT));
	glGetDoublev( GL_MODELVIEW_MATRIX, dc.modelview.data() );
	glGetDoublev( GL_PROJECTION_MATRIX, dc.projection.data() );

	//dc.drawBuffer = ds->getFrameBuffer();

	// Process events.
	ServiceManager* im = SystemManager::instance()->getServiceManager();
	int av = im->getAvailableEvents();
	if(av != 0)
	{
		Event evts[OMICRON_MAX_EVENTS];
		im->getEvents(evts, ServiceManager::MaxEvents);

		// Dispatch events to application server.
		for( int evtNum = 0; evtNum < av; evtNum++)
		{
			as->handleEvent(evts[evtNum]);
		}
	}

	dc.eye = DrawContext::EyeCyclop;
	dc.task = DrawContext::SceneDrawTask;
	ac->draw(dc);
	dc.task = DrawContext::OverlayDrawTask;
	ac->draw(dc);

	glPopMatrix();

	//glFlush();
	glutPostRedisplay();

	// poll the input manager for new events.
	im->poll();

	if(SystemManager::instance()->isExitRequested())
	{
		exit(0);
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GlutDisplaySystem::GlutDisplaySystem():
	mySys(NULL),
	myFrameBuffer(NULL)
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
GlutDisplaySystem::~GlutDisplaySystem()
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GlutDisplaySystem::setup(Setting& setting) 
{
	String sCfg;
	setting.lookupValue("config", sCfg);

	int width;
	int height;
	myFov = 60;
	myNearz = 1.0f;
	myFarz = 100;

	libconfig::ArgumentHelper ah;
	ah.newInt("width", "Resolution width", width);
	ah.newInt("height", "Resolution width", height);
	ah.newNamedInt('f', "fov", "fov", "field of view", myFov);
	ah.newNamedDouble('z', "nearz", "nearZ", "near Z clipping plane", myNearz);
	ah.newNamedDouble('Z', "farz", "farZ", "far Z clipping plane", myFarz);
	ah.process(sCfg.c_str());

	myResolution[0] = width;
	myResolution[1] = height;
	myAspect = (float)width / height;

	mySetting = &setting;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GlutDisplaySystem::updateProjectionMatrix()
{
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(myFov, myAspect, myNearz, myFarz);
	glMatrixMode(GL_MODELVIEW);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GlutDisplaySystem::initialize(SystemManager* sys)
{
	mySys = sys;

	char* argv = "";
	int argcp = 1;

	// Setup and initialize Glut
	glutInit(&argcp, &argv);
	glutInitWindowPosition(0, 0);
	glutInitWindowSize(myResolution[0], myResolution[1]);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DEPTH);
	glutCreateWindow(sys->getApplication()->getName()); 

	glewSetContext(&sGLEWContext);

	// Init glew
#ifndef __APPLE__
	glewInit();
#endif

	glutDisplayFunc(displayCallback); 

	myGpuContext = new GpuContext();

	// Setup and initialize the application server and client.
	ApplicationBase* app = SystemManager::instance()->getApplication();
	if(app)
	{
		myAppServer = new Engine(app);
		myAppClient = new Renderer(myAppServer);

		myAppServer->initialize();

		//myFrameBuffer = new RenderTarget();
		//myFrameBuffer->initialize(RenderTarget::TypeFrameBuffer, myResolution[0], myResolution[1]);
		//myAppClient->setup();
		myAppClient->setGpuContext(myGpuContext);
		myAppClient->initialize();
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GlutDisplaySystem::run()
{
	glutMainLoop();
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void GlutDisplaySystem::cleanup()
{
}
