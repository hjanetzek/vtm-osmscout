LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS)
 
LOCAL_MODULE    := osmscout
LOCAL_C_INCLUDES := . include libosmscout/libosmscout/include libosmscout/libosmscout-map/include 
 
LOCAL_CFLAGS := $(LOCAL_C_INCLUDES:%=-I%) -O2 -Wall -D__ANDROID__ -std=gnu++0x -Wno-sign-compare -g -Wno-psabi -frtti
LOCAL_CPPFLAGS := $(LOCAL_C_INCLUDES:%=-I%) -O2 -Wall -D__ANDROID__ -std=gnu++0x -Wno-sign-compare -g -Wno-psabi -frtti
LOCAL_LDLIBS := -lm -llog  -lstdc++
LOCAL_ARM_MODE  := arm
 
LOCAL_SRC_FILES := libosmscout/libosmscout/src/osmscout/ost/Scanner.cpp\
	libosmscout/libosmscout/src/osmscout/ost/Parser.cpp\
	libosmscout/libosmscout/src/osmscout/OptimizeWaysLowZoom.cpp\
	libosmscout/libosmscout/src/osmscout/AreaAreaIndex.cpp\
	libosmscout/libosmscout/src/osmscout/Area.cpp\
	libosmscout/libosmscout/src/osmscout/Types.cpp\
	libosmscout/libosmscout/src/osmscout/util/File.cpp\
	libosmscout/libosmscout/src/osmscout/util/Geometry.cpp\
	libosmscout/libosmscout/src/osmscout/util/FileWriter.cpp\
	libosmscout/libosmscout/src/osmscout/util/Color.cpp\
	libosmscout/libosmscout/src/osmscout/util/StopClock.cpp\
	libosmscout/libosmscout/src/osmscout/util/FileScanner.cpp\
	libosmscout/libosmscout/src/osmscout/util/String.cpp\
	libosmscout/libosmscout/src/osmscout/util/Transformation.cpp\
	libosmscout/libosmscout/src/osmscout/util/Magnification.cpp\
	libosmscout/libosmscout/src/osmscout/util/Projection.cpp\
	libosmscout/libosmscout/src/osmscout/WaterIndex.cpp\
	libosmscout/libosmscout/src/osmscout/Way.cpp\
	libosmscout/libosmscout/src/osmscout/Location.cpp\
	libosmscout/libosmscout/src/osmscout/Database.cpp\
	libosmscout/libosmscout/src/osmscout/AreaWayIndex.cpp\
	libosmscout/libosmscout/src/osmscout/GroundTile.cpp\
	libosmscout/libosmscout/src/osmscout/TypeConfigLoader.cpp\
	libosmscout/libosmscout/src/osmscout/CityStreetIndex.cpp\
	libosmscout/libosmscout/src/osmscout/Node.cpp\
	libosmscout/libosmscout/src/osmscout/OptimizeAreasLowZoom.cpp\
	libosmscout/libosmscout/src/osmscout/AreaNodeIndex.cpp\
	libosmscout/libosmscout/src/osmscout/TypeConfig.cpp\
	libosmscout/libosmscout/src/osmscout/AttributeAccess.cpp\
	libosmscout/libosmscout-map/src/osmscout/StyleConfig.cpp\
	libosmscout/libosmscout-map/src/osmscout/MapPainter.cpp\
	libosmscout/libosmscout-map/src/osmscout/oss/Scanner.cpp\
	libosmscout/libosmscout-map/src/osmscout/oss/Parser.cpp\
	libosmscout/libosmscout-map/src/osmscout/StyleConfigLoader.cpp\
	libosmscout-jni/jniTypeConfig.cpp\
	libosmscout-jni/jniMercatorProjection.cpp\
	libosmscout-jni/jniStyleConfig.cpp\
	libosmscout-jni/jniObjectTypeSets.cpp\
	libosmscout-jni/jniMapData.cpp\
	libosmscout-jni/jniMapPainterCanvas.cpp\
	libosmscout-jni/jniDatabase.cpp\
	libosmscout-jni/jniMapParameter.cpp\
	libosmscout-jni/jniOnLoad.cpp
 
include $(BUILD_SHARED_LIBRARY)
