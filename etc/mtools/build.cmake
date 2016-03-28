#set default arguments
if("${ARG2}" STREQUAL "")
    message("SYNTAX: omega build <local-dir> [debug]")
    message("  Builds a local omegalib copy")
    message("ARGUMENTS:")
    message("  - local-dir: name of local installation directory")
    message("  - debug (optional): if specified, build in debug mode")
    message("EXAMPLE: omega build master debug")
    
    return()
endif()

if("${ARG3}" STREQUAL "")
    set(ARG3 Release)
endif()    

execute_process(COMMAND ${CMAKE_COMMAND}
    --build ./ --config ${ARG3}
    WORKING_DIRECTORY ${ARG2}/build)

    
