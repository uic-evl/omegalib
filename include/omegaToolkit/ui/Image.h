/******************************************************************************
 * THE OMEGA LIB PROJECT
 *-----------------------------------------------------------------------------
 * Copyright 2010-2013		Electronic Visualization Laboratory, 
 *							University of Illinois at Chicago
 * Authors:										
 *  Alessandro Febretti		febret@gmail.com
 *-----------------------------------------------------------------------------
 * Copyright (c) 2010-2013, Electronic Visualization Laboratory,  
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
 * What's in this file:
 *	A widget displaying a single image
 ******************************************************************************/
#ifndef __IMAGE_H__
#define __IMAGE_H__

#include "omegaToolkit/omegaToolkitConfig.h"
#include "omega/ImageUtils.h"
#include "omegaToolkit/ui/Widget.h"

namespace omegaToolkit { namespace ui {
	///////////////////////////////////////////////////////////////////////////
	class OTK_API Image: public Widget
	{
	friend class ImageRenderable;
	public:
		static Image* create(Container* container);

	public:
		Image(Engine* srv);
		virtual ~Image();

		Renderable* createRenderable();

		PixelData* getData();
		void setData(PixelData* data);

		void flipX(bool value);
		void flipY(bool value);

        //! Sets the pixel rect in the source pixeldata to use on the image.
        //! If rect is set to (0,0,0,0) the full image will be used.
        void setSourceRect(int x, int y, int w, int h);
        // Sets the destination rect within the Image widget rect to use
        //! as the image output. If rect is set to (0,0,0,0) the full image
        //! area will be used.
        void setDestRect(int x, int y, int w, int h);

	protected:
		Ref<PixelData> myData;
		uint myFlipFlags;

        Rect myDestRect;
        Rect mySourceRect;
        bool myUseFullSource;
        bool myUseFullDest;
    };

	///////////////////////////////////////////////////////////////////////////
	class OTK_API ImageRenderable: public WidgetRenderable
	{
	public:
		ImageRenderable(Image* owner): 
		  WidgetRenderable(owner), 
		  myOwner(owner),
		  myTextureUniform(0) {}

		virtual ~ImageRenderable();
		virtual void refresh();
		virtual void drawContent(const DrawContext& context);

	private:
		Image* myOwner;
		GLuint myTextureUniform;
	};

	///////////////////////////////////////////////////////////////////////////
	inline PixelData* Image::getData() 
	{ return myData; }
}; }; // namespace omegaToolkit
#endif