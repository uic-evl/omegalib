#-------------------------------------------------------------------------------
# create the examples package
set(PACKAGE_NAME core.examples)
set(PACKAGE_DISPLAY_NAME "Examples")
set(PACKAGE_DESCRIPTION "Omegalib core examples")
set(PACKAGE_DEPENDENCIES "")
set(PACKAGE_VERSION ${OMEGALIB_VERSION})

setup_package()

if(WIN32)
    file(INSTALL DESTINATION ${PACKAGE_DIR}/bin
        TYPE FILE
        FILES
            ${BIN_DIR}/ohello.exe
            ${BIN_DIR}/ohelloWidgets.exe
            ${BIN_DIR}/text2texture.exe
        )
else()
    file(INSTALL DESTINATION ${PACKAGE_DIR}/bin
        TYPE FILE
        FILES
            ${BIN_DIR}/ohello
            ${BIN_DIR}/ohelloWidgets
            ${BIN_DIR}/text2texture
        )
endif()

file(INSTALL DESTINATION ${PACKAGE_DIR}/examples
    TYPE DIRECTORY
    FILES
        ${SOURCE_DIR}/examples/python
    )

#-------------------------------------------------------------------------------
# create the utils package
set(PACKAGE_NAME core.utils)
set(PACKAGE_DISPLAY_NAME "Utilities")
set(PACKAGE_DESCRIPTION 
    "Omegalib core utilities. Includes mission control server, input server and asset cache server")
set(PACKAGE_DEPENDENCIES "")
set(PACKAGE_VERSION ${OMEGALIB_VERSION})

setup_package()

if(WIN32)
    file(INSTALL DESTINATION ${PACKAGE_DIR}/bin
        TYPE FILE
        FILES
            ${BIN_DIR}/mcsend.exe
            ${BIN_DIR}/mcserver.exe
            ${BIN_DIR}/ocachesrv.exe
            ${BIN_DIR}/ocachesync.exe
            ${BIN_DIR}/oimg.exe
            ${BIN_DIR}/oinputserver.exe
            ${BIN_DIR}/olauncher.exe
        )
else()
    file(INSTALL DESTINATION ${PACKAGE_DIR}/bin
        TYPE FILE
        FILES
            ${BIN_DIR}/mcsend
            ${BIN_DIR}/mcserver
            ${BIN_DIR}/ocachesrv
            ${BIN_DIR}/ocachesync
            ${BIN_DIR}/oimg
            ${BIN_DIR}/oinputserver
            ${BIN_DIR}/olauncher
        )
endif()

    