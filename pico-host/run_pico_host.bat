echo %date%
echo %time%
echo %cd%

rmdir /s /q build 
mkdir build
cd build

cmake -G "NMake Makefiles" ..
nmake

pause