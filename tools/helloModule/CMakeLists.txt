module_version(1.0)

add_library(helloModule MODULE helloModule.cpp)
	
target_link_libraries(helloModule omega)

declare_native_module(helloModule)
