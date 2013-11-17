vtm-osmscout
============

vtm tilesource for libosmscout offline maps.

depends on library project:
https://github.com/opensciencemap/vtm-android


== build JNI ==
 - create a link 'jni/libosmscout' to libosmscout directory and copy config headers:
 
```
 cd jni
 ln -s ../../libosmscout libosmscout 
 cp include/Config.h  ../../libosmscout/include/osmscout/private/Config.h
 cp include/CoreFeatures.h  ../../libosmscout/include/osmscout/CoreFeatures.h
 cp include/MapFeatures.h ../../libosmscout-map/include/osmscout/MapFeatures.h

 export NDK_HOME=/home/jeff/android-ndk-r8d
 
 ndk-build
```
