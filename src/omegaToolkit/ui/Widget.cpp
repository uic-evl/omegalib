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
 *	Widget is the base class of the user interface utility toolkit.
 ******************************************************************************/
#include "omegaToolkit/UiModule.h"
#include "omegaToolkit/ui/Widget.h"
#include "omegaToolkit/ui/Container.h"
//#include "omegaToolkit/ui/DefaultSkin.h"
#include "omega/DrawInterface.h"
#include "omega/EventSharingModule.h"
#include "omegaToolkit/UiScriptCommand.h"

#include "omega/glheaders.h"

#include <float.h>

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

NameGenerator Widget::mysNameGenerator("Widget_");

// Table of widgets by Id (used by getSource)
ui::Widget* Widget::mysWidgets[Widget::MaxWidgets];

///////////////////////////////////////////////////////////////////////////////
Widget::Widget(Engine* server):
    myLayer(Middle),
    myStereo(false),
    myInitialized(false),
    myServer(server),
    myEventHandler(NULL),
    myContainer(NULL),
    myVisible(true),
    myEnabled(true),
    myNavigationEnabled(true),
    myDebugModeColor(255, 0, 255),
    myDebugModeEnabled(false),
    myAutosize(false),
    myRotation(0),
    myMaximumSize(FLT_MAX, FLT_MAX),
    myMinimumSize(0, 0),
    myActive(false),
    myPointerInside(false),
    myHorizontalNextWidget(NULL),
    myHorizontalPrevWidget(NULL),
    myVerticalNextWidget(NULL),
    myVerticalPrevWidget(NULL),
    mySize(Vector2f::Zero()),
    myPosition(Vector2f::Zero()),
    myBlendMode(BlendInherit),
    myAlpha(1.0f),
    myScale(1.0f),
    myUserData(NULL),
    myDraggable(false),
    myDragging(false),
    myPinned(false),
    myShaderEnabled(true),
    mySizeAnchorEnabled(false),
    mySizeAnchor(Vector2f::Zero())
{
    myId = mysNameGenerator.getNext();
    myName = mysNameGenerator.generate();

    myFillEnabled = false;
    memset(myBorders, 0, sizeof(BorderStyle) * 4);

    mysWidgets[myId] = this;

    // Set the default shader.
    setShaderName("ui/widget-base");
}

///////////////////////////////////////////////////////////////////////////////
Widget::~Widget()
{
    if(myInitialized)
    {
        dispose();
    }
    //ofmsg("~Widget %1%", %myName);
    mysWidgets[myId] = NULL;
}

///////////////////////////////////////////////////////////////////////////////
WidgetFactory* Widget::getFactory()
{
    return UiModule::instance()->getWidgetFactory();
}

///////////////////////////////////////////////////////////////////////////////
float Widget::getAlpha() 
{ 
    // Alpha is modulated by alpha from container of this widget.
    if(myContainer == NULL) return myAlpha;
    return myAlpha * myContainer->getAlpha(); 
}

///////////////////////////////////////////////////////////////////////////////
void Widget::setCenter(const omega::Vector2f& value)
{
    setPosition(value - mySize / 2);
}

///////////////////////////////////////////////////////////////////////////////
Vector2f Widget::getCenter()
{
    return myPosition + mySize / 2;
}

///////////////////////////////////////////////////////////////////////////////
void Widget::setUIEventCommand(const String& command)
{
    if(myUiEventCommand == NULL) myUiEventCommand = new UiScriptCommand();
    myUiEventCommand->setCommand(command);
    myEventHandler = myUiEventCommand;
}

///////////////////////////////////////////////////////////////////////////////
bool Widget::isGamepadInteractionEnabled()
{
    return UiModule::instance()->getGamepadInteractionEnabled();
}

///////////////////////////////////////////////////////////////////////////////
bool Widget::isPointerInteractionEnabled()
{
    return UiModule::instance()->getPointerInteractionEnabled();
}

///////////////////////////////////////////////////////////////////////////////
void Widget::update(const omega::UpdateContext& context) 
{
    if(!myInitialized)
    {
        //ofmsg("Initializing Widget: %1%", %getName());
        initialize(myServer);
        myInitialized = true;
        // Force a refresh, to make sure all renderables end up initialized
        // correctly.
        refresh();
    }
    if(myUpdateCommand.size() > 0)
    {
        PythonInterpreter* interp = SystemManager::instance()->getScriptInterpreter();
        interp->eval(myUpdateCommand);
    }
}

///////////////////////////////////////////////////////////////////////////////
void Widget::handleEvent(const Event& evt) 
{
    UiModule* ui = UiModule::instance();
    if(isGamepadInteractionEnabled())
    {
        if(myActive)
        {
            if(evt.isButtonDown(Event::ButtonDown))
            {
                evt.setProcessed();
                if(myVerticalNextWidget != NULL)
                {
                    ui->activateWidget(myVerticalNextWidget);
                    playMenuScrollSound();
                }
            }
            else if(evt.isButtonDown(Event::ButtonUp))
            {
                evt.setProcessed();
                if(myVerticalPrevWidget != NULL)
                {
                    ui->activateWidget(myVerticalPrevWidget);
                    playMenuScrollSound();
                }
            }
            else if(evt.isButtonDown(Event::ButtonLeft))
            {
                evt.setProcessed();
                if(myHorizontalPrevWidget != NULL)
                {
                    ui->activateWidget(myHorizontalPrevWidget);
                    playMenuScrollSound();
                }
            }
            else if(evt.isButtonDown(Event::ButtonRight))
            {
                evt.setProcessed();
                if(myHorizontalNextWidget != NULL)
                {
                    ui->activateWidget(myHorizontalNextWidget);
                    playMenuScrollSound();
                }
            }
            // NOTE: This change was introduced on Nov 26th 2012
            // https://code.google.com/p/omegalib/source/detail?r=2334&path=/trunk/src/omegaToolkit/ui/Widget.cpp
            // To support event dispatch to non-interactive widgets like Images.
            // This breaks cluster event distribution after event sharing is moved
            // after local event handling in ConfigImpl, lines 259-265
            // dispathUIEvent marks events as local, which prevents events from
            // broadcasting.
            // If we want to re-enable the following block of code, we need to 
            // do it on an opt-in basis (only for widgets that want it), and we
            // probably need to copy the event to a separate one, so the original
            // does not get makred as local and still gets distributed.
            // -----------------------------------------------------------------
            // If gamepad interaction is enabled and event is a down or up event, 
            // (and not one of the navigation buttons, dispatch it to possible listeners)
            // NOTE: we do not dispatch Move or Update events here to avoid clogging the listeners with events they do not care about
            // In the future, we may add an option to selectively enable dispatch of those events.
            //else if(evt.getType() == Event::Down || evt.getType() == Event::Up)
            //{
            //	dispatchUIEvent(evt);
            //}
            // -----------------------------------------------------------------
        }
    }

    if(isPointerInteractionEnabled())
    {
        Vector2f pos2d = Vector2f(evt.getPosition().x(), evt.getPosition().y());
        // NOTE: Drag move and end does not depend on the pointer actually being on
        // the controller
        if(myDraggable)
        {
            if(evt.getType() == Event::Move && myDragging)
            {
                Vector2f delta = pos2d - myUserMovePosition;
                myUserMovePosition = pos2d;

                setPosition(myPosition + delta);
                evt.setProcessed();
            }
            else if(myDragging && evt.getType() == Event::Up)
            {
                myDragging = false;
                myActive = false;
                evt.setProcessed();
            }
        }
        if(simpleHitTest(transformPoint(pos2d)))
        {
            if(!evt.isProcessed())
            {
                myPointerInside = true;
                // Some kind of pointer event happened over this widget: make it active.
                if(!isActive() && evt.getType() == Event::Down)
                {
                    ui->activateWidget(this);
                }

                // If pointer interaction is enabled and event is a down or up event, dispatch it to possible listeners.
                // NOTE: we do not dispatch Move or Update events here to avoid clogging the listeners with events they do not care about
                // In the future, we may add an option to selectively enable dispatch of those events.
                if(evt.getType() == Event::Down || evt.getType() == Event::Up)
                {
                    // omsg("here");
                    dispatchUIEvent(evt);
                }

                if(myDraggable)
                {
                    if(evt.getType() == Event::Down)
                    {
                        myUserMovePosition = pos2d;
                        evt.setProcessed();
                        myDragging = true;
                        myActive = true;
                    }
                }
            }
        }
        else
        {
            myPointerInside = false;
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void Widget::clearSizeConstaints()
{
    myMaximumSize = Vector2f(FLT_MAX, FLT_MAX);
    myMinimumSize = Vector2f(0, 0);
}

///////////////////////////////////////////////////////////////////////////////
void Widget::setContainer(Container* value) 
{ 
    //oassert(value && value->getManager() == myManager);
    myContainer = value; 
}

///////////////////////////////////////////////////////////////////////////////
bool Widget::hitTest(const Vector2f& point)
{
    return simpleHitTest(transformPoint(point));
}

///////////////////////////////////////////////////////////////////////////////
bool Widget::simpleHitTest(const Vector2f& point)
{
    float x1 = 0; //myPosition[0];
    float y1 = 0; //myPosition[1];
    float x2 = mySize[0]; // myPosition[0] + mySize[0];
    float y2 = mySize[1]; //myPosition[1] + mySize[1];

    if(point[0] >= x1 && point[1] >= y1 && point[0] < x2 && point[1] < y2) return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////
bool Widget::simpleHitTest(const Vector2f& point, const Vector2f& pos, const Vector2f& size)
{
    float x1 = pos[0];
    float y1 = pos[1];
    float x2 = pos[0] + size[0];
    float y2 = pos[1] + size[1];

    if(point[0] >= x1 && point[1] >= y1 && point[0] < x2 && point[1] < y2) return true;
    return false;
}

///////////////////////////////////////////////////////////////////////////////
Vector2f Widget::transformPoint(const Vector2f& point)
{
    Vector2f res = point;
    if(myContainer)
    {
        res = myContainer->transformPoint(point);
    }
    // Adjust the point position (taking the widget scale into account)
    res -= (myPosition + mySize * (1 - myScale) * 0.5f);

    if(myRotation != 0)
    {
        Vector2f center = myPosition + (mySize / 2);
        res = point - center;
        float s = sin(-myRotation * omega::Math::DegToRad);
        float c = cos(-myRotation * omega::Math::DegToRad);
        float x = point[0];
        float y = point[1];
        res[0] = x * c - y * s;;
        res[1] = y * c + x * s;
        res += center;
    }
    return res / myScale;
}

///////////////////////////////////////////////////////////////////////////////
void Widget::dispatchUIEvent(const Event& evt)
{
    if(myEventHandler != NULL) 
    {
        // If the local events option is set, mark ui events as local before dispatching them to
        // the event listener. Local events do not get broadcast to other nodes in clustered systems.
        if(UiModule::instance()->isLocalEventsEnabled())
        {
            EventSharingModule::markLocal(evt);
        }
        myEventHandler->handleEvent(evt);
    }
    else if(myContainer != NULL) 
    {
            myContainer->dispatchUIEvent(evt);
    }
}

///////////////////////////////////////////////////////////////////////////////
void Widget::playMenuScrollSound()
{
    if(getEngine()->getSoundEnvironment() != NULL)
    {
        if(SystemManager::settingExists("config/sound"))
        {
            Sound* sound = getEngine()->getSoundEnvironment()->getSound("scrollMenuSound");
            if( sound != NULL )
            {
                SoundInstance* inst = new SoundInstance(sound);
                inst->setLocalPosition( getContainer()->get3dSettings().position );
                inst->play();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void Widget::requestLayoutRefresh() 
{ 
    myNeedLayoutRefresh = true; 
    if(myContainer != NULL) 
        myContainer->requestLayoutRefresh(); 
}

///////////////////////////////////////////////////////////////////////////////
bool Widget::needLayoutRefresh() 
{ 
    return myNeedLayoutRefresh; 
}

///////////////////////////////////////////////////////////////////////////////
void Widget::layout()
{ 
    myNeedLayoutRefresh = false; 
}

///////////////////////////////////////////////////////////////////////////////
void Widget::updateSize()
{
    if(needLayoutRefresh())
    {
        if(myAutosize) autosize();
    }
    if(mySizeAnchorEnabled)
    {
        if(myContainer != NULL)
        {
            if(mySizeAnchor[0] >= 0)
            {
                int aw = myContainer->getWidth() - mySizeAnchor[0] - myPosition[0] - (myContainer->getMargin() * 2);
                setActualSize(aw, Horizontal);
                requestLayoutRefresh();
            }
            if(mySizeAnchor[1] >= 0)
            {
                int ah = myContainer->getHeight() - mySizeAnchor[1] - myPosition[1] - (myContainer->getMargin() * 2);
                setActualSize(ah, Vertical);
                requestLayoutRefresh();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void Widget::setActualSize(int value, Orientation orientation, bool force) 
{
    if(mySize[orientation] != value)
    {
        //requestLayoutRefresh();
        if(!force)
        {
            if(value < myMinimumSize[orientation]) value = myMinimumSize[orientation];
            if(value > myMaximumSize[orientation]) value = myMaximumSize[orientation];
        }
        mySize[orientation] = value; 
    }
}

///////////////////////////////////////////////////////////////////////////////
void Widget::setNavigationEnabled(bool value)
{
    myNavigationEnabled = value;
    if(myContainer != NULL) myContainer->updateChildrenNavigation();
}

///////////////////////////////////////////////////////////////////////////////
int Widget::getId()
{
    return myId;
}

///////////////////////////////////////////////////////////////////////////////
Renderable* Widget::createRenderable()
{
    return new WidgetRenderable(this);
}

///////////////////////////////////////////////////////////////////////////////
void Widget::setStyle(const String& style)
{
    String nnstyle = StringUtils::replaceAll(style, "\n", "");
    Vector<String> tokens = StringUtils::split(nnstyle, ";");
    foreach(String token, tokens)
    {
        Vector<String> args = StringUtils::split(token, ":");
        if(args.size() < 2)
        {
            ofwarn("Widget::setStyle: error in style element %1%", %token);
        }
        else
        {
            setStyleValue(args[0], args[1]);
        }
    }
    updateStyle();
}

///////////////////////////////////////////////////////////////////////////////
String Widget::getStyleValue(const String& key, const String& defaultValue)
{
    Dictionary<String, String>::iterator it = myStyleDictionary.find(key);
    if(it == myStyleDictionary.end()) return defaultValue;
    return it->second;
}

///////////////////////////////////////////////////////////////////////////////
void Widget::setStyleValue(const String& key, const String& value)
{
    String pk = key;
    String pv = value;
    StringUtils::trim(pk);
    StringUtils::trim(pv);
    myStyleDictionary[pk] = pv;
    updateStyle();
}

///////////////////////////////////////////////////////////////////////////////
void Widget::BorderStyle::fromString(const String& str)
{
    Vector<String> args = StringUtils::split(str, " ");
    width = boost::lexical_cast<int>(args[0]);
    if(args.size() == 2) color = Color(args[1]);
}

///////////////////////////////////////////////////////////////////////////////
void Widget::updateStyle()
{
    String bkstyle = getStyleValue("fill");
    if(bkstyle != "")
    {
        myFillEnabled = true;
        myFillColor = Color(bkstyle);
    }
    else
    {
        myFillEnabled = false;
    }

    String bdstyle = getStyleValue("border");
    if(bdstyle != "")
    {
        myBorders[0].fromString(bdstyle);
        myBorders[1] = myBorders[0];
        myBorders[2] = myBorders[0];
        myBorders[3] = myBorders[0];
    }

    bdstyle = getStyleValue("border-top");
    if(bdstyle != "") myBorders[0].fromString(bdstyle);
    bdstyle = getStyleValue("border-right");
    if(bdstyle != "") myBorders[1].fromString(bdstyle);
    bdstyle = getStyleValue("border-bottom");
    if(bdstyle != "") myBorders[2].fromString(bdstyle);
    bdstyle = getStyleValue("border-left");
    if(bdstyle != "") myBorders[3].fromString(bdstyle);
}

///////////////////////////////////////////////////////////////////////////////
bool Widget::isIn3DContainer()
{
    return myContainer->isIn3DContainer();
}

///////////////////////////////////////////////////////////////////////////////
void WidgetRenderable::refresh()
{
    if(myOwner->myShaderEnabled)
    {
        String progName = myOwner->getShaderName();
        String vsName = progName + ".vert";
        String fsName = progName + ".frag";
        myShaderProgram = getRenderer()->getOrCreateProgram(progName, vsName, fsName);
        if(oglError)
        {
            myShaderProgram = 0;
            return;
        }
        if(myShaderProgram != 0)
        {
            myAlphaUniform = glGetUniformLocation(myShaderProgram, "unif_Alpha");
            if(oglError)
            {
                myShaderProgram = 0;
                return;
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void WidgetRenderable::preDraw()
{
    glPushMatrix();

    // Setup transformation.
    Vector2f center = myOwner->myPosition + (myOwner->mySize / 2);
    Vector2f mcenter = -center;

    float scale = myOwner->getScale();
    glTranslatef(center[0], center[1], 0.0f);
    glRotatef(myOwner->myRotation, 0, 0, 1);
    glScalef(scale, scale, scale);
    glTranslatef(mcenter[0], mcenter[1], 0.0f);

    glTranslatef((float)myOwner->myPosition[0], (float)myOwner->myPosition[1], 0);

    pushDrawAttributes();
}

///////////////////////////////////////////////////////////////////////////////
void WidgetRenderable::postDraw()
{
    // reset transform.
    glPopMatrix();
    popDrawAttributes();
}

///////////////////////////////////////////////////////////////////////////////
void WidgetRenderable::pushDrawAttributes()
{
    if(myShaderProgram != 0)
    {
        glUseProgram(myShaderProgram);
        glUniform1f(myAlphaUniform, myOwner->getAlpha());
    }
    // Set default color to white.
    glColor4ub(255,255,255,255);

    Widget::BlendMode bm = myOwner->getBlendMode();
    if(bm != Widget::BlendInherit)
    {
        glPushAttrib(GL_ENABLE_BIT);
        if(bm == Widget::BlendDisabled)
        {
            glDisable(GL_BLEND);
        }
        else
        {
            glEnable(GL_BLEND);
            if(bm == Widget::BlendAdditive)
            {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE);
            }
            else if(bm == Widget::BlendNormal)
            {
                glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void WidgetRenderable::popDrawAttributes()
{
    if(myShaderProgram != 0)
    {
        glUseProgram(0);
    }
    if(myOwner->getBlendMode() != Widget::BlendInherit)
    {
        glDisable(GL_BLEND);
        glPopAttrib();
    }
}

///////////////////////////////////////////////////////////////////////////////
void WidgetRenderable::draw(const DrawContext& context)
{
    if(myOwner->isVisible())
    {
        if(myOwner->isStereo())
        {
            if(context.eye != DrawContext::EyeCyclop) 
            {
                preDraw();
                drawContent(context);
                postDraw();
            }
        }
        else
        {
            if(context.eye == DrawContext::EyeCyclop) 
            {
                preDraw();
                drawContent(context);
                postDraw();
            }
        }
    }
}

///////////////////////////////////////////////////////////////////////////////
void WidgetRenderable::drawContent(const DrawContext& context)
{
    DrawInterface* di = getRenderer();
    if(myOwner->myFillEnabled)
    {
        di->drawRect(Vector2f::Zero(), myOwner->mySize, myOwner->myFillColor);
    }
    if(myOwner->myBorders[0].width != 0)
    {
        glLineWidth(myOwner->myBorders[0].width);
        glColor4fv(myOwner->myBorders[0].color.data());
        glBegin(GL_LINES);
        glVertex2f(0, 0); glVertex2f(myOwner->mySize[0], 0);
        glEnd();
    }
    if(myOwner->myBorders[1].width != 0)
    {
        glLineWidth(myOwner->myBorders[1].width);
        glColor4fv(myOwner->myBorders[1].color.data());
        glBegin(GL_LINES);
        glVertex2f(myOwner->mySize[0], 0); glVertex2f(myOwner->mySize[0], myOwner->mySize[1]);
        glEnd();
    }
    if(myOwner->myBorders[2].width != 0)
    {
        glLineWidth(myOwner->myBorders[2].width);
        glColor4fv(myOwner->myBorders[2].color.data());
        glBegin(GL_LINES);
        glVertex2f(myOwner->mySize[0], myOwner->mySize[1]); glVertex2f(0, myOwner->mySize[1]);
        glEnd();
    }
    if(myOwner->myBorders[3].width != 0)
    {
        glLineWidth(myOwner->myBorders[3].width);
        glColor4fv(myOwner->myBorders[3].color.data());
        glBegin(GL_LINES);
        glVertex2f(0, myOwner->mySize[1]); glVertex2f(0, 0);
        glEnd();
    }
    if(myOwner->myDebugModeEnabled)
    {
        di->drawRectOutline(Vector2f::Zero(), myOwner->mySize, myOwner->myDebugModeColor);
    }
}

