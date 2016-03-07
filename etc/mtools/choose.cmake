#set default arguments
if("${ARG2}" STREQUAL "")
    return()
endif()

set(CHOOSE_FILE ${CMAKE_CURRENT_LIST_DIR}/../../../choose_${ARG2}.bat)

file(REMOVE ${CHOOSE_FILE})
file(APPEND ${CHOOSE_FILE} "set OMEGA_HOME=${CMAKE_CURRENT_LIST_DIR}/../../\n")
file(APPEND ${CHOOSE_FILE} "set PATH=${CMAKE_CURRENT_LIST_DIR}/../../build/bin/release;%PATH%\n")

message("Choose script prepared")
message("call choose_${ARG2}.bat to switch your omegalib environment to ${ARG2}")
