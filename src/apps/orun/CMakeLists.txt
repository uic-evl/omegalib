set(LIBS omega omegaToolkit)

# If we have vtk support link against vtk lib
if(MODULES_omegaVtk)
	set(LIBS ${LIBS} omegaVtk)
	include_directories(${VTK_INCLUDES})
endif()

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

if(MODULES_cyclops)
  add_dependencies(orun cyclops)
endif()

if(MODULES_omegaVtk)
  add_dependencies(orun omegaVtk)
endif()

set_target_properties(orun PROPERTIES FOLDER apps)

target_link_libraries(orun ${LIBS})
if(OMEGA_ENABLE_AUTO_UPDATE)
	target_link_libraries(orun ${CMAKE_SOURCE_DIR}/external/libs32/WinSparkle.lib)
endif()
