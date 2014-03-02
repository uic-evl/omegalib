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
#include "omega/ImageUtils.h"
#include "omega/SystemManager.h"
#include "FreeImage.h"

#ifdef OMEGA_USE_FASTIMAGE
#include "image.h"
#endif

using namespace omega;

// Vector of preallocated memory blocks for image loading.
Vector<void*> ImageUtils::sPreallocBlocks;
size_t ImageUtils::sPreallocBlockSize;
int ImageUtils::sLoadPreallocBlock = -1;

Lock sImageQueueLock;
//Lock sImageLoaderLock;

Queue< Ref<ImageUtils::LoadImageAsyncTask> > sImageQueue;
bool sShutdownLoaderThread = false;

bool ImageUtils::sVerbose = false;

int ImageUtils::sNumLoaderThreads = 4;

List<Thread*> ImageUtils::sImageLoaderThread;

///////////////////////////////////////////////////////////////////////////////////////////////////
class ImageLoaderThread: public Thread
{
public:
	ImageLoaderThread()
	{}

	virtual void threadProc()
	{
		omsg("ImageLoaderThread: start");

		while(!sShutdownLoaderThread)
		{
			sImageQueueLock.lock();
			if(sImageQueue.size() > 0)
			{

				Ref<ImageUtils::LoadImageAsyncTask> task = sImageQueue.front();
				sImageQueue.pop();

				sImageQueueLock.unlock();

				Ref<PixelData> res = ImageUtils::loadImage(task->getData().path, task->getData().isFullPath);
				
				if(!sShutdownLoaderThread)
				{
					task->getData().image = res;
					task->notifyComplete();
				}
				//sImageQueueLock.unlock();

			}
			else
			{
				sImageQueueLock.unlock();
			}
			osleep(100);
		}

		omsg("ImageLoaderThread: shutdown");
	}

private:
};

///////////////////////////////////////////////////////////////////////////////////////////////////
bool ImageUtils::preallocateBlocks(size_t size, int numBlocks)
{
	ofmsg("ImageUtils::preallocateBlocks: allocating %1% blocks of size %2%Kb", %numBlocks %(size / 1024));
	for(int i = 0; i < numBlocks; i++)
	{
		void* block = malloc(size);
		if(block != NULL)
		{
			sPreallocBlocks.push_back(block);
		}
		else
		{
			ofwarn("ImageUtils::preallocateBlocks: failed allocating block %1%", %i);
			return false;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
int ImageUtils::getNumPreallocatedBlocks()
{
	return sPreallocBlocks.size();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void* ImageUtils::getPreallocatedBlock(int blockIndex)
{
	if(blockIndex < sPreallocBlocks.size()) return sPreallocBlocks[blockIndex];
	return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ImageUtils::setLoadPreallocatedBlock(int blockId)
{
	sLoadPreallocBlock = blockId;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ImageUtils::internalInitialize()
{
	FreeImage_Initialise();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
void ImageUtils::internalDispose()
{
	sShutdownLoaderThread = true;

	foreach(Thread* t, sImageLoaderThread) t->stop();

	FreeImage_DeInitialise();

	// Clean up preallocated memory blocks.
	foreach(void* ptr, sPreallocBlocks)
	{
		free(ptr);
	}
	sPreallocBlocks.empty();
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ImageUtils::LoadImageAsyncTask* ImageUtils::loadImageAsync(const String& filename, bool hasFullPath)
{
	if(sImageLoaderThread.size() == 0)
	{
		for(int i = 0; i < sNumLoaderThreads; i++)
		{
			Thread* t = new ImageLoaderThread();
			t->start();
			sImageLoaderThread.push_back(t);;
		}
	}

	sImageQueueLock.lock();
	LoadImageAsyncTask* task = new LoadImageAsyncTask();
	task->setData( LoadImageAsyncTask::Data(filename, hasFullPath) );
	task->setTaskId(filename);
	sImageQueue.push(task);
	sImageQueueLock.unlock();
	return task;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Ref<PixelData> ImageUtils::ffbmpToPixelData(FIBITMAP*& image, const String& filename)
{
	int bpp = FreeImage_GetBPP(image);
	int width = FreeImage_GetWidth(image);
	int height = FreeImage_GetHeight(image);

	// If blockId is not -1, use a preallocated memory block to load this image.
	byte* pdata = NULL;
	if(sLoadPreallocBlock != -1) pdata = (byte*)getPreallocatedBlock(sLoadPreallocBlock);

	Ref<PixelData> pixelData;
	int pixelOffset;
	if(bpp == 24)
	{
		pixelData = new PixelData(PixelData::FormatRgb, width, height, pdata);
		pixelOffset = 3;
	}
	else if(bpp == 32)
	{
		pixelData = new PixelData(PixelData::FormatRgba, width, height, pdata);
		pixelOffset = 4;
	}
	else if(bpp == 8)
	{
		// COnvert 8 bit palettized images to 24 bits.
		FIBITMAP* temp = image;
		image = FreeImage_ConvertTo24Bits(image);
		FreeImage_Unload(temp);
		pixelData = new PixelData(PixelData::FormatRgb, width, height, pdata);
		pixelOffset = 3;
	}
	else
	{
		ofwarn("ImageUtils::loadImage: unhandled bpp (%1%) while loading %2%", %bpp %filename);
		return NULL;
	}

	if(sVerbose) ofmsg("Image loaded: %1%. Size: %2%x%3%", %filename %width %height);
	
	byte* data = pixelData->map();
	
	for(int i = 0; i < height; i++)
	{
		char* pixels = (char*)FreeImage_GetScanLine(image, i);
		for(int j = 0; j < width; j++)
		{
			int k = i * width + j;

			data[k * pixelOffset + 0] = pixels[j * pixelOffset + 2];
			data[k * pixelOffset + 1] = pixels[j * pixelOffset + 1];
			data[k * pixelOffset + 2] = pixels[j * pixelOffset + 0];
			if(bpp == 32) data[k * pixelOffset + 3] = pixels[j * pixelOffset + 3];
			//data[j * pixelOffset + 3] = pixels[j * pixelOffset + 3];
		}
	}
	pixelData->unmap();
	
	return pixelData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Ref<PixelData> ImageUtils::loadImage(const String& filename, bool hasFullPath)
{
	String path;
	if(!hasFullPath)
	{
		if(!DataManager::findFile(filename, path))
		{
			//ofmsg("LOOKUP: %1%%2%", %ogetdataprefix() %filename);
			if(!DataManager::findFile(ogetdataprefix() + filename, path))
			{
				// Try adding the 
				ofwarn("ImageUtils::loadImage: could not load %1%: file not found.", %filename);
				return NULL;
			}
		}
	}
	else
	{
		path = filename;
	}

	uint bpp = 0;
	int width = 0;
	int height = 0;

	FREE_IMAGE_FORMAT format = FreeImage_GetFileType(path.c_str(), 0);
	
#ifdef OMEGA_USE_FASTIMAGE
	// Use the fast image loader for jpegs only for now.
	if(format == FIF_JPEG)
	{
		int b;
		int channels;
		byte* p = (byte*)image_read(path.c_str(), &width, &height, &channels, &b);
		bpp = channels * b;
		ofmsg("IMAGE %1% %2% %3% %4%", %width %height %channels %b);
		Ref<PixelData> pixelData;
		if(channels == 3)
		{
			pixelData = new PixelData(PixelData::FormatRgb, width, height, p);
		}
		else if(channels == 4)
		{
			pixelData = new PixelData(PixelData::FormatRgba, width, height, p);
		}
		// The pixel data object will own the image pointer, so re-enable delete.
		pixelData->setDeleteDisabled(true);
		return pixelData;
	}
#endif

	//OMEGA_STAT_BEGIN(imageLoad)
	FIBITMAP* image = FreeImage_Load(format, path.c_str());
	//OMEGA_STAT_END(imageLoad)
	if(image == NULL)
	{
		ofwarn("ImageUtils::loadImage: could not load %1%: unsupported file format, corrupted file or out of memory.", %filename);
		return NULL;
	}

	Ref<PixelData> pixelData = ffbmpToPixelData(image, filename);
	FreeImage_Unload(image);

	return pixelData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
Ref<PixelData> ImageUtils::loadImageFromStream(std::istream& fin, const String& streamName)
{
	// get length of file:
    fin.seekg (0, fin.end);
    int length = fin.tellg();
    fin.seekg (0, fin.beg);

    char * buffer = new char [length];
    fin.read(buffer,length);

	FIMEMORY* mem = FreeImage_OpenMemory((BYTE*)buffer, length);

	uint bpp = 0;
	int width = 0;
	int height = 0;

	FREE_IMAGE_FORMAT format = FreeImage_GetFileTypeFromMemory(mem);
	
	FIBITMAP* image = FreeImage_LoadFromMemory(format, mem);
	if(image == NULL)
	{
		ofwarn("ImageUtils::loadImage: could not load %1%: unsupported file format, corrupted file or out of memory.", %streamName);
		return NULL;
	}

	Ref<PixelData> pixelData = ffbmpToPixelData(image, streamName);
	
	FreeImage_Unload(image);
	FreeImage_CloseMemory(mem);
	delete buffer;

	return pixelData;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
ByteArray* ImageUtils::encode(PixelData* data, ImageFormat format)
{
	switch (format){
	// PNG
	case FormatPng :
		{
			// Allocate a freeimage bitmap and memory buffer to do the conversion.
			//FIBITMAP* fibmp = FreeImage_Allocate(data->getWidth(), data->getHeight(), 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
			FIMEMORY* fmem = FreeImage_OpenMemory();

			// For some reason it looks like color masks are ignored right now. Maybe it is just when encoding to png.
			byte* pdpixels = data->map();
			FIBITMAP* fibmp = FreeImage_ConvertFromRawBits(
				pdpixels,
				data->getWidth(),
				data->getHeight(),
				data->getPitch(),
				data->getBpp(),
				data->getRedMask(), data->getGreenMask(), data->getBlueMask());

			// Encode the bitmap to a freeimage memory buffer
			FreeImage_SaveToMemory(FIF_PNG, fibmp, fmem);

			// Copy the freeimage memory buffer to omegalib bytearray
			BYTE* fmemdata = NULL;
			DWORD fmemsize = 0;
			FreeImage_AcquireMemory(fmem, &fmemdata, &fmemsize);
			ByteArray* encodedData = new ByteArray(fmemsize);
			encodedData->copyFrom(fmemdata, fmemsize);

			// Release resources
			FreeImage_CloseMemory(fmem);
			FreeImage_Unload(fibmp);

			data->unmap();

			return encodedData;
		}
	// JPEG
	case FormatJpeg :
	// JPEG is default
	default:
		{
			// Allocate a freeimage bitmap and memory buffer to do the conversion.
			//FIBITMAP* fibmp = FreeImage_Allocate(data->getWidth(), data->getHeight(), 32, FI_RGBA_RED_MASK, FI_RGBA_GREEN_MASK, FI_RGBA_BLUE_MASK);
			FIMEMORY* fmem = FreeImage_OpenMemory();

			// For some reason it looks like color masks are ignored right now. Maybe it is just when encoding to png.
			byte* pdpixels = data->map();
			FIBITMAP* fibmp = FreeImage_ConvertFromRawBits(
				pdpixels,
				data->getWidth(),
				data->getHeight(),
				data->getPitch(),
				data->getBpp(),
				data->getRedMask(), data->getGreenMask(), data->getBlueMask());

			// Encode the bitmap to a freeimage memory buffer
			FreeImage_SaveToMemory(FIF_JPEG, fibmp, fmem);

			// Copy the freeimage memory buffer to omegalib bytearray
			BYTE* fmemdata = NULL;
			DWORD fmemsize = 0;
			FreeImage_AcquireMemory(fmem, &fmemdata, &fmemsize);
			ByteArray* encodedData = new ByteArray(fmemsize);
			encodedData->copyFrom(fmemdata, fmemsize);

			// Release resources
			FreeImage_CloseMemory(fmem);
			FreeImage_Unload(fibmp);

			data->unmap();

			return encodedData;
		}

	}
}
