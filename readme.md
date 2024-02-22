
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

## Linux
### Install
```
sudo apt-get install autoconf automake autoconf-archive
sudo apt-get install python3-jinja2
sudo apt install libx11-dev libxft-dev libxext-dev
sudo apt install libx11-dev libxkbcommon-x11-dev libx11-xcb-dev
sudo apt-get install libtool bison gperf libgles2-mesa-dev libxrandr-dev libxi-dev libxcursor-dev libxdamage-dev libxinerama-dev
sudo apt-get install nasm
```

On Linux set VCPKG_ROOT in config.sh
```
source config.sh

cmake --preset=debug 
cmake --build build_debug
--OR--
cmake --preset=release
cmake --build build_release
```

### options
```
Add --fresh to the preset command
Add --target clean to the build command
Add --target install to the build command
```

## OS-X
### Install
```
brew install gh
brew install autoconf automake autoconf-archive
brew install nasm
```

On MacOS set VCPKG_ROOT in config.sh
```
source config.sh

cmake --preset=debug-apple 
cmake --build build_debug
--OR--
cmake --preset=release-apple -G Xcode
cmake --build build_release --config Release
```


# Demo shortcuts
## CMD
```
ssh -i UKDemo2_key.pem azureuser@172.187.136.181
ssh -i NewDemo1_key.pem lab@4.231.169.194
```
## WSL
```
cd ~/.vs/anywhere/out/build/linux-release/anywhere/location
cp -r server/ /mnt/c/data/.
```

## CMD
```
cd c:\users\drjwc\
scp -i UKDemo2_key.pem -r C:\data\server azureuser@172.187.136.181:~/.
scp -i NewDemo1_key.pem -r C:\data\server lab@4.231.169.194:~/.
```
## CMD
```
cd C:\views\anywhere\out\build\x64-release\anywhere\form\muxer
cd ~/.vs/anywhere/out/build/linux-release/anywhere/form/muxer

muxer -o https://172.187.136.181:20000/essence/store/james-upload/ -i c:\data\james-uk -v -u 1 -c 1
muxer -o https://4.231.169.194:20000/essence/store/james-upload/ -i c:\data\james-ireland -v -u 2 -c 1
muxer.exe -i https://172.187.136.181:20000/essence/store/james-hevc/ -o null -t -p 99
```

## WSL
```
./muxer -i https://172.187.136.181:20000/essence/store/james-hevc/ -o null -t -p 99
```

## Stores
```
https://172.187.136.181:20000/essence/store/james.mp4/
https://172.187.136.181:20000/essence/store/james-hevc/
https://172.187.136.181:20000/essence/store/james-uplaod/
https://4.231.169.194:20000/essence/store/james-hevc/
c:\data\james-hevc
/home/drjwc/import/james-hevc/
```

## URLs
```
https://172.187.136.181:20000/index.html - opens james.mp4 for scrub demo
https://172.187.136.181:20000/index2.html - opens james-hevc for scrub demo
https://172.187.136.181:20000/index3.html - opens james-upload for upload chasing frame demo
https://172.187.136.181:20000/index4.html - opens james-hevc - from Redis - for scrub demo
```
