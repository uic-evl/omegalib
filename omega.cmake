# Download an update to this and the rest of the opm scripts.
set(OPM_URL "https://uic-evl.github.io/omegalib")

find_package(Git REQUIRED QUIET)

file(DOWNLOAD ${OPM_URL}/omega.cmake ./cmake/omega.cmake)

# replace OPM_URL with urs to s3 storage where our distro tools are.
set(OPM_URL "http://omegalib.s3.amazonaws.com/maintenance-utils")

# run omega add to add the selected modules
if(EXISTS ${ARG2}/tools/mtools/)
    set(TOOLDIR "${ARG2}/tools/mtools")
elseif(EXISTS ${ARG2}/etc/mtools/)
    set(TOOLDIR "${ARG2}/etc/mtools")
endif()


# If we are running any tool other than get, we run the tool from
# the distribution itself. get is embedded in this script, since we
# run it to download a distribution.
if(NOT ${ARG1} STREQUAL "get")
    include("${TOOLDIR}/${ARG1}.cmake")
    return()
endif()

#set default arguments
if("${ARG1}" STREQUAL "")
    message("SYNTAX: omega [command] [distribution] [arguments]")
    message("Installed distributions:")

    file(GLOB children RELATIVE ${CMAKE_CURRENT_LIST_DIR}/.. ${CMAKE_CURRENT_LIST_DIR}/../*)
    foreach(child ${children})
        if(IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../${child} AND IS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/../${child}/system)
            set(DISTDIR ${CMAKE_CURRENT_LIST_DIR}/../${child})
            # parse version
            file(STRINGS ${DISTDIR}/include/version.h
                OMEGALIB_VERSION_MAJOR
                REGEX "^#define OMEGA_VERSION_MAJOR.*")

            file(STRINGS ${DISTDIR}/include/version.h
                OMEGALIB_VERSION_MINOR
                REGEX "^#define OMEGA_VERSION_MINOR.*")

            file(STRINGS ${DISTDIR}/include/version.h
                OMEGALIB_VERSION_REVISION
                REGEX "^#define OMEGA_VERSION_REVISION.*")
                
            string(REGEX MATCH "[0-9]+" OMEGALIB_VERSION_MAJOR ${OMEGALIB_VERSION_MAJOR})
            string(REGEX MATCH "[0-9]+" OMEGALIB_VERSION_MINOR ${OMEGALIB_VERSION_MINOR})
            string(REGEX MATCH "[0-9]+" OMEGALIB_VERSION_REVISION ${OMEGALIB_VERSION_REVISION})

            if(${OMEGALIB_VERSION_REVISION} GREATER 0)
                set(OMEGALIB_VERSION ${OMEGALIB_VERSION_MAJOR}.${OMEGALIB_VERSION_MINOR}.${OMEGALIB_VERSION_REVISION})
            else()
                set(OMEGALIB_VERSION ${OMEGALIB_VERSION_MAJOR}.${OMEGALIB_VERSION_MINOR})
            endif()            
        
            message("---- ${child} (version ${OMEGALIB_VERSION})")
        endif()
    endforeach()
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
if(NOT EXISTS "${LOCAL_DIR_NAME}")
    set(URL "https://github.com/uic-evl/omegalib.git")
    execute_process(COMMAND ${GIT_EXECUTABLE}
        clone ${URL} ${LOCAL_DIR_NAME} --recursive)
    execute_process(COMMAND ${GIT_EXECUTABLE}
        checkout ${REMOTE_VERSION_NAME} -q
        WORKING_DIRECTORY ${LOCAL_DIR_NAME})
    execute_process(COMMAND ${GIT_EXECUTABLE}
        submodule update -q 
        WORKING_DIRECTORY ${LOCAL_DIR_NAME})
endif()

# run cmake once to create the build directory
file(MAKE_DIRECTORY ${LOCAL_DIR_NAME}/build)
if(WIN32)
    if("${ARG4}" STREQUAL "x86")
        execute_process(COMMAND ${CMAKE_COMMAND}
            ../ -G "Visual Studio 12 2013"
            WORKING_DIRECTORY ${LOCAL_DIR_NAME}/build)
    else()
        execute_process(COMMAND ${CMAKE_COMMAND}
            ../ -G "Visual Studio 12 2013 Win64"
            WORKING_DIRECTORY ${LOCAL_DIR_NAME}/build)
    endif()
else()
    execute_process(COMMAND ${CMAKE_COMMAND}
        ../ -DCMAKE_BUILD_TYPE="Release"
        WORKING_DIRECTORY ${LOCAL_DIR_NAME}/build)
endif()

set(ARG2 ${LOCAL_DIR_NAME})

# run omega add to add the selected modules
if(EXISTS ${ARG2}/tools/mtools/)
    set(TOOLDIR "${ARG2}/tools/mtools/")
elseif(EXISTS ${ARG2}/etc/mtools/)
    set(TOOLDIR "${ARG2}/etc/mtools/")
endif()

# run omega add to add the selected modules
include("${TOOLDIR}/add.cmake")

# run omega build to build the downloaded version (take away ARG3 since build
# uses it as a configuration name and we just want release)
set(ARG3 "")
include("${TOOLDIR}/build.cmake")
