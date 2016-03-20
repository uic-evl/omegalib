set(LIBS omega omegaToolkit)

if(OMEGA_OS_WIN)
    add_executable(orun orun.cpp icon.rc)
else()
    add_executable(orun orun.cpp)
endif()

set_target_properties(orun PROPERTIES FOLDER apps)

target_link_libraries(orun ${LIBS})
