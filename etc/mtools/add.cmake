#set default arguments
if("${ARG2}" STREQUAL "")
    message("SYNTAX: omega add <local-dir> <modules>")
    message("  Adds optional modules to a local omegalib copy")
    message("ARGUMENTS:")
    message("  - local-dir: name of local installation directory")
    message("  - modules: a list of optional modules to install with omegalib")
    message("      modules are separated by ; with no space between them. Use")
    message("      common-modules to install a set of commonly used omegalib")
    message("      modules.")
    message("EXAMPLE: omega add master omegaVtk;omegaOsg")
    
    return()
endif()

# Update the installed modules
execute_process(COMMAND 
    ${CMAKE_COMMAND} ./ -DMODULES_ADD="${ARG3}"
    WORKING_DIRECTORY ${ARG2}/build)

message("----------------------------------------------------------------------")
message("NOTE: If you installed native modules, you need to rebuild this omegalib")
message("installation to complete the update. Do:")
message("  > omega build ${ARG2}")
message("----------------------------------------------------------------------")

