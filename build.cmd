REM This batch file builds for Visual Studio 2019.
cmake -B build -G "Visual Studio 16 2019" -A x64
cmake --build build --config Release
start build/quad-benchmark.sln
