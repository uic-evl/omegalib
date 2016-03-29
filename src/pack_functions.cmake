#-------------------------------------------------------------------------------
# For simple script modules, this macro will package the full module directory
macro(pack_module_dir)
    file(INSTALL DESTINATION ${PACKAGE_DIR}/modules
        TYPE DIRECTORY
        FILES
            ${SOURCE_DIR}/modules/${PACKAGE_NAME}
        PATTERN ".git" EXCLUDE
        )
endmacro()

#-------------------------------------------------------------------------------
function(pack_native_module NAME)
    if(WIN32)
        file(INSTALL DESTINATION ${PACKAGE_DIR}/bin
            TYPE FILE
            FILES
                ${BIN_DIR}/${NAME}.pyd
            )
    elseif(APPLE)
        file(INSTALL DESTINATION ${PACKAGE_DIR}/bin
            TYPE FILE
            FILES
                ${BIN_DIR}/${NAME}.so
            )
    endif()
endfunction()

#-------------------------------------------------------------------------------
function(create_launcher APPLICATION_NAME APP_SCRIPT)
	if(WIN32)
		file(WRITE ${ARG2}/install/launcher_tmp.au3 
            "Run(\"bin\\orun.exe modules/${MODULE_NAME}/${APP_SCRIPT}.py -D \" & @ScriptDir & \" \" & $CmdLineRaw, \"\")")
		set(LAUNCHER_COMPILER ${ARG2}/etc/Aut2exe.exe)
		set(ICON_FILE ${MODULE_DIR}/${APP_SCRIPT}.ico)
		if(NOT EXISTS ${ICON_FILE})
			set(ICON_FILE ${ARG2}/install/config/omega64-transparent.ico)
		endif()
		execute_process(COMMAND ${LAUNCHER_COMPILER} 
            /in ${ARG2}/install/launcher_tmp.au3 
            /out ${PACKAGE_DIR}/${APPLICATION_NAME}.exe 
            /icon ${ICON_FILE})
	endif()
endfunction()

#-------------------------------------------------------------------------------
function(pack_enable NAME)
    set(PACK_${NAME} true CACHE INTERNAL "")
endfunction()

#-------------------------------------------------------------------------------
function(pack_disable NAME)
    set(PACK_${NAME} false CACHE INTERNAL "")
endfunction()

#-------------------------------------------------------------------------------
function(pack_dir DIRECTORY)
    file(INSTALL DESTINATION ${PACKAGE_DIR}/modules/${MODULE_NAME}
        TYPE DIRECTORY
        FILES
            ${MODULE_DIR}/${DIRECTORY}
        )
endfunction()