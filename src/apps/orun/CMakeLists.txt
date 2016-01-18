set(LIBS omega omegaToolkit)

# If we have osg support link against osg and cyclops lib
if(MODULES_cyclops)
    set(LIBS ${LIBS} omegaOsg cyclops)
    include_directories(${OSG_INCLUDES})
endif()

if(OMEGA_OS_WIN)
    add_executable(orun orun.cpp icon.rc)
else()
    add_executable(orun orun.cpp)
endif()

set_target_properties(orun PROPERTIES FOLDER apps)

target_link_libraries(orun ${LIBS})
