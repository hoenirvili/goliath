@echo off
if not defined DevEnvDir (
    call "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat" x86 10.0.16299.0
)
cmake  -DCMAKE_BUILD_TYPE=Release -G Ninja -Bbuild -H. 
ninja -C build -j8 cfgtrace
ninja -C build -j8 demo