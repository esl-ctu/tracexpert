[Back to the top (index)](README.md)

# Build instructions

TraceExpert is written mostly in C++ using Qt6 Framework. Officialy supported Qt version is Qt 6.8 LTS. It is built using ninja/CMake, recommended setup is using Qt Creator. Dependencies/build setup is listed below.

## Release 1.0 build setup (Windows platform)

- Git submodules
    - /ads (Qt Advanced Docking System)
    - /hdf5 (HDF5 library, version 2.0.0)
- Windows 11
- Visual Studio Community 2026
    - Desktop development with C++
        - C++ core desktop features
        - C++ CMake tools for Windows
        - Windows 11 SDK
        - MSVC v143 - VS 2022 C++ x64/x86 build tools (v14.44)
- Qt
    - Qt 6.8.3
        - MSVC 2022 64-bit
        - Additional Libraries
    - Build Tools        
        - CMake
        - Ninja
    - Qt Creator (optional, recommended)
- Git
- Pandoc

Picoscope plug-ins (ps6000, ps6000a) are further linked against

- PicoSDK 11.1.0.418


