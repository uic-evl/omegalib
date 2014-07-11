#-------------------------------------------------------------------------------
function(select_module_version MODULE_VERSION DIR MODULE_NAME)
    string(REPLACE "X" ${MODULE_VERSION} MODULE_VERSION "vX")
    message("Fetching and setting version for ${MODULE_NAME}")
    # fetch to make sure tags are up to date.
    execute_process(COMMAND ${GIT_EXECUTABLE} fetch WORKING_DIRECTORY ${DIR})
    
    # Can we find a tag with the full omegalib version name (i.e. v6.1)
    execute_process(COMMAND ${GIT_EXECUTABLE} tag -l ${MODULE_VERSION} 
        WORKING_DIRECTORY ${DIR} OUTPUT_VARIABLE RESULT)
        
    if(NOT ${RESULT} STREQUAL "")
        # Tag found: check it out
        message("            >>> checking out tag ${MODULE_VERSION}")
        execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${MODULE_VERSION} -q
            WORKING_DIRECTORY ${DIR} OUTPUT_VARIABLE RESULT)
            
    else()
        # Can we find a tag CONTAINING the major version name?
        # i.e. tag v3v4v5v6 will match version v4.      
        string(REGEX MATCH "v[0-9]+" MODULE_VERSION_MAJOR ${MODULE_VERSION})
        execute_process(COMMAND ${GIT_EXECUTABLE} tag -l *${MODULE_VERSION_MAJOR}* 
            WORKING_DIRECTORY ${DIR} OUTPUT_VARIABLE RESULT)
        
        if(NOT ${RESULT} STREQUAL "")
            # remove trailing newline
            string(REPLACE "\n" "" RESULT ${RESULT})
            
            # Tag found: check it out
            message("            >>> checking out tag ${RESULT}")
            execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${RESULT} -q 
                WORKING_DIRECTORY ${DIR} OUTPUT_VARIABLE RESULT)
                
            # no versioned tag/branch found for this module.
            # just keep using whatever branch/tag we're on.
        endif()
    endif()
endfunction()

#-------------------------------------------------------------------------------
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
        
        select_module_version(${OMEGALIB_VERSION} ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME} ${MODULE_NAME})
        
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
			set(CUR_MODULE "by module ${MODULE_NAME}")
			foreach(dependency ${${MODULE_NAME}_DEPS_LIST})
				request_dependency(${dependency})
			endforeach()
		endif()
	endif()
endfunction()

#-------------------------------------------------------------------------------
macro(request_dependency MODULE_NAME)
	if(NOT MODULES_${MODULE_NAME})
		set(MODULES_${MODULE_NAME} true CACHE BOOL " " FORCE)
		set(REGENERATE_REQUESTED true CACHE BOOL "" FORCE)
        if(NOT CUR_MODULE)
            set(CUR_MODULE "by ${CMAKE_CURRENT_SOURCE_DIR}")
        endif()
		message("Module ${MODULE_NAME} is required ${CUR_MODULE} but not currently installed. Marking for installation...")
	endif()
endmacro()

#-------------------------------------------------------------------------------
macro(exit_on_missing_dependency() MODULE_NAME)
	if(${REGENERATE_REQUESTED})
		return()
	endif()
endmacro()