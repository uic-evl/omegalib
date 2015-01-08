@echo off
REM if the omegalib maintenance utility has not been installed yet, create and 
REM run the bootstrap script.
IF EXIST cmake GOTO READY
md cmake
cd cmake
echo file(DOWNLOAD https://raw.githubusercontent.com/uic-evl/omegalib/master/tools/maintenance-util/omega.cmake ./omega.cmake) > bootstrap.cmake
cmake -P bootstrap.cmake
cd ..

:READY
cmake -DARG1="%1" -DARG2="%2" -DARG3="%3" -DARG4="%4" -P cmake/omega.cmake