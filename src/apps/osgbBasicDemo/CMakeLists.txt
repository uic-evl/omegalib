# Source files
set(SRCS 
		osgbBasicDemo.cpp)

###################################################################################################
# Setup compile info
add_executable(osgbBasicDemo ${SRCS})
set_target_properties(osgbBasicDemo PROPERTIES FOLDER osgBulletExamples)
target_link_libraries(osgbBasicDemo
	omega
	omegaToolkit
	omegaOsg
	${BULLET_LIBS}
	${OSGBULLET_LIBS})