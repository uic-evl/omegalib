function(module_def MODULE_NAME URL DESCRIPTION)
	set(MODULES_${MODULE_NAME} false CACHE BOOL ${DESCRIPTION})
	
	list(FIND MODULES_LIST ${MODULE_NAME} found)
	if(NOT ${found} EQUAL -1)
		set(MODULES_${MODULE_NAME} true CACHE BOOL " " FORCE)
	endif()
	
	if(MODULES_${MODULE_NAME})
		if(NOT EXISTS ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME})
			message(STATUS "Installing module ${MODULE_NAME}...")
			execute_process(COMMAND ${GIT_EXECUTABLE} clone ${URL} ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME})
			message(STATUS "Module ${MODULE_NAME} installed")
		endif()
		file(APPEND ${MODULES_CMAKE_FILE} "add_subdirectory(${MODULE_NAME})\n")
		# substitute dashes with underscores in macro module names ('-' is
		# not a valid character
		string(REPLACE "-" "_" MACRO_MODULE_NAME ${MODULE_NAME})
		file(APPEND ${MODULES_CONFIG_FILE} "#define ${MACRO_MODULE_NAME}_ENABLED\n")

		# find dependencies
		file(STRINGS ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME}/CMakeLists.txt 
			${MODULE_NAME}_DEPS_RAW
			REGEX "^request_dependency([a-zA-Z0-9_]*)")
			
		if(NOT "${${MODULE_NAME}_DEPS_RAW}" STREQUAL "")
			string(REGEX REPLACE "request_dependency\\(([a-zA-Z0-9_]*)\\)" "\\1 " ${MODULE_NAME}_DEPS_STR ${${MODULE_NAME}_DEPS_RAW})
			separate_arguments(${MODULE_NAME}_DEPS_LIST WINDOWS_COMMAND "${${MODULE_NAME}_DEPS_STR}")
			
			foreach(dependency ${${MODULE_NAME}_DEPS_LIST})
				request_dependency(${dependency})
			endforeach()
		endif()
	endif()
endfunction()

macro(request_dependency MODULE_NAME)
	if(NOT MODULES_${MODULE_NAME})
		set(MODULES_${MODULE_NAME} true CACHE BOOL " " FORCE)
		set(REGENERATE_REQUESTED true CACHE BOOL "" FORCE)
		message("Module ${MODULE_NAME} is required but not currently installed. Marking for installation...")
	endif()
endmacro()

macro(exit_on_missing_dependency() MODULE_NAME)
	if(${REGENERATE_REQUESTED})
		return()
	endif()
endmacro()