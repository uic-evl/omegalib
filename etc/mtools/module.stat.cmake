set(MODDIR ${CMAKE_CURRENT_LIST_DIR}/../../modules)
file(GLOB children RELATIVE ${MODDIR} ${MODDIR}/*)
foreach(child ${children})
    if(IS_DIRECTORY ${MODDIR}/${child} AND IS_DIRECTORY ${MODDIR}/${child}/.git)
        message("---- ${child}")
        execute_process(COMMAND ${GIT_EXECUTABLE} status -s -uno
            WORKING_DIRECTORY ${MODDIR}/${child})
    endif()
endforeach()