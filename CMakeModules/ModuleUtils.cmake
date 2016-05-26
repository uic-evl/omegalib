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
    #message("in module ${MODULE_NAME}")
	
	if(MODULES_${MODULE_NAME})
		if(NOT EXISTS ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME})
			message(STATUS "Installing module ${MODULE_NAME}...")
			execute_process(COMMAND ${GIT_EXECUTABLE} clone ${URL} ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME})
			message(STATUS "Module ${MODULE_NAME} installed")
		endif()
        
        select_module_branch(${GIT_BRANCH} ${CMAKE_SOURCE_DIR}/modules/${MODULE_NAME} ${MODULE_NAME})
                
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
    set(REQUESTED_MODULES ${REQUESTED_MODULES} PARENT_SCOPE)
endfunction()

#-------------------------------------------------------------------------------
function(add_module MODULE_ID)
    # MODULE_ID = git_org/module_name
    get_filename_component(MODULE_GIT_ORG ${MODULE_ID} DIRECTORY)
    get_filename_component(MODULE_NAME ${MODULE_ID} NAME)
    
    if("${MODULE_GIT_ORG}" STREQUAL "")
        set(MODULE_GIT_ORG ${OMEGA_DEFAULT_MODULE_ORGANIZATION})
    endif()

    # Local module support
    if("${MODULE_GIT_ORG}" STREQUAL ".")
        set(MODULES_${MODULE_NAME}_DESCRIPTION "Module ${MODULE_NAME}" CACHE STRING "Module ${MODULE_NAME} description")
    else()
        # If we don't have a module description in the cache, pull it from the
        # online repository now.
        if(NOT EXISTS MODULES_${MODULE_NAME}_DESCRIPTION)
            file(DOWNLOAD https://raw.githubusercontent.com/${MODULE_GIT_ORG}/${MODULE_NAME}/master/README.md ${CMAKE_BINARY_DIR}/moduleList/README.md)
            file(STRINGS ${CMAKE_BINARY_DIR}/moduleList/README.md RAW_DESC)
            if(RAW_DESC)
                list(GET RAW_DESC 0 DESC)
                if("${DESC}" STREQUAL "")
                    set(MODULES_${MODULE_NAME}_DESCRIPTION "Module ${MODULE_NAME}" CACHE STRING "Module ${MODULE_NAME} description")
                else()
                    set(MODULES_${MODULE_NAME}_DESCRIPTION ${DESC} CACHE STRING "Module ${MODULE_NAME} description")
                endif()
            else()
                set(MODULES_${MODULE_NAME}_DESCRIPTION "Module ${MODULE_NAME}" CACHE STRING "Module ${MODULE_NAME} description")
            endif()
        endif()
    endif()
    
    set(MODULES_${MODULE_NAME} true CACHE BOOL ${MODULES_${MODULE_NAME}_DESCRIPTION})
    #module_def(${MODULE_NAME} https://github.com/${MODULE_GIT_ORG}/${MODULE_NAME}.git ${MODULES_${MODULE_NAME}_DESCRIPTION})
endfunction()

#-------------------------------------------------------------------------------
function(request_dependency MODULE_FULLNAME)
    #message("Requesting dep ${MODULE_FULLNAME}")
    get_filename_component(MODULE_GIT_ORG ${MODULE_FULLNAME} DIRECTORY)
    get_filename_component(MODULE_NAME ${MODULE_FULLNAME} NAME)
    
	if(NOT MODULES_${MODULE_NAME})
		set(MODULES_${MODULE_NAME} true CACHE BOOL " " FORCE)
        if(NOT CUR_MODULE)
            set(CUR_MODULE "by ${CMAKE_CURRENT_SOURCE_DIR}")
        endif()
		message("Module ${MODULE_NAME} is required ${CUR_MODULE} but not currently installed. Marking for installation...")
        
        add_module(${MODULE_FULLNAME})
        # hub-less modules always contain an org/user name. If we are requesting
        # a hub-less module, add it here.
        #if(NOT "${MODULE_GIT_ORG}" STREQUAL "")
        #endif()
	endif()
    # I'd like to use if(NOT .. IN_LIST here but that's supported by cmake 3.3+
    # and we don't have it installed on travis.
    list(FIND REQUESTED_MODULES "${MODULE_FULLNAME}" index)
    if(${index} EQUAL -1)
		set(REGENERATE_REQUESTED true CACHE BOOL "" FORCE)
        list(APPEND REQUESTED_MODULES ${MODULE_FULLNAME})
        set(REQUESTED_MODULES ${REQUESTED_MODULES} PARENT_SCOPE)
    endif()
endfunction()

#-------------------------------------------------------------------------------
# Entry point for module processing, used by src/CMakeLists.txt
macro(process_modules)
    # MODULES_LIST = modules that the user wants installed
    # REQUESTED_MODULES = modules requested by the user + all resolved dependencies
    set(MODULES "" CACHE STRING "The list of enabled modules")
    set(REQUESTED_MODULES "")
    set(REGENERATE_REQUESTED true CACHE BOOL "" FORCE)
    
    # Add modules from the MODULES_ADD variable to the list of initial modules
    # remove modules from the MODULES_REMOVE variable from the list of initial modules
    list(APPEND MODULES ${MODULES_ADD})
    if(NOT "${MODULES_REMOVE}" STREQUAL "")
        list(REMOVE_ITEM MODULES ${MODULES_REMOVE})
    endif()
    set(MODULES_ADD "" CACHE STRING "" FORCE)
    set(MODULES_REMOVE "" CACHE STRING "" FORCE)
    list(REMOVE_DUPLICATES MODULES)
    set(MODULES ${MODULES} CACHE STRING "The list of enabled modules" FORCE)
    
    if(NOT "${MODULES}" STREQUAL "")
        # First step: request modules that the user wants. 
        separate_arguments(MODULES_LIST WINDOWS_COMMAND "${MODULES}")
        string(REPLACE " " ";" MODULES_LIST ${MODULES_LIST})
        foreach(MODULE ${MODULES_LIST})
            string(REPLACE "\"" "" UNQUOTEDMODULE ${MODULE})
            request_dependency(${UNQUOTEDMODULE})
        endforeach()

        # Keep running until all module dependencies are resolved.
        while(REGENERATE_REQUESTED)
            set(REGENERATE_REQUESTED false CACHE BOOL "" FORCE)
            
            # delete the modulesConfig.h file, it will be regenerated by module_def
            # for native modules.
            set(MODULES_CONFIG_FILE ${CMAKE_BINARY_DIR}/include/modulesConfig.h)
            file(REMOVE ${MODULES_CONFIG_FILE})
            file(APPEND ${MODULES_CONFIG_FILE} "//auto-generated file\n")

            # create the pack.cmake file. This file contains the commands to create
            # packaged modules for the omegalib installer. Each module will append
            # its own pack.cmake file to this one.
            set(PACK_FILE ${CMAKE_SOURCE_DIR}/install/pack.cmake)
            file(REMOVE ${PACK_FILE}.in)
            file(READ pack_header.cmake PACK_HEADER_FILE_CONTENTS)
            file(READ pack_core.cmake PACK_CORE_FILE_CONTENTS)
            file(APPEND ${PACK_FILE}.in "${PACK_HEADER_FILE_CONTENTS} ${PACK_CORE_FILE_CONTENTS}")

            # Loop through all the requested modules, loading the module definitions.
            # The module_def call will process dependencies for each module,
            # adding missing dependencies to REQUESTED_MODULES until all dependencies are
            # resolved and REGENERATE_REQUESTED stays false.
            list(REMOVE_DUPLICATES REQUESTED_MODULES)
            foreach(MODULE_ID ${REQUESTED_MODULES})
                get_filename_component(MODULE_GIT_ORG ${MODULE_ID} DIRECTORY)
                get_filename_component(MODULE_NAME ${MODULE_ID} NAME)
                
                if("${MODULE_GIT_ORG}" STREQUAL "")
                    set(MODULE_GIT_ORG ${OMEGA_DEFAULT_MODULE_ORGANIZATION})
                endif()
                # Local module support
                if("${MODULE_GIT_ORG}" STREQUAL ".")
                    module_def(
                        ${MODULE_NAME} ./${MODULE_NAME}.git 
                        ${MODULES_${MODULE_NAME}_DESCRIPTION})
                else()
                    module_def(
                        ${MODULE_NAME} https://github.com/${MODULE_GIT_ORG}/${MODULE_NAME}.git 
                        ${MODULES_${MODULE_NAME}_DESCRIPTION})
                endif()
            endforeach()
            
            configure_file(${PACK_FILE}.in ${PACK_FILE} @ONLY)
        endwhile()

        # Add the modules subdirectory. This will include cmake scripts for all native modules
        #add_subdirectory(${CMAKE_SOURCE_DIR}/modules ${CMAKE_BINARY_DIR}/modules)
        list(REMOVE_DUPLICATES REQUESTED_MODULES)
        list(REVERSE REQUESTED_MODULES)
        foreach(MODULE ${REQUESTED_MODULES})
            add_subdirectory(${CMAKE_SOURCE_DIR}/modules/${MODULE} ${CMAKE_BINARY_DIR}/modules/${MODULE})
        endforeach()
    endif()
endmacro()

################################################################################
# MODULE API
# The following macros are used inside module CMakeList.txt files

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
