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
 * What's in this file
 *	Implements a module drawing an on-screen console and stats display.
 ******************************************************************************/
#include "omega/Console.h"
#include "omega/Font.h"
#include "omega/SystemManager.h"
#include "omega/DisplaySystem.h"
#include "omega/glheaders.h"
#include "omega/StatsManager.h"

using namespace omega;


///////////////////////////////////////////////////////////////////////////////
Console* Console::createAndInitialize()
{
    Console* c = new Console();
    ModuleServices::addModule(c);
    return c;
}

///////////////////////////////////////////////////////////////////////////////
Console::Console():
    EngineModule("console"),
    myLines(16),
    myBackgroundColor(Color(0, 0, 0, 0.6f)),
    myDrawFlags(DrawNone)
{
    setPriority(EngineModule::PriorityLowest);

    myConsoleColors['!'] = Color(0.8f, 0.8f, 0.1f);
    myConsoleColors['*'] = Color(1.0f, 0.2f, 0.1f);
    myConsoleColors['^'] = Color(0.8f, 0.8f, 0.8f);
    myConsoleColors['&'] = Color(0.2f, 1.0f, 0.2f);
    myConsoleColors['@'] = Color(0.8f, 0.7f, 1.0f);

    myHeadline = SystemManager::instance()->getApplication()->getName();

    //omsg("Console: adding log listener");
    ologaddlistener(this);
}

///////////////////////////////////////////////////////////////////////////////
Console::~Console()
{
    //omsg("~Console: removing log listener");
    ologremlistener(this);
}

///////////////////////////////////////////////////////////////////////////////
void Console::initializeRenderer(Renderer* renderer)
{
    renderer->addRenderPass(new ConsoleRenderPass(renderer, this));
}

///////////////////////////////////////////////////////////////////////////////
void Console::initialize() 
{
    Config* syscfg = SystemManager::instance()->getSystemConfig();

    if(syscfg->exists("config/console/lines"))
    {
        Setting& linesSetting = syscfg->lookup("config/console");
        setNumLines(linesSetting["lines"]);
    }
    if(syscfg->exists("config/console/font"))
    {
        Setting& fontSetting = syscfg->lookup("config/console/font");
        setFont(FontInfo("console", fontSetting["filename"], fontSetting["size"]));
    }
    else
    {
        setFont(FontInfo("console", "fonts/arial.ttf", 12));
    }
}

///////////////////////////////////////////////////////////////////////////////
void Console::handleEvent(const Event& evt) 
{
    if( evt.getServiceType() == Service::Keyboard )
    {
        // Tab = toggle on-screen console.
        if(evt.getSourceId() == 259 && evt.getType() == Event::Down) 
        {
            if(myDrawFlags == DrawNone) myDrawFlags = DrawLog;
            else myDrawFlags = DrawNone;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
bool Console::handleCommand(const String& cmd) 
{ 
    Vector<String> args = StringUtils::split(cmd);
    if(args[0] == "?"  && args.size() == 1)
    {
        omsg("Console");
        omsg("\t c [ls]      - toggle console. l toggles log, s toggles stats.");
    }
    else if(args[0] == "c")
    {
        if(args.size() == 2)
        {
            if(args[1].find('l') != -1)
            {
                if(myDrawFlags & DrawLog) myDrawFlags &= ~DrawLog;
                else myDrawFlags |= DrawLog;
            }
            if(args[1].find('s') != -1)
            {
                if(myDrawFlags == DrawStats) myDrawFlags &= ~DrawStats;
                else myDrawFlags |= DrawStats;
            }
        }
        else
        {
            if(myDrawFlags == DrawNone) myDrawFlags = DrawLog;
            else myDrawFlags = DrawNone;
        }
        return true;
    }
    return false;
}

///////////////////////////////////////////////////////////////////////////////
void Console::addLine(const String& line)
{
    myLock.lock();
    if(line[0] == '|')
    {
        myHeadline = line.substr(1);
    }
    else
    {
        myLineBuffer.push_back(line);
        while(myLineBuffer.size() > myLines)
        {
            myLineBuffer.pop_front();
        }
    }
    myLock.unlock();
}

///////////////////////////////////////////////////////////////////////////////
// The 1000 priority is to make sure the console gets rendered after everything
// else.
ConsoleRenderPass::ConsoleRenderPass(Renderer* renderer, Console* owner): 
    RenderPass(renderer, "console", 1000),
    myOwner(owner), myFont(NULL)
{}

///////////////////////////////////////////////////////////////////////////////
void ConsoleRenderPass::render(Renderer* renderer, const DrawContext& context)
{
    if(context.task == DrawContext::OverlayDrawTask && 
        context.eye == DrawContext::EyeCyclop &&
        myOwner->getDrawFlags() != Console::DrawNone)
    {
        DrawInterface* di = getClient()->getRenderer();
        di->beginDraw2D(context);
        glPushAttrib(GL_ALL_ATTRIB_BITS);

        // We assume the transforms and viewport have already been set correctly.
        FontInfo& fi = myOwner->myFont;
        if(myFont == NULL)
        {
            myFont = di->createFont(fi.name, fi.filename, fi.size);
        }

        float x = 0; 
        float y = 0;
        float lineHeight = fi.size + 4;
        float lineWidth = fi.size * 100; //SystemManager::instance()->getDisplaySystem()->getCanvasSize().x(); 

        if(myOwner->getDrawFlags() & Console::DrawLog)
        {
            drawLog(
                Vector2f(x, y), 
                Vector2f(lineWidth, lineHeight * myOwner->getNumLines()), 
                context);
        }

        if(myOwner->getDrawFlags() & Console::DrawStats)
        {
            if(myOwner->getDrawFlags() & Console::DrawLog) y += lineHeight * myOwner->getNumLines() + 10;
            drawStats(Vector2f(x, y), Vector2f(lineWidth, 100), context);
        }

        glPopAttrib();
        di->endDraw();
    }
}

///////////////////////////////////////////////////////////////////////////////
void ConsoleRenderPass::drawLog(Vector2f pos, Vector2f size, const DrawContext& context)
{
    FontInfo& fi = myOwner->myFont;
    float x = 0; 
    float y = 0;
    float lineHeight = fi.size + 4;
    float lineWidth = fi.size * 100; //SystemManager::instance()->getDisplaySystem()->getCanvasSize().x(); 

    const DisplayTileConfig* tile = context.tile;
    float cx = tile->offset.x();
    float cy = tile->offset.y();
    
    DrawInterface* di = getClient()->getRenderer();
    di->drawRect(
        Vector2f(cx, cy), Vector2f(lineWidth, lineHeight * (myOwner->myLines + 1)), myOwner->myBackgroundColor);

    if(myFont != NULL)
    {
        di->drawRectOutline(Vector2f(cx - 1, cy), Vector2f(lineWidth + 2, lineHeight - 2), Color::Gray);
        di->drawText(myOwner->myHeadline, myFont, Vector2f(cx + x + 2, cy + y + 2), Font::HALeft | Font::VATop, Color(1.0f, 0.9f, 1.0f, 1.0f));
        y += lineHeight;

        foreach(String& s, myOwner->myLineBuffer)
        {
            if(myOwner->myConsoleColors.find(s[0]) != myOwner->myConsoleColors.end())
            {
                di->drawText(s.substr(1), myFont, Vector2f(cx + x + 2, cy + y + 2), Font::HALeft | Font::VATop, myOwner->myConsoleColors[s[0]]);
            }
            else
            {
                di->drawText(s, myFont, Vector2f(cx + x + 2, cy + y + 2), Font::HALeft | Font::VATop, Color(1, 1, 0, 1));
            }
            y += lineHeight;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void ConsoleRenderPass::drawStats(Vector2f pos, Vector2f size, const DrawContext& context)
{
    StatsManager* sm = SystemManager::instance()->getStatsManager();
    DrawInterface* di = getClient()->getRenderer();

    const DisplayTileConfig* tile = context.tile;
    float cx = tile->offset.x();
    float cy = tile->offset.y();
    pos += Vector2f(cx, cy);

    di->drawRect(pos, size, Color(0,0,0,0.8f));

    pos[1] += 10;
    
    foreach(Stat* s, sm->getStats())
    {
        if(s->getType() == StatsManager::Time && s->isValid())
        {
            di->drawRect(
                pos + Vector2f(5, 0),
                Vector2f(s->getCur(), 16),
                Color(0.6f, 0.1f, 0.1f));

            di->drawText(s->getName(), 
                myFont, 
                pos + Vector2f(5, 0), 
                Font::HALeft | Font::VAMiddle, Color::White);
            
            pos += Vector2f(0, 20);
        }
    }
    di->drawText("Enabled Cameras:", 
        myFont, 
        pos + Vector2f(5, 0), 
        Font::HALeft | Font::VAMiddle, Color::White);
    pos += Vector2f(0, 20);

    // Print enabled cameras
    Engine* e = getClient()->getEngine();
    foreach(Camera* cam, e->getCameras())
    {
        bool enabledScene = cam->isEnabledInContext(DrawContext::SceneDrawTask, context.tile);
        bool enabledOverlay = cam->isEnabledInContext(DrawContext::OverlayDrawTask, context.tile);
        if(enabledScene || enabledOverlay)
        {
            String ec = "    ";
            ec.append(cam->getName() + "(");
            if(enabledScene) ec.append("Scene");
            if(enabledOverlay) ec.append("Overlay");
            ec.append(") ");
            di->drawText(ec, 
                myFont, 
                pos + Vector2f(5, 0), 
                Font::HALeft | Font::VAMiddle, Color::White);
            pos += Vector2f(0, 20);
        }
    }
    Camera* cam = e->getDefaultCamera();
    bool enabledScene = cam->isEnabledInContext(DrawContext::SceneDrawTask, context.tile);
    bool enabledOverlay = cam->isEnabledInContext(DrawContext::OverlayDrawTask, context.tile);
    if(enabledScene || enabledOverlay)
    {
        String ec = "    ";
        ec.append(cam->getName() + "(");
        if(enabledScene) ec.append("Scene");
        if(enabledOverlay) ec.append("Overlay");
        ec.append(") ");
        di->drawText(ec, 
            myFont, 
            pos + Vector2f(5, 0), 
            Font::HALeft | Font::VAMiddle, Color::White);
    }
}
