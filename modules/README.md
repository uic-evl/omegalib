This directory contains omegalib modules. Modules are a way to extend omegalib through C++ or python, and can be imported into omegalib python scripts with the standard `import` statement. C++ Modules are compiled automatically during the omegalib build process. Python modules do not require compilation, you just need to copy them into your application directory, or place them in one of the python module search paths.

To add a C++ module to the omegalib build, modify the `CMakeLists.txt` file in this directory. All C++ modules are in the `native` directory.

`native/templateModule` is a good place to start if you want to create your own C++ module. `sprite` is a good examples of a Python module.

For more information on how modules work read this wiki page: https://github.com/febret/omegalib/wiki/ExtendingOmegalib

## The modules.py script
`modules.py` is a module management script. Run it as-is to get a state of all the modules (to check if you have local code changes). Run `modules.py -v` to get extended information on all modules. Run `module.py -p` to update modules (only done if no local changes are detected)
