###################################################################################################
# THE OMEGA LIB PROJECT
#-------------------------------------------------------------------------------------------------
# Copyright 2010-2013		Electronic Visualization Laboratory, University of Illinois at Chicago
# Authors:										
#  Alessandro Febretti		febret@gmail.com
#-------------------------------------------------------------------------------------------------
# Copyright (c) 2010-2013, Electronic Visualization Laboratory, University of Illinois at Chicago
# All rights reserved.
# Redistribution and use in source and binary forms, with or without modification, are permitted 
# provided that the following conditions are met:
# 
# Redistributions of source code must retain the above copyright notice, this list of conditions 
# and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
# notice, this list of conditions and the following disclaimer in the documentation and/or other 
# materials provided with the distribution. 
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
# USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
###################################################################################################

###################################################################################################
# Compile definitions
add_definitions( -D_CRT_SECURE_NO_WARNINGS -DOTK_EXPORTING -DEQ_FABRIC_STATIC -DFREEGLUT_STATIC )

###################################################################################################
# Source files
set( srcs 
		ToolkitUtils.cpp
		ControllerManipulator.cpp
		WandManipulator.cpp
		MouseManipulator.cpp
		DefaultMouseInteractor.cpp
		DefaultTwoHandsInteractor.cpp
		omegaToolkitPythonApi.cpp
		SceneEditorModule.cpp
		UiModule.cpp
		UiRenderPass.cpp
		UiScriptCommand.cpp
		
		# omega::ui
		ui/AbstractButton.cpp
		ui/Button.cpp
		ui/Container.cpp
		ui/Image.cpp
		ui/Label.cpp
		ui/Menu.cpp
		ui/MenuManager.cpp
		ui/DefaultSkin.cpp
		ui/Slider.cpp
		ui/Widget.cpp
        )
		
###################################################################################################
# Headers
set( headers 
		${CMAKE_SOURCE_DIR}/include/omegaToolkit.h
		
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ToolkitUtils.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ControllerManipulator.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/WandManipulator.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/DefaultMouseInteractor.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/MouseManipulator.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/DefaultTwoHandsInteractor.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/SceneEditorModule.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/UiModule.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/UiRenderPass.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/UiScriptCommand.h
		
		# omega::ui
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/AbstractButton.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/Button.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/Container.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/Image.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/Label.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/Menu.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/MenuManager.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/DefaultSkin.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/Slider.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/Widget.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/WidgetFactory.h
        ) 

###################################################################################################
source_group(omegaToolkit REGULAR_EXPRESSION omegaToolkit/)		
source_group(omegaToolkit::ui REGULAR_EXPRESSION omegaToolkit/ui/)		
source_group(vjson REGULAR_EXPRESSION vjson/)

###################################################################################################
# Setup compile info
enable_precompiled_headers(precompiled.h srcs)
add_library( omegaToolkit SHARED ${srcs} ${headers})

###################################################################################################
# Setup link info.
target_link_libraries( omegaToolkit omega)

###################################################################################################
# configure include header

