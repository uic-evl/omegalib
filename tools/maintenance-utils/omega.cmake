# Download an update to this and the rest of the opm scripts.
set(OPM_URL "http://omegalib.s3.amazonaws.com/maintenance-utils")

find_package(Git REQUIRED QUIET)

set(SCRIPTS "omega;get;build;add;update")
foreach(S IN LISTS SCRIPTS)
    file(DOWNLOAD ${OPM_URL}/${S}.cmake ./cmake/${S}.cmake)
endforeach()

# invoke a script
if("${ARG1}" STREQUAL "")
    message("Welcome to the omegalib maintenance utility")
    message("SYNTAX: omega [get|build|add|update]")
    message("  Type omega followed by one of the supported commands to get help")
    message("  for that command.")
    message("  If you just want to quickly install omegalib type:")
    message("  > omega get master common-modules")
else()
        include("${CMAKE_CURRENT_LIST_DIR}/${ARG1}.cmake")
endif()
