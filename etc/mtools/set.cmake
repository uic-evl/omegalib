#set default arguments
if("${ARG2}" STREQUAL "")
    message("SYNTAX: omega set <local-dir> [variable] [value]")
    message("  Sets configuration variables or gets information about them")
    message("ARGUMENTS:")
    message("  - local-dir: name of local installation directory")
    message("  - variable: name or prefix of a configuration variable.")
    message("      If left unspecified omega get will list all available")
    message("      configuration variables")
    message("  - value: value to set for the variable. If unspecified, omega get")
    message("      If left unspecified omega get will list all variables without")
    message("      setting them")
    message("EXAMPLE: omega set OMEGA_BUILD_EXAMPLES true")
    
    return()
endif()

if("${ARG4}" STREQUAL "")
    # Update the installed modules
    execute_process(COMMAND 
        ${CMAKE_COMMAND} ./ -N -L
        WORKING_DIRECTORY ${ARG2}/build OUTPUT_VARIABLE output)
        
    string(REGEX MATCHALL "${ARG3}[^\n]*" matches ${output})
    foreach(match ${matches})
        message(${match})
    endforeach()
else()
    execute_process(COMMAND 
        ${CMAKE_COMMAND} ./ -D${ARG3}="${ARG4}"
        WORKING_DIRECTORY ${ARG2}/build)
endif()
