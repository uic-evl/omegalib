#-------------------------------------------------------------------------------
function(select_module_branch BRANCH_NAME DIR MODULE_NAME)

    # Can we find a tag with the branch name?
    execute_process(COMMAND ${GIT_EXECUTABLE} branch --list --all *${BRANCH_NAME} 
        WORKING_DIRECTORY ${DIR} OUTPUT_VARIABLE RESULT)
        
        #message("${MODULE_NAME}: looking for branch ${BRANCH_NAME}")
    if(NOT ${RESULT} STREQUAL "")
        # Branch found: check it out
        #message("${MODULE_NAME}: switching to branch ${BRANCH_NAME}")
        execute_process(COMMAND ${GIT_EXECUTABLE} checkout ${BRANCH_NAME} -q
            WORKING_DIRECTORY ${DIR} OUTPUT_VARIABLE RESULT)
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
        
        select_module_branch(${GIT_BRANCH} ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME} ${MODULE_NAME})
        
        # Add this module to the list of enabled modules. 
		set(ENABLED_MODULES "${ENABLED_MODULES};${MODULE_NAME}" CACHE INTERNAL "")
        
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
        
        # find module version
        set(${MODULE_NAME}_VERSION "1.0")
		file(STRINGS ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME}/CMakeLists.txt 
			${MODULE_NAME}_VERSION_RAW
			REGEX "^module_version([a-zA-Z0-9_\\.]*)")
		if(NOT "${${MODULE_NAME}_VERSION_RAW}" STREQUAL "")
			string(REGEX REPLACE "module_version\\(([a-zA-Z0-9_\\.]*)\\)" "\\1 " ${MODULE_NAME}_VERSION ${${MODULE_NAME}_VERSION_RAW})
        endif()
        message("${MODULE_NAME} version ${${MODULE_NAME}_VERSION}")

        # find module group
        # set(${MODULE_NAME}_GROUP "")
		# file(STRINGS ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME}/CMakeLists.txt 
			# ${MODULE_NAME}_GROUP_RAW
			# REGEX "^module_group([a-zA-Z0-9_\\.]*)")
		# if(NOT "${${MODULE_NAME}_GROUP_RAW}" STREQUAL "")
			# string(REGEX REPLACE "module_group\\(([a-zA-Z0-9_\\.]*)\\)" "\\1" ${MODULE_NAME}_GROUP ${${MODULE_NAME}_GROUP_RAW})
        # endif()
        #message("${MODULE_NAME} group ${${MODULE_NAME}_GROUP}")
        
        # add module pack file
        if(EXISTS ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME}/pack.cmake)
            file(READ ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME}/pack.cmake PACK_FILE_CONTENTS)
            file(APPEND ${PACK_FILE}.in "#====================================================\n")
            file(APPEND ${PACK_FILE}.in "#${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME}/pack.cmake\n")
			#if("${${MODULE_NAME}_GROUP}" STREQUAL "")
				file(APPEND ${PACK_FILE}.in "set(PACKAGE_NAME ${MODULE_NAME})\n")
			#else()
			#	file(APPEND ${PACK_FILE}.in "set(PACKAGE_NAME ${${MODULE_NAME}_GROUP}.${MODULE_NAME})\n")
			#endif()
			
            file(APPEND ${PACK_FILE}.in "setup_package()\n")
            file(APPEND ${PACK_FILE}.in "${PACK_FILE_CONTENTS}")
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
macro(module_version VER)
    set(${CMAKE_CURRENT_SOURCE_DIR}_VERSION ${VER})
endmacro()

#-------------------------------------------------------------------------------
# macro(module_group VER)
    # set(${CMAKE_CURRENT_SOURCE_DIR}_GROUP ${VER})
# endmacro()

#-------------------------------------------------------------------------------
macro(exit_on_missing_dependency() MODULE_NAME)
	if(${REGENERATE_REQUESTED})
		return()
	endif()
endmacro()

#-------------------------------------------------------------------------------
macro(declare_native_module MODULE_NAME)
    set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "")
    if(WIN32)
        set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules SUFFIX ".pyd")
    else()
        set_target_properties(${MODULE_NAME} PROPERTIES SUFFIX ".so")
    endif()
endmacro()


#-------------------------------------------------------------------------------
macro(merge_pack_file)
    file(READ pack.cmake PACK_FILE_CONTENTS)
    file(APPEND ${PACK_FILE}.in "${PACK_FILE_CONTENTS}")
endmacro()