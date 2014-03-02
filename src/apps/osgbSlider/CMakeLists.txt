# Source files
set(SRCS 
		osgbSlider.cpp)
    
###############################################################################
# Setup compile info
add_executable(osgbSlider ${SRCS})
set_target_properties(osgbSlider PROPERTIES FOLDER osgBulletExamples)
target_link_libraries(osgbSlider
	omega
	omegaToolkit
	omegaOsg
	${BULLET_LIBS}
	${OSGBULLET_LIBS})