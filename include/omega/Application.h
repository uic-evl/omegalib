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
#ifndef __ENGINE_APPLICATION_H__
#define __ENGINE_APPLICATION_H__

#include "osystem.h"
#include "Renderer.h"
#include "omega/ApplicationBase.h"
#include "omega/SystemManager.h"
#include "omega/SharedDataServices.h"
#include "omega/EventSharingModule.h"

namespace omega {
	class RenderPass;
	
	///////////////////////////////////////////////////////////////////////////////////////////////
	//! a convenience application class to create omegaToolkit applications
	template<typename T> 
	class Application: public ApplicationBase
	{
	public:
		Application(const String& name): myAppName(name), myExecutableName(name)
		{ }

		virtual const char* getName() 
		{ return myAppName.c_str(); }

		virtual void setExecutableName(const String& name) 
		{ myExecutableName = name; }

		virtual const char* getExecutableName() 
		{ return myExecutableName.c_str(); }
		
		virtual void initialize() 
		{ 
			ModuleServices::addModule(new T());
		}

	private:
		String myAppName;
		String myExecutableName;
	};
}; // namespace omega

#endif
