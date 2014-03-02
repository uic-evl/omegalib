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
#ifndef __FONT_H__
#define __FONT_H__

#include "omega/osystem.h"

class FTFont;

namespace omega {
	///////////////////////////////////////////////////////////////////////////////////////////////
	struct FontInfo
	{
		FontInfo():
			name(""), filename(""), size(0) {} 
		FontInfo(String theName, String theFilename, int theSize):
			name(theName), filename(theFilename), size(theSize) {} 

		String name;
		String filename;
		int size;
	};

	///////////////////////////////////////////////////////////////////////////////////////////////
	class OMEGA_API Font: public ReferenceType
	{
	public:
		static void lock();
		static void unlock();

	public:
		enum Align {HALeft = 1 << 0, HARight = 1 << 1, HACenter = 1 << 2,
					VATop = 1 << 3, VABottom = 1 << 4, VAMiddle = 1 << 5};
	public:
		Font(FTFont* fontImpl): myFontImpl(fontImpl) {}

		void render(const String& text, float x, float y);
		Vector2f computeSize(const omega::String& text);

	private:
		static Lock sLock;
		FTFont* myFontImpl;
	};
}; // namespace omega

#endif