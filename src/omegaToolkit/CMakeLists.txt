# Compile definitions
add_definitions( -D_CRT_SECURE_NO_WARNINGS -DOTK_EXPORTING -DEQ_FABRIC_STATIC -DFREEGLUT_STATIC )

# Source files
set( srcs 
		ToolkitUtils.cpp
		ControllerManipulator.cpp
		WandManipulator.cpp
		MouseManipulator.cpp
		DefaultMouseInteractor.cpp
		DefaultTwoHandsInteractor.cpp
		ImageBroadcastModule.cpp
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
		ui/TextBox.cpp
		ui/Widget.cpp
        )
		
# Headers
set( headers 
		${CMAKE_SOURCE_DIR}/include/omegaToolkit.h
		
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ToolkitUtils.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ControllerManipulator.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/WandManipulator.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/DefaultMouseInteractor.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/MouseManipulator.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/DefaultTwoHandsInteractor.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ImageBroadcastModule.h
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
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/TextBox.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/Widget.h
		${CMAKE_SOURCE_DIR}/include/omegaToolkit/ui/WidgetFactory.h
        ) 

# Set source groups (VS eye candy)
source_group(omegaToolkit REGULAR_EXPRESSION omegaToolkit/)		
source_group(omegaToolkit::ui REGULAR_EXPRESSION omegaToolkit/ui/)		
source_group(vjson REGULAR_EXPRESSION vjson/)

# Setup compile info
enable_precompiled_headers(precompiled.h srcs)
add_library( omegaToolkit SHARED ${srcs} ${headers})

# Setup link info.
target_link_libraries( omegaToolkit omega)

