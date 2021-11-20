wine /opt/premake5.exe gmake
make config=release CC=x86_64-w64-mingw32-gcc
cp bin/core.dll core.dll
