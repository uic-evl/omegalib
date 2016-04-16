#If we are building an offline installer, there is nothing we need to do here.
if("${ARG3}" STREQUAL "offline")
    return()
endif()

#set default arguments
if("${ARG2}" STREQUAL "")
    message("SYNTAX: omega pack.publish <local-dir>")
    message("  Publish the package binaries on S3")
    message("ARGUMENTS:")
    message("  - local-dir: name of local installation directory")
    message("EXAMPLE: omega pack.publish master")
    
    return()
endif()

if(WIN32)
	if(NOT EXISTS cmake/AWSCLI)
		message("Downloading Amazon AWS tools...")
		file(DOWNLOAD ${OPM_URL}/AWSCLI_win64.tar.gz cmake/AWSCLI_win64.tar.gz SHOW_PROGRESS)
		execute_process(COMMAND ${CMAKE_COMMAND} -E tar xzf
			AWSCLI_win64.tar.gz WORKING_DIRECTORY cmake)
		
		message("---------------------------------------------------------------------")
		message("NOTE: Sinc you just installed AWS, yu will need to configure it")
		message("with your access keys before being able to publish.")
		message("See here for information:")
		message("  http://docs.aws.amazon.com/cli/latest/userguide/cli-chap-getting-started.html")
		message("Once configured, run this command again.")
		message("---------------------------------------------------------------------")
		return()
	endif()

	set(REPO_S3_URL "s3://omegalib/repo/release/windows")
	message("---- Publishing packages")
	execute_process(COMMAND ${CMAKE_SOURCE_DIR}/cmake/AWSCLI/aws s3 sync  
		repository ${REPO_S3_URL}
		WORKING_DIRECTORY ${ARG2}/install)
	message("---- Publishing done")
else()
	#NOTE: On OSX we expect AWS to be installed system-wide.
	set(REPO_S3_URL "s3://omegalib/repo/release/osx")
	message("---- Publishing packages")
	execute_process(COMMAND aws s3 sync  
	repository ${REPO_S3_URL}
	WORKING_DIRECTORY ${ARG2}/install)
	message("---- Publishing done")
endif()