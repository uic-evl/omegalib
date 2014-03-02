include(ExternalProject)

if(OMEGA_OS_WIN)
	set(EXTLIB_NAME python-win32)
elseif(OMEGA_OS_LINUX)
	if(OMEGA_ARCH_32)
		set(EXTLIB_NAME python-linux-x86)
	else(OMEGA_ARCH_32)
		set(EXTLIB_NAME python-linux-x64)
	endif(OMEGA_ARCH_32)
endif(OMEGA_OS_WIN)

set(EXTLIB_TGZ ${CMAKE_BINARY_DIR}/${EXTLIB_NAME}.tar.gz)
set(EXTLIB_DIR ${CMAKE_BINARY_DIR}/${EXTLIB_NAME})
set(PYTHON_BINARY_DIR ${CMAKE_BINARY_DIR}/${EXTLIB_NAME})

if(NOT EXISTS ${CMAKE_BINARY_DIR}/${EXTLIB_NAME}.tar.gz)
  message(STATUS "Downloading Python binary archive...")
  file(DOWNLOAD http://omegalib.googlecode.com/files/${EXTLIB_NAME}.tar.gz ${EXTLIB_TGZ} SHOW_PROGRESS)
endif(NOT EXISTS ${CMAKE_BINARY_DIR}/${EXTLIB_NAME}.tar.gz)

if(NOT EXISTS ${EXTLIB_DIR})
  message(STATUS "Extracting Python...")
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzfh
    ${EXTLIB_TGZ} WORKING_DIRECTORY ${CMAKE_BINARY_DIR})
endif(NOT EXISTS ${EXTLIB_DIR})

set(PYTHON_INCLUDES ${EXTLIB_DIR}/include)

if(OMEGA_OS_WIN)
	set(PYTHON_LIBS optimized ${EXTLIB_DIR}/libs/python3.lib debug ${EXTLIB_DIR}/libs/python3.lib)
	
	# Copy the dlls into the target directories
	file(COPY ${EXTLIB_DIR}/DLLs DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_DEBUG} PATTERN "*.dll")
	file(COPY ${EXTLIB_DIR}/DLLs DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY_RELEASE} PATTERN "*.dll")

elseif(OMEGA_OS_LINUX)
	set(PYTHON_LIBS optimized ${EXTLIB_DIR}/libs/libpython32.a debug ${EXTLIB_DIR}/libs/libpython32.a)
endif(OMEGA_OS_WIN)
