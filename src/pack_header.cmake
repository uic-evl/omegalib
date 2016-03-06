#-------------------------------------------------------------------------------
# Helper macro to setup installer packages
macro(setup_package)
    set(PACKAGE_ROOT_DIR @CMAKE_INSTALL_PREFIX@/packages/${PACKAGE_NAME})
    set(PACKAGE_DIR ${PACKAGE_ROOT_DIR}/data)
    file(REMOVE_RECURSE @CMAKE_INSTALL_PREFIX@/packages/${PACKAGE_NAME})
    file(MAKE_DIRECTORY @CMAKE_INSTALL_PREFIX@/packages/${PACKAGE_NAME}/data)
    file(MAKE_DIRECTORY @CMAKE_INSTALL_PREFIX@/packages/${PACKAGE_NAME}/meta)
    configure_file(${PACKAGE_CONFIG_TEMPLATE} ${PACKAGE_ROOT_DIR}/meta/package.xml)
endmacro()

#set the default configuration for packages
set(PACK_EXAMPLES false CACHE INTERNAL "")
set(PACK_CORE_UI true CACHE INTERNAL "")

#-------------------------------------------------------------------------------
# pack instructions for the omegalib core.
# Copy over some variables, they will be substituted by configure_file.
# Needed since this file will be executed from outside the main omegalib cmake
# run.
set(PACKAGE_ROOT_DIR @CMAKE_INSTALL_PREFIX@/packages/core)
set(PACKAGE_DIR @CMAKE_INSTALL_PREFIX@/packages/core/data)
set(OMEGALIB_VERSION @OMEGALIB_VERSION@)
set(BUILD_DIR @CMAKE_BINARY_DIR@)
set(SOURCE_DIR @CMAKE_SOURCE_DIR@)

# needed to avoid wrong substitutions in installer files
set(RootDir "RootDir")
set(ApplicationsDir "ApplicationsDir")

set(PACKAGE_CONFIG_TEMPLATE ${PACKAGE_ROOT_DIR}/meta/package.xml.in)

if(WIN32)
    set(BIN_DIR ${BUILD_DIR}/bin/release)
else()
    set(BIN_DIR ${BUILD_DIR}/bin)
endif()

# Save the current date to a variable. Will be used during packaging.
string(TIMESTAMP BUILD_DATE "%Y-%m-%d")

if(WIN32)
	set(REPOSITORY_LOCATION "release/windows")
elseif(APPLE)
	set(REPOSITORY_LOCATION "release/osx")
endif()

file(REMOVE_RECURSE ${PACKAGE_DIR})
file(MAKE_DIRECTORY ${PACKAGE_DIR})

configure_file(
    @CMAKE_INSTALL_PREFIX@/config/config-offline.xml.in 
    @CMAKE_INSTALL_PREFIX@/config/config-offline.xml)

configure_file(
    @CMAKE_INSTALL_PREFIX@/config/config-online.xml.in 
    @CMAKE_INSTALL_PREFIX@/config/config-online.xml)
    
configure_file(
    ${PACKAGE_ROOT_DIR}/meta/core-package.xml.in 
    ${PACKAGE_ROOT_DIR}/meta/package.xml)
