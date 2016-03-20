#include <omegaGl.h>
#include "omegaToolkit/CameraStreamer.h"

using namespace omegaToolkit;


typedef IEncoder* (*EncoderCreateFunc)();

class EncoderFactory: public ReferenceType
{
public:
  Ref<Library> library;
  EncoderCreateFunc create;
};

Dictionary<String, Ref<EncoderFactory> > sRegisteredEncoders;

///////////////////////////////////////////////////////////////////////////////
class ImageSequenceEncoder : public IEncoder
{
public:
    ImageSequenceEncoder(ImageUtils::ImageFormat format) : 
        myFormat(format)
    {
        olog(Verbose, "[createEncoder] creating ImageSequenceEncoder");
    }

    virtual RenderTarget::Type getRenderTargetType() { return RenderTarget::RenderOffscreen; }

    bool initialize() { return true; }
    void shutdown() {  }
    bool configure(int width, int height, int fps, int quality) { return true; }

    bool encodeFrame(RenderTarget* rt)
    {
        rt->readback();
        PixelData* pixels = rt->getOffscreenColorTarget();
        if(pixels != NULL)
        {
            myData = ImageUtils::encode(pixels, myFormat);
            //ImageUtils::saveImage("out.jpg", pixels, myFormat);
            return true;
        }
        return false;
    }

    bool dataAvailable() 
    {
        return !myData.isNull();
    }

    bool lockBitstream(const void** stptr, uint32_t* bytes)
    {
        if(!myData.isNull())
        {
            *bytes = (uint32_t)myData->getSize();
            *stptr = myData->getData();
            return true;
        }
        return false;
    }

    void unlockBitstream() 
    {
        myData = NULL;
    }

private:
    ImageUtils::ImageFormat myFormat;
    Ref<ByteArray> myData;
};

///////////////////////////////////////////////////////////////////////////////
IEncoder* sCreateEncoder(const String& name)
{
    if(name == "jpeg") return new ImageSequenceEncoder(ImageUtils::FormatJpeg);
    if(name == "png") return new ImageSequenceEncoder(ImageUtils::FormatPng);

    // If this encoder type has not been registered yet, try to load it now
    if(sRegisteredEncoders.find(name) == sRegisteredEncoders.end())
    {
        oflog(Verbose, "[createEncoder] finding encoder <%1%>", %name);
        
#ifdef OMEGA_OS_WIN
        String libname = name + ".dll";
#else
        String libname = "lib"; libname = libname + name + ".so";
#endif
        String libPath;
        if(!DataManager::findFile(libname, libPath))
        {
            ofwarn("[createEncoder] could not find encoder library %1%", %libname);
            return NULL;
        }
        oflog(Verbose, "[createEncoder] loading encoder library <%1%>", %libPath);
        Ref<EncoderFactory> ef = new EncoderFactory();
        ef->library = new Library();
        if(!ef->library->open(libPath)) return NULL;
        
        ef->create = (EncoderCreateFunc)ef->library->getFunctionPointer("createEncoder");
        if(ef->create == NULL)
        {
            ofwarn("[createEncoder] could not find entry point createEncoder in library %1%", %libPath);
            return NULL;
        }
        
        sRegisteredEncoders[name] = ef;
    }
    
    oflog(Verbose, "[createEncoder] returning encoder <%1%>", %name);
    return sRegisteredEncoders[name]->create();
}

///////////////////////////////////////////////////////////////////////////////
CameraStreamer::CameraStreamer(const String& encoderName):
    myEncoder(NULL),
    myTargetFps(20),
    myEncoderName(encoderName)
{
    myTimer.start();
    myLastFrameTime = 0;
}

///////////////////////////////////////////////////////////////////////////////
CameraStreamer::~CameraStreamer()
{
    if(myEncoder != NULL)
    {
        myEncoder->shutdown();
        delete myEncoder;
        myEncoder = NULL;
    }
}

///////////////////////////////////////////////////////////////////////////////
IEncoder* CameraStreamer::lockEncoder() 
{ 
    myEncoderLock.lock();
    return myEncoder; 
}

///////////////////////////////////////////////////////////////////////////////
void CameraStreamer::unlockEncoder()
{
    myEncoderLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void CameraStreamer::initialize(Camera* c, const DrawContext& context)
{
    myEncoderLock.lock();
    Renderer* r = context.renderer;
    Vector2i size = myResolution;
    
    oflog(Verbose, "[CameraStreamer::initialize] tile pixel size: <%1%>", %size);

    IEncoder* e = sCreateEncoder(myEncoderName);

    RenderTarget::Type rtt = e->getRenderTargetType();
    myRenderTarget = r->createRenderTarget(rtt);
    if(rtt == RenderTarget::RenderToTexture)
    {
        myRenderTexture = r->createTexture();
        myRenderTexture->initialize(size[0], size[1], Texture::TypeRectangle);
        myDepthTexture = r->createTexture();
        myDepthTexture->initialize(size[0], size[1], Texture::TypeRectangle, Texture::ChannelDepth, Texture::FormatFloat);
        myRenderTarget->setTextureTarget(myRenderTexture, myDepthTexture);
    }
    else
    {
        myPixels = new PixelData(PixelData::FormatRgb, size[0], size[1]);
        myRenderTarget->setReadbackTarget(myPixels);
    }

    if(!e->initialize())
    {
        owarn("[CameraStreamer::initialize] encoder initialization failed");
    }
    else

    {
        e->configure(size[0], size[1]);
        myEncoder = e;
    }
    myEncoderLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void CameraStreamer::reset(Camera* c, const DrawContext& context)
{
    myEncoderLock.lock();
    
    Renderer* r = context.renderer;
    Vector2i size = myResolution;
    
    oflog(Verbose, "[CameraStreamer::reset] tile pixel size: <%1%>", %size);
    if(myEncoder->getRenderTargetType() == RenderTarget::RenderToTexture)
    {
        myRenderTexture->resize(size[0], size[1]);
        myDepthTexture->resize(size[0], size[1]);
    }
    else
    {
        myPixels->resize(size[0], size[1]);
    }
    
    myEncoder->configure(size[0], size[1]);
    
    myEncoderLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
void CameraStreamer::beginDraw(Camera* cam, DrawContext& context)
{
    if(myRenderTarget == NULL) initialize(cam, context);
    
    // If the output tile size changed, reset the encoder.
    if(myResolution[0] != myRenderTarget->getWidth() ||
        myResolution[1] != myRenderTarget->getHeight())
    {
        reset(cam, context);
    }

    myRenderTarget->bind();
}

///////////////////////////////////////////////////////////////////////////////
void CameraStreamer::endDraw(Camera* cam, DrawContext& context)
{
    myRenderTarget->unbind();
}

///////////////////////////////////////////////////////////////////////////////
void CameraStreamer::startFrame(Camera* cam, const FrameInfo& frame)
{
    if(myRenderTarget != NULL) myRenderTarget->clear();
}

///////////////////////////////////////////////////////////////////////////////
void CameraStreamer::finishFrame(Camera* cam, const FrameInfo& frame)
{
    if(myEncoder != NULL)
    {
        // If enough time has passed since the last frime, send a new one
        // to the encoder.
        if(myTimer.getElapsedTimeInMilliSec() - myLastFrameTime > (1000.0f / myTargetFps))
        {
            myEncoder->encodeFrame(myRenderTarget);
            myLastFrameTime = myTimer.getElapsedTimeInMilliSec();
        }
    }
}
