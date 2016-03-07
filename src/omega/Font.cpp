/**************************************************************************************************
 * THE OMEGA LIB PROJECT
 *-------------------------------------------------------------------------------------------------
 * Copyright 2010-2015		Electronic Visualization Laboratory, University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-------------------------------------------------------------------------------------------------
 * Copyright (c) 2010-2015, Electronic Visualization Laboratory, University of Illinois at Chicago
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
#include "omega/Font.h"
#include "omega/glheaders.h"

#include "FTGL/ftgl.h"

using namespace omega;


Lock Font::sLock;

///////////////////////////////////////////////////////////////////////////////////////////////////
void Font::lock()
{
	sLock.lock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Font::unlock()
{
	sLock.unlock();
}

////////////////////////////////////////////////////////////////////////////////
Dictionary<String, FTFont*> sFontCache;
Vector2f Font::getTextSize(const String& text, const String& font)
{
    // Add font to cache if needed.
    if(sFontCache.find(font) == sFontCache.end())
    {
	    Vector<String> args = StringUtils::split(font);
	    if(args.size() < 2)
	    {
		    owarn("Font::getTextSize: Invalid font creation arguments");
		    return Vector2f::Zero();
	    }
	    String fontFile = args[0];
	    int fontSize = boost::lexical_cast<int>(args[1]);
            String fontPath;
	    if(!DataManager::findFile(fontFile, fontPath))
	    {
		    ofwarn("Font::getTextSize: could not find font file %1%", %fontFile);
		    return Vector2f::Zero();
	    }

	    FTFont* fontImpl = new FTBitmapFont(fontPath.c_str());

	    if(fontImpl->Error())
	    {
		    ofwarn("Font %1% failed to open", %fontFile);
		    delete fontImpl;
		    return Vector2f::Zero();
	    }

	    if(!fontImpl->FaceSize(fontSize))
	    {
		    ofwarn("Font %1% failed to set size %2%", %fontFile %fontSize);
		    delete fontImpl;
	    }

        sFontCache[font] = fontImpl;
    }
    FTFont* fontImpl = sFontCache[font];
	FTBBox bbox = fontImpl->BBox(text.c_str());
	Vector2f size = Vector2f((int)bbox.Upper().Xf(), (int)bbox.Upper().Yf());
    return size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Vector2f Font::getTextWSize(const std::wstring& text, const String& font)
{
	// Add font to cache if needed.
    if(sFontCache.find(font) == sFontCache.end())
    {
	    Vector<String> args = StringUtils::split(font);
	    if(args.size() < 2)
	    {
		    owarn("Font::getTextSize: Invalid font creation arguments");
		    return Vector2f::Zero();
	    }
	    String fontFile = args[0];
	    int fontSize = boost::lexical_cast<int>(args[1]);
            String fontPath;
	    if(!DataManager::findFile(fontFile, fontPath))
	    {
		    ofwarn("Font::getTextSize: could not find font file %1%", %fontFile);
		    return Vector2f::Zero();
	    }

	    FTFont* fontImpl = new FTBitmapFont(fontPath.c_str());

	    if(fontImpl->Error())
	    {
		    ofwarn("Font %1% failed to open", %fontFile);
		    delete fontImpl;
		    return Vector2f::Zero();
	    }

	    if(!fontImpl->FaceSize(fontSize))
	    {
		    ofwarn("Font %1% failed to set size %2%", %fontFile %fontSize);
		    delete fontImpl;
	    }

        sFontCache[font] = fontImpl;
    }
    FTFont* fontImpl = sFontCache[font];
	FTBBox bbox = fontImpl->BBox(text.c_str());
	Vector2f size = Vector2f((int)bbox.Upper().Xf(), (int)bbox.Upper().Yf());
    return size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Vector2f Font::computeSize(const omega::String& text) 
{ 
	Font::lock();
	FTBBox bbox = myFontImpl->BBox(text.c_str());
	Vector2f size = Vector2f((int)bbox.Upper().Xf(), (int)bbox.Upper().Yf());
	Font::unlock();
	return size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Vector2f Font::computeWSize(const std::wstring& text) 
{ 
	Font::lock();
	FTBBox bbox = myFontImpl->BBox(text.c_str());
	Vector2f size = Vector2f((int)bbox.Upper().Xf(), (int)bbox.Upper().Yf());
	Font::unlock();
	return size;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Font::render(const omega::String& text, float x, float y) 
{ 
	Font::lock();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(1.0f, -1.0f, 1.0f);

	myFontImpl->Render(text.c_str(), (int)text.length(), FTPoint(x, y, 0.0f)); 

	glPopMatrix();
	Font::unlock();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void Font::render(const std::wstring& text, float x, float y) 
{ 
	Font::lock();
	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glScalef(1.0f, -1.0f, 1.0f);

	myFontImpl->Render(text.c_str(), (int)text.length(), FTPoint(x, y, 0.0f)); 

	glPopMatrix();
	Font::unlock();
}
