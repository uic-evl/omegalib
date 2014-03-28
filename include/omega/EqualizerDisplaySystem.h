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
 *	A cluster display system implementation based on the Equalizer parallel
 *  rendering framework.
 *  NOTE: This class only implements display configuration / startup / shutdown.
 *  additional classes are in the eqinternal directory
 ******************************************************************************/
#ifndef __EQUALIZER_DISPLAY_SYSTEM_H__
#define __EQUALIZER_DISPLAY_SYSTEM_H__

#include "DisplaySystem.h"
#include "ApplicationBase.h"

namespace omega
{
    ///////////////////////////////////////////////////////////////////////////////////////////////
    // Forward Declarations.
    class EqualizerNodeFactory;
    class ViewImpl;
    class ConfigImpl;
    class Engine;

    ///////////////////////////////////////////////////////////////////////////////////////////////
    // This class is used to route equalizer log into the omega log system.
    class EqualizerLogStreamBuf: public std::streambuf
    {
    protected:
        virtual int overflow ( int c = EOF )
        {
            if(c == '\n')
            {
                omsg(myStringStream.str().c_str());
                myStringStream.str(""); 
            }
            else
            {
                myStringStream << (char)c;
            }
            return 0;
        }
    private:
        std::ostringstream myStringStream;
    };

    ///////////////////////////////////////////////////////////////////////////////////////////////
    class OMEGA_API EqualizerDisplaySystem: public DisplaySystem
    {
    public:
        EqualizerDisplaySystem();
        virtual ~EqualizerDisplaySystem();

        // sets up the display system. Called before initalize.
        void setup(Setting& setting);

        void initialize(SystemManager* sys); 
        void run(); 
        void cleanup(); 

        virtual void killCluster();
        virtual DisplaySystemType getId() { return DisplaySystem::Equalizer; }
        bool isDebugMouseEnabled() { return myDebugMouse; }
        virtual void refreshSettings();

        //! Returns the size of the display canvas.
        virtual Vector2i getCanvasSize();

        //! @internal Finish equalizer display system initialization.
        //! This method is called from the node init function. Performs observer initialization.
        void finishInitialize(ConfigImpl* config, Engine* engine);

        void exitConfig();

    private:
        void generateEqConfig();
        void setupEqInitArgs(int& numArgs, const char** argv);
        String buildTileConfig(String& indent, const String tileName, int x, int y, int width, int height, int device, int curdevice, bool fullscreen, bool borderless, bool offscreen);

    private:
        SystemManager* mySys;

        // Configuration
        Setting* mySetting;

        // Equalizer stuff.
        EqualizerNodeFactory* myNodeFactory;
        ConfigImpl* myConfig;

        // Debug
        bool myDebugMouse;
    };

}; // namespace omega

#endif