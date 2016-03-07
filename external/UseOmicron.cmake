set(OMEGA_USE_EXTERNAL_OMICRON false CACHE BOOL "When set to true, use an external build of omicron")

set(OMICRON_BASE_DIR ${CMAKE_BINARY_DIR}/src/omicron)
set(OMICRON_BINARY_DIR ${OMICRON_BASE_DIR}/omicron)
set(OMICRON_SOURCE_DIR ${CMAKE_SOURCE_DIR}/src/omicron)

if(OMEGA_TOOL_VS12 AND OMEGA_ARCH_32)
    set(OMICRON_USE_VRPN true)
else()
    set(OMICRON_USE_VRPN false)
endif()

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
        -DCMAKE_MACOSX_RPATH=${CMAKE_MACOSX_RPATH}
        -DCMAKE_INSTALL_RPATH=${CMAKE_INSTALL_RPATH}
        -DCMAKE_BUILD_WITH_INSTALL_RPATH=${CMAKE_BUILD_WITH_INSTALL_RPATH}
        -DOMICRON_USE_CUSTOM_OUTPUT:BOOL=true
        -DCMAKE_CXX_FLAGS:STRING=${CMAKE_CXX_FLAGS}
        # Disable build of omicron examples (they se external projects and look for binary files in the wrong place
        # due to binary file redirection we do here.
        -DOMICRON_BUILD_EXAMPLES:BOOL=false
        -DOMICRON_USE_VRPN:BOOL=${OMICRON_USE_VRPN}
        
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
if(MSVC)
    # omicron
    set(OMICRON_LIB_DEBUG ${OMICRON_LIB_DIR_DEBUG}/omicron.lib)
    set(OMICRON_LIB_RELEASE ${OMICRON_LIB_DIR_RELEASE}/omicron.lib)
else()
    if(APPLE)
        if(CMAKE_GENERATOR STREQUAL "Xcode")
            set(OMICRON_LIB_DEBUG ${OMICRON_BIN_DIR}/debug/libomicron.dylib)
            set(OMICRON_LIB_RELEASE ${OMICRON_BIN_DIR}/release/libomicron.dylib)
        else(CMAKE_GENERATOR STREQUAL "Xcode")
            set(OMICRON_LIB_DEBUG ${OMICRON_BIN_DIR}/libomicron.dylib)
            set(OMICRON_LIB_RELEASE ${OMICRON_BIN_DIR}/libomicron.dylib)
        endif(CMAKE_GENERATOR STREQUAL "Xcode")
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
