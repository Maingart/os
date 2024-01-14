if exist dist rmdir /S /Q dist
git fetch && cmake -G "MinGW Makefiles" -B ./dist
cd ./dist && mingw32-make
