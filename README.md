vtm-osmscout
============

vtm tilesource for libosmscout offline maps.

depends on library project:
https://github.com/hjanetzek/vtm

### build JNI
 
```
 git submodule init 
 git submodule update
 cd jni
 ./copy_headers.sh
 export NDK_HOME=/path-to/android-ndk-r8d
 ant [-v]
```
