#set default arguments
if("${ARG2}" STREQUAL "")
    message("SYNTAX: omega pack-build <local-dir>")
    message("  Generates binaries for online and offline installation")
    message("ARGUMENTS:")
    message("  - local-dir: name of local installation directory")
    message("EXAMPLE: omega pack-build master")
    
    return()
endif()

if(NOT WIN32)
    message("ERROR: omega pack is currently only supported on windows")
    return()
endif()

if(NOT EXISTS cmake/qtifw)
    file(MAKE_DIRECTORY cmake/qtifw)
    set(TOOLS "archivegen.exe;binarycreator.exe;installerbase.exe;repogen.exe")
    message("Downloading the Qt Installer Framework tools")
    foreach(S IN LISTS TOOLS)
        message("...${S}")
        file(DOWNLOAD ${OPM_URL}/qtifw-1.5.0-win/${S} cmake/qtifw/${S})
    endforeach()
endif()

# Delete the local repositoryor the repogen command will fail
file(REMOVE_RECURSE ${ARG2}/install/repository)

#Generate the online and offline installers
message("---- Building offline installer")
execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/qtifw/binarycreator.exe 
    -c config/config-offline.xml -p packages OmegalibOfflineSetup.exe
    WORKING_DIRECTORY ${ARG2}/install)

message("---- Building online repository")
execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/qtifw/repogen.exe 
    -p packages repository
    WORKING_DIRECTORY ${ARG2}/install)

message("---- Building online installer")
execute_process(COMMAND ${CMAKE_CURRENT_LIST_DIR}/qtifw/binarycreator.exe 
    -c config/config-online.xml -p packages -n OmegalibSetup.exe
    WORKING_DIRECTORY ${ARG2}/install)