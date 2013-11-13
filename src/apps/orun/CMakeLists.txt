set(LIBS omega omegaToolkit)

# If we have vtk support link against vtk lib
if(OMEGA_BUILD_VTK_LIB)
	set(LIBS ${LIBS} omegaVtk)
endif()

# If we have osg support link against osg and cyclops lib
if(MODULES_cyclops)
	set(LIBS ${LIBS} omegaOsg cyclops)
	include_directories(${OSG_INCLUDES})
endif()

add_executable(orun 
	orun.cpp)

if(MODULES_cyclops)
  add_dependencies(orun cyclops)
endif()

set_target_properties(orun PROPERTIES FOLDER apps)

target_link_libraries(orun ${LIBS})
if(OMEGA_ENABLE_AUTO_UPDATE)
	target_link_libraries(orun ${CMAKE_SOURCE_DIR}/external/libs32/WinSparkle.lib)
endif()
