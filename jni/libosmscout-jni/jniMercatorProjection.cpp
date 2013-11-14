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
#ifdef __ANDROID__
#include <android/log.h>
#define DEBUG_TAG "OsmScoutJni:MercatorProjection"
#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, __VA_ARGS__)
#endif
#include <osmscout/util/Projection.h>

#include <jniObjectArray.h>
//#include <osmscout/system/Math.h>
//#include <osmscout/system/Assert.h>

using namespace osmscout;

//namespace osmscout {
//   static const double gradtorad = 2 * M_PI / 360;
//
//   class TileProjection : MercatorProjection {
//   public:
//      bool
//      Set(long x, long y, int zoom, int size);
//
//      bool
//      GeoToPixel(double lon, double lat, double& x, double& y) const;
//
//   };
//
//   bool
//   TileProjection::Set(long x, long y, int zoom, int size)
//	 {
//      if (valid &&
//	    this->lonMin == lonMin &&
//	    this->lonMax == lonMax &&
//	    this->latMin == latMin &&
//	    this->latMax == latMax &&
//	    this->magnification == magnification &&
//	    this->width == width) {
//	 return true;
//      }
//
//      valid = true;
//
//      this->lonMin = lonMin;
//      this->lonMax = lonMax;
//      this->latMin = latMin;
//      this->latMax = latMax;
//      this->magnification = magnification;
//      this->width = width;
//
//      // Make a copy of the context information
//      this->lon = (lonMin + lonMax) / 2;
//      this->lat = atan(sinh((atanh(sin(latMax * gradtorad)) + atanh(sin(latMin * gradtorad))) / 2))
//	    / gradtorad;
//
//      scale = (width - 1) / (gradtorad * (lonMax - lonMin));
//      scaleGradtorad = scale * gradtorad;
//
//      // Width of an pixel in meter
//      double d = (lonMax - lonMin) * gradtorad;
//
//      pixelSize = d * 180 * 60 / M_PI * 1852.216 / width;
//
//      this->height = (atanh(sin(latMax * gradtorad)) - atanh(sin(latMin * gradtorad))) * scale;
//
////      lonOffset = lonMin * scale * gradtorad;
////      latOffset = scale * atanh(sin(latMin * gradtorad));
//
//      long z = size << zoom;
//      long dx = x - (z >> 1);
//      long dy = y - (z >> 1);
//
//      divx = 180000000.0 / (z >> 1);
//      divy = z / PIx4;
//
//      return true;
//   }
//   bool
//   TileProjection::GeoToPixel(double lon, double lat,
//	 double& x, double& y) const
//	 {
//      assert(valid);
//
//      x = lon * scaleGradtorad - lonOffset;
//      //y = height - (scale * atanh(sin(lat * gradtorad)) - latOffset);
//
//
//      double sinLatitude = sin(lat * (M_PI / 180));
//      y = 0.5 - log((1 + sinLatitude) / (1 - sinLatitude)) / (4 * M_PI);
//
//      x = (lon / divx - lonOffset);
//
//      double sinLat = sin(lat * gradtorad);
//      y = (this->width - (log((1.0 + sinLat) / (1.0 - sinLat)) * divy + latOffset));
//
//      return true;
//   }
//
//}

extern JniObjectArray<MercatorProjection> *gProjectionArray;

#ifdef __cplusplus
extern "C" {
#endif

   jint
   Java_osm_scout_MercatorProjection_jniConstructor(JNIEnv *env, jobject object) {
      MercatorProjection *nativeProjection = new MercatorProjection();

      return gProjectionArray->Add(nativeProjection);
   }

   void
   Java_osm_scout_MercatorProjection_jniDestructor(JNIEnv *env, jobject object,
	 int projectionIndex) {
      MercatorProjection *nativeProjection = gProjectionArray->GetAndRemove(projectionIndex);

      if (!nativeProjection) {
	 printf("jniDestructor(): NULL Projection object");
      } else
	 delete nativeProjection;
   }

   jboolean
   Java_osm_scout_MercatorProjection_jniSet(JNIEnv *env, jobject object, int projectionIndex,
	 jdouble lon, jdouble lat, jdouble magnification, jint width, jint height) {
      MercatorProjection *nativeProjection = gProjectionArray->Get(projectionIndex);

      if (!nativeProjection) {
	 printf("jniSet(): NULL Projection object");
	 return JNI_FALSE;
      }

      return nativeProjection->Set(lon, lat, magnification, width, height);
   }

   jboolean
   Java_osm_scout_MercatorProjection_jniSetBounds(JNIEnv *env, jobject object, int projectionIndex,
	 jdouble minLon, jdouble minLat, jdouble maxLon, jdouble maxLat, jdouble magnification,
	 jint width) {
      MercatorProjection *nativeProjection = gProjectionArray->Get(projectionIndex);

      if (!nativeProjection) {
	 printf("jniSet(): NULL Projection object");
	 return JNI_FALSE;
      }

      return nativeProjection->Set(minLon, minLat, maxLon, maxLat, magnification, width);
   }

   jobject
   Java_osm_scout_MercatorProjection_jniGetBoundaries(JNIEnv *env, jobject object,
	 int projectionIndex) {
      MercatorProjection *nativeProjection = gProjectionArray->Get(projectionIndex);

      if (!nativeProjection) {
	 printf("jniGetBoundaries(): NULL Projection object");
	 return NULL;
      }

      jclass javaClass = env->FindClass("osm/scout/GeoBox");
      jmethodID methodId = env->GetMethodID(javaClass, "<init>", "(DDDD)V");
      jobject geoBox = env->NewObject(javaClass, methodId, nativeProjection->GetLonMin(),
	    nativeProjection->GetLonMax(), nativeProjection->GetLatMin(),
	    nativeProjection->GetLatMax());

      return geoBox;
   }

   jobject
   Java_osm_scout_MercatorProjection_jniPixelToGeo(JNIEnv *env, jobject object, int projectionIndex,
	 double x, double y) {
      MercatorProjection *nativeProjection = gProjectionArray->Get(projectionIndex);

      if (!nativeProjection) {
	 printf("jniPixelToGeo(): NULL Projection object");
	 return NULL;
      }

      double lon, lat;

      if (!nativeProjection->PixelToGeo(x, y, lon, lat)) return NULL;

      jclass javaClass = env->FindClass("osm/scout/GeoPos");
      jmethodID methodId = env->GetMethodID(javaClass, "<init>", "(DD)V");
      jobject geoPos = env->NewObject(javaClass, methodId, lon, lat);

      return geoPos;
   }

   jobject
   Java_osm_scout_MercatorProjection_jniGeoToPixel(JNIEnv *env, jobject object, int projectionIndex,
	 double lon, double lat) {
      MercatorProjection *nativeProjection = gProjectionArray->Get(projectionIndex);

      if (!nativeProjection) {
	 printf("jniGeoToPixel(): NULL Projection object");
	 return NULL;
      }

      double x, y;

      if (!nativeProjection->GeoToPixel(lon, lat, x, y)) return NULL;

      jclass javaClass = env->FindClass("android/graphics/PointF");
      jmethodID methodId = env->GetMethodID(javaClass, "<init>", "(FF)V");
      jobject point = env->NewObject(javaClass, methodId, (float) x, (float) y);

      return point;
   }

#ifdef __cplusplus
}
#endif

