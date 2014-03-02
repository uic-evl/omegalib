if(UNIX)
	add_definitions( -D__unix__ -DHAVE_FCNTL_H )
endif(UNIX)

set(freeglut_SOURCES
			 ./src/freeglut_cursor.c 
			 ./src/freeglut_callbacks.c 
			 ./src/freeglut_display.c 
			 ./src/freeglut_ext.c 
			 ./src/freeglut_font.c 
			 ./src/freeglut_glutfont_definitions.c 
			 ./src/freeglut_font_data.c 
			 ./src/freeglut_stroke_roman.c 
			 ./src/freeglut_stroke_mono_roman.c 
			 ./src/freeglut_gamemode.c 
			 ./src/freeglut_joystick.c 
			 ./src/freeglut_geometry.c 
			 ./src/freeglut_init.c 
			 ./src/freeglut_input_devices.c 
			 ./src/freeglut_spaceball.c 
			 ./src/freeglut_main.c 
			 ./src/freeglut_menu.c 
			 ./src/freeglut_misc.c 
			 ./src/freeglut_overlay.c 
			 ./src/freeglut_state.c 
			 ./src/freeglut_structure.c 
			 ./src/freeglut_teapot.c 
			 ./src/freeglut_videoresize.c 
			 ./src/freeglut_window.c
)
include_directories(./include)
add_library(freeglut STATIC ${freeglut_SOURCES})
SET_TARGET_PROPERTIES(freeglut PROPERTIES FOLDER 3rdparty)
