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
*	A basic desktop display system using GLFW
******************************************************************************/
#include <omega.h>
#include <GLFW/glfw3.h>

using namespace omega;

///////////////////////////////////////////////////////////////////////////////
class GLFWDisplaySystem : public DisplaySystem
{
public:
	GLFWDisplaySystem();
	virtual void run();

private:
	bool myInitialized;
	Ref<GpuContext> myGpuContext;
	Ref<Renderer> myRenderer;
	Ref<Engine> myEngine;
};

///////////////////////////////////////////////////////////////////////////////
static void errorCallback(int error, const char* description)
{
	oferror("[GLFW] %1% ", %description);
}

///////////////////////////////////////////////////////////////////////////////
GLFWDisplaySystem::GLFWDisplaySystem()
{
}

///////////////////////////////////////////////////////////////////////////////
void GLFWDisplaySystem::run()
{
	GLFWwindow* window;

	glfwSetErrorCallback(errorCallback);

	if (!glfwInit()) oexit(-1);

	DisplayConfig& dcfg = getDisplayConfig();
	ApplicationBase* app = SystemManager::instance()->getApplication();
	myEngine = new Engine(app);
	myRenderer = new Renderer(myEngine);
	myEngine->initialize();
	
	myGpuContext = new GpuContext();
	myRenderer->setGpuContext(myGpuContext);
	myRenderer->initialize();

	DisplayTileConfig* tile = dcfg.tileGrid[0][0];
	Vector2i& ws = tile->pixelSize;
	window = glfwCreateWindow(ws[0], ws[1], app->getName(), NULL, NULL);
	oassert(window != NULL);

	glfwMakeContextCurrent(window);
	glfwSwapInterval(1);


	float lt = 0.0f;
	uint64 frame = 0;
	UpdateContext uc;
	DrawContext dc;
	dc.tile = tile;
	dc.gpuContext = myGpuContext;
	dc.renderer = myRenderer;
	while (!SystemManager::instance()->isExitRequested())
	{
		// Compute dt.
		float t = (float)((double)clock() / CLOCKS_PER_SEC);
		uc.dt = t - lt;
		lt = t;

		myEngine->update(uc);
		// Process events.
		ServiceManager* im = SystemManager::instance()->getServiceManager();
		int av = im->getAvailableEvents();
		if (av != 0)
		{
			Event evts[OMICRON_MAX_EVENTS];
			im->getEvents(evts, ServiceManager::MaxEvents);

			// Dispatch events to application server.
			for (int evtNum = 0; evtNum < av; evtNum++)
			{
				myEngine->handleEvent(evts[evtNum]);
			}
		}

		// Handle window resize
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		if (tile->activeRect.width() != width ||
			tile->activeRect.height() != height)
		{
			Vector2i ws(width, height);
			tile->activeRect.max = tile->activeRect.min + ws;
			tile->pixelSize = ws;
			tile->activeCanvasRect.max = ws;
			tile->displayConfig.setCanvasRect(tile->activeCanvasRect);
		}
		myRenderer->prepare(dc);
		dc.drawFrame(frame++);
		glfwSwapBuffers(window);

		// Poll the service manager for new events.
		glfwPollEvents();
		im->poll();
	}

	glfwDestroyWindow(window);
	glfwTerminate();
}

///////////////////////////////////////////////////////////////////////////////
// Entry point
extern "C"
#ifdef OMEGA_OS_WIN
	__declspec(dllexport)
#endif
DisplaySystem* createDisplaySystem()
{ return new GLFWDisplaySystem(); }