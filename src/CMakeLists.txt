include(FileInformation)
include(ExternalProject)
#include(OmegaAddTutorial)
include(UsePch)

# Set virtual memory size for precompiled header
if(WIN32)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm500")
endif()

include(${OmegaLib_SOURCE_DIR}/external/UseOmicron.cmake)

include_directories(../modules)

# devlib installing is not supported yet on omegalib 5, so force this to false
#set(OMEGA_INSTALL_DEVLIBS false CACHE BOOL "When set to true, and install will include C++ development libraries in addition to the python interpreter")
set(OMEGA_INSTALL_DEVLIBS false)

###############################################################################
# Visual Leak Detector Support
if(OMEGA_OS_WIN)
	set(OMEGA_ENABLE_VLD false CACHE BOOL "Enables Visual Leak Detector support for this build.")
endif()

###############################################################################
# See if we can setup Python support for this build
set(OMEGA_ENABLE_PYTHON true CACHE BOOL "Enables python support if available.")
if(OMEGA_ENABLE_PYTHON)
	if(WIN32)
		# On windows use the included python distribution
		set(OMEGA_USE_PYTHON true)
		include(${OmegaLib_SOURCE_DIR}/external/UsePython.cmake)
		set(PYTHON_INCLUDE_DIRS ${CMAKE_SOURCE_DIR}/modules/python/include)
		set(PYTHON_LIBRARIES ${CMAKE_SOURCE_DIR}/modules/python/libs/python27.lib)
		include_directories(${PYTHON_INCLUDE_DIRS})
		# copy the python dll to the output folders
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/python27.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/python27.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} COPYONLY)
    else()
		find_package(PythonLibs 2.7)
		if(PYTHONLIBS_FOUND)
			set(OMEGA_USE_PYTHON true)
			include_directories(${PYTHON_INCLUDE_DIRS})
			include(${OmegaLib_SOURCE_DIR}/external/UsePython.cmake)
		else()
			message("Python libraries not found on this system. Omegalib Python support will be disabled")
		endif()
	endif()
else()
	set(OMEGA_USE_PYTHON false)
endif()

###############################################################################
# Options
if(WIN32)
	set(OMEGA_ENABLE_AUTO_UPDATE true CACHE BOOL "Enables the omegalib binaries auto updater (WinSparkle) for a windows build. Supported by 32bit builds only.")
endif()

###############################################################################
# Set include paths
if(APPLE)
	include_directories(
	  ${CMAKE_BINARY_DIR}/include/
	  ${CMAKE_SOURCE_DIR}/include/
	  ${CMAKE_SOURCE_DIR}/external/include
	  ${OMICRON_INCLUDE_DIRS}
	  )
else(APPLE)
	include_directories(
	  ${CMAKE_BINARY_DIR}/include/
	  ${CMAKE_SOURCE_DIR}/include/
  	  ${CMAKE_SOURCE_DIR}/src/3rdparty/glew  
	  ${CMAKE_SOURCE_DIR}/external/include
	  ${OMICRON_INCLUDE_DIRS}
	  )
endif(APPLE)

###############################################################################
# Add libraries and applications to the project.

# Add some libraries used for display support.
include(${OmegaLib_SOURCE_DIR}/external/UseFreeImage.cmake)

include(${OmegaLib_SOURCE_DIR}/external/UseFreeType.cmake)
include(${OmegaLib_SOURCE_DIR}/external/UseFtgl.cmake)
include_directories(
  ${OmegaLib_BINARY_DIR}/ftgl/src/
  ${OmegaLib_BINARY_DIR}/freetype/include/
  ${OmegaLib_BINARY_DIR}/FreeImage/Source/
)

if(NOT APPLE)
	add_subdirectory(3rdparty/glew)
endif(NOT APPLE)

# Add several example applications
set(OMEGA_BUILD_EXAMPLES true CACHE BOOL "Build C++ examples")
if(OMEGA_BUILD_EXAMPLES)
    add_subdirectory(${CMAKE_SOURCE_DIR}/examples/ohello ${CMAKE_BINARY_DIR}/examples/ohello)
    add_subdirectory(${CMAKE_SOURCE_DIR}/examples/ohello2 ${CMAKE_BINARY_DIR}/examples/ohello2)
    add_subdirectory(${CMAKE_SOURCE_DIR}/examples/ohelloWidgets ${CMAKE_BINARY_DIR}/examples/ohelloWidgets)
    add_subdirectory(${CMAKE_SOURCE_DIR}/examples/omegaSound ${CMAKE_BINARY_DIR}/examples/omegaSound)
    add_subdirectory(${CMAKE_SOURCE_DIR}/examples/ohelloView ${CMAKE_BINARY_DIR}/examples/ohelloView)
endif()

# Add the omegaToolkit library
add_subdirectory(omegaToolkit)

# OSX uses its own glut, do not compile it internally (Should we do the same under linux?)
set(OMEGA_USE_DISPLAY_GLUT false CACHE BOOL "Enable GLUT display system support")
if(OMEGA_USE_DISPLAY_GLUT)
	if(APPLE)
		find_package(GLUT)
	endif(APPLE)
    if(NOT APPLE) 
	    add_subdirectory(3rdparty/freeglut)
    endif()
endif()

# Add the core omega project
add_subdirectory(omega)

# Generate the Omegalib config fule used by external applications to find omegalib.
# The @ONLY option is to substitute only variables in form @VAR@. This is because
# OmegalibConfig.cmake uses ${VAR} variables that will be substituted by the
# external application.
configure_file(${CMAKE_SOURCE_DIR}/CMakeModules/OmegalibConfig.cmake.in ${CMAKE_BINARY_DIR}/OmegalibConfig.cmake @ONLY)

# Windows-specific config operations
if(WIN32)
	# Set up the MakeRelease script. It will be used to generate release packages on windows
	configure_file(${CMAKE_SOURCE_DIR}/MakeRelease.bat.in ${CMAKE_BINARY_DIR}/MakeRelease.bat)
	# Copy The WinSparkle dll to output folder. WInsparkle is used to notify users of omegalib updates.
	if(OMEGA_ENABLE_AUTO_UPDATE)
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/WinSparkle.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/WinSparkle.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} COPYONLY)
	endif()
endif()

###############################################################################
# Setup the module loader
set(MODULES_UPDATE_LIST true CACHE BOOL "Enable to download and re-generate the list of installed modules")

if(MODULES_UPDATE_LIST)
	include(ModuleUtils)
	find_package(Git)
	set(REGENERATE_REQUESTED false CACHE BOOL "" FORCE)
	if(GIT_EXECUTABLE)
		set(OMEGA_HUB_URL "http://github.com/omega-hub/hub" CACHE STRING "url to git repository containing the module index")
		# the modules variable is a list of enabled modues, used to simplify configuration.
		set(MODULES " " CACHE STRING "The list of enabled modules")
		separate_arguments(MODULES_LIST WINDOWS_COMMAND "${MODULES}")
		
		# Delete the native modules CMakeLists file. it will be regenerated by 
		# module_def calls for native modules
		set(MODULES_CMAKE_FILE ${CMAKE_SOURCE_DIR}/modules/CMakeLists.txt)
		file(REMOVE ${MODULES_CMAKE_FILE})
		file(APPEND ${MODULES_CMAKE_FILE} "#auto-generated file\n")
		
		# delete the modulesConfig.h file, it will be regenerated by module_def
		# for native modules.
		set(MODULES_CONFIG_FILE ${CMAKE_SOURCE_DIR}/modules/modulesConfig.h)
		file(REMOVE ${MODULES_CONFIG_FILE})
		file(APPEND ${MODULES_CONFIG_FILE} "//auto-generated file\n")
		
		# download the index CMakeLists file. Download the file to a secondary location, so if the
		# download fails, we can still use the previous copy of the module list
		file(DOWNLOAD ${OMEGA_HUB_URL}/raw/master/CMakeLists.txt ${CMAKE_BINARY_DIR}/moduleList/CMakeLists.txt SHOW_PROGRESS)
		# If the newly-downloaded file contains something, copy it over the old module list.
		filestat(SIZE ${CMAKE_BINARY_DIR}/moduleList/CMakeLists.txt MODULE_LIST_SIZE)
		if(NOT ${MODULE_LIST_SIZE} EQUAL 0)
			file(COPY ${CMAKE_BINARY_DIR}/moduleList/CMakeLists.txt DESTINATION ${CMAKE_BINARY_DIR}/modules)
		else()
			message("Module list download failed, using cached copy")
		endif()
		if(EXISTS ${CMAKE_BINARY_DIR}/modules/CMakeLists.txt)
			# Add the CMakeLists with the module index. Need to specify a binary 
			# output path since the file is not in the source tree.
			add_subdirectory(${CMAKE_BINARY_DIR}/modules ${CMAKE_BINARY_DIR}/modules/build)
		else()
			message("Could not find module master index file")
		endif()
	endif()
	
	# Keep running until all module dependencies are resolved.
	while(REGENERATE_REQUESTED)
		set(REGENERATE_REQUESTED false CACHE BOOL "" FORCE)
		
		# Delete the native modules CMakeLists file. it will be regenerated by 
		# module_def calls for native modules
		set(MODULES_CMAKE_FILE ${CMAKE_SOURCE_DIR}/modules/CMakeLists.txt)
		file(REMOVE ${MODULES_CMAKE_FILE})
		file(APPEND ${MODULES_CMAKE_FILE} "#auto-generated file\n")
		
		# delete the modulesConfig.h file, it will be regenerated by module_def
		# for native modules.
		set(MODULES_CONFIG_FILE ${CMAKE_SOURCE_DIR}/modules/modulesConfig.h)
		file(REMOVE ${MODULES_CONFIG_FILE})
		file(APPEND ${MODULES_CONFIG_FILE} "//auto-generated file\n")
		
		# This function contains all the module definitions and is defined in the hub CMakeListst file.
		process_omegalib_modules()
		message("REGENERATE_REQUESTED ${REGENERATE_REQUESTED}")
	endwhile()
endif()

# Add the modules subdirectory. This will include cmake scripts for all native modules
add_subdirectory(${CMAKE_SOURCE_DIR}/modules ${CMAKE_BINARY_DIR}/modules)

# Add this after the modules declaration so that the apps are agnostic of the enabled modules. (dependency fix).
if(OMEGA_USE_PYTHON)
	# We need to add orun here because its config file uses build options defined above
	add_subdirectory(apps/orun)
	add_subdirectory(apps/mcsend)
	add_subdirectory(apps/mcserver)
	add_subdirectory(apps/olauncher)
endif()

if(${REGENERATE_REQUESTED})
	message(FATAL_ERROR "Please run Configure again to install missing modules.")
endif()

###############################################################################
# Install commands
if(${OMEGA_INSTALL_DEVLIBS})
	install(DIRECTORY ../include DESTINATION omegalib)
	install(DIRECTORY ../CMakeModules DESTINATION omegalib)
endif()

# omicron
if(${OMEGA_INSTALL_DEVLIBS})
	install(DIRECTORY ../omicron/CMakeModules DESTINATION omegalib/omicron)
	install(DIRECTORY ../omicron/include DESTINATION omegalib)
	install(DIRECTORY ../omicron/external/include DESTINATION omegalib/external)
	install(FILES ${CMAKE_BINARY_DIR}/omicron/omicron/UseOmicron.cmake DESTINATION omegalib/omicron/omicron)
	install(DIRECTORY ${CMAKE_BINARY_DIR}/omicron/omicron/include DESTINATION omegalib/omicron/omicron)
endif()

if(WIN32)
	# on windows, install only the release dir, and also copy the export libs.
	install(DIRECTORY ${CMAKE_BINARY_DIR}/bin/release/ DESTINATION omegalib/bin)
	if(${OMEGA_INSTALL_DEVLIBS})
		install(DIRECTORY ${CMAKE_BINARY_DIR}/lib/release/ DESTINATION omegalib/lib FILES_MATCHING PATTERN "*.lib")
	endif()
	#Also install python (on windows)
	install(DIRECTORY ../modules/python/DLLs DESTINATION omegalib/modules/python)
	install(DIRECTORY ../modules/python/Lib DESTINATION omegalib/modules/python)
	
else()
	install(DIRECTORY ${CMAKE_BINARY_DIR}/bin DESTINATION omegalib)
endif()

# data
install(DIRECTORY ../fonts DESTINATION omegalib)
install(DIRECTORY ../menu_sounds DESTINATION omegalib)
install(DIRECTORY ../ui DESTINATION omegalib)
install(DIRECTORY ../system DESTINATION omegalib)
install(FILES ../default.cfg DESTINATION omegalib)
install(FILES ../default_init.py DESTINATION omegalib)
install(FILES ../omegalib-transparent-white.png DESTINATION omegalib)
if(${OMEGA_INSTALL_DEVLIBS})
	install(FILES ${CMAKE_BINARY_DIR}/OmegalibConfig.cmake DESTINATION omegalib)
	install(FILES ${CMAKE_BINARY_DIR}/include/omegaConfig.h DESTINATION omegalib/include)
endif()

