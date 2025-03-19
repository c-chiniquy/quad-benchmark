# Quad Benchmark

## About the project

This project benchmarks several quad rendering methods in D3D12.
My rendering abstraction layer [iglo](https://github.com/c-chiniquy/iglo) is used.

## List of Benchmarks

- 1 Draw call per quad (slowest method)
- Simple Batching
- Vertex generation methods
  - Indexing (with both dynamic and static index buffer)
  - Vertex pulling (with both raw and structured buffers)
  - Instancing
- GPU only (no CPU->GPU data transfers and CPU doesn't write any vertex data)

## Minimum system requirements

- Windows 10 (version 1909 or later)
- Shader model 6.6 capable graphics card and graphics drivers

## Build on Windows

- Install [CMake](https://cmake.org/download/). Version 3.22 or higher is required.
- Ensure CMake is accessible from the command line. 
- For Visual Studio: Run `build.cmd`. Generated project files will appear in `/build/`. 
- For other:
  ```
  cmake -B build
  cmake --build build --config Release
  ```
- CMake will automatically download AgilitySDK if needed and place it in the build folder.

## Third Party Libraries

- [iglo](https://github.com/c-chiniquy/iglo)
