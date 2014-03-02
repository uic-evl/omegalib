###################################################################################################
# Source files
set(SRCS 
		osgbCollision.cpp)
    
###################################################################################################
# Setup compile info
add_executable(osgbCollision ${SRCS})
set_target_properties(osgbCollision PROPERTIES FOLDER osgBulletExamples)
target_link_libraries(osgbCollision
	omega
	omegaToolkit
	omegaOsg
	${BULLET_LIBS}
	${OSGBULLET_LIBS})