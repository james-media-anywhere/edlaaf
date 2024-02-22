# Intro
See https://edl2aaf.sourceforge.net/
Official source: https://sourceforge.net/projects/edl2aaf/

This is a repo I started over 20 years ago ... !

# Building
We require that vcpkg and cmake are installed.
Also cmake must be on the path.

We recommend using Github CLI (https://github.com/cli/cli) for github access

## Windows
On Windows set VCPKG_ROOT in config.bat
```
config.bat

cmake --preset=debug 
cmake --build build_debug
--OR--
cmake --preset=release
cmake --build build_release --config Release  
--OR-- PREFFERED --
cmake --preset=release-symbols
cmake --build build_release --config RelWithDebInfo  
```
