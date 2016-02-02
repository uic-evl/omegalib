#set default arguments
if("${ARG2}" STREQUAL "")
    message("SYNTAX: omega pack.prepare <local-dir>")
    message("  Prepares files for packaging into installers")
    message("  Run pack-build after this to generate the installer binaries")
    message("ARGUMENTS:")
    message("  - local-dir: name of local installation directory")
    message("EXAMPLE: omega pack master")
    
    return()
endif()

#if(NOT WIN32)
#    message("ERROR: omega pack is currently only supported on windows")
#    return()
#endif()

#Make sure the pack.cmake file is updated
execute_process(COMMAND ${CMAKE_COMMAND} ./ WORKING_DIRECTORY ${ARG2}/build)

#include file with functions used by packaging scripts
include("${ARG2}/src/pack_functions.cmake")

#Run the pack commands
include("${ARG2}/install/pack.cmake")

    