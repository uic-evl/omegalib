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
 *********************************************************************************************************************/
#ifndef __EQUALIZER_DISPLAY_SYSTEM_H__
#define __EQUALIZER_DISPLAY_SYSTEM_H__

#include "DisplaySystem.h"
#include "Application.h"

namespace omega
{
//typedef co::base::uint128_t uint128_t;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Forward Declarations.
class EqualizerNodeFactory;
class ViewImpl;
class ConfigImpl;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// This class is used to route equalizer log into the omega log system.
class EqualizerLogStreamBuf: public std::streambuf
{
protected:
	virtual int overflow ( int c = EOF )
	{
		if(c == '\n')
		{
			omsg(myStringStream.str().c_str());
			myStringStream.str(""); 
		}
		else
		{
			myStringStream << (char)c;
		}
		return 0;
	}
private:
    std::ostringstream myStringStream;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
class OMEGA_API EqualizerDisplaySystem: public DisplaySystem
{
public:
	EqualizerDisplaySystem();
	~EqualizerDisplaySystem();

	// sets up the display system. Called before initalize.
	void setup(Setting& setting);

	void initialize(SystemManager* sys); 
	void run(); 
	void cleanup(); 

	String getDisplayConfig() { return myDisplayConfig; }
	void setDisplayConfig(const String& value) { myDisplayConfig = value; }

	Observer* getObserver(int observerId);

	//! Layer and view management.
	//@{
	void setLayer(const char* viewName, Layer::Enum layer);
	Layer::Enum getLayer(const char* viewName);
	//}@

	virtual DisplaySystemType getId() { return DisplaySystem::Equalizer; }
	bool isDebugMouseEnabled() { return myDebugMouse; }

	//! Returns the size of the display canvas.
	virtual Vector2i getCanvasSize();
	//! Returns a view ray given a global (canvas) pointer position in pixel coordinates
	virtual Ray getViewRay(Vector2i position);
	//! Returns a view ray given a loal pointer positon and a channel index.
	Ray	getViewRay(Vector2i position, int channelX, int channelY);

private:
	void initLayers();
	void initObservers();

private:
	SystemManager* mySys;

	// Display config
	Setting* mySetting;
	String myDisplayConfig;

	// Equalizer stuff.
    EqualizerNodeFactory* myNodeFactory;
	ConfigImpl* myConfig;

	// Observers.
	std::vector<Observer*> myObservers;

	// Debug
	bool myDebugMouse;
};

}; // namespace omega

#endif