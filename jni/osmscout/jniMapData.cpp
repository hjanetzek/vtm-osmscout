/*
 This source is part of the libosmscout library
 Copyright (C) 2010  Tim Teulings

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

 This library is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 Lesser General Public License for more details.

 You should have received a copy of the GNU Lesser General Public
 License along with this library; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

#include <jni.h>
#include <string.h>
//#include <android/log.h>

#include <osmscout/MapPainter.h>

#include <jniObjectArray.h>

#define DEBUG_TAG "OsmScoutJni:MapData"

using namespace osmscout;

extern JniObjectArray<MapData> *gMapDataArray;

#ifdef __cplusplus
extern "C" {
#endif

   jint
   Java_osm_scout_MapData_jniConstructor(JNIEnv *env, jobject object) {
      MapData *nativeMapData = new MapData();

      return gMapDataArray->Add(nativeMapData);
   }

   void
   Java_osm_scout_MapData_jniDestructor(JNIEnv *env, jobject object, int mapDataIndex) {
      printf("destructor called\n");
      MapData *nativeMapData = gMapDataArray->GetAndRemove(mapDataIndex);

      if (!nativeMapData) {
	 printf("eek\n");
	 //  __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG,
	 //                     "jniDestructor(): NULL object");
      } else
	 delete nativeMapData;
   }

   jint
   Java_osm_scout_MapData_jniGetWays(JNIEnv *env, jobject object, int mapDataIndex) {
      MapData *nativeMapData = gMapDataArray->Get(mapDataIndex);

      printf("get size: %d\n", mapDataIndex);

      if (!nativeMapData) {
	 printf("eek2\n");

	 //  __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG,
	 //                     "jniDestructor(): NULL object");
	 return -1;
      }
      return nativeMapData->ways.size();
   }
#ifdef __cplusplus
}
#endif

