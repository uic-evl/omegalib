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
# Equalizer support enabled: uncompress and prepare the external project.
if(APPLE)
    ExternalProject_Add(
		equalizer
		URL ${CMAKE_SOURCE_DIR}/external/equalizer.tar.gz
		#CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}
		CMAKE_ARGS 
			-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}
			-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}
            -DCMAKE_OSX_SYSROOT:PATH=/Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX${CURRENT_OSX_VERSION}.sdk
            -DCMAKE_OSX_DEPLOYMENT_TARGET:VAR=${CURRENT_OSX_VERSION}
			-DEQUALIZER_PREFER_AGL:BOOL=OFF
			-DEQUALIZER_USE_CUDA:BOOL=OFF
			-DEQUALIZER_USE_BOOST:BOOL=OFF
			INSTALL_COMMAND ""
            PATCH_COMMAND patch -p1 < ${CMAKE_SOURCE_DIR}/external/equalizer.${CURRENT_OSX_VERSION}.patch
		)
else(APPLE)
	ExternalProject_Add(
		equalizer
		URL ${CMAKE_SOURCE_DIR}/external/equalizer.tar.gz
		#CMAKE_ARGS -DCMAKE_INSTALL_PREFIX:PATH=${CMAKE_BINARY_DIR}
		CMAKE_ARGS 
			-DEQUALIZER_USE_CUDA:BOOL=OFF
			-DEQUALIZER_USE_BOOST:BOOL=OFF
			-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}
			-DCMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}
			-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_DEBUG:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG}
			-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_RELEASE:PATH=${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE}
			INSTALL_COMMAND ""
			#PATCH_COMMAND patch < ${CMAKE_SOURCE_DIR}/external/equalizer.patch
		)
endif(APPLE)

set_target_properties(equalizer PROPERTIES FOLDER "3rdparty")
# define path to libraries built by the equalizer external project
set(EQUALIZER_BINARY_DIR ${CMAKE_BINARY_DIR}/src/omega/equalizer-prefix/src/equalizer-build)
# NEED SECTIONS DEPENDENT ON BUILD TOOL, NOT OS!
if(WIN32)
	set(EQUALIZER_EQ_LIB_DEBUG ${EQUALIZER_BINARY_DIR}/libs/client/Debug/Equalizer.lib)
	set(EQUALIZER_CO_LIB_DEBUG ${EQUALIZER_BINARY_DIR}/libs/collage/Debug/Collage.lib)
	set(EQUALIZER_LIBS_DEBUG debug ${EQUALIZER_EQ_LIB_DEBUG} debug ${EQUALIZER_CO_LIB_DEBUG})
	set(EQUALIZER_EQ_LIB_RELEASE ${EQUALIZER_BINARY_DIR}/libs/client/Release/Equalizer.lib)
	set(EQUALIZER_CO_LIB_RELEASE ${EQUALIZER_BINARY_DIR}/libs/collage/Release/Collage.lib)
	set(EQUALIZER_LIBS_RELEASE optimized ${EQUALIZER_EQ_LIB_RELEASE} optimized ${EQUALIZER_CO_LIB_RELEASE})
	# install(
		# FILES ${EQUALIZER_CLIENT_LIBS_DEBUG}
		# DESTINATION lib/Debug
		# COMPONENT omegalib
	# )
	# install(
		# FILES ${EQUALIZER_CLIENT_LIBS_RELEASE}
		# DESTINATION lib/Debug
		# COMPONENT omegalib
	# )
else(WIN32)
	if(APPLE)
		set(EQUALIZER_EQ_LIB_DEBUG ${EQUALIZER_BINARY_DIR}/libs/client/libEqualizer.dylib)
		set(EQUALIZER_CO_LIB_DEBUG ${EQUALIZER_BINARY_DIR}/libs/collage/libCollage.dylib)
		install(DIRECTORY ${EQUALIZER_BINARY_DIR}/libs/client/ DESTINATION omegalib/bin FILES_MATCHING PATTERN "*.dylib")
		install(DIRECTORY ${EQUALIZER_BINARY_DIR}/libs/server/ DESTINATION omegalib/bin FILES_MATCHING PATTERN "*.dylib")
		install(DIRECTORY ${EQUALIZER_BINARY_DIR}/libs/collage/ DESTINATION omegalib/bin FILES_MATCHING PATTERN "*.dylib")
	else(APPLE)
		set(EQUALIZER_EQ_LIB_DEBUG ${EQUALIZER_BINARY_DIR}/libs/client/libEqualizer.so)
		set(EQUALIZER_CO_LIB_DEBUG ${EQUALIZER_BINARY_DIR}/libs/collage/libCollage.so)
		install(DIRECTORY ${EQUALIZER_BINARY_DIR}/libs/client/ DESTINATION omegalib/bin FILES_MATCHING PATTERN "*.so*")
		install(DIRECTORY ${EQUALIZER_BINARY_DIR}/libs/server/ DESTINATION omegalib/bin FILES_MATCHING PATTERN "*.so*")
		install(DIRECTORY ${EQUALIZER_BINARY_DIR}/libs/collage/ DESTINATION omegalib/bin FILES_MATCHING PATTERN "*.so*")
	endif(APPLE)	
	set(EQUALIZER_LIBS_DEBUG ${EQUALIZER_EQ_LIB_DEBUG} ${EQUALIZER_CO_LIB_DEBUG})
	set(EQUALIZER_LIBS_RELEASE ${EQUALIZER_EQ_LIB_DEBUG} ${EQUALIZER_CO_LIB_DEBUG})
endif(WIN32)
set(EQUALIZER_LIBS ${EQUALIZER_LIBS_DEBUG} ${EQUALIZER_LIBS_RELEASE})
set(EQUALIZER_INCLUDES ${CMAKE_BINARY_DIR}/src/omega/equalizer-prefix/src/equalizer-build/include)
