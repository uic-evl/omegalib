# Download and build GLFW
set(GLFW_BASE_DIR ${CMAKE_BINARY_DIR}/3rdparty/GLFW)
set(GLFW_INCLUDE_DIR ${GLFW_BASE_DIR}/source/include)
ExternalProject_Add(
	GLFW
	URL http://github.com/omega-hub/glfw/archive/3.1.1.tar.gz
	CMAKE_ARGS 
		-DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
		-DGLFW_BUILD_DOCS=False
		-DGLFW_BUILD_EXAMPLES=False
		-DGLFW_BUILD_TESTS=False
		-DGLFW_INSTALL=False
		-DGLFW_USE_RETINA=False
	INSTALL_COMMAND ""
	# directories
	TMP_DIR ${CMAKE_BINARY_DIR}/3rdparty/tmp
	STAMP_DIR ${CMAKE_BINARY_DIR}/3rdparty/stamp
	DOWNLOAD_DIR ${GLFW_BASE_DIR}
	SOURCE_DIR ${GLFW_BASE_DIR}/source
	BINARY_DIR ${GLFW_BASE_DIR}/build
	)
set_target_properties(GLFW PROPERTIES FOLDER "3rdparty")
if(WIN32)
	set(GLFW_LIBS 
		debug ${GLFW_BASE_DIR}/build/src/Debug/glfw3.lib
		optimized ${GLFW_BASE_DIR}/build/src/Release/glfw3.lib
	)
elseif(CMAKE_GENERATOR STREQUAL "Xcode")
	set(GLFW_LIBS 
		debug ${GLFW_BASE_DIR}/build/src/Debug/libglfw3.a
		optimized ${GLFW_BASE_DIR}/build/src/Release/libglfw3.a
	)
elseif(APPLE)
	set(GLFW_LIBS ${GLFW_BASE_DIR}/build/src/libglfw3.a)
else()
	set(GLFW_LIBS ${GLFW_BASE_DIR}/build/src/libglfw3.a -lXxf86vm -lXrandr -lXinerama -lXi -lXcursor)
endif()

# build displaySystem_GLFW
add_library(displaySystem_GLFW SHARED displaySystem_GLFW.cpp)
include_directories(${GLFW_INCLUDE_DIR})
target_link_libraries(displaySystem_GLFW omega ${GLFW_LIBS})
add_dependencies(displaySystem_GLFW GLFW)

if(APPLE)
	set_target_properties(displaySystem_GLFW PROPERTIES LINK_FLAGS "-framework Cocoa -framework OpenGL -framework IOKit -framework CoreFoundation -framework CoreVideo")
endif()
