#set default arguments
if("${ARG2}" STREQUAL "")
    message("SYNTAX: omega pack.build <local-dir> [offline]")
    message("  Generates binaries for online and offline installation")
    message("ARGUMENTS:")
    message("  - local-dir: name of local installation directory")
    message("  - offline: builds a self-contained offline installer")
    message("EXAMPLE: omega pack-build master")
    
    return()
endif()

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
file(REMOVE_RECURSE ${ARG2}/install/repository)

if("${ARG3}" STREQUAL "offline")
    message("---- Building offline installer")
    if(WIN32)
        execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/qtifw/binarycreator.exe 
            -c config/config-offline.xml -p packages OmegalibOfflineSetup.exe
            WORKING_DIRECTORY ${ARG2}/install)
    else()
        execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/qtifw/binarycreator
            -c config/config-offline.xml -p packages OmegalibOfflineSetup
            WORKING_DIRECTORY ${ARG2}/install)
    endif()
else()
    if(WIN32)
        message("---- Building online repository")
        execute_process(COMMAND ${CMAKE_SOURCE_DIR}/cmake/qtifw/repogen.exe
            -p packages repository
            WORKING_DIRECTORY ${ARG2}/install)
        message("---- Building online installer")
        execute_process(COMMAND ${CMAKE_SOURCE_DIR}/cmake/qtifw/binarycreator.exe
            -c config/config-online.xml -p packages -n OmegalibSetup.exe
            WORKING_DIRECTORY ${ARG2}/install)
    else()
        message("---- Building online repository")
        execute_process(COMMAND ../../cmake/qtifw/repogen
            -p packages repository
            WORKING_DIRECTORY ${ARG2}/install)
        message("---- Building online installer")
        execute_process(COMMAND ../../cmake/qtifw/binarycreator
            -c config/config-online.xml -p packages -n OmegalibSetup
            WORKING_DIRECTORY ${ARG2}/install)
    endif()
endif()