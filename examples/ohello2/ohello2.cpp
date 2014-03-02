/********************************************************************************************************************** 
 * THE OMEGA LIB PROJECT
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright 2010								Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti							febret@gmail.com
 *---------------------------------------------------------------------------------------------------------------------
 * Copyright (c) 2010, Electronic Visualization Laboratory, University of Illinois at Chicago
 * All rights reserved.
 * Redistribution and use in source and binary forms, with or without modification, are permitted provided that the 
 * following conditions are met:
 * 
 * Redistributions of source code must retain the above copyright notice, this list of conditions and the following 
 * disclaimer. Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
 * and the following disclaimer in the documentation and/or other materials provided with the distribution. 
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
 * INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR 
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE 
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *---------------------------------------------------------------------------------------------------------------------
 *	ohello2
 *		A slightly more complex version of ohello, implementing event handling and data sharing. ohello2 defines a 
 *		new class, HelloServer, that reads mouse events and uses it to rotate the rendered object. 
 *		HelloServer also implements SharedObject to synchronize rotation data across all nodes.
 *********************************************************************************************************************/
#include <omega.h>
#include <omegaGl.h>

using namespace omega;

class HelloApplication;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class HelloRenderPass: public RenderPass
{
public:
	HelloRenderPass(Renderer* client, HelloApplication* app): RenderPass(client, "HelloRenderPass"), myApplication(app) {}
	virtual void initialize();
	virtual void render(Renderer* client, const DrawContext& context);

private:
	HelloApplication* myApplication;

	Vector3s myNormals[6];
	Vector4i myFaces[6]; 
	Vector3s myVertices[8];
	Color myFaceColors[6];
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class HelloApplication: public EngineModule
{
public:
	HelloApplication(): EngineModule("HelloApplication") { enableSharedData(); }

	virtual void initializeRenderer(Renderer* r) 
	{ 
		r->addRenderPass(new HelloRenderPass(r, this));
	}

	float getYaw() { return myYaw; }
	float getPitch() { return myPitch; }

	virtual void handleEvent(const Event& evt);
	virtual void commitSharedData(SharedOStream& out);
	virtual void updateSharedData(SharedIStream& in);

private:
	float myYaw;
	float myPitch;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void HelloRenderPass::initialize()
{
	RenderPass::initialize();

	// Initialize cube normals.
	myNormals[0] = Vector3s(-1, 0, 0);
	myNormals[1] = Vector3s(0, 1, 0);
	myNormals[2] = Vector3s(1, 0, 0);
	myNormals[3] = Vector3s(0, -1, 0);
	myNormals[4] = Vector3s(0, 0, 1);
	myNormals[5] = Vector3s(0, 0, -1);

	// Initialize cube face indices.
	myFaces[0] = Vector4i(0, 1, 2, 3);
	myFaces[1] = Vector4i(3, 2, 6, 7);
	myFaces[2] = Vector4i(7, 6, 5, 4);
	myFaces[3] = Vector4i(4, 5, 1, 0);
	myFaces[4] = Vector4i(5, 6, 2, 1);
	myFaces[5] = Vector4i(7, 4, 0, 3);

	// Initialize cube face colors.
	myFaceColors[0] = Color::Aqua;
	myFaceColors[1] = Color::Orange;
	myFaceColors[2] = Color::Olive;
	myFaceColors[3] = Color::Navy;
	myFaceColors[4] = Color::Red;
	myFaceColors[5] = Color::Yellow;

	// Setup cube vertex data
	float size = 0.2f;
	myVertices[0][0] = myVertices[1][0] = myVertices[2][0] = myVertices[3][0] = -size;
	myVertices[4][0] = myVertices[5][0] = myVertices[6][0] = myVertices[7][0] = size;
	myVertices[0][1] = myVertices[1][1] = myVertices[4][1] = myVertices[5][1] = -size;
	myVertices[2][1] = myVertices[3][1] = myVertices[6][1] = myVertices[7][1] = size;
	myVertices[0][2] = myVertices[3][2] = myVertices[4][2] = myVertices[7][2] = size;
	myVertices[1][2] = myVertices[2][2] = myVertices[5][2] = myVertices[6][2] = -size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void HelloRenderPass::render(Renderer* client, const DrawContext& context)
{
	if(context.task == DrawContext::SceneDrawTask)
	{
		client->getRenderer()->beginDraw3D(context);

		// Enable depth testing and lighting.
		glEnable(GL_DEPTH_TEST);
		glEnable(GL_LIGHTING);
	
		// Setup light.
		glEnable(GL_LIGHT0);
		glEnable(GL_COLOR_MATERIAL);
		glLightfv(GL_LIGHT0, GL_COLOR, Color(1.0, 1.0, 1.0).data());
		glLightfv(GL_LIGHT0, GL_POSITION, Vector3s(0.0f, 0.0f, 1.0f).data());

		// Draw a rotating teapot.
		glTranslatef(0, 2, -2); 
		glRotatef(10, 1, 0, 0);
		glRotatef(myApplication->getYaw(), 0, 1, 0);
		glRotatef(myApplication->getPitch(), 1, 0, 0);

		// Draw a box
		for (int i = 0; i < 6; i++) 
		{
			glBegin(GL_QUADS);
			glColor3fv(myFaceColors[i].data());
			glNormal3fv(myNormals[i].data());
			glVertex3fv(myVertices[myFaces[i][0]].data());
			glVertex3fv(myVertices[myFaces[i][1]].data());
			glVertex3fv(myVertices[myFaces[i][2]].data());
			glVertex3fv(myVertices[myFaces[i][3]].data());
			glEnd();
		}

		client->getRenderer()->endDraw();
	}
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void HelloApplication::handleEvent(const Event& evt)
{
	if(evt.getServiceType() == Service::Pointer)
	{
		// Normalize the mouse position using the total display resolution, 
		// then multiply to get 180 degree rotations
		DisplaySystem* ds = getEngine()->getDisplaySystem();
		Vector2i resolution = ds->getCanvasSize();
		myYaw = (evt.getPosition().x() / resolution[0]) * 180;
		myPitch = (evt.getPosition().y() / resolution[1]) * 180;
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void HelloApplication::commitSharedData(SharedOStream& out)
{
	out << myYaw << myPitch;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void HelloApplication::updateSharedData(SharedIStream& in)
{
 	in >> myYaw >> myPitch;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ApplicationBase entry point
int main(int argc, char** argv)
{
	Application<HelloApplication> app("ohello2");
    return omain(app, argc, argv);
}
