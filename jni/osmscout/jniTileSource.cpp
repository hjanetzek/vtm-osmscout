/*
 This source is part of the libosmscout library
 Copyright (C) 2010  Tim Teulings
 Copyright (C) 2014  Hannes Janetzek

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
#include <math.h>

#include <osmscout/Database.h>
#include <osmscout/Node.h>
#include <osmscout/MapPainter.h>

#include <jniTileSource.h>
#include <jniObjectArray.h>

#define DEBUG_TAG "OsmScoutJni:TileSource"
#include <log.h>

//#ifdef __ANDROID__
//#include <android/log.h>
//#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, __VA_ARGS__)
//#endif

using namespace osmscout;

extern JniObjectArray<MapData> *gMapDataArray;
extern JniObjectArray<TileSource> *gMapPainterArray;
extern JniObjectArray<MapParameter> *gMapParameterArray;
extern JniObjectArray<MercatorProjection> *gProjectionArray;
extern JniObjectArray<StyleConfig> *gStyleConfigArray;

namespace osmscout {

   TileSource::TileSource(JNIEnv *env, jobject object)
   : coordBuffer(new CoordBufferImpl<Vertex2D>()),
	       transBuffer(coordBuffer) {

      mJniEnv = env;

      jclass c = mJniEnv->FindClass("org/oscim/osmscout/TileDataSource");
      mProcessArea = mJniEnv->GetMethodID(c, "processArea", "()V");
      mProcessPath = mJniEnv->GetMethodID(c, "processPath", "()V");

      mAddTag = mJniEnv->GetMethodID(c, "addTag", "(Ljava/lang/String;Ljava/lang/String;)V");
      mAddTag2 = mJniEnv->GetMethodID(c, "addTag", "(Lorg/oscim/core/Tag;)V");

      mGetPoints = mJniEnv->GetMethodID(c, "ensurePointSize", "(I)[F");
      mGetIndices = mJniEnv->GetMethodID(c, "ensureIndexSize", "(I)[S");

      c = mJniEnv->FindClass("org/oscim/core/Tag");
      mTagClass = reinterpret_cast<jclass>(mJniEnv->NewGlobalRef(c));
      mNewTag = mJniEnv->GetMethodID(c, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
   }

   TileSource::~TileSource() {
      //mJniEnv->DeleteGlobalRef(mMapElement);
      //mJniEnv->DeleteGlobalRef(mMapElementClass);
      mJniEnv->DeleteGlobalRef(mTagClass);

      for (std::vector<jstring>::const_iterator it = mTagKeys.begin(); it != mTagKeys.end(); it++) {
	 mJniEnv->DeleteGlobalRef((jobject) &it);
      }
      mTagKeys.clear();

      for (std::vector<jobject>::const_iterator it = mTags.begin(); it != mTags.end(); it++) {
	 mJniEnv->DeleteGlobalRef((jobject) &it);
      }

      mTags.clear();

      printf("< cleared >");
   }


   void
   TileSource::PrepareAreas(const StyleConfig& styleConfig, const Projection& projection,
	 const MapParameter& parameter, const MapData& data) {
      //areaData.clear();

      for (std::vector<AreaRef>::const_iterator a = data.areas.begin();
	    a != data.areas.end(); ++a) {

	 const AreaRef& area = *a;

	 std::vector<PolyData> data(area->rings.size());

	 for (size_t i = 0; i < area->rings.size(); i++) {
	    if (area->rings[i].ring == Area::masterRingId) {
	       continue;
	    }

	    transBuffer.TransformArea(projection, parameter.GetOptimizeAreaNodes(),
		  area->rings[i].nodes, data[i].transStart, data[i].transEnd,
		  parameter.GetOptimizeErrorToleranceDots());
	 }

	 size_t ringId = Area::outerRingId;
	 bool foundRing = true;

	 while (foundRing) {
	    foundRing = false;

	    for (size_t i = 0; i < area->rings.size(); i++) {
	       const Area::Ring& ring = area->rings[i];

	       if (ring.ring == ringId) {
		  FillStyleRef fillStyle;

		  if (ring.ring == Area::outerRingId) {
		     styleConfig.GetAreaFillStyle(area->GetType(), ring.GetAttributes(),
			   projection, parameter.GetDPI(), fillStyle);

		  } else if (ring.GetType() != typeIgnore) {
		     styleConfig.GetAreaFillStyle(ring.GetType(), ring.GetAttributes(),
			   projection, parameter.GetDPI(), fillStyle);
		  }

		  if (fillStyle.Invalid()) {
		     continue;
		  }

		  foundRing = true;

		  if (!IsVisible(projection, ring.nodes, fillStyle->GetBorderWidth() / 2)) {
		     areasDrawn++;
		     continue;
		  }

		  DrawArea(data, area, i, ringId);
		  areasSegments++;
	       }
	    }

	    ringId++;
	 }
      }
   }

   bool
   TileSource::IsVisible(const Projection& projection,
	 const std::vector<GeoCoord>& nodes,
	 double pixelOffset) const
	 {
      if (nodes.empty()) {
	 return false;
      }

      // Bounding box
      double lonMin = nodes[0].GetLon();
      double lonMax = nodes[0].GetLon();
      double latMin = nodes[0].GetLat();
      double latMax = nodes[0].GetLat();

      for (size_t i = 1; i < nodes.size(); i++) {
	 lonMin = std::min(lonMin, nodes[i].GetLon());
	 lonMax = std::max(lonMax, nodes[i].GetLon());
	 latMin = std::min(latMin, nodes[i].GetLat());
	 latMax = std::max(latMax, nodes[i].GetLat());
      }

      double xMin;
      double xMax;
      double yMin;
      double yMax;
      double y1;
      double y2;

      if (!projection.GeoToPixel(lonMin,
	    latMax,
	    xMin,
	    y1)) {
	 return false;
      }

      if (!projection.GeoToPixel(lonMax,
	    latMin,
	    xMax,
	    y2)) {
	 return false;
      }

      yMax = std::max(y1, y2);
      yMin = std::min(y1, y2);

      xMin -= pixelOffset;
      yMin -= pixelOffset;

      xMax += pixelOffset;
      yMax += pixelOffset;

      return !(xMin >= projection.GetWidth() ||
	    yMin >= projection.GetHeight() ||
	    xMax < 0 ||
	    yMax < 0);
   }
   void
   TileSource::DrawArea(const std::vector<PolyData>& data, const AreaRef& area,
	 size_t outerId, size_t ringId) {
      int allPoints = 0;
      int numRings = 2; // plus 1 to set end marker

      size_t j = outerId + 1;
      while (j < area->rings.size() && area->rings[j].ring == ringId + 1
	    && area->rings[j].GetType() == typeIgnore) {

	 const PolyData& clipData = data[j];
	 allPoints += clipData.transEnd - clipData.transStart + 1;
	 numRings++;

	 j++;
      }

      int numPoints = data[outerId].transEnd - data[outerId].transStart + 1;
      allPoints += numPoints;

      unsigned char isCopy = 0;
      jobject obj_points = mJniEnv->CallObjectMethod(mTileSourceObject, mGetPoints, (jint) allPoints);
      jfloatArray arr_points = reinterpret_cast<jfloatArray>(obj_points);

      float *points = (float*) mJniEnv->GetPrimitiveArrayCritical(arr_points, &isCopy);
      if (points == NULL) {
	 return;
      }

      jobject obj_indices = mJniEnv->CallObjectMethod(mTileSourceObject, mGetIndices, (jint) numRings);
      jshortArray arr_indices = reinterpret_cast<jshortArray>(obj_indices);

      short *indices = (short*) mJniEnv->GetPrimitiveArrayCritical(arr_indices, &isCopy);

      if (indices == NULL) {
	 mJniEnv->ReleasePrimitiveArrayCritical(arr_points, points, JNI_ABORT);
	 return;
      }

      indices[0] = numPoints * 2;

      size_t transStart = data[outerId].transStart;

      for (int i = 0; i < numPoints; i++) {
	 points[(i * 2) + 0] = (float) coordBuffer->buffer[transStart + i].GetX();
	 points[(i * 2) + 1] = (float) coordBuffer->buffer[transStart + i].GetY();
      }
      int ring = 1;

      int pos = numPoints * 2;

      if (numRings > 2) {

	 j = outerId + 1;
	 while (j < area->rings.size() && area->rings[j].ring == ringId + 1
	       && area->rings[j].GetType() == typeIgnore) {

	    const PolyData& clipData = data[j];
	    int numPoints = clipData.transEnd - clipData.transStart + 1;

	    for (int i = 0; i < numPoints; i++) {
	       points[pos + (i * 2) + 0] =
		     (float) coordBuffer->buffer[clipData.transStart + i].GetX();
	       points[pos + (i * 2) + 1] =
		     (float) coordBuffer->buffer[clipData.transStart + i].GetY();
	    }
	    pos += numPoints * 2;
	    indices[ring++] = numPoints * 2;

	    j++;
	 }
      }

      // end marker
      indices[numRings - 1] = -1;

      mJniEnv->ReleasePrimitiveArrayCritical(arr_points, points, JNI_ABORT);
      mJniEnv->ReleasePrimitiveArrayCritical(arr_indices, indices, JNI_ABORT);
      mJniEnv->DeleteLocalRef(obj_points);
      mJniEnv->DeleteLocalRef(obj_indices);

      TypeId id = area->GetType();
      const std::vector<Tag>& tags = area->rings[outerId].attributes.GetTags();

      if (id >= 0 && id < mTags.size())
	 mJniEnv->CallVoidMethod(mTileSourceObject, mAddTag2, mTags[id]);

      for (int i = 0, n = tags.size(); i < n; i++) {
	 const Tag& tag = tags[i];
	 //printf("add tag %d %d %s\n", n, tag.key, tag.value.c_str());
	 jstring value = mJniEnv->NewStringUTF(tag.value.c_str());
	 mJniEnv->CallVoidMethod(mTileSourceObject, mAddTag, mTagKeys[tag.key], value);
	 mJniEnv->DeleteLocalRef(value);
      }

      mJniEnv->CallVoidMethod(mTileSourceObject, mProcessArea);

   }

   void
   TileSource::PrepareWays(const StyleConfig& styleConfig, const Projection& projection,
	 const MapParameter& parameter, const MapData& data) {

      size_t transStart = 0;
      size_t transEnd = 0;

      for (std::vector<WayRef>::const_iterator w = data.ways.begin(); w != data.ways.end(); ++w) {
	 const WayRef& way = *w;

//	 PrepareWaySegment(styleConfig, projection, parameter,
//	       ObjectFileRef(way->GetFileOffset(), refWay), way->GetAttributes(), way->nodes,
//	       way->ids);

	 if (!IsVisible(projection, way->nodes, 4)) {
	    waysDrawn++;
	    continue;
	 }

	 transBuffer.TransformWay(projection, parameter.GetOptimizeWayNodes(), way->nodes,
	       transStart, transEnd, parameter.GetOptimizeErrorToleranceDots());

	 int numPoints = transEnd - transStart + 1;

	 unsigned char isCopy = 0;
	 jobject obj_points = mJniEnv->CallObjectMethod(mTileSourceObject, mGetPoints, (jint) numPoints);
	 jfloatArray arr_points = reinterpret_cast<jfloatArray>(obj_points);

	 float *points = (float*) mJniEnv->GetPrimitiveArrayCritical(arr_points, &isCopy);
	 if (points == NULL) {
	    return;
	 }

	 jobject obj_indices = mJniEnv->CallObjectMethod(mTileSourceObject, mGetIndices, (jint) 1);
	 jshortArray arr_indices = reinterpret_cast<jshortArray>(obj_indices);

	 short *indices = (short*) mJniEnv->GetPrimitiveArrayCritical(arr_indices, &isCopy);

	 if (indices == NULL) {
	    mJniEnv->ReleasePrimitiveArrayCritical(arr_points, points, JNI_ABORT);
	    return;
	 }

	 indices[0] = numPoints * 2;

	 for (int i = 0; i < numPoints; i++) {
	    points[(i * 2) + 0] = (float) coordBuffer->buffer[transStart + i].GetX();
	    points[(i * 2) + 1] = (float) coordBuffer->buffer[transStart + i].GetY();
	 }

	 indices[1] = -1;

	 mJniEnv->ReleasePrimitiveArrayCritical(arr_points, points, JNI_ABORT);
	 mJniEnv->ReleasePrimitiveArrayCritical(arr_indices, indices, JNI_ABORT);
	 mJniEnv->DeleteLocalRef(obj_points);
	 mJniEnv->DeleteLocalRef(obj_indices);

	 const TypeId id = way->GetType();

	 if (id >= 0 && id < mTags.size())
	    mJniEnv->CallVoidMethod(mTileSourceObject, mAddTag2, mTags[id]);

	 const std::string& name = way->GetName();
	 if (name.length() > 0) {
	    //printf("add name %s\n", name.c_str());
	    jstring value = mJniEnv->NewStringUTF(name.c_str());
	    mJniEnv->CallVoidMethod(mTileSourceObject, mAddTag, mTagKeys[1], value);
	    mJniEnv->DeleteLocalRef(value);
	 }

	 for (int i = 0, n = way->GetAttributes().GetTags().size(); i < n; i++) {
	    const Tag& tag = way->GetAttributes().GetTags()[i];

	    //printf("add tag %d %d %s\n", n, tag.key, tag.value.c_str());
	    jstring value = mJniEnv->NewStringUTF(tag.value.c_str());
	    mJniEnv->CallVoidMethod(mTileSourceObject, mAddTag, mTagKeys[tag.key], value);
	    mJniEnv->DeleteLocalRef(value);
	 }

	 mJniEnv->CallVoidMethod(mTileSourceObject, mProcessPath);

      }
   }

   void
   TileSource::DrawGround(const Projection& projection, const MapParameter& parameter,
	 const FillStyle& style) {

   }


   bool
    TileSource::DrawMap(const StyleConfig& styleConfig, const Projection& projection,
 	 const MapParameter& parameter, const MapData& data, JNIEnv *env, jobject object) {
       mJniEnv = env;
       mPainterClass = env->FindClass("org/oscim/osmscout/TileDataSource");
       mTileSourceObject = object;

       mMinimumLineWidth = parameter.GetLineMinWidthPixel() * 25.4 / parameter.GetDPI();

       if (mTagKeys.size() == 0) initKeys(styleConfig);

       return Draw(styleConfig, projection, parameter, data);
    }

    void
    TileSource::initKeys(const StyleConfig& styleConfig) {
       jclass c = mJniEnv->FindClass("java/lang/String");

       jmethodID internalize = mJniEnv->GetMethodID(c, "intern", "()Ljava/lang/String;");
       jmethodID makeTag = mJniEnv->GetStaticMethodID(mPainterClass, "makeTag",
 	    "(Ljava/lang/String;)Lorg/oscim/core/Tag;");

       for (std::vector<TypeInfo>::const_iterator tags =
 	    styleConfig.GetTypeConfig()->GetTypes().begin();
 	    tags != styleConfig.GetTypeConfig()->GetTypes().end(); tags++) {
 	 const TypeInfo tag = *tags;
 	 std::string s = tag.GetName();

 	 jstring tagstring = mJniEnv->NewStringUTF(s.c_str());
 	 jobject jtag = mJniEnv->CallStaticObjectMethod(mPainterClass, makeTag, tagstring);

 	 mTags.push_back(mJniEnv->NewGlobalRef(jtag));

 	 mJniEnv->DeleteLocalRef(jtag);
 	 mJniEnv->DeleteLocalRef(tagstring);

 	 // printf("tags >> %s \n", s.c_str());
       }

       for (std::vector<TagInfo>::const_iterator tags =
 	    styleConfig.GetTypeConfig()->GetTags().begin();
 	    tags != styleConfig.GetTypeConfig()->GetTags().end(); tags++) {
 	 const TagInfo tag = *tags;
 	 std::string s = tag.GetName();

 	 jstring js = mJniEnv->NewStringUTF(s.c_str());
 	 jobject internal = mJniEnv->CallObjectMethod(js, internalize);

 	 mTagKeys.push_back(reinterpret_cast<jstring>(mJniEnv->NewGlobalRef(internal)));

 	 mJniEnv->DeleteLocalRef(js);
 	 mJniEnv->DeleteLocalRef(internal);

 	 //printf("keys >> %s \n", s.c_str());
       }
    }

    bool
    TileSource::Draw(const StyleConfig& styleConfig, const Projection& projection,
 	 const MapParameter& parameter, const MapData& data) {

       waysSegments = 0;
       waysDrawn = 0;

       areasSegments = 0;
       areasDrawn = 0;

       nodesDrawn = 0;

       transBuffer.Reset();

       if (parameter.IsAborted()) {
 	 return false;
       }

       if (parameter.IsDebugPerformance()) {
 	 std::cout << "Draw: [";
 	 std::cout << projection.GetLatMin() << ",";
 	 std::cout << projection.GetLonMin() << "-";
 	 std::cout << projection.GetLatMax() << ",";
 	 std::cout << projection.GetLonMax() << "] ";
 	 std::cout << projection.GetMagnification().GetMagnification() << "x" << "/"
 	       << projection.GetMagnification().GetLevel() << " ";
 	 std::cout << projection.GetWidth() << "x" << projection.GetHeight() << " "
 	       << parameter.GetDPI() << " DPI" << std::endl;
       }

       PrepareAreas(styleConfig, projection, parameter, data);

       PrepareWays(styleConfig, projection, parameter, data);

       printf("ways drawn %d, areas drawn %d", (int) waysDrawn, (int) areasDrawn);
       return true;
    }

}

#ifdef __cplusplus
extern "C" {
#endif

#define JNI(X) JNIEXPORT Java_org_oscim_osmscout_TileDataSource_##X

   jint
   JNI(jniConstructor)(JNIEnv *env, jobject object) {
      TileSource *nativeMapPainter = new TileSource(env, object);

      return gMapPainterArray->Add(nativeMapPainter);
   }

   void
   JNI(jniDestructor)(JNIEnv *env, jobject object, int mapPainterIndex) {
      TileSource *nativeMapPainter = gMapPainterArray->GetAndRemove(mapPainterIndex);

      if (!nativeMapPainter)
	 printf("jniDestructor(): NULL object");
      else
	 delete nativeMapPainter;
   }

   jboolean
   JNI(jniDrawMap)(JNIEnv *env, jobject object, int mapPainterIndex,
	 int styleConfigIndex, int projectionIndex, int mapParameterIndex, int mapDataIndex) {
      TileSource *nativeMapPainter = gMapPainterArray->Get(mapPainterIndex);

      if (!nativeMapPainter) {
	 printf("jniDrawMap(): NULL MapPainter object\n");
	 return JNI_FALSE;
      }

      StyleConfig *nativeStyleConfig = gStyleConfigArray->Get(styleConfigIndex);

      if (!nativeStyleConfig) {
	 printf("jniDrawMap(): NULL StyleConfig pointer\n");
	 return JNI_FALSE;
      }

      MercatorProjection *nativeProjection = gProjectionArray->Get(projectionIndex);

      if (!nativeProjection) {
	 printf("jniDrawMap(): NULL Projection pointer\n");
	 return JNI_FALSE;
      }

      MapParameter *nativeMapParameter = gMapParameterArray->Get(mapParameterIndex);

      if (!nativeMapParameter) {
	 printf("jniDrawMap(): NULL MapParameter pointer\n");
	 return JNI_FALSE;
      }

      MapData *nativeMapData = gMapDataArray->Get(mapDataIndex);

      if (!nativeMapData) {
	 printf("jniDrawMap(): NULL MapData pointer\n");
	 return JNI_FALSE;
      }

      bool result = nativeMapPainter->DrawMap(*nativeStyleConfig, *nativeProjection,
	    *nativeMapParameter, *nativeMapData, env, object);

      if (result) {
	 printf("jniDrawMap(): DrawMap() Ok!\n");
      } else {
	 printf("jniDrawMap(): DrawMap() failed!\n");
      }

      return result;
   }

#ifdef __cplusplus
}
#endif

