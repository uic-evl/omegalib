###################################################################################################
# THE OMEGA LIB PROJECT
#-------------------------------------------------------------------------------------------------
# Copyright 2010-2011		Electronic Visualization Laboratory, University of Illinois at Chicago
# Authors:										
#  Alessandro Febretti		febret@gmail.com
#-------------------------------------------------------------------------------------------------
# Copyright (c) 2010-2011, Electronic Visualization Laboratory, University of Illinois at Chicago
# All rights reserved.
# Redistribution and use in source and binary forms, with or without modification, are permitted 
# provided that the following conditions are met:
# 
# Redistributions of source code must retain the above copyright notice, this list of conditions 
# and the following disclaimer. Redistributions in binary form must reproduce the above copyright 
# notice, this list of conditions and the following disclaimer in the documentation and/or other 
# materials provided with the distribution. 
# 
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR 
# IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF MERCHANTABILITY AND 
# FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
# CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL 
# DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE  GOODS OR SERVICES; LOSS OF 
# USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
# WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN 
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
###################################################################################################
set(OMEGA_USE_EXTERNAL_OMICRON false CACHE BOOL "When set to true, use an external build of omicron")

if(OMEGA_USE_EXTERNAL_OMICRON)
	include(${CMAKE_SOURCE_DIR}/external/FindOmicron.cmake)
else()
	set(OMICRON_BASE_DIR ${CMAKE_BINARY_DIR}/omicron)
	set(OMICRON_BINARY_DIR ${OMICRON_BASE_DIR}/omicron)
	set(OMICRON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/omicron)

	ExternalProject_Add(
		omicron
		PREFIX omicron
		DOWNLOAD_COMMAND ""
		UPDATE_COMMAND ""
		INSTALL_COMMAND ""
		SOURCE_DIR ${OMICRON_SOURCE_DIR}
		BINARY_DIR ${OMICRON_BINARY_DIR}
		STAMP_DIR ${OMICRON_BASE_DIR}/stamp
		TMP_DIR ${OMICRON_BASE_DIR}/tmp
		CMAKE_ARGS 
			-DOMICRON_USE_CUSTOM_OUTPUT:BOOL=true
            -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
			# Disable build of omicron examples (they se external projects and look for binary files in the wrong place
			# due to binary file redirection we do here.
			-DOMICRON_BUILD_EXAMPLES:BOOL=false
			
			-DOMICRON_LIBRARY_OUTPUT_DIRECTORY:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY}
			-DOMICRON_ARCHIVE_OUTPUT_DIRECTORY:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY}
			-DOMICRON_RUNTIME_OUTPUT_DIRECTORY:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
			
			-DOMICRON_LIBRARY_OUTPUT_DIRECTORY_DEBUG:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG}
			-DOMICRON_LIBRARY_OUTPUT_DIRECTORY_RELEASE:PATH=${CMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE}
			-DOMICRON_ARCHIVE_OUTPUT_DIRECTORY_DEBUG:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG}
			-DOMICRON_ARCHIVE_OUTPUT_DIRECTORY_RELEASE:PATH=${CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE}
			-DOMICRON_RUNTIME_OUTPUT_DIRECTORY_DEBUG:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}
			-DOMICRON_RUNTIME_OUTPUT_DIRECTORY_RELEASE:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}
            #-DCMAKE_OSX_SYSROOT:PATH=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.7.sdk
            #-DCMAKE_OSX_DEPLOYMENT_TARGET:VAR=10.7
		)

	set_target_properties(omicron PROPERTIES FOLDER "3rdparty")

	# the following are the include directories needed to build a 3rd party omegalib application.
	# in the future, just ${OMICRON_ROOT_DIR}/include will be needed, but for now, multiple paths 
	# have to be specified. If building a project without Cmake, remember to specify ALL these directories
	# as include paths for your compiler.
	set(OMICRON_INCLUDE_DIRS ${OMICRON_BINARY_DIR}/include ${OMICRON_SOURCE_DIR}/include ${OMICRON_SOURCE_DIR}/external/include)
		  
	set(OMICRON_LIB_DIR_RELEASE ${CMAKE_BINARY_DIR}/lib/release)
	set(OMICRON_LIB_DIR_DEBUG ${CMAKE_BINARY_DIR}/lib/debug)
	set(OMICRON_LIB_DIR ${CMAKE_BINARY_DIR}/lib)
	set(OMICRON_BIN_DIR ${CMAKE_BINARY_DIR}/bin)

	###################################################################################################
	# Set the output directories for libraries and binary files
	if(MSVC OR CMAKE_GENERATOR STREQUAL "Xcode")
		# omicron
		set(OMICRON_LIB_DEBUG ${OMICRON_LIB_DIR_DEBUG}/omicron.lib)
		set(OMICRON_LIB_RELEASE ${OMICRON_LIB_DIR_RELEASE}/omicron.lib)
	else()
		if(APPLE)
			set(OMICRON_LIB_DEBUG ${OMICRON_BIN_DIR}/libomicron.dylib)
			set(OMICRON_LIB_RELEASE ${OMICRON_BIN_DIR}/libomicron.dylib)
		else(APPLE)
			set(OMICRON_LIB_DEBUG ${OMICRON_BIN_DIR}/libomicron.so)
			set(OMICRON_LIB_RELEASE ${OMICRON_BIN_DIR}/libomicron.so)
		endif(APPLE)
	endif()

	set(OMICRON_LIB debug ${OMICRON_LIB_DEBUG} optimized ${OMICRON_LIB_RELEASE})

	# On linux, asio depends on pthreads so add it as a dependency.
	if(UNIX)
		set(OMICRON_LIB ${OMICRON_LIB} pthread)
	endif(UNIX)
endif()
