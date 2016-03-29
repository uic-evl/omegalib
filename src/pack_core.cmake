# Copy core files.
if(WIN32)
	file(INSTALL DESTINATION ${PACKAGE_DIR}/modules/python 
        TYPE DIRECTORY
        FILES
            ${SOURCE_DIR}/modules/python/DLLs
            ${SOURCE_DIR}/modules/python/Lib)

    file(INSTALL DESTINATION ${PACKAGE_DIR}/bin
        TYPE FILE
        FILES
            # Dlls
            ${BIN_DIR}/msvcp120.dll
            ${BIN_DIR}/msvcr120.dll
            ${BIN_DIR}/omega.dll
            ${BIN_DIR}/omegaToolkit.dll
            ${BIN_DIR}/omicron.dll
            ${BIN_DIR}/PQMTClient.dll
            ${BIN_DIR}/python27.dll
            ${BIN_DIR}/displaySystem_GLFW.dll
            # Executables
            ${BIN_DIR}/orun.exe
        )
    
elseif(APPLE)
    file(INSTALL DESTINATION ${PACKAGE_DIR}/bin
        TYPE FILE
        FILES
            ${BIN_DIR}/libomega.dylib
            ${BIN_DIR}/libomegaToolkit.dylib
            ${BIN_DIR}/libomicron.dylib
            ${BIN_DIR}/libdisplaySystem_GLFW.dylib
		)
    file(INSTALL DESTINATION ${PACKAGE_DIR}/bin
        TYPE FILE
        FILES
            # Executables
            ${BIN_DIR}/orun
        PERMISSIONS 
			OWNER_READ OWNER_WRITE OWNER_EXECUTE
			WORLD_READ WORLD_EXECUTE
		   )
endif()

file(INSTALL DESTINATION ${PACKAGE_DIR}
    TYPE DIRECTORY
    FILES
        ${SOURCE_DIR}/system
    )
    
file(INSTALL DESTINATION ${PACKAGE_DIR}
	TYPE FILE
	FILES
		${SOURCE_DIR}/default.cfg
		${SOURCE_DIR}/default_init.py
		${SOURCE_DIR}/omegalib-transparent-white.png
	)
    