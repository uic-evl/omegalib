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

endmacro()


#set the default configuration for packages
set(PACK_EXAMPLES false CACHE INTERNAL "")
set(PACK_CORE_EQUALIZER true CACHE INTERNAL "")
set(PACK_CORE_UI true CACHE INTERNAL "")

#include file with functions used by packaging scripts
include("${ARG2}/src/pack_functions.cmake")

# pack the application and all its dependencies
pack_module(${ARG3} ${APP_DIR})
message(${PACK_CORE_EQUALIZER})
# pack the omegalib runtime core
include("${ARG2}/src/pack_core.cmake")
