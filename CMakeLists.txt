set_property(GLOBAL PROPERTY USE_FOLDERS On)
cmake_minimum_required(VERSION 2.8.3) 
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMakeModules;${CMAKE_MODULE_PATH}")

project(OmegaLib)

# The default home directory is the omegalib source directoy. At runtime this
# will be replaced by the OMEGA_HOME environment variable when available.
set(OMEGA_HOME "${OmegaLib_SOURCE_DIR}")

set(CMAKE_INSTALL_PREFIX ${CMAKE_SOURCE_DIR}/install)

include(CPack)

if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
endif()

#-------------------------------------------------------------------------------
# Create a build name depending on the OS, Toolset and architecture.
if(CMAKE_GENERATOR STREQUAL "Visual Studio 12 2013")
    set(OMEGA_OS_WIN 1)
    set(OMEGA_ARCH_32 1)
    set(OMEGA_ARCH_WIN32 1)
    set(OMEGA_TOOL_VS12 1)
elseif(CMAKE_GENERATOR STREQUAL "Visual Studio 12 2013 Win64")
    set(OMEGA_OS_WIN 1)
    set(OMEGA_ARCH_64 1)
    set(OMEGA_ARCH_WIN32 1)
    set(OMEGA_TOOL_VS12 1)
elseif(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
    if(APPLE)
        execute_process(COMMAND sw_vers -productVersion	OUTPUT_VARIABLE _CURRENT_OSX_VERSION OUTPUT_STRIP_TRAILING_WHITESPACE)
        string(REGEX REPLACE "^([0-9]+\\.[0-9]+).*$" "\\1" CURRENT_OSX_VERSION "${_CURRENT_OSX_VERSION}" )
	    set(CMAKE_MACOSX_RPATH ON)
        set(CMAKE_INSTALL_RPATH "@executable_path")
        set(CMAKE_BUILD_WITH_INSTALL_RPATH True)
        set(OMEGA_OS_OSX 1)
        set(OMEGA_ARCH_64 1)
        set(OMEGA_TOOL_GCC 1)
        set(CMAKE_CXX_FLAGS "-Wno-ignored-qualifiers -Wno-enum-compare -Wno-deprecated-declarations -Wno-logical-op-parentheses -Wno-switch -Wno-c++11-compat-deprecated-writable-strings -Wno-return-type-c-linkage -Wno-unused-value -Wno-dynamic-class-memaccess -Wno-parentheses-equality -stdlib=libstdc++" CACHE STRING "compile flags" FORCE)
    else()
        if("${CMAKE_SIZEOF_VOID_P}" STREQUAL "8")
            set(OMEGA_OS_LINUX 1)
            set(OMEGA_ARCH_64 1)
            set(OMEGA_ARCH_AMD64 1)
            set(OMEGA_TOOL_GCC 1)
        else()
            set(OMEGA_OS_LINUX 1)
            set(OMEGA_ARCH_32 1)
            set(OMEGA_ARCH_I386 1)
            set(OMEGA_TOOL_GCC 1)
        endif()
        set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -ldl -lrt")	
        # NOTE: If I add following line to disable some gcc warnings
        # I will get failed link errors due to the -fPIC x86_64 invalid relocation
        # on osgBullet. Maybe some CXX flags gets rewritten somewhat?
		#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wp,-w -Wno-int-to-pointer-cast -Wno-unused-value -Wno-enum-compare -Wno-write-strings")

        # needed on Redhat-based ditros or you will get a link error for forkpty/openpty
        # (move this to omega CmakeLists only?)
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -lutil -fPIC")
        #set(CMAKE_POSITION_INDEPENDENT_CODE On)
	endif()
elseif(CMAKE_GENERATOR STREQUAL "Xcode")
    SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -stdlib=libstdc++" CACHE STRING "" FORCE)
    SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -stdlib=libstdc++" CACHE STRING "" FORCE)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -stdlib=libstdc++" CACHE STRING "" FORCE)
    SET(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -stdlib=libstdc++" CACHE STRING "" FORCE)
    SET(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -stdlib=libstdc++" CACHE STRING "" FORCE)
    execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_VERSION)
    set(OMEGA_OS_OSX 1)
    set(OMEGA_ARCH_32 1)
    set(OMEGA_ARCH_I386 1)
    set(OMEGA_TOOL_GCC 1)
endif()

#-------------------------------------------------------------------------------
# Set the output directories for libraries and binary files
if(MSVC OR CMAKE_GENERATOR STREQUAL "Xcode")
    # Since visual studio builds are multiconfiguration, set two separate directories for debug and release builds
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG ${OmegaLib_BINARY_DIR}/bin/debug)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE ${OmegaLib_BINARY_DIR}/bin/release)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OmegaLib_BINARY_DIR}/bin/release)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${OmegaLib_BINARY_DIR}/lib/debug)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${OmegaLib_BINARY_DIR}/lib/release)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OmegaLib_BINARY_DIR}/lib/release)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG ${OmegaLib_BINARY_DIR}/bin/debug)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE ${OmegaLib_BINARY_DIR}/bin/release)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO ${OmegaLib_BINARY_DIR}/bin/release)
    # Make sure the output bin directories exist, so the cmake scripts can copy 
    # third party dlls there if needed.
    file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG})
    file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE})
else(MSVC OR CMAKE_GENERATOR STREQUAL "Xcode")
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${OmegaLib_BINARY_DIR}/bin)
    set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${OmegaLib_BINARY_DIR}/lib)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${OmegaLib_BINARY_DIR}/bin)
    file(MAKE_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY})
    file(MAKE_DIRECTORY ${CMAKE_LIBRARY_OUTPUT_DIRECTORY})
endif(MSVC OR CMAKE_GENERATOR STREQUAL "Xcode")

include(OmegalibVersion)
 
# Add subdirectiories
add_subdirectory(src)


