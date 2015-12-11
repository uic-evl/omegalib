# Download an update to this and the rest of the opm scripts.
set(OPM_URL "http://omegalib.s3.amazonaws.com/maintenance-utils")

find_package(Git REQUIRED QUIET)

file(DOWNLOAD ${OPM_URL}/omega.cmake ./cmake/omega.cmake)

# If this is the firts execution, run an utils.update
if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/tools.update.cmake")
    file(DOWNLOAD ${OPM_URL}/tools.update.cmake ./cmake/tools.update.cmake)
    include("${CMAKE_CURRENT_LIST_DIR}/tools.update.cmake")
endif()

# invoke a script
if("${ARG1}" STREQUAL "")
    message("Welcome to the omegalib maintenance utilities")
    message("SYNTAX: omega [get|build|add|update|pack|utils]")
    message("  Type omega followed by one of the supported commands to get help")
    message("  for that command.")
    message("  If you just want to quickly install omegalib type:")
    message("  > omega get master common-modules")
else()
        include("${CMAKE_CURRENT_LIST_DIR}/${ARG1}.cmake")
endif()
