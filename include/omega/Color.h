/**************************************************************************************************
* THE OMICRON PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#ifndef __COLOR_H__
#define __COLOR_H__

#include "osystem.h"
#include "omicron/StringUtils.h"

namespace omega
{
	///////////////////////////////////////////////////////////////////////////////////////////////
	class OMEGA_API Color
	{
	public:
		const float& operator[](int i) const { return myData[i]; }
		float& operator[](int i) { return myData[i]; }
		float* data() { return (float*)myData; }
		const float* data() const { return (const float*)myData; }

	public:
		static const Color& getColorByIndex(int index);
		static const Color White;
		static const Color Black;
		static const Color Orange;
		static const Color Silver;
		static const Color Gray;
		static const Color Red;
		static const Color Maroon;
		static const Color Yellow;
		static const Color Olive;
		static const Color Lime;
		static const Color Green;
		static const Color Aqua;
		static const Color Teal;
		static const Color Blue;
		static const Color Navy;
		static const Color Fuchsia;
		static const Color Purple;

		static bool isValidColor(const String& lname)
		{
			if(lname[0]=='#') return true;
			if(lname == "white") return true;
			if(lname == "black") return true;
			if(lname == "orange") return true;
			if(lname == "silver") return true;
			if(lname == "gray") return true;
			if(lname == "red") return true;
			if(lname == "maroon") return true;
			if(lname == "yellow") return true;
			if(lname == "olive") return true;
			if(lname == "lime") return true;
			if(lname == "green") return true;
			if(lname == "aqua") return true;
			if(lname == "teal") return true;
			if(lname == "blue") return true;
			if(lname == "navy") return true;
			if(lname == "fuchsia") return true;
			if(lname == "purple") return true;
			return false;
		}

	public:
		Color(float r = 1.0f, float g = 1.0f, float b = 1.0f, float a = 1.0f)
		{
			myData[0] = r;
			myData[1] = g;
			myData[2] = b;
			myData[3] = a;
		}

		Color(const Color& color) { memcpy(myData, color.myData, 4 * sizeof(float)); }

		Color(const String& name)
		{
			String lname = name;
			StringUtils::toLowerCase(lname);
			if(lname[0] == '#')
			{
				uint x;
				sscanf(lname.c_str(), "#%x", &x);		
				if(lname.size() == 7)
				{
					// Parse rgb
					myData[0] = (float)((x & 0xff0000) >> 16) / 256;
					// green
					myData[1] = (float)((x & 0x00ff00) >> 8) / 256;
					// blue
					myData[2] = (float)((x & 0x0000ff)) / 256;
					// alpha
					myData[3] = 1.0f;
				}
				else
				{
					// Parse rgba
					// red
					myData[0] = (float)((x & 0xff000000) >> 24) / 256;
					// green
					myData[1] = (float)((x & 0x00ff0000) >> 16) / 256;
					// blue
					myData[2] = (float)((x & 0x0000ff00) >> 8) / 256;
					// alpha
					myData[3] = (float)((x & 0x000000ff)) / 256;
				}
			}
			else
			{
				if(lname == "white") *this = White;
				if(lname == "black") *this = Black;
				if(lname == "orange") *this = Orange;
				if(lname == "silver") *this = Silver;
				if(lname == "gray") *this = Gray;
				if(lname == "red") *this = Red;
				if(lname == "maroon") *this = Maroon;
				if(lname == "yellow") *this = Yellow;
				if(lname == "olive") *this = Olive;
				if(lname == "lime") *this = Lime;
				if(lname == "green") *this = Green;
				if(lname == "aqua") *this = Aqua;
				if(lname == "teal") *this = Teal;
				if(lname == "blue") *this = Blue;
				if(lname == "navy") *this = Navy;
				if(lname == "fuchsia") *this = Fuchsia;
				if(lname == "purple") *this = Purple;
			}
		}

		float getRed() { return myData[0]; }
		void setRed(float value) { myData[0] = value; }

		float getGreen() { return myData[1]; }
		void setGreen(float value) { myData[1] = value; }

		float getBlue() { return myData[2]; }
		void setBlue(float value) { myData[2] = value; }

		float getAlpha() { return myData[3]; }
		void setAlpha(float value) { myData[3] = value; }

		Color scale(float f, bool alpha = false) { return Color(myData[0] * f, myData[1] * f, myData[2] * f, alpha ? myData[3] * f : myData[3]); }
	private:
		float myData[4];
	};
}; // namespace omicron

#endif