find_package(OpenGL REQUIRED)

# fix for OSX 10.9
if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libstdc++")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -stdlib=libstdc++")
endif()

# Detect whether we are setting up omegalib for a build or an 
# install environment. Build environments always have a cmake 
# cache file, so look for it.
set(INSTALL_ENVIRONMENT true)
if(EXISTS ${Omegalib_DIR}/CMakeCache.txt)
	message(STATUS "Using an omegalib BUILD environment")
	set(INSTALL_ENVIRONMENT false)
else()
	message(STATUS "Using an omegalib INSTALL environment")
	# Adjust several config variables to work with an install environment

	# fix OpenSceneGraph libs
	string(REPLACE ${OMEGA_BINARY_DIR} ${Omegalib_DIR} OSG_LIBS "${OSG_LIBS}")
	# fix osgWorks libs	
	string(REPLACE ${Omegalib_DIR}/modules/omegaOsg/osgWorks-prefix/src/osgWorks-build/lib ${Omegalib_DIR}/lib OSG_LIBS "${OSG_LIBS}")
	#message("${OSG_LIBS}")
	# fix osg include dir
	string(REPLACE ${OMEGA_BINARY_DIR} ${Omegalib_DIR} OSG_INCLUDES "${OSG_INCLUDES}")
	#message("${OSG_INCLUDES}")

	#Finally, replace OMEGA_BINARY_DIR with Omegalib_DIR
	set(OMEGA_BINARY_DIR ${Omegalib_DIR})
	set(OMEGA_SOURCE_DIR ${Omegalib_DIR})
endif()

if(OMEGA_BINARY_DIR)
	set(OMICRON_DEFAULT_BINARY_DIR ${OMEGA_BINARY_DIR}/src/omicron/omicron)
	#set(OMICRON_BIN_DIR ${OMEGA_BINARY_DIR}/bin)
	#set(OMICRON_LIB_DIR ${OMEGA_BINARY_DIR}/lib)
	include(${OMEGA_SOURCE_DIR}/src/omicron/CMakeModules/FindOmicron.cmake)

	# the following are the include directories needed to build a 3rd party omegalib application.
	# in the future, just ${OMEGA_ROOT_DIR}/include will be needed, but for now, multiple paths 
	# have to be specified. If building a project without Cmake, remember to specify ALL these directories
	# as include paths for your compiler.
	set(OMEGA_INCLUDE_DIRS 
		${OMICRON_INCLUDE_DIRS} 
		${OMEGA_BINARY_DIR}/include 
		${OMEGA_SOURCE_DIR}/include 
		${OMEGA_SOURCE_DIR}/src/glew 
		${OMEGA_SOURCE_DIR}/external/include  
		${OMEGA_SOURCE_DIR}/modules
		${OSG_INCLUDES} 
		${PYTHON_INCLUDES})

	# No debug libs in an install environment
	if(INSTALL_ENVIRONMENT)
		set(OMEGA_LIB_DIR_RELEASE ${OMEGA_BINARY_DIR}/lib)
		set(OMEGA_LIB_DIR_DEBUG ${OMEGA_BINARY_DIR}/lib)
	else()
		set(OMEGA_LIB_DIR_RELEASE ${OMEGA_BINARY_DIR}/lib/release)
		set(OMEGA_LIB_DIR_DEBUG ${OMEGA_BINARY_DIR}/lib/debug)
	endif()
	set(OMEGA_LIB_DIR ${OMEGA_BINARY_DIR}/lib)

	# No debug binaries in an install environment
	if(INSTALL_ENVIRONMENT)
		set(OMEGA_BIN_DIR_RELEASE ${OMEGA_BINARY_DIR}/bin)
		set(OMEGA_BIN_DIR_DEBUG ${OMEGA_BINARY_DIR}/bin)
	else()
		set(OMEGA_BIN_DIR_RELEASE ${OMEGA_BINARY_DIR}/bin/release)
		set(OMEGA_BIN_DIR_DEBUG ${OMEGA_BINARY_DIR}/bin/debug)
	endif()
	set(OMEGA_BIN_DIR ${OMEGA_BINARY_DIR}/bin)

	###################################################################################################
	# Set the output directories for libraries and binary files
	if(MSVC)
		# Since visual studio and Xcode builds are multiconfiguration, set two separate directories for debug and release builds
		
		set(OMICRON_LIB_DEBUG ${OMEGA_LIB_DIR_DEBUG}/omicron.lib)
		set(OMICRON_LIB_RELEASE ${OMEGA_LIB_DIR_RELEASE}/omicron.lib)
		
		# omega
		set(OMEGA_LIB_DEBUG ${OMEGA_LIB_DIR_DEBUG}/omega.lib)
		set(OMEGA_LIB_RELEASE ${OMEGA_LIB_DIR_RELEASE}/omega.lib)
		
		# omegaToolkit
		set(OTK_LIB_DEBUG ${OMEGA_LIB_DIR_DEBUG}/omegaToolkit.lib)
		set(OTK_LIB_RELEASE ${OMEGA_LIB_DIR_RELEASE}/omegaToolkit.lib)
		
		# omegaOsg
		set(OOSG_LIB_DEBUG ${OMEGA_LIB_DIR_DEBUG}/omegaOsg.lib)
		set(OOSG_LIB_RELEASE ${OMEGA_LIB_DIR_RELEASE}/omegaOsg.lib)
		
		# omegaVtk
		set(OVTK_LIB_DEBUG ${OMEGA_LIB_DIR_DEBUG}/omegaVtk.lib)
		set(OVTK_LIB_RELEASE ${OMEGA_LIB_DIR_RELEASE}/omegaVtk.lib)
		
		# cyclops
		set(CY_LIB_DEBUG ${OMEGA_LIB_DIR_DEBUG}/cyclops.lib)
		set(CY_LIB_RELEASE ${OMEGA_LIB_DIR_RELEASE}/cyclops.lib)
		
	elseif(UNIX)
		if(APPLE) 
			set(OMICRON_LIB_DEBUG ${OMEGA_BIN_DIR}/libomicron.dylib)
			set(OMICRON_LIB_RELEASE ${OMEGA_BIN_DIR}/libomicron.dylib)
			
			# omega
			set(OMEGA_LIB_DEBUG ${OMEGA_BIN_DIR}/libomega.dylib)
			set(OMEGA_LIB_RELEASE ${OMEGA_BIN_DIR}/libomega.dylib)
			
			# omegaToolkit
			set(OTK_LIB_DEBUG ${OMEGA_BIN_DIR}/libomegaToolkit.dylib)
			set(OTK_LIB_RELEASE ${OMEGA_BIN_DIR}/libomegaToolkit.dylib)
			
			# oosg
			set(OOSG_LIB_DEBUG ${OMEGA_BIN_DIR}/libomegaOsg.dylib)
			set(OOSG_LIB_RELEASE ${OMEGA_BIN_DIR}/libomegaOsg.dylib)
			
			# oosg
			set(OVTK_LIB_DEBUG ${OMEGA_BIN_DIR}/libomegaVtk.dylib)
			set(OVTK_LIB_RELEASE ${OMEGA_BIN_DIR}/libomegaVtk.dylib)
			
			# cyclops
			set(CY_LIB_DEBUG ${OMEGA_BIN_DIR}/libcyclops.dylib)
			set(CY_LIB_RELEASE ${OMEGA_BIN_DIR}/libcyclops.dylib)
			
		else() # generic linux
			set(OMICRON_LIB_DEBUG ${OMEGA_BIN_DIR}/libomicron.so)
			set(OMICRON_LIB_RELEASE ${OMEGA_BIN_DIR}/libomicron.so)
			
			# omega
			set(OMEGA_LIB_DEBUG ${OMEGA_BIN_DIR}/libomega.so)
			set(OMEGA_LIB_RELEASE ${OMEGA_BIN_DIR}/libomega.so)
			
			# omegaToolkit
			set(OTK_LIB_DEBUG ${OMEGA_BIN_DIR}/libomegaToolkit.so)
			set(OTK_LIB_RELEASE ${OMEGA_BIN_DIR}/libomegaToolkit.so)
			
			# oosg
			set(OOSG_LIB_DEBUG ${OMEGA_BIN_DIR}/libomegaOsg.so)
			set(OOSG_LIB_RELEASE ${OMEGA_BIN_DIR}/libomegaOsg.so)
			
			# oosg
			set(OVTK_LIB_DEBUG ${OMEGA_BIN_DIR}/libomegaVtk.so)
			set(OVTK_LIB_RELEASE ${OMEGA_BIN_DIR}/libomegaVtk.so)
			
			# cyclops
			set(CY_LIB_DEBUG ${OMEGA_BIN_DIR}/libcyclops.so)
			set(CY_LIB_RELEASE ${OMEGA_BIN_DIR}/libcyclops.so)
		endif()
	endif()

	#set(OMEGA_LIB debug ${OMEGA_LIB_DEBUG} debug ${OPENGL_LIBRARY} debug ${GLEW_LIB_DEBUG} debug ${LIBCONFIG_LIB_DEBUG} optimized ${OMEGA_LIB_RELEASE} optimized ${GLEW_LIB_RELEASE} optimized ${OPENGL_LIBRARY} optimized ${LIBCONFIG_LIB_RELEASE})
	set(OMEGA_LIB debug ${OMICRON_LIB_DEBUG} optimized ${OMICRON_LIB_RELEASE} debug ${OMEGA_LIB_DEBUG} debug ${OPENGL_LIBRARY} optimized ${OMEGA_LIB_RELEASE} optimized ${OPENGL_LIBRARY})
	
	# add pthreads dependency for linux build
	if(CMAKE_GENERATOR STREQUAL "Unix Makefiles")
		set(OMEGA_LIB ${OMEGA_LIB} pthread)
	endif()
	
	set(OMEGA_TOOLKIT_LIB debug ${OTK_LIB_DEBUG} optimized ${OTK_LIB_RELEASE})
	set(OMEGA_OSG_LIB debug ${OOSG_LIB_DEBUG} optimized ${OOSG_LIB_RELEASE} ${OSG_LIBS})
	set(OMEGA_VTK_LIB debug ${OVTK_LIB_DEBUG} optimized ${OVTK_LIB_RELEASE})
	set(CYCLOPS_LIB debug ${CY_LIB_DEBUG} optimized ${CY_LIB_RELEASE})

	###################################################################################################
	# Visual studio specific options.
	if(MSVC)
		# Exclude libcmt when linking in visual studio
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib")
		set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib")
		add_definitions(-D_CRT_SECURE_NO_WARNINGS /wd4244 /wd4018)
	endif(MSVC)
endif(OMEGA_BINARY_DIR)

