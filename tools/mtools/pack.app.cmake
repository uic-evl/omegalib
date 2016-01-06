if("${ARG2}" STREQUAL "")
    message("SYNTAX: omega pack.app [local-dir] [app/module]")
    return()
endif()

if(NOT WIN32)
    message("ERROR: omega pack.app is currently only supported on windows")
    return()
endif()

if(WIN32)
    set(BIN_DIR ${ARG2}/build/bin/release)
else()
    set(BIN_DIR ${ARG2}/build/bin)
endif()

set(PACKAGE_DIR pack/${ARG3})

# By default the app directory is apps/<appname>
# If we can't find it, we assume the application is inside the omegalib modules directory
set(APP_DIR apps/${ARG3})
if(NOT EXISTS ${APP_DIR})
    set(APP_DIR ${ARG2}/modules/${ARG3})
endif()
set(SOURCE_DIR ${ARG2})
set(PACKAGED_MODULES "")

function(pack_module MODULE_NAME MODULE_DIR)
        message("Packing dependent module ${MODULE_NAME}")
        list(APPEND PACKAGED_MODULES ${MODULE_NAME})
        message(${PACKAGED_MODULES})
        
        #Run the pack commands for the module
        include("${MODULE_DIR}/pack.cmake")
        
		# find dependencies and pack them
		file(STRINGS "${MODULE_DIR}/CMakeLists.txt"
			${MODULE_NAME}_DEPS_RAW
			REGEX "^request_dependency([a-zA-Z0-9_]*)")

		if(NOT "${${MODULE_NAME}_DEPS_RAW}" STREQUAL "")
			string(REGEX REPLACE "request_dependency\\(([a-zA-Z0-9_]*)\\)" "\\1 " ${MODULE_NAME}_DEPS_STR ${${MODULE_NAME}_DEPS_RAW})
			separate_arguments(${MODULE_NAME}_DEPS_LIST WINDOWS_COMMAND "${${MODULE_NAME}_DEPS_STR}")
			foreach(dependency ${${MODULE_NAME}_DEPS_LIST})
                list(FIND PACKAGED_MODULES ${dependency} _i)
                if(${_i} EQUAL -1)
                    pack_module(${dependency} "${ARG2}/modules/${dependency}")
                endif()
			endforeach()
		endif()
endfunction()

function(create_launcher APPLICATION_NAME APP_SCRIPT)
    file(WRITE ${ARG2}/install/launcher_tmp.au3 "Run(\"bin\\orun.exe modules/${MODULE_NAME}/${APP_SCRIPT}.py -D \" & @ScriptDir & \" \" & $CmdLineRaw, \"\")")
    set(LAUNCHER_COMPILER ${ARG2}/tools/Aut2exe.exe)
    set(ICON_FILE ${MODULE_DIR}/${APP_SCRIPT}.ico)
    if(NOT EXISTS ${ICON_FILE})
        set(ICON_FILE ${ARG2}/install/config/omega64-transparent.ico)
    endif()
    execute_process(COMMAND ${LAUNCHER_COMPILER} /in ${ARG2}/install/launcher_tmp.au3 /out ${PACKAGE_DIR}/${APPLICATION_NAME}.exe /icon ${ICON_FILE})
endfunction()

function(pack_enable NAME)
    set(PACK_${NAME} true CACHE INTERNAL "")
endfunction()

function(pack_disable NAME)
    set(PACK_${NAME} false CACHE INTERNAL "")
endfunction()

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
# For simple script modules, this macro will package the full module directory
macro(pack_module_dir)
    file(INSTALL DESTINATION ${PACKAGE_DIR}/modules
        TYPE DIRECTORY
        FILES
            ${MODULE_DIR}
        PATTERN ".git" EXCLUDE
        )
endmacro()

function(pack_dir DIRECTORY)
    file(INSTALL DESTINATION ${PACKAGE_DIR}/modules/${MODULE_NAME}
        TYPE DIRECTORY
        FILES
            ${MODULE_DIR}/${DIRECTORY}
        )
endfunction()


#set the default configuration for packages
set(PACK_EXAMPLES false CACHE INTERNAL "")
set(PACK_CORE_EQUALIZER true CACHE INTERNAL "")
set(PACK_CORE_UI true CACHE INTERNAL "")

# pack the application and all its dependencies
pack_module(${ARG3} ${APP_DIR})
message(${PACK_CORE_EQUALIZER})
# pack the omegalib runtime core
include("${ARG2}/src/pack_core.cmake")