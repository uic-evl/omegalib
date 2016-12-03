if("${ARG2}" STREQUAL "")
    message("SYNTAX: omega pack.app [local-dir] [app/module]")
    return()
endif()

# ARG2 - distribution
# ARG3 - app name (also name of the module in distribution)

include(${ARG2}/CMakeModules/ModuleUtils.cmake)

if(WIN32)
    set(BIN_DIR ${ARG2}/build/bin/release)
else()
    set(BIN_DIR ${ARG2}/build/bin)
endif()

set(CALLING_DIR ${CMAKE_SOURCE_DIR})
set(PACKAGE_DIR packs/${ARG3})
set(CMAKE_SOURCE_DIR ${ARG2})
set(CMAKE_BINARY_DIR ${ARG2}/build)
set(CMAKE_INSTALL_PREFIX ${PACKAGE_DIR})

# copy the core install files to the package target dir
file(INSTALL DESTINATION ${PACKAGE_DIR}
    TYPE DIRECTORY
    FILES
        ${CMAKE_SOURCE_DIR}/install/config
    )
file(INSTALL DESTINATION ${PACKAGE_DIR}/packages/core
    TYPE DIRECTORY
    FILES
        ${CMAKE_SOURCE_DIR}/install/packages/core/meta
    )
    
# Prepare to run process_modules in pack-app mode. Set all the 
# required variables
set(PACK_APP_MODE TRUE)
set(MODULES_ADD ${ARG3})

process_modules()
include(${ARG2}/src/pack_functions.cmake)
include(${PACKAGE_DIR}/pack.cmake)

if(NOT EXISTS cmake/qtifw)
    file(MAKE_DIRECTORY cmake/qtifw)
    message("Downloading the Qt Installer Framework tools")
    if(WIN32)
		set(TOOLS "archivegen.exe;binarycreator.exe;installerbase.exe;repogen.exe")
		foreach(S IN LISTS TOOLS)
			message("...${S}")
			file(DOWNLOAD ${OPM_URL}/qtifw-1.5.0-win/${S} cmake/qtifw/${S})
		endforeach()
    elseif(APPLE)
		set(TOOLS "archivegen;binarycreator;installerbase;repogen")
		foreach(S IN LISTS TOOLS)
			message("...${S}")
			file(DOWNLOAD ${OPM_URL}/qtifw-2.0.0-osx/${S} cmake/qtifw/${S})
			execute_process(COMMAND chmod +x ${S} WORKING_DIRECTORY cmake/qtifw)
		endforeach()
    endif()
endif()

# Delete the local repositoryor the repogen command will fail
#file(REMOVE_RECURSE ${ARG2}/install/repository)

message("---- Building offline installer")
if(WIN32)
    execute_process(COMMAND ${CALLING_DIR}/cmake/qtifw/binarycreator.exe 
        -c config/config-offline.xml -p packages ${ARG3}Setup.exe
        WORKING_DIRECTORY ${CALLING_DIR}/packs/${ARG3})
else()
    execute_process(COMMAND ${CALLING_DIR}/cmake/qtifw/binarycreator
        -c config/config-offline.xml -p packages ${ARG3}Setup
        WORKING_DIRECTORY ${CALLING_DIR}/packs/${ARG3})
endif()
