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
 *	A container of widgets that supports automatic layout
 ******************************************************************************/
#ifndef __CONTAINER_H__
#define __CONTAINER_H__

#include "omegaToolkit/ui/Widget.h"
#include "omegaToolkit/UiRenderPass.h"

namespace omegaToolkit { namespace ui {

    ////////////////////////////////////////////////////////////////////////////
    struct Container3dSettings
    {
        Container3dSettings():
            enable3d(false),
            center(false),
            node(NULL),
            position(Vector3f::Zero()),
            normal(Vector3f::UnitZ()), 
            up(Vector3f::UnitY()),
            // The default scale is based on common display dot pitch, so that 3d uis drawn on 
            // display plane will be about the same size of corresponding 2d ui.
            scale(0.002f), alpha(1.0) {}

        bool enable3d;
        Vector3f position;
        Vector3f normal;
        Vector3f up;

        float alpha;
        bool center;

        //! The 3d scale is the conversion factor between pixel sizes and world units.
        //! For example, a 100x300 pixel container with a scale of 0.01 will be drawn as a 
        // billboard 1x3 meters big in 3d mode.
        float scale;

        //! node is used to make transforms relative to a scene node.
        SceneNode* node;
    };

    ////////////////////////////////////////////////////////////////////////////
    class OTK_API Container: public Widget
    {
    //friend class Engine;
    friend class ContainerRenderable;
    friend class UiRenderPass;
    public:
        enum Layout {LayoutFree, LayoutHorizontal, LayoutVertical, LayoutGridHorizontal, LayoutGridVertical};
        enum HorizontalAlign { AlignRight, AlignLeft, AlignCenter};
        enum VerticalAlign { AlignTop, AlignMiddle, AlignBottom};

        static Container* create(Layout layout, Container* parent);

    public:
        Container(Engine* server);
        virtual ~Container();

        virtual Renderable* createRenderable();

        virtual void handleEvent(const omega::Event& evt);
        virtual void update(const omega::UpdateContext& context);

        void load(Setting& setting);

        //! Child management
        //@{
        //! Add a child to this widget.
        void addChild(Widget* child);
        //! Remove a child from this widget.
        void removeChild(Widget* child);
        //! Return the number of children
        int getNumChildren();
        //! Find a child by its name
        Widget* getChildByName(const String& name);
        Widget* getChildByIndex(int index);
        Widget* getChildBefore(const Widget* w);
        Widget* getChildAfter(const Widget* w);
        //@}

        //!Layout options
        //@{
        void setLayout(Layout layout);
        Layout getLayout();
        //! Sets the margin between the container's content and its borders.
        void setPadding(float value);
        float getPadding();
        //! Sets the padding space between elements within the container.
        void setMargin(float value);
        float getMargin();
        HorizontalAlign getHorizontalAlign();
        void setHorizontalAlign(HorizontalAlign value);
        VerticalAlign getVerticalAlign();
        void setVerticalAlign(VerticalAlign value);
        int getGridRows();
        int getGridColumns();
        void setGridRows(int value);
        void setGridColumns(int value);
        bool isClippingEnabled();
        void setClippingEnabled(bool value);
        //@}

        //! Returns true if the event happens within the container boundaries. This method
        //! supports all pointer and ray-generating events, and works with 2D and 3D mode containers.
        bool isEventInside(const Event& evt);
        //! For 3D mode containers: converts a ray event to a pointer event with 2D coordintes in the container coordinate space.
        //! Returns true if the event happens within the container boundaries, and could be converted to a pointer event successfully.
        bool rayToPointerEvent(const Event& inEvt, Event& outEvt);

        virtual void layout();

        //! Gets the container 3d settings.
        Container3dSettings& get3dSettings() { return my3dSettings; }
        //! Returns true if this widget is part of a container that will be drawn
        //! in 3D mode.
        virtual bool isIn3DContainer();

        virtual void updateSize();
        virtual void autosize();

        //! Pixel output
        //@{
        bool isPixelOutputEnabled();
        void setPixelOutputEnabled(bool value);
        PixelData* getPixels();
        //@}

        //! Recomputes navigation links for widgets in this container.
        //! Only enabled widgets take part in navigation.
        void updateChildrenNavigation();

    protected:
        virtual void activate();

    private:
        int expandStep(int childSpace, Orientation orientation);
        void updateChildrenLayoutPosition(Orientation orientation);
        void updateChildrenFreeBounds(Orientation orientation);
        void resetChildrenSize(Orientation orientation);
        void computeLinearLayout(Orientation orientation);
        void computeGridLayout(Orientation orientation);

    private:
        List< Ref<Widget> > myChildren;
        List<Widget*> myChildrenToRemove;

        Layout myLayout;
        float myPadding;
        float myMargin;
        int myGridRows;
        int myGridColumns;
        HorizontalAlign myHorizontalAlign;
        VerticalAlign myVerticalAlign;
        bool myClipping;

        Container3dSettings my3dSettings;

        Ref<PixelData> myPixels;
        bool myPixelOutputEnabled;
    };

    ////////////////////////////////////////////////////////////////////////////
    class OTK_API ContainerRenderable: public WidgetRenderable
    {
    public:
        ContainerRenderable(Container* owner): WidgetRenderable(owner), myOwner(owner), myRenderTarget(NULL), myTexture(NULL) {}
        virtual void draw(const DrawContext& context);

    protected:
        void draw3d(const DrawContext& context);
        void drawChildren(const DrawContext& context, bool containerOnly);
        void beginDraw(const DrawContext& context);
        void endDraw(const DrawContext& context);

    private:
        // We use a raw pointer here to avoid cyclic references.
        Container* myOwner;

        // Stuff used for 3d ui rendering.
        Ref<RenderTarget> myRenderTarget;
        Ref<Texture> myTexture;
    };

    ////////////////////////////////////////////////////////////////////////////
    inline int Container::getNumChildren() 
    { return myChildren.size(); }

    ////////////////////////////////////////////////////////////////////////////
    inline void Container::setLayout(Layout layout) 
    { myLayout = layout; }

    ////////////////////////////////////////////////////////////////////////////
    inline Container::Layout Container::getLayout() 
    { return myLayout; }

    ////////////////////////////////////////////////////////////////////////////
    inline float Container::getPadding()
    { return myPadding; }

    ////////////////////////////////////////////////////////////////////////////
    inline float Container::getMargin() 
    { return myMargin; }

    ////////////////////////////////////////////////////////////////////////////
    inline void Container::setPadding(float value) 
    { myPadding = value; requestLayoutRefresh(); }

    ////////////////////////////////////////////////////////////////////////////
    inline void Container::setMargin(float value) 
    { myMargin = value; requestLayoutRefresh(); }

    ////////////////////////////////////////////////////////////////////////////
    inline Container::HorizontalAlign Container::getHorizontalAlign()
    { return myHorizontalAlign; }

    ////////////////////////////////////////////////////////////////////////////
    inline void Container::setHorizontalAlign(HorizontalAlign value) 
    { myHorizontalAlign = value; requestLayoutRefresh();}

    ////////////////////////////////////////////////////////////////////////////
    inline Container::VerticalAlign Container::getVerticalAlign() 
    { return myVerticalAlign; }

    ////////////////////////////////////////////////////////////////////////////
    inline void Container::setVerticalAlign(VerticalAlign value) 
    { myVerticalAlign = value; requestLayoutRefresh();}

    ////////////////////////////////////////////////////////////////////////////
    inline int Container::getGridRows()
    { return myGridRows; }

    ////////////////////////////////////////////////////////////////////////////
    inline int Container::getGridColumns()
    { return myGridColumns; }

    ////////////////////////////////////////////////////////////////////////////
    inline void Container::setGridRows(int value)
    { myGridRows = value; }

    ////////////////////////////////////////////////////////////////////////////
    inline void Container::setGridColumns(int value)
    { myGridColumns = value; }

    ///////////////////////////////////////////////////////////////////////////
    inline bool Container::isIn3DContainer()
    {
        if(my3dSettings.enable3d) return true;
        if(myContainer != NULL) return myContainer->isIn3DContainer();
        return false;
    }
};};
#endif