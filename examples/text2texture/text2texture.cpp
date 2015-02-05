/******************************************************************************
* THE OMEGA LIB PROJECT
*-----------------------------------------------------------------------------
* Copyright 2010-2015		Electronic Visualization Laboratory,
*							University of Illinois at Chicago
* Authors:
*  Alessandro Febretti		febret@gmail.com
*-----------------------------------------------------------------------------
* Copyright (c) 2010-2015, Electronic Visualization Laboratory,
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
*	An example of fast text rendering using a texture buffer.
******************************************************************************/
#include <omega.h>
#include <omegaGl.h>
#include "StringTextureSource.h"

using namespace omega;

class T2TApplication;

///////////////////////////////////////////////////////////////////////////////
class T2TRenderPass : public RenderPass
{
public:
    T2TRenderPass(Renderer* client, T2TApplication* app) :
        myApplication(app),
        RenderPass(client, "T2TRenderPass") 
    {
        myDirectDrawTime = Stat::create("direct draw", StatsManager::Time);
        myDirectDrawTime->setMask(1);
        myDirectDrawTime->setColor(Color(0.4f, 0.2f, 0.2f));

        myTexturedDrawTime = Stat::create("textured draw", StatsManager::Time);
        myTexturedDrawTime->setMask(1);
        myTexturedDrawTime->setColor(Color(0.2f, 0.4f, 0.2f));
    }

    virtual void render(Renderer* client, const DrawContext& context);

private:
    T2TApplication* myApplication;

    Ref<Font> myFont;

    // Timers to measure the performance of direct and textured text drawing.
    Ref<Stat> myDirectDrawTime;
    Ref<Stat> myTexturedDrawTime;
};

///////////////////////////////////////////////////////////////////////////////
class T2TApplication : public EngineModule
{
    friend class T2TRenderPass;
public:
    T2TApplication() :
        EngineModule("T2TApplication"),
        fontStyle("fonts/arial.ttf 10")
    {
    }

    virtual void initialize()
    {
        text = "Hello World From Omegalib Hello World From Omegalib Hello World From Omegalib Hello World From Omegalib Hello World From Omegalib Hello World From Omegalib Hello World From Omegalib";
        getEngine()->getConsole()->setDrawFlags(Console::DrawStats);

        stringTextureSource = new StringTextureSource();
        stringTextureSource->setText(text);
        stringTextureSource->setFontStyle(fontStyle);
        stringTextureSource->setFontColor(Color::Lime);

        SystemManager::instance()->getStatsManager()->setStatMask(1);
    }

    virtual void initializeRenderer(Renderer* r) 
    { 
        RenderPass* rp = new T2TRenderPass(r, this);
		r->addRenderPass(rp);
	}

private:
    Ref<StringTextureSource> stringTextureSource;
    String text;
    String fontStyle;
};

///////////////////////////////////////////////////////////////////////////////
void T2TRenderPass::render(Renderer* client, const DrawContext& context)
{
    DrawInterface* di = client->getRenderer();

    if(myFont == NULL) myFont = di->getFont(myApplication->fontStyle);

    if(context.task == DrawContext::OverlayDrawTask)
    {
        DisplayConfig& dcfg = client->getDisplaySystem()->getDisplayConfig();
        di->beginDraw2D(context);

        // Render textured text
        myTexturedDrawTime->startTiming();
        glEnable(GL_TEXTURE_2D);
        glColor4f(1, 1, 1, 1);
        Texture* tx = myApplication->stringTextureSource->getTexture(context);
        di->drawRectTexture(tx, Vector2f(5, 320), Vector2f(tx->getWidth(), tx->getHeight()));
        myTexturedDrawTime->stopTiming();

        // Render direct text
        myDirectDrawTime->startTiming();
        di->drawText(myApplication->text, myFont, Vector2f(5, 300), Font::VATop | Font::HALeft, Color::Red);
        myDirectDrawTime->stopTiming();

        di->endDraw();
    }
}

///////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    Application<T2TApplication> app("text2texture");
    return omain(app, argc, argv);
}
