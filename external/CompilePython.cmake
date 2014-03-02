ExternalProject_Add(Python
	DOWNLOAD_DIR ${CMAKE_BINARY_DIR}/python
	URL "http://www.python.org/ftp/python/2.7.3/Python-2.7.3.tgz"
	UPDATE_COMMAND ""
	CONFIGURE_COMMAND <SOURCE_DIR>/configure --with-universal-archs=intel --enable-universalsdk --enable-shared
	BUILD_COMMAND "make"
	INSTALL_COMMAND ""
)

#file(COPY ${CMAKE_BINARY_DIR}/src/omega/Python-prefix/src/Python-build/pyconfig.h DESTINATION ${CMAKE_BINARY_DIR}/src/omega/Python-prefix/src/Python/Include)
set(PYTHON_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/src/Python-prefix/src/Python/Include ${CMAKE_BINARY_DIR}/src/Python-prefix/src/Python-build)

#set(PYTHON_INCLUDE_DIRS ${CMAKE_BINARY_DIR}/src/Python-prefix/src/Python-build)
set(PYTHON_LIBRARIES ${CMAKE_BINARY_DIR}/src/Python-prefix/src/Python-build/libpython2.7.dylib)
