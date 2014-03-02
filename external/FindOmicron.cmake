###################################################################################################
# THE OMICRON PROJECT
#-------------------------------------------------------------------------------------------------
# Copyright 2010-2012		Electronic Visualization Laboratory, University of Illinois at Chicago
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
if(COMMAND cmake_policy)
      cmake_policy(SET CMP0003 NEW)
      #cmake_policy(SET CMP0008 NEW)
endif(COMMAND cmake_policy)

set(OMICRON_BINARY_DIR ${OMICRON_DEFAULT_BINARY_DIR} CACHE PATH "Path of the omegalib bin directory (the one containing the include, bin and lib folders")

if(OMICRON_BINARY_DIR)
	include(${OMICRON_BINARY_DIR}/UseOmicron.cmake)

	# the following are the include directories needed to build a 3rd party omegalib application.
	# in the future, just ${OMICRON_ROOT_DIR}/include will be needed, but for now, multiple paths 
	# have to be specified. If building a project without Cmake, remember to specify ALL these directories
	# as include paths for your compiler.
	set(OMICRON_INCLUDE_DIRS ${OMICRON_BINARY_DIR}/include ${OMICRON_SOURCE_DIR}/include ${OMICRON_SOURCE_DIR}/external/include)
	
	if(OMICRON_LIB_DIR)
		set(OMICRON_LIB_DIR_RELEASE ${OMICRON_LIB_DIR}/release)
		set(OMICRON_LIB_DIR_DEBUG ${OMICRON_LIB_DIR}/debug)
	else()
		set(OMICRON_LIB_DIR_RELEASE ${OMICRON_BINARY_DIR}/lib/release)
		set(OMICRON_LIB_DIR_DEBUG ${OMICRON_BINARY_DIR}/lib/debug)
		set(OMICRON_LIB_DIR ${OMICRON_BINARY_DIR}/lib)
	endif()

	if(OMICRON_BIN_DIR)
		set(OMICRON_BIN_DIR_RELEASE ${OMICRON_BIN_DIR}/release)
		set(OMICRON_BIN_DIR_DEBUG ${OMICRON_BIN_DIR}/debug)
	else()
		set(OMICRON_BIN_DIR_RELEASE ${OMICRON_BINARY_DIR}/bin/release)
		set(OMICRON_BIN_DIR_DEBUG ${OMICRON_BINARY_DIR}/bin/debug)
		set(OMICRON_BIN_DIR ${OMICRON_BINARY_DIR}/bin)
	endif()

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
			# omicron
			set(OMICRON_LIB_DEBUG ${OMICRON_BIN_DIR}/libomicron.so)
			set(OMICRON_LIB_RELEASE ${OMICRON_BIN_DIR}/libomicron.so)
		endif()
	endif()

	set(OMICRON_LIB debug ${OMICRON_LIB_DEBUG} optimized ${OMICRON_LIB_RELEASE})
	
	# On linux, asio depends on pthreads so add it as a dependency.
	if(UNIX)
		set(OMICRON_LIB ${OMICRON_LIB} pthread)
	endif(UNIX)
	
	###################################################################################################
	# Visual studio specific options.
	if(MSVC)
		# Exclude libcmt when linking in visual studio
		set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib")
		set(CMAKE_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} /NODEFAULTLIB:libcmt.lib")
		add_definitions(-D_CRT_SECURE_NO_WARNINGS /wd4244 /wd4018)
	endif(MSVC)
endif(OMICRON_BINARY_DIR)

