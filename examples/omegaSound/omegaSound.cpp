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
 *	omegaSound
 *		Based on ohello2 with additional sound functionality. Draws a colored cube at the wand position and plays a sound
 *		and flashes yellow on any button press. Button 3 (cross on PS3 Navigator controller) will play a stereo sound
 *		clip as background music. All other positional sounds are mono .wav files.
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
	HelloApplication(): EngineModule("HelloApplication")
	{ enableSharedData(); }

	~HelloApplication()
	{
		delete sound;
		delete sound1;
		delete music;
	}

	virtual void initialize()
	{
		env = getEngine()->getSoundEnvironment();
		env->setAssetDirectory("/Users/evldemo/sounds/");
		
		// All sound files should be in .wav format
		// For positional sounds, file should be mono.
		// Rest of important sound code is in handleEvent()
		sound = env->loadSoundFromFile("beep", "/menu_sounds/menu_select.wav");
		music = env->loadSoundFromFile("music", "/music/filmic.wav");
		sound1 = env->loadSoundFromFile("tricorder", "/arthur/TOS-tricorder.wav");

		// playStereo() is for ambient music.
		// Currently available functions:
		//	 setVolume(float) - amplitude from (0.0 - 1.0)
		//	 setLoop(bool)
		//	 stop() - this means this instance is finished and a new
		//			  sound instance will need to be created to play
		SoundInstance* musicInst = new SoundInstance(music);
		musicInst->setVolume(0.2f);
		musicInst->playStereo();

		changeCubeColor = false;
	}


	virtual void initializeRenderer(Renderer* r) 
	{ 
		r->addRenderPass(new HelloRenderPass(r, this));
	}

	float getXPos() { return xPos; }
	float getYPos() { return yPos; }
	float getZPos() { return zPos; }
	
	bool isCubeColoredBySound() { return changeCubeColor; }
	void resetCubeColor() { changeCubeColor = false; }

	virtual void handleEvent(const Event& evt);
	virtual void commitSharedData(SharedOStream& out);
	virtual void updateSharedData(SharedIStream& in);

private:
	float xPos;
	float yPos;
	float zPos;
	float dist;
	
	bool changeCubeColor;

	SoundEnvironment* env;
	Sound* sound;
	Sound* sound1;
	Sound* music;
	SoundInstance* soundLoopInst;
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

		// Draw a rotating box.
		glTranslatef(myApplication->getXPos(), myApplication->getYPos(), myApplication->getZPos()); 
		glRotatef(10, 1, 0, 0);
		//glRotatef(myApplication->getYaw(), 0, 1, 0);
		//glRotatef(myApplication->getPitch(), 1, 0, 0);

		// Draw a box
		for (int i = 0; i < 6; i++) 
		{
			glBegin(GL_QUADS);
			if( myApplication->isCubeColoredBySound() )
			{
				glColor3fv(myFaceColors[5].data());
			}
			else
				glColor3fv(myFaceColors[0].data());
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
	if(evt.getServiceType() == Service::Wand)
	{
				
		xPos = evt.getPosition().x();
		yPos = evt.getPosition().y();
		zPos = evt.getPosition().z();
		
		if( soundLoopInst != NULL )
			soundLoopInst->setPosition( evt.getPosition() );
	
		if( evt.getType() == Event::Down )
		{
			if( evt.getFlags() == Event::Button3 ) // Wand cross button
			{
				// playStereo() is for ambient music.
				// Currently available functions:
				//	 setVolume(float) - amplitude from (0.0 - 1.0)
				//	 setLoop(bool)
				//	 stop() - this means this instance is finished and a new
				//			  sound instance will need to be created to play
				SoundInstance* musicInst = new SoundInstance(music);
				musicInst->setVolume(0.2f);
				musicInst->playStereo();
			}
			else if( evt.getFlags() == Event::Button5 ) // Wand L1
			{
				env->getSoundManager()->stopAllSounds();
			}
			else if( evt.getFlags() == Event::ButtonLeft ) // DPad
			{
				SoundInstance* soundInst = new SoundInstance(sound);
				soundInst->setPosition( evt.getPosition() );
				soundInst->play();
				changeCubeColor = true;
			}
			else if( evt.getFlags() == Event::ButtonRight ) // DPad
			{
				if( soundLoopInst != NULL || !soundLoopInst->isPlaying() )
				{
					// Positional sounds use play()
					// Currently available functions:
					//	 stop() - this means this instance is finished and a new
					//			  sound instance will need to be created to play
					//	 setPosition(Vector3f) 
					//	 setVolume(float) - amplitude from (0.0 - 1.0)
					//	 setLoop(bool)
					//	 setMix(float) - wetness of sound (0.0 - 1.0)
					//	 setReverb(float) - room size / reverb amount (0.0 - 1.0)
					//	 setWidth(int) - number of speakers to spread sound across (1-20)
					//			This will eventually be replaced with a sound radius
					soundLoopInst = new SoundInstance(sound1);
					soundLoopInst->setPosition( evt.getPosition() );
					soundLoopInst->setLoop(true);
					soundLoopInst->setVolume(0.2f);
					soundLoopInst->setWidth(3);
					soundLoopInst->play();
				}
				else if( soundLoopInst != NULL )
				{
					soundLoopInst->stop();
				}
				
				changeCubeColor = true;
			}
			else
			{
				SoundInstance* soundInst = new SoundInstance(sound);
				soundInst->setPosition( evt.getPosition() );
				soundInst->setRoomSize(1.0);
				soundInst->setWetness(1.0);
				//soundInst->setVolume(0.5);
				soundInst->play();
				changeCubeColor = true;
			}
			
			
		}
		else if( evt.getType() == Event::Up )
		{
			changeCubeColor = false;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void HelloApplication::commitSharedData(SharedOStream& out)
{
	out << xPos << yPos << zPos << changeCubeColor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void HelloApplication::updateSharedData(SharedIStream& in)
{
 	in >> xPos >> yPos >> zPos >> changeCubeColor;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ApplicationBase entry point
int main(int argc, char** argv)
{
	Application<HelloApplication> app("omegaSound");
    return omain(app, argc, argv);
}
