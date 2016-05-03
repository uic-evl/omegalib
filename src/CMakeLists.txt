include(FileInformation)
include(ExternalProject)
include(UsePch)

#-------------------------------------------------------------------------------
if(OMEGA_OS_WIN)
    add_definitions(-D_WIN32_WINNT=0x0600)
    add_definitions(-DWINNT=0x600)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /DWIN32 /Zm500")
    # Visual studio 2013 runtime
    if(OMEGA_TOOL_VS12)
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/msvcp120.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/msvcp120.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} COPYONLY)
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/msvcr120.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} COPYONLY)
		configure_file(${CMAKE_SOURCE_DIR}/external/libs32/msvcr120.dll ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} COPYONLY)
    endif()
endif()

#-------------------------------------------------------------------------------
# Set include paths
include_directories(
    ${CMAKE_BINARY_DIR}/include/
    ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/src/glew  
    ${CMAKE_BINARY_DIR}/ftgl/src/
    ${CMAKE_BINARY_DIR}/freetype/include/
    ${CMAKE_BINARY_DIR}/FreeImage/Source/
    ${CMAKE_SOURCE_DIR}/modules
  )

#-------------------------------------------------------------------------------
# Add libraries and applications to the project.

# External projects
include(${CMAKE_SOURCE_DIR}/external/UsePython.cmake)
include(${CMAKE_SOURCE_DIR}/external/UseFreeImage.cmake)
include(${CMAKE_SOURCE_DIR}/external/UseFreeType.cmake)
include(${CMAKE_SOURCE_DIR}/external/UseFtgl.cmake)
include(${CMAKE_SOURCE_DIR}/external/UseOmicron.cmake)
include_directories(${OMICRON_INCLUDE_DIRS})

# Omegalib core libs
add_subdirectory(glew)
add_subdirectory(omegaToolkit)
add_subdirectory(omega)
add_subdirectory(displaySystem_GLFW)

#-------------------------------------------------------------------------------
# Process optional modules
include(ModuleUtils)
find_package(Git REQUIRED)

set(OMEGA_DEFAULT_MODULE_ORGANIZATION "omega-hub" CACHE STRING "The default gihub user or oganization hosting omegalib modules")

process_modules()

# We need to add orun here because its config file uses build options defined above
add_subdirectory(orun)

# Generate the Omegalib config fule used by external applications to find omegalib.
# The @ONLY option is to substitute only variables in form @VAR@. This is because
# OmegalibConfig.cmake uses ${VAR} variables that will be substituted by the
# external application.
configure_file(${CMAKE_SOURCE_DIR}/CMakeModules/OmegalibConfig.cmake.in ${CMAKE_BINARY_DIR}/OmegalibConfig.cmake @ONLY)

