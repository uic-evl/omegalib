#set default arguments
if("${ARG2}" STREQUAL "")
    message("SYNTAX: omega get [local-dir:]<version> [modules]")
    message("  Downloads and builds omegalib and optional modules")
    message("ARGUMENTS:")
    message("  - local-dir (optional): name of local installation directory")
    message("      default value is the version name.")
    message("  - version: name of the omegalib version to download. Use master")
    message("      to download the latest version.")
    message("  - modules: a list of optional modules to install with omegalib")
    message("      modules are separated by ; with no space between them. Use")
    message("      commonModules to install a set of commonly used omegalib")
    message("      modules.")
    message("EXAMPLE: omega get master common-modules")
    
    return()
endif()

# turn input "remote version->local dir" string into a list    
string(REPLACE ":" ";" ARG2_LIST "${ARG2}")
list(LENGTH ARG2_LIST len)
list(GET ARG2_LIST 0 REMOTE_VERSION_NAME)
list(GET ARG2_LIST 0 LOCAL_DIR_NAME)
if(${len} EQUAL 2)
    list(GET ARG2_LIST 1 REMOTE_VERSION_NAME)
endif()

# now we have the desired omegalib version in REMOTE_VERSION_NAME and
# the local directory where we want to clone it in LOCAL_DIR_NAME
message("version: ${REMOTE_VERSION_NAME}")
message("dir: ${LOCAL_DIR_NAME}")

# Clone omegalib repo
set(URL "https://github.com/uic-evl/omegalib.git")
execute_process(COMMAND ${GIT_EXECUTABLE}
    clone ${URL} ${LOCAL_DIR_NAME} --recursive)
execute_process(COMMAND ${GIT_EXECUTABLE}
    checkout ${REMOTE_VERSION_NAME} -q
    WORKING_DIRECTORY ${LOCAL_DIR_NAME})
execute_process(COMMAND ${GIT_EXECUTABLE}
    submodule update -q 
    WORKING_DIRECTORY ${LOCAL_DIR_NAME})

# run cmake once to create the build directory
file(MAKE_DIRECTORY ${LOCAL_DIR_NAME}/build)
if(WIN32)
    # On windows, always generate a VS2012 solution.
    execute_process(COMMAND ${CMAKE_COMMAND}
        ../ -G "Visual Studio 11 2012"
        WORKING_DIRECTORY ${LOCAL_DIR_NAME}/build)
else()
    execute_process(COMMAND ${CMAKE_COMMAND}
        ../ 
        WORKING_DIRECTORY ${LOCAL_DIR_NAME}/build)
endif()

# run omega add to add the selected modules
include("${CMAKE_CURRENT_LIST_DIR}/add.cmake")

# run omega build to build the downloaded version (take away ARG3 since build
# uses it as a configuration name and we just want release)
set(ARG3 "")
include("${CMAKE_CURRENT_LIST_DIR}/build.cmake")
