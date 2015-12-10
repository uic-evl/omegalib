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
            ${BIN_DIR}/Collage.dll
            ${BIN_DIR}/Equalizer.dll
            ${BIN_DIR}/EqualizerServer.dll
            ${BIN_DIR}/msvcp120.dll
            ${BIN_DIR}/msvcr120.dll
            ${BIN_DIR}/omega.dll
            ${BIN_DIR}/omegaToolkit.dll
            ${BIN_DIR}/omicron.dll
            ${BIN_DIR}/PQMTClient.dll
            ${BIN_DIR}/pthread.dll
            ${BIN_DIR}/python27.dll
            ${BIN_DIR}/displaySystem_GLFW.dll
            ${BIN_DIR}/displaySystem_Equalizer.dll
            # Executables
            ${BIN_DIR}/orun.exe
        )
        
    file(APPEND ${PACKAGE_DIR}/orun.bat ".\\bin\\orun.exe -D %~dp0%")
    
elseif(APPLE)
	set(EQUALIZER_DIR ${BUILD_DIR}/3rdparty/equalizer/build/libs)

    file(INSTALL DESTINATION ${PACKAGE_DIR}/bin
        TYPE FILE
        FILES
            # Dlls
            ${BIN_DIR}/libCollage.dylib
            ${BIN_DIR}/libCollage.0.3.0.dylib
            ${BIN_DIR}/libCollage.0.3.1.dylib
            ${BIN_DIR}/libEqualizer.dylib
            ${BIN_DIR}/libEqualizer.1.0.0.dylib
            ${BIN_DIR}/libEqualizer.1.0.2.dylib
            ${BIN_DIR}/libEqualizerServer.dylib
            ${BIN_DIR}/libEqualizerServer.1.0.0.dylib
            ${BIN_DIR}/libEqualizerServer.1.0.2.dylib
            ${BIN_DIR}/libomega.dylib
            ${BIN_DIR}/libomegaToolkit.dylib
            ${BIN_DIR}/libomicron.dylib
            ${BIN_DIR}/libdisplaySystem_GLFW.dylib
            ${BIN_DIR}/libdisplaySystem_Equalizer.dylib
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
        ${SOURCE_DIR}/fonts
        ${SOURCE_DIR}/menu_sounds
        ${SOURCE_DIR}/ui
        ${SOURCE_DIR}/system
    )
    
file(INSTALL DESTINATION ${PACKAGE_DIR}
    TYPE FILE
    FILES
        ${SOURCE_DIR}/default.cfg
        ${SOURCE_DIR}/default_init.py
        ${SOURCE_DIR}/omegalib-transparent-white.png
    )


    