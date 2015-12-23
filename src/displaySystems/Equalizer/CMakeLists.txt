include(${CMAKE_SOURCE_DIR}/external/UseEqualizer.cmake)
include_directories(${EQUALIZER_INCLUDES})

# Include X11 headers on OSX    
if(APPLE)
    if(${CURRENT_OSX_VERSION} STREQUAL "10.8" OR ${CURRENT_OSX_VERSION} STREQUAL "10.7")
        include_directories(SYSTEM /usr/X11R6/include)
    else()
        include_directories(SYSTEM /opt/X11/include)
    endif()
endif()

add_library(displaySystem_Equalizer SHARED 
    displaySystem_Equalizer.cpp
    EqualizerDisplaySystem.cpp
    ChannelImpl.cpp
    ConfigImpl.cpp
    NodeImpl.cpp
    WindowImpl.cpp)

target_link_libraries(displaySystem_Equalizer ${EQUALIZER_LIBS} omega)
set_target_properties(displaySystem_Equalizer PROPERTIES FOLDER "displaySystems")
add_dependencies(displaySystem_Equalizer equalizer omega)
