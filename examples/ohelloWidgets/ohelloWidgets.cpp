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
 *	An example using the omegaToolkit widget library to draw a variety of
 *	standard and custom widgets.
 ******************************************************************************/
#include <omega.h>
#include <omegaToolkit.h>
#include <omegaGl.h>

using namespace omega;
using namespace omegaToolkit;
using namespace omegaToolkit::ui;

///////////////////////////////////////////////////////////////////////////////
// This is an example of a custom widget. Widget is divided in two parts. The
// Actual widget class, and the widget renderable, which contains the user
// custom draw code for the widget. In this example, we implement the 
// renderable as a nested class for convenience, but this is not required.
class AnimatedPlot: public Widget
{
    class Renderable: public WidgetRenderable
    {
        class AnimatedPlot* myOwner;
    public:
        Renderable(AnimatedPlot* owner): 
          myOwner(owner), WidgetRenderable(owner)
        {}

        void drawContent(const DrawContext& context)
        {
            // Calling the base WidgetRenderable draw function is not
            // required but is useful to draw widget background and borders.
            WidgetRenderable::drawContent(context);

            int w = myOwner->getWidth();
            int h = myOwner->getHeight() / 2;

            glBegin(GL_LINE_STRIP);
            glColor4fv(myOwner->myColor.data());
            for(int x = 0; x < w; x++)
            {
                float value = sin(x * myOwner->myFrequency + 
                    myOwner->myPhase + myOwner->myTime * 5) * myOwner->myAmplitude;
                glVertex2f(x, value + h);
            }
            glEnd();
        }
    };

public:
    // Make the renderable a friend, so it can use our private members.
    friend class Renderable;

    virtual Renderable* createRenderable() 
    { return new Renderable(this); }

    // Convenience function for instantiating this widget (follows the
    // convention used by other widgets)
    static AnimatedPlot* create(Container* container)
    { 
        AnimatedPlot* ap = new AnimatedPlot(Engine::instance());
        container->addChild(ap);
        return ap;
    }

    AnimatedPlot(Engine* e): 
        Widget(e),
            myPhase(0),
            myFrequency(0.01f),
            myTime(0),
            myColor(Color::White),
            myAmplitude(10)
    {}

    void setPlotOptions(float phase, float amplitude, float frequency, const Color& color)
    {
        myPhase = phase;
        myAmplitude = amplitude;
        myFrequency = frequency;
        myColor = color;
    }

    // Use the global time to update the plot
    virtual void update(const UpdateContext& context)
    {
        myTime = context.time;
    }

private:
    // Plot parameters.
    float myPhase;
    float myAmplitude;
    float myFrequency;
    float myTime;
    Color myColor;
};


///////////////////////////////////////////////////////////////////////////////
// The following *Example classes are just a convenient way to group
// multiple related widgets together. The same code could go in the main
// WidgetApplication class.

///////////////////////////////////////////////////////////////////////////////
// Container with examples of different font sizes
class FontsExample
{
public:
    void initialize(Container* ui)
    {
        myFontsContainer = Container::create(Container::LayoutVertical, ui);
        myFontsContainer->setPosition(Vector2f(5, 20));
        //myFontsContainer->setStyle("border: 2 #ffffff; fill: #00000080");
        for(int sz = 5; sz <= 30; sz += 5)
        {
            Label* fnt = Label::create(myFontsContainer);
            fnt->setFont(ostr("system/fonts/arial.ttf %1%", %sz));
            fnt->setText(ostr("Font size: %1%", %sz));
        }
    }

private:
    Ref<Container> myFontsContainer;
};

///////////////////////////////////////////////////////////////////////////////
// Example of custom widget instantiation
class CustomWidgetExample
{
public:
    void initialize(Container* ui)
    {
        // We use a free layout, so we can overlay the three plots.
        myCustomWidgetContainer = Container::create(Container::LayoutFree, ui);
        myCustomWidgetContainer->setPosition(Vector2f(5, 300));
        myCustomWidgetContainer->setStyle("border: 2 #ffffff; fill: #00000080");
        myCustomWidgetContainer->setSize(Vector2f(400,140));
        myCustomWidgetContainer->setAutosize(false);
        myCustomWidgetContainer->setClippingEnabled(true);

        myPlot1 = AnimatedPlot::create(myCustomWidgetContainer);
        myPlot1->setPlotOptions(0, 40, 0.4f, Color::Red);
        myPlot1->setSize(Vector2f(400,140));

        myPlot2 = AnimatedPlot::create(myCustomWidgetContainer);
        myPlot2->setPlotOptions(7, 80, 0.3f, Color::Green);
        myPlot2->setSize(Vector2f(400,140));

        myPlot3 = AnimatedPlot::create(myCustomWidgetContainer);
        myPlot3->setPlotOptions(13, 120, 0.1f, Color::Blue);
        myPlot3->setSize(Vector2f(400,140));
    }

private:
    Ref<Container> myCustomWidgetContainer;
    Ref<AnimatedPlot> myPlot1;
    Ref<AnimatedPlot> myPlot2;
    Ref<AnimatedPlot> myPlot3;
};

///////////////////////////////////////////////////////////////////////////////
// Example on how to create a movable window.
class DragExample
{
public:
    void initialize(Container* ui)
    {
        myDraggableContainer = Container::create(Container::LayoutVertical, ui);
        myDraggableContainer->setPosition(Vector2f(400, 20));
        myDraggableContainer->setStyle("border: 2 #ffffff; fill: #00000080");
        myDraggableContainer->setSize(Vector2f(140,140));

        myDraggableBar = Container::create(Container::LayoutHorizontal, myDraggableContainer);
        myDraggableBar->setPosition(Vector2f(5, 5));
        myDraggableBar->setStyle("fill: #a0a0a0");
        myDraggableBar->setSize(Vector2f(140,20));
        myDraggableBar->setAutosize(false);

        Label* fnt = Label::create(myDraggableBar);
        fnt->setColor(Color::Black);
        fnt->setText("Drag Me!");

        Image* img = Image::create(myDraggableContainer);
        img->setData(ImageUtils::loadImage("docs/icon.png"));
        // Make the image smaller.
        img->setSize(Vector2f(128,128));

        myDraggableBar->setDraggable(true);
        myDraggableBar->setPinned(true);
    }

private:
    Ref<Container> myDraggableContainer;
    Ref<Container> myDraggableBar;
};

///////////////////////////////////////////////////////////////////////////////
// This example shows how to set a container layout. It also shows how to 
// capture events from buttons.
class LayoutExample: public IEventListener
{
public:
    void initialize(Container* ui)
    {
        myLayoutContainer = Container::create(Container::LayoutVertical, ui);
        myLayoutContainer->setPosition(Vector2f(200, 20));
        myLayoutContainer->setStyle("border: 2 #ffffff; fill: #00030080");

        myVLayoutButton = Button::create(myLayoutContainer);
        myVLayoutButton->getLabel()->setText("Vertical Layout");
        myVLayoutButton->setCheckable(true);
        myVLayoutButton->setChecked(true);
        myVLayoutButton->setRadio(true);
        myVLayoutButton->setUIEventHandler(this);

        myHLayoutButton = Button::create(myLayoutContainer);
        myHLayoutButton->getLabel()->setText("Horizontal Layout");
        myHLayoutButton->setCheckable(true);
        myHLayoutButton->setRadio(true);
        myHLayoutButton->setUIEventHandler(this);

        myFLayoutButton = Button::create(myLayoutContainer);
        myFLayoutButton->getLabel()->setText("Free Layout");
        myFLayoutButton->setCheckable(true);
        myFLayoutButton->setRadio(true);
        myFLayoutButton->setUIEventHandler(this);
    }

    void handleEvent(const Event& e)
    {
        // This function is called when one of the buttons has beel clicked.
        // Check which radio button is currently checked.
        if(myVLayoutButton->isChecked())
        {
            myLayoutContainer->setLayout(Container::LayoutVertical);
            myLayoutContainer->requestLayoutRefresh();
        }
        else if(myHLayoutButton->isChecked())
        {
            myLayoutContainer->setLayout(Container::LayoutHorizontal);
            myLayoutContainer->requestLayoutRefresh();
        }
        else if(myFLayoutButton->isChecked())
        {
            // For the free layout, let's change the button positions
            myLayoutContainer->setLayout(Container::LayoutFree);
            myVLayoutButton->setPosition(Vector2f(10, 10));
            myHLayoutButton->setPosition(Vector2f(30, 50));
            myFLayoutButton->setPosition(Vector2f(0, 80));
            myLayoutContainer->requestLayoutRefresh();
        }
    }

private:
    Ref<Container> myLayoutContainer;
    Ref<Button> myVLayoutButton;
    Ref<Button> myHLayoutButton;
    Ref<Button> myFLayoutButton;
};

///////////////////////////////////////////////////////////////////////////////
// This example shows how to control 2D and 3D widget transformations.
// It arranges a set of labels in circle, then attached the labels container
// To a scene node to spin it in 3D space.
class TransformExample
{
public:
    void initialize(Container* ui)
    {
        // Container with examples of transformation
        // We size this container manually to fit the rotated text.
        myXformContainer1 = Container::create(Container::LayoutFree, ui);
        myXformContainer1->setAutosize(false);
        myXformContainer1->setSize(Vector2f(400,400));
        myXformContainer2 = Container::create(Container::LayoutFree, ui);
        myXformContainer2->setAutosize(false);
        myXformContainer2->setSize(Vector2f(400,400));

        // Create a fan of rotated text
        for(int a = 0; a < 360; a += 30)
        {
            Label* l = Label::create(myXformContainer1);
            l->setText("Hello");
            l->setAutosize(false);
            l->setFont("system/fonts/arial.ttf 30");
            l->setPosition(Vector2f(0,200));
            l->setWidth(400);
            l->setHorizontalAlign(Label::AlignLeft);
            l->setRotation(a);

            Label* l2 = Label::create(myXformContainer2);
            l2->setText("Widgets");
            l2->setAutosize(false);
            l2->setFont("system/fonts/arial.ttf 30");
            l2->setPosition(Vector2f(0,200));
            l2->setWidth(400);
            l2->setHorizontalAlign(Label::AlignLeft);
            l2->setRotation(a);
        }

        // Turn the containers 3d surfaces and attach them to a node.
        myPivot1 = new SceneNode(Engine::instance());
        myPivot1->pitch(-0.6f);
        myPivot1->yaw(-0.6f);

        myPivot2 = new SceneNode(Engine::instance());
        myPivot2->setPosition(0, 2, -1.5f);
        myPivot2->pitch(-0.6f);
        myPivot2->yaw(-0.6f);
        myPivot2->addChild(myPivot1);

        Container3dSettings& c3ds1 = myXformContainer1->get3dSettings();
        c3ds1.enable3d = true;
        c3ds1.node = myPivot1;
        c3ds1.center = true; 
        c3ds1.alpha = 0.6f;

        Container3dSettings& c3ds2 = myXformContainer2->get3dSettings();
        c3ds2.enable3d = true;
        c3ds2.node = myPivot2;
        c3ds2.center = true; 
        c3ds2.alpha = 0.6f;
    }

    void update(const UpdateContext& ctx)
    {
        myPivot1->roll(ctx.dt / 4);
        myPivot2->roll(ctx.dt / 4);
    }

private:
    Ref<SceneNode> myPivot1;
    Ref<SceneNode> myPivot2;

    Ref<Container> myXformContainer1;
    Ref<Container> myXformContainer2;
};

///////////////////////////////////////////////////////////////////////////////
class WidgetApplication: public EngineModule
{
public:
    WidgetApplication(): EngineModule("WidgetApplication") {}

    virtual void initialize()
    {
        Camera* c = getEngine()->getDefaultCamera();
        c->setBackgroundColor(Color(0.1f, 0.1f, 0.15f));

        // Create and initialize the UI management module.
        myUiModule = UiModule::createAndInitialize();
        myUi = myUiModule->getUi();

        myMainLabel = Label::create(myUi);
        myMainLabel->setText("Hello Widgets!!");

        myFontsExample.initialize(myUi);
        myLayoutExample.initialize(myUi);
        myTransformExample.initialize(myUi);
        myCustomWidgetExample.initialize(myUi);
        myDragExample.initialize(myUi);
    }

    virtual void update(const UpdateContext& context)
    {
        // Call the update function of examples that have animations
        myTransformExample.update(context);
    }

private:
    // The ui manager
    Ref<UiModule> myUiModule;
    // The root ui container
    Ref<Container> myUi;

    // Widgets
    Ref<Label> myMainLabel;
    FontsExample myFontsExample;
    LayoutExample myLayoutExample;
    TransformExample myTransformExample;
    DragExample myDragExample;
    CustomWidgetExample myCustomWidgetExample;
};

///////////////////////////////////////////////////////////////////////////////
// ApplicationBase entry point
int main(int argc, char** argv)
{
    Application<WidgetApplication> app("ohelloWidgets");
    return omain(app, argc, argv);
}
