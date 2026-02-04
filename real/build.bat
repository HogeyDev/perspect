if not exist "build" mkdir build
gcc -o build/main.exe src/*.c -lpthread
.\build\main.exe
