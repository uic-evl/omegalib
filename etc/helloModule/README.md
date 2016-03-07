Example basic native module.

## How to build
copy the helloModule directory in modules

**If you use CMake:**
- set the MODULES_LOCAL cmake variable t0 "helloModule"

**If you use the omega maintenance tools**
Assuming you have your omegalib build in a directory called master type
```
> omega set master MODULES_LOCAL "signac"
> omega build master
```

## Testing the module
Start orun and type the following in the console:
```
import helloModule
helloModule.hello()
```
