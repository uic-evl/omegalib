/******************************************************************************
* THE OMEGA LIB PROJECT
*-----------------------------------------------------------------------------
* Copyright 2010-2014		Electronic Visualization Laboratory,
*							University of Illinois at Chicago
* Authors:
*  Alessandro Febretti		febret@gmail.com
*-----------------------------------------------------------------------------
* Copyright (c) 2010-2014, Electronic Visualization Laboratory,
* University of Illinois at Chicago
* All rights reserved.
* Redistribution and use in source and binary forms, with or without
* modification, are permitted provided that the following conditions are met:
*
* Redistributions of source code must retain the above copyright notice, this
* list of conditions and the following disclaimer. Redistributions in binary
* form must reproduce the above copyright notice, this list of conditions and
* the following disclaimer in the documentation and/or other materials
* provided with the distribution.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
* LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
* SUBSTITUTE  GOODS OR  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
* INTERRUPTION) HOWEVER  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
* CONTRACT, STRICT LIABILITY,  OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
* ARISING IN ANY WAY OUT OF THE USE  OF THIS SOFTWARE, EVEN IF ADVISED OF THE
* POSSIBILITY OF SUCH DAMAGE.
*-----------------------------------------------------------------------------
* What's in this file
*	A module that switches a camera stereo mode on or off based on it's tracker
*   position and orientation. Useful to disable stereo rendering when a user is
*   taking off their glasses.
*****************************************************************************/
#include "omegaToolkit/CameraStereoSwitcher.h"

using namespace omegaToolkit;

///////////////////////////////////////////////////////////////////////////////
CameraStereoSwitcher* CameraStereoSwitcher::create(const String& name)
{
    Setting& s = SystemManager::instance()->settingLookup("config/modules/" + name);

    CameraStereoSwitcher* css = new CameraStereoSwitcher();
    css->setup(s);
    ModuleServices::addModule(css);
    return css;
}

///////////////////////////////////////////////////////////////////////////////
void CameraStereoSwitcher::setup(Setting& s)
{
    myYThreshold = Config::getFloatValue("yThreshold", s);
    myDisabledOrientation = Config::getVector3fValue("disabledOrientation", s, Vector3f::UnitY());
    myOrientationThreshold = Config::getFloatValue("orientationThreshold", s, 0);

    // Read from camera config!
    myNeutralHeadPos = Vector3f(0, 1.8f, 0);
}

///////////////////////////////////////////////////////////////////////////////
void CameraStereoSwitcher::initialize()
{
    myEnabled = true;
    myTargetCamera = getEngine()->getDefaultCamera();
}

///////////////////////////////////////////////////////////////////////////////
void CameraStereoSwitcher::handleEvent(const Event& evt)
{
    if(myEnabled)
    {
        DisplayConfig& dcfg = SystemManager::instance()->getDisplaySystem()->getDisplayConfig();
        if(evt.getServiceType() == Service::Mocap &&
            evt.getSourceId() == myTargetCamera->getTrackerSourceId())
        {
            if(evt.getPosition().y() < myYThreshold)
            {
                dcfg.forceMono = true;
                myTargetCamera->setTrackingEnabled(false);
                myTargetCamera->setHeadOffset(myNeutralHeadPos);
            }
            else
            {
                dcfg.forceMono = false;
                myTargetCamera->setTrackingEnabled(true);
            }
        }
    }
}
