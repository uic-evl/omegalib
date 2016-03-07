if(WIN32)
	set(EXTLIB_NAME python)
	set(EXTLIB_TGZ ${CMAKE_BINARY_DIR}/${EXTLIB_NAME}.tar.gz)
	set(EXTLIB_DIR ${CMAKE_SOURCE_DIR}/modules/${EXTLIB_NAME})
    
    if(OMEGA_ARCH_32)
        set(EXTLIB_URL http://omegalib.s3.amazonaws.com/python/python-windows-x86.tar.gz)
    else()
        set(EXTLIB_URL http://omegalib.s3.amazonaws.com/python/python-windows-x64.tar.gz)
    endif()
    
    
	if(NOT EXISTS ${EXTLIB_DIR})
  message(STATUS "Downloading Python...")
  file(DOWNLOAD ${EXTLIB_URL} ${EXTLIB_TGZ} SHOW_PROGRESS)
  execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
		${EXTLIB_TGZ} WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}/modules)
	endif(NOT EXISTS ${EXTLIB_DIR})
endif()
