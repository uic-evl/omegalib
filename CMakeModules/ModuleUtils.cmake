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
		string(REPLACE "." "_" MACRO_MODULE_NAME ${MACRO_MODULE_NAME})
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
        
        # add module pack file
        if(EXISTS ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME}/pack.cmake)
            file(READ ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME}/pack.cmake PACK_FILE_CONTENTS)
            file(APPEND ${PACK_FILE}.in "#====================================================\n")
            file(APPEND ${PACK_FILE}.in "#${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME}/pack.cmake\n")
            file(APPEND ${PACK_FILE}.in "set(PACKAGE_NAME ${MODULE_NAME})\n")
			
            file(APPEND ${PACK_FILE}.in "set(PACKAGE_DISPLAY_NAME ${MODULE_NAME})\n")
            file(APPEND ${PACK_FILE}.in "set(MODULE_DIR ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME})\n")
            file(APPEND ${PACK_FILE}.in "set(MODULE_NAME ${MODULE_NAME})\n")
            file(APPEND ${PACK_FILE}.in "set(PACKAGE_DESCRIPTION \"${DESCRIPTION}\")\n")
            string(REPLACE ";" "," PACKAGE_DEPENDENCIES "${${MODULE_NAME}_DEPS_LIST}")
            
            file(APPEND ${PACK_FILE}.in "set(PACKAGE_DEPENDENCIES \"${PACKAGE_DEPENDENCIES}\")\n")
            
            # parse a module version from CMakeLists or add a version.txt file
            file(APPEND ${PACK_FILE}.in "set(PACKAGE_VERSION ${${MODULE_NAME}_VERSION})\n")
            file(APPEND ${PACK_FILE}.in "setup_package()\n")
            file(APPEND ${PACK_FILE}.in "${PACK_FILE_CONTENTS}")
        endif()
	endif()
endfunction()

#-------------------------------------------------------------------------------
macro(add_module MODULE_ID)
    # MODULE_ID = git_org/module_name
    get_filename_component(MODULE_GIT_ORG ${MODULE_ID} DIRECTORY)
    get_filename_component(MODULE_NAME ${MODULE_ID} NAME)
    # If we don't have a module description in the cache, pull it from the
    # online repository now.
    if(NOT EXISTS MODULES_${MODULE_NAME}_DESCRIPTION)
		file(DOWNLOAD https://raw.githubusercontent.com/${MODULE_GIT_ORG}/${MODULE_NAME}/master/README.md ${CMAKE_BINARY_DIR}/moduleList/README.md)
        file(STRINGS ${CMAKE_BINARY_DIR}/moduleList/README.md RAW_DESC)
        message(STATUS ${RAW_DESC})
        list(GET RAW_DESC 0 DESC)
        message(STATUS ${DESC})
        set(MODULES_${MODULE_NAME}_DESCRIPTION ${DESC} CACHE STRING "Module ${MODULE_NAME} description")
    endif()
    
    set(MODULES_${MODULE_NAME} true CACHE BOOL ${MODULES_${MODULE_NAME}_DESCRIPTION})
    module_def(${MODULE_NAME} https://github.com/${MODULE_GIT_ORG}/${MODULE_NAME}.git ${MODULES_${MODULE_NAME}_DESCRIPTION})
endmacro()

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
macro(declare_native_module MODULE_NAME)
    set_target_properties(${MODULE_NAME} PROPERTIES PREFIX "")
    if(WIN32)
        set_target_properties(${MODULE_NAME} PROPERTIES FOLDER modules SUFFIX ".pyd")
    else()
        set_target_properties(${MODULE_NAME} PROPERTIES SUFFIX ".so")
    endif()
endmacro()
