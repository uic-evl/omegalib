#set default arguments
if("${ARG2}" STREQUAL "")
    message("SYNTAX: omega update <local-dir>")
    message("  Updates a local omegalib install and all its modules")
    message("ARGUMENTS:")
    message("  - local-dir: name of local installation directory")
    message("EXAMPLE: omega update master")
    return()
endif()

# Update core lib
message("---- Updating omegalib core...")
execute_process(COMMAND ${GIT_EXECUTABLE} pull
    WORKING_DIRECTORY ${ARG2})
execute_process(COMMAND ${GIT_EXECUTABLE} submodule update omicron
    WORKING_DIRECTORY ${ARG2})
    
# update modules
set(MODDIR ${CMAKE_CURRENT_LIST_DIR}/../../modules)
message("in ${MODDIR}")
file(GLOB children RELATIVE ${MODDIR} ${MODDIR}/*)
foreach(child ${children})
    if(IS_DIRECTORY ${MODDIR}/${child} AND IS_DIRECTORY ${MODDIR}/${child}/.git)
        message("---- Updating ${child}...")
        execute_process(COMMAND ${GIT_EXECUTABLE} pull
            WORKING_DIRECTORY ${MODDIR}/${child})
    endif()
endforeach()

message("----------------------------------------------------------------------")
message("Omegalib updated. If the core library or any native modules have")
message("been updated, you need to rebuild omegalib. Do:")
message("  > omega build ${ARG2}")
message("----------------------------------------------------------------------")

