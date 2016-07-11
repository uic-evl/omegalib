This directory contains omegalib modules. Modules are a way to extend omegalib through C++ or python, and can be imported into omegalib python scripts with the standard `import` statement. C++ Modules are compiled automatically during the omegalib build process. Python modules do not require compilation, you just need to copy them into your application directory, or place them in one of the python module search paths.

Modules are installed from a **module hub** containing separate repositories for each module. The default hub is https://github.com/omega-hub/hub

You can install modules through **cmake**:
```
omegalib build directory> cmake ./ -DMODULES="module;names;here"
```

Or using the **omegalib maintenance tools** (See omegalib main github page)
```
>omega add [local install] [module name]
```

For more information on how modules work read this wiki page: https://github.com/uic-evl/omegalib/wiki/Omegalib-modules

