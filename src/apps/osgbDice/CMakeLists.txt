# Source files
 set(SRCS 
		osgbDice.cpp)
    
###################################################################################################
# Setup compile info
add_executable(osgbDice ${SRCS})
set_target_properties(osgbDice PROPERTIES FOLDER osgBulletExamples)
target_link_libraries(osgbDice
	omega
	omegaToolkit
	omegaOsg
	${BULLET_LIBS}
	${OSGBULLET_LIBS})