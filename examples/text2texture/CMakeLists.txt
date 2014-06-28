add_executable(text2texture text2texture.cpp StringTextureSource.cpp)
set_target_properties(text2texture PROPERTIES FOLDER examples)
target_link_libraries(text2texture omega)


