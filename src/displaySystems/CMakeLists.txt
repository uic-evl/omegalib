add_subdirectory(GLFW)

# Equalizer Display Module
set(OMEGA_USE_DISPLAY_EQUALIZER false CACHE BOOL "Enable Equalizer display system support")
if(OMEGA_USE_DISPLAY_EQUALIZER)
    add_subdirectory(Equalizer)
endif()
