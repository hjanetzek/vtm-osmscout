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
#define DEBUG_TAG "OsmScoutJni:MapPainterCanvas"
#define printf(...) __android_log_print(ANDROID_LOG_DEBUG, DEBUG_TAG, __VA_ARGS__)
#endif

#include <math.h>

#include <osmscout/AdminRegion.h>
#include <osmscout/Database.h>
#include <osmscout/Node.h>
#include <osmscout/MapPainter.h>

#include <jniMapPainterCanvas.h>
#include <jniObjectArray.h>

using namespace osmscout;

extern JniObjectArray<MapData> *gMapDataArray;
extern JniObjectArray<MapPainterCanvas> *gMapPainterArray;
extern JniObjectArray<MapParameter> *gMapParameterArray;
extern JniObjectArray<MercatorProjection> *gProjectionArray;
extern JniObjectArray<StyleConfig> *gStyleConfigArray;

namespace osmscout {

   MapPainterCanvas::MapPainterCanvas(JNIEnv *env) :
	 MapPainter(new CoordBufferImpl<Vertex2D>()), coordBuffer(
	       (CoordBufferImpl<Vertex2D>*) transBuffer.buffer) {
      mJniEnv = env;
      jclass c = mJniEnv->FindClass("org/oscim/core/MapElement");
      jmethodID rectMethodId = mJniEnv->GetMethodID(c, "<init>", "(II)V");

      mMapElement = mJniEnv->NewObject(c, rectMethodId, (jint) 1024, (jint) 32);
      mMapElement = mJniEnv->NewGlobalRef(mMapElement);
      mMapElementClass = reinterpret_cast<jclass>(mJniEnv->NewGlobalRef(c));

      mGetPoints = mJniEnv->GetMethodID(c, "ensurePointSize", "(I)[F");
      mGetIndices = mJniEnv->GetMethodID(c, "ensureIndexSize", "(I)[S");
      mAddTag = mJniEnv->GetMethodID(c, "addTag", "(Ljava/lang/String;Ljava/lang/String;)V");
      mAddTag2 = mJniEnv->GetMethodID(c, "addTag", "(Lorg/oscim/core/Tag;)V");

      c = mJniEnv->FindClass("osm/scout/MapPainterCanvas");
      mProcessArea = mJniEnv->GetMethodID(c, "processArea", "(Lorg/oscim/core/MapElement;)V");
      mProcessPath = mJniEnv->GetMethodID(c, "processPath", "(Lorg/oscim/core/MapElement;)V");

      c = mJniEnv->FindClass("org/oscim/core/Tag");
      mTagClass = reinterpret_cast<jclass>(mJniEnv->NewGlobalRef(c));
      mNewTag = mJniEnv->GetMethodID(c, "<init>", "(Ljava/lang/String;Ljava/lang/String;)V");
   }

   MapPainterCanvas::~MapPainterCanvas() {
      mJniEnv->DeleteGlobalRef(mMapElement);
      mJniEnv->DeleteGlobalRef(mMapElementClass);
      mJniEnv->DeleteGlobalRef(mTagClass);

      for (std::vector<jstring>::const_iterator it = mTagKeys.begin(); it != mTagKeys.end(); it++) {
	 mJniEnv->DeleteGlobalRef((jobject) &it);
      }
      mTagKeys.clear();

      for (std::vector<jobject>::const_iterator it = mTags.begin(); it != mTags.end(); it++) {
	 mJniEnv->DeleteGlobalRef((jobject) &it);
      }

      mTags.clear();

      printf("cleared!!!");
   }

   bool
   MapPainterCanvas::HasIcon(const StyleConfig& styleConfig, const MapParameter& parameter,
	 IconStyle& style) {
      // Already loaded with error
      if (style.GetIconId() == 0) {
	 return false;
      }

      unsigned int iconIndex = style.GetIconId() - 1;

      if (iconIndex < mIconLoaded.size() && mIconLoaded[iconIndex]) {
	 return true;
      }

      jmethodID methodId = mJniEnv->GetMethodID(mPainterClass, "loadIconPNG",
	    "(Ljava/lang/String;I)Z");

      if (!methodId) return false;

      for (std::list<std::string>::const_iterator path = parameter.GetIconPaths().begin();
	    path != parameter.GetIconPaths().end(); ++path) {
	 std::string filename = *path + "/" + style.GetIconName() + ".png";

	 jstring iconName = mJniEnv->NewStringUTF(filename.c_str());

	 bool loaded = mJniEnv->CallBooleanMethod(mPainterObject, methodId, iconName, iconIndex);

	 mJniEnv->DeleteLocalRef(iconName);

	 if (loaded) {
	    if (iconIndex >= mIconLoaded.size()) {
	       mIconLoaded.resize(iconIndex + 1, false);
	    }

	    mIconLoaded[iconIndex] = true;

	    return true;
	 }
      }

      // Error loading icon file
      style.SetIconId(0);
      return false;
   }

   bool
   MapPainterCanvas::HasPattern(const MapParameter& parameter, const FillStyle& style) {
      // Pattern already loaded with error
      if (style.GetPatternId() == 0) {
	 return false;
      }

      unsigned int patternIndex = style.GetPatternId() - 1;

      if (patternIndex < mPatternLoaded.size() && mPatternLoaded[patternIndex]) {
	 return true;
      }

      jmethodID methodId = mJniEnv->GetMethodID(mPainterClass, "loadPatternPNG",
	    "(Ljava/lang/String;I)Z");

      if (!methodId) return false;

      for (std::list<std::string>::const_iterator path = parameter.GetPatternPaths().begin();
	    path != parameter.GetPatternPaths().end(); ++path) {
	 std::string filename = *path + "/" + style.GetPatternName() + ".png";

	 jstring patternName = mJniEnv->NewStringUTF(filename.c_str());

	 bool loaded = mJniEnv->CallBooleanMethod(mPainterObject, methodId, patternName,
	       patternIndex);

	 mJniEnv->DeleteLocalRef(patternName);

	 if (loaded) {
	    if (patternIndex >= mPatternLoaded.size()) {
	       mPatternLoaded.resize(patternIndex + 1, false);
	    }

	    mPatternLoaded[patternIndex] = true;

	    return true;
	 }
      }

      // Error loading icon file
      style.SetPatternId(0);
      return false;
   }

   void
   MapPainterCanvas::GetTextDimension(const MapParameter& parameter, double fontSize,
	 const std::string& text, double& xOff, double& yOff, double& width, double& height) {
      /// FIXME
      return;

      jmethodID methodId = mJniEnv->GetMethodID(mPainterClass, "getTextDimension",
	    "(Ljava/lang/String;F)Landroid/graphics/Rect;");

      if (!methodId) return;

      jfloat javaFontSize = fontSize;

      jstring javaText = mJniEnv->NewStringUTF(text.c_str());

      jobject javaRect = mJniEnv->CallObjectMethod(mPainterObject, methodId, javaText,
	    javaFontSize);

      mJniEnv->DeleteLocalRef(javaText);

      xOff = yOff = 0.0;

      jclass rectClass = mJniEnv->FindClass("android/graphics/Rect");

      methodId = mJniEnv->GetMethodID(rectClass, "width", "()I");
      width = (double) mJniEnv->CallIntMethod(javaRect, methodId);

      methodId = mJniEnv->GetMethodID(rectClass, "height", "()I");
      height = (double) mJniEnv->CallIntMethod(javaRect, methodId);

      mJniEnv->DeleteLocalRef(javaRect);
      mJniEnv->DeleteLocalRef(rectClass);
   }

   void
   MapPainterCanvas::DrawLabel(const Projection& projection, const MapParameter& parameter,
	 const LabelData& label) {
      jstring javaText;
      jint javaTextColor;
      jint javaTextStyle;
      jfloat javaFontSize;
      jfloat javaX;
      jfloat javaY;
      jobject javaBox;
      jint javaBgColor;
      jint javaBorderColor;

      /// FIXME
      return;

      javaText = mJniEnv->NewStringUTF(label.text.c_str());
      javaFontSize = label.fontSize;
      javaX = (jfloat) label.x;
      javaY = (jfloat) label.y;

      if (dynamic_cast<const TextStyle*>(label.style.Get()) != NULL) {

	 const TextStyle* style = dynamic_cast<const TextStyle*>(label.style.Get());

	 javaTextColor = GetColorInt(style->GetTextColor().GetR(), style->GetTextColor().GetG(),
	       style->GetTextColor().GetB(), style->GetTextColor().GetA());

	 javaTextStyle = style->GetStyle();

	 javaBox = NULL;
      } else if (dynamic_cast<const ShieldStyle*>(label.style.Get()) != NULL) {

	 const ShieldStyle* style = dynamic_cast<const ShieldStyle*>(label.style.Get());

	 javaTextColor = GetColorInt(style->GetTextColor().GetR(), style->GetTextColor().GetG(),
	       style->GetTextColor().GetB(), style->GetTextColor().GetA());

	 javaTextStyle = TextStyle::normal;

	 jclass rectClass = mJniEnv->FindClass("android/graphics/RectF");
	 jmethodID rectMethodId = mJniEnv->GetMethodID(rectClass, "<init>", "(FFFF)V");
	 javaBox = mJniEnv->NewObject(rectClass, rectMethodId, (jfloat) label.bx1,
	       (jfloat) label.by1 + 1, (jfloat) label.bx2 + 1, (jfloat) label.by2 + 2);

	 javaBgColor = GetColorInt(style->GetBgColor());
	 javaBorderColor = GetColorInt(style->GetBorderColor());
      } else
	 return;

      jmethodID methodId = mJniEnv->GetMethodID(mPainterClass, "drawLabel",
	    "(Ljava/lang/String;FFFIILandroid/graphics/RectF;II)V");

      if (!methodId) return;

      mJniEnv->CallVoidMethod(mPainterObject, methodId, javaText, javaFontSize, javaX, javaY,
	    javaTextColor, javaTextStyle, javaBox, javaBgColor, javaBorderColor);

      mJniEnv->DeleteLocalRef(javaText);
   }

   void
   MapPainterCanvas::DrawContourLabel(const Projection& projection, const MapParameter& parameter,
	 const PathTextStyle& style, const std::string& text, size_t transStart, size_t transEnd) {
//    jmethodID methodId=mJniEnv->GetMethodID(mPainterClass, "drawContourLabel",
//                     "(Ljava/lang/String;IFF[F[F)V");
//
//    if (!methodId)
//      return;
//
//    jstring javaText=mJniEnv->NewStringUTF(text.c_str());
//
//    jint textColor=GetColorInt(style.GetTextColor());
//
//    jfloat pathLenght=0.0;
//
//    int numPoints=transEnd-transStart+1;
//
//    float *x=new float[numPoints];
//    float *y=new float[numPoints];
//
//    if (transBuffer.buffer[transStart].x<=transBuffer.buffer[transEnd].x)
//    {
//      // Path orientation is from left to right
//      // Direct copy of the data
//
//      for(int i=0; i<numPoints; i++)
//      {
//        x[i]=(float)transBuffer.buffer[transStart+i].getX();
//        y[i]=(float)transBuffer.buffer[transStart+i].getY();
//
//        if (i!=0)
//          pathLenght+=sqrt(pow(x[i]-x[i-1], 2.0)+pow(y[i]-y[i-1], 2.0));
//      }
//    }
//    else
//    {
//      // Path orientation is from right to left
//      // Inverse copy of the data
//
//      for(int i=0; i<numPoints; i++)
//      {
//        x[i]=(float)transBuffer.buffer[transEnd-i].getX();
//        y[i]=(float)transBuffer.buffer[transEnd-i].getY();
//
//        if (i!=0)
//          pathLenght+=sqrt(pow(x[i]-x[i-1], 2.0)+pow(y[i]-y[i-1], 2.0));
//      }
//    }
//
//    jfloatArray jArrayX=mJniEnv->NewFloatArray(numPoints);
//    jfloatArray jArrayY=mJniEnv->NewFloatArray(numPoints);
//
//    mJniEnv->SetFloatArrayRegion(jArrayX, 0, numPoints, x);
//    mJniEnv->SetFloatArrayRegion(jArrayY, 0, numPoints, y);
//
//    mJniEnv->CallVoidMethod(mPainterObject, methodId, javaText,
//                            textColor, (jfloat)style.GetSize(),
//                            pathLenght, jArrayX, jArrayY);
//
//    delete x;
//    delete y;
//
//    mJniEnv->DeleteLocalRef(jArrayX);
//    mJniEnv->DeleteLocalRef(jArrayY);
   }

   void
   MapPainterCanvas::DrawPrimitivePath(const Projection& projection, const MapParameter& parameter,
	 const DrawPrimitiveRef& p, double x, double y, double minX, double minY, double maxX,
	 double maxY) {
      DrawPrimitivePath(projection, parameter, p, x, y, minX, minY, maxX, maxY,
      NULL, NULL, NULL);
   }

   void
   MapPainterCanvas::DrawPrimitivePath(const Projection& projection, const MapParameter& parameter,
	 const DrawPrimitiveRef& p, double x, double y, double minX, double minY, double maxX,
	 double maxY, double* onPathX, double* onPathY, double* segmentLengths) {
      DrawPrimitive* primitive = p.Get();
      double width = maxX - minX;
      double height = maxY - minY;

      if (dynamic_cast<PolygonPrimitive*>(primitive) != NULL) {
	 PolygonPrimitive* polygon = dynamic_cast<PolygonPrimitive*>(primitive);

	 int numPoints = polygon->GetCoords().size();

	 float *arrayX = new float[numPoints];
	 float *arrayY = new float[numPoints];

	 int i = 0;

	 for (std::list<Coord>::const_iterator pixel = polygon->GetCoords().begin();
	       pixel != polygon->GetCoords().end(); ++pixel) {
	    arrayX[i] = x + ConvertWidthToPixel(parameter, pixel->x - width / 2);
	    arrayY[i] = y + ConvertWidthToPixel(parameter, maxY - pixel->y - height / 2);

	    i++;
	 }

	 MapPathOnPath(arrayX, arrayY, numPoints, onPathX, onPathY, segmentLengths);

	 SetPolygonFillPath(arrayX, arrayY, numPoints);

	 delete arrayX;
	 delete arrayY;
      } else if (dynamic_cast<RectanglePrimitive*>(primitive) != NULL) {
	 RectanglePrimitive* rectangle = dynamic_cast<RectanglePrimitive*>(primitive);

	 float *arrayX = new float[4];
	 float *arrayY = new float[4];

	 // Top-left corner
	 arrayX[0] = x + ConvertWidthToPixel(parameter, rectangle->GetTopLeft().x - width / 2);
	 arrayY[0] = y
	       + ConvertWidthToPixel(parameter, maxY - rectangle->GetTopLeft().y - height / 2);

	 // Top-right corner
	 arrayX[1] = arrayX[0] + ConvertWidthToPixel(parameter, rectangle->GetWidth());
	 arrayY[1] = arrayY[0];

	 // Bottom-right corner
	 arrayX[2] = arrayX[1];
	 arrayY[2] = arrayY[0] + ConvertWidthToPixel(parameter, rectangle->GetHeight());

	 // Bottom-left corner
	 arrayX[3] = arrayX[0];
	 arrayY[3] = arrayY[2];

	 MapPathOnPath(arrayX, arrayY, 4, onPathX, onPathY, segmentLengths);

	 SetPolygonFillPath(arrayX, arrayY, 4);

	 delete arrayX;
	 delete arrayY;
      } else if (dynamic_cast<CirclePrimitive*>(primitive) != NULL) {
	 CirclePrimitive* circle = dynamic_cast<CirclePrimitive*>(primitive);

	 jmethodID methodId = mJniEnv->GetMethodID(mPainterClass, "setCircleFillPath", "(FFF)V");

	 jfloat posX = x + ConvertWidthToPixel(parameter, circle->GetCenter().x - width / 2);
	 jfloat posY = y
	       + ConvertWidthToPixel(parameter, maxY - circle->GetCenter().y - height / 2);
	 jfloat radius = ConvertWidthToPixel(parameter, circle->GetRadius());

	 mJniEnv->CallVoidMethod(mPainterObject, methodId, posX, posY, radius);
      }
   }

   void
   MapPainterCanvas::DrawSymbol(const Projection& projection, const MapParameter& parameter,
	 const Symbol& symbol, double x, double y) {
      double minX;
      double minY;
      double maxX;
      double maxY;

      symbol.GetBoundingBox(minX, minY, maxX, maxY);

      for (std::list<DrawPrimitiveRef>::const_iterator p = symbol.GetPrimitives().begin();
	    p != symbol.GetPrimitives().end(); ++p) {
	 FillStyleRef fillStyle = (*p)->GetFillStyle();

	 DrawPrimitivePath(projection, parameter, *p, x, y, minX, minY, maxX, maxY);

	 DrawFillStyle(projection, parameter, *fillStyle);
      }
   }

   void
   MapPainterCanvas::DrawContourSymbol(const Projection& projection, const MapParameter& parameter,
	 const Symbol& symbol, double space, size_t transStart, size_t transEnd) {
//    double lineLength=0;
//
//    int numPoints=transEnd-transStart+1;
//
//    double *onPathX=new double[numPoints];
//    double *onPathY=new double[numPoints];
//
//    double *segmentLengths=new double[numPoints-1];
//
//    for(int i=0; i<numPoints; i++)
//    {
//      onPathX[i]=(double)transBuffer.buffer[transStart+i].getX();
//      onPathY[i]=(double)transBuffer.buffer[transStart+i].getY();
//
//      if (i!=0)
//        segmentLengths[i-1]=sqrt(pow(onPathX[i]-onPathX[i-1], 2.0)+
//                                 pow(onPathY[i]-onPathY[i-1], 2.0));
//
//      lineLength+=segmentLengths[i-1];
//    }
//
//    double minX;
//    double minY;
//    double maxX;
//    double maxY;
//
//    symbol.GetBoundingBox(minX,minY,maxX,maxY);
//
//    double width=ConvertWidthToPixel(parameter,maxX-minX);
//    double height=ConvertWidthToPixel(parameter,maxY-minY);
//
//    for (std::list<DrawPrimitiveRef>::const_iterator p=symbol.GetPrimitives().begin();
//         p!=symbol.GetPrimitives().end();
//         ++p)
//    {
//      FillStyleRef fillStyle=(*p)->GetFillStyle();
//
//      double offset=space/2;
//
//      while (offset+width<lineLength)
//      {
//        DrawPrimitivePath(projection,
//                          parameter,
//                          *p,
//                          offset+width/2,
//                          0,
//                          minX,
//                          minY,
//                          maxX,
//                          maxY,
//                          onPathX, onPathY,
//                          segmentLengths);
//
//        DrawFillStyle(projection,
//                      parameter,
//                      *fillStyle);
//
//        offset+=width+space;
//      }
//    }
   }

   void
   MapPainterCanvas::SetPolygonFillPath(float* x, float* y, int numPoints) {
//      jfloatArray jArrayX = mJniEnv->NewFloatArray(numPoints);
//      jfloatArray jArrayY = mJniEnv->NewFloatArray(numPoints);
//
//      mJniEnv->SetFloatArrayRegion(jArrayX, 0, numPoints, x);
//      mJniEnv->SetFloatArrayRegion(jArrayY, 0, numPoints, y);
//
//      printf("set fill\n");
//
//      jmethodID methodId = mJniEnv->GetMethodID(mPainterClass, "setPolygonFillPath", "([F[F)V");
//
//      printf("set fill<\n");
//
//      mJniEnv->CallVoidMethod(mPainterObject, methodId, jArrayX, jArrayY);
//
//      mJniEnv->DeleteLocalRef(jArrayX);
//      mJniEnv->DeleteLocalRef(jArrayY);
   }

   void
   MapPainterCanvas::MapPathOnPath(float* arrayX, float* arrayY, int numPoints, double* onPathX,
	 double* onPathY, double* segmentLengths) {
      if ((onPathX == NULL) || (onPathY == NULL) || (segmentLengths == NULL)) return;

//      for (int i = 0; i < numPoints; i++) {
//	 // First, find the segment for the given point
//	 int s = 0;
//
//	 while (arrayX[i] > segmentLengths[s]) {
//	    arrayX[i] -= segmentLengths[s];
//	    s++;
//	 }
//
//	 // Relative offset in the current segment ([0..1])
//	 double ratio = arrayX[i] / segmentLengths[s];
//
//	 // Line polynomial
//	 double x = onPathX[s] * (1 - ratio) + onPathX[s + 1] * ratio;
//	 double y = onPathY[s] * (1 - ratio) + onPathY[s + 1] * ratio;
//
//	 // Line gradient
//	 double dx = -(onPathX[s] - onPathX[s + 1]);
//	 double dy = -(onPathY[s] - onPathY[s + 1]);
//
//	 // optimization for: ratio = the_y / sqrt (dx * dx + dy * dy)
//	 ratio = arrayY[i] / segmentLengths[s];
//	 x += -dy * ratio;
//	 y += dx * ratio;
//
//	 arrayX[i] = (float) x;
//	 arrayY[i] = (float) y;
//      }
   }

   void
   MapPainterCanvas::DrawIcon(const IconStyle* style, double x, double y) {
//      jmethodID methodId = mJniEnv->GetMethodID(mPainterClass, "drawIcon", "(IFF)V");
//      if (!methodId) return;
//
//      jint iconIndex = style->GetIconId() - 1;
//
//      mJniEnv->CallVoidMethod(mPainterObject, methodId, iconIndex, (jfloat) x, (jfloat) y);
   }

   void
   MapPainterCanvas::DrawPath(const Projection& projection, const MapParameter& parameter,
	 const Color& color, double width, const std::vector<double>& dash,
	 LineStyle::CapStyle startCap, LineStyle::CapStyle endCap, size_t transStart,
	 size_t transEnd) {

//      jint javaColor = GetColorInt(color);
//
//      jmethodID methodId = mJniEnv->GetMethodID(mPainterClass, "drawPath", "(IF[FZZ[F[F)V");
//
//      if (!methodId) return;
//
//      jfloatArray javaDash = NULL;
//      float *dashArray = NULL;
//
//      if (!dash.empty()) {
//
//	 javaDash = mJniEnv->NewFloatArray(dash.size());
//
//	 dashArray = new float[dash.size()];
//
//	 for (unsigned int i = 0; i < dash.size(); i++) {
//	    dashArray[i] = dash[i] * width;
//	 }
//
//	 mJniEnv->SetFloatArrayRegion(javaDash, 0, dash.size(), dashArray);
//      }
//
//      jboolean roundedStartCap = JNI_FALSE;
//      jboolean roundedEndCap = JNI_FALSE;
//
//      if (dash.empty()) {
//
//	 if (startCap == LineStyle::capRound) roundedStartCap = JNI_TRUE;
//
//	 if (endCap == LineStyle::capRound) roundedEndCap = JNI_TRUE;
//      }
//
//      int numPoints = transEnd - transStart + 1;
//
//      float *x = new float[numPoints];
//      float *y = new float[numPoints];
//
//      for (int i = 0; i < numPoints; i++) {
//	 x[i] = (float) coordBuffer->buffer[transStart + i].GetX();
//	 y[i] = (float) coordBuffer->buffer[transStart + i].GetY();
//      }
//
//      jfloatArray jArrayX = mJniEnv->NewFloatArray(numPoints);
//      jfloatArray jArrayY = mJniEnv->NewFloatArray(numPoints);
//
//      mJniEnv->SetFloatArrayRegion(jArrayX, 0, numPoints, x);
//      mJniEnv->SetFloatArrayRegion(jArrayY, 0, numPoints, y);
//
//      mJniEnv->CallVoidMethod(mPainterObject, methodId, javaColor, (jfloat) width, javaDash,
//	    roundedStartCap, roundedEndCap, jArrayX, jArrayY);
//
//      delete x;
//      delete y;
//
//      mJniEnv->DeleteLocalRef(jArrayX);
//      mJniEnv->DeleteLocalRef(jArrayY);
//
//      if (javaDash) {
//
//	 delete dashArray;
//	 mJniEnv->DeleteLocalRef(javaDash);
//      }
   }

   void
   MapPainterCanvas::DrawFillStyle(const Projection& projection, const MapParameter& parameter,
	 const FillStyle& fill) {
//      if (fill.HasPattern() && projection.GetMagnification() >= fill.GetPatternMinMag()
//	    && HasPattern(parameter, fill)) {
//
//	 jint patternId = fill.GetPatternId() - 1;
//	 printf("draw pattern\n");
//	 jmethodID methodId = mJniEnv->GetMethodID(mPainterClass, "drawPatternArea", "(I)V");
//
//	 printf("draw pattern<\n");
//
//	 if (!methodId) return;
//
//	 mJniEnv->CallVoidMethod(mPainterObject, methodId, patternId);
//      } else if (fill.GetFillColor().IsVisible()) {
//
//	 jint color = GetColorInt(fill.GetFillColor());
//	 printf("draw filled\n");
//
//	 jmethodID methodId = mJniEnv->GetMethodID(mPainterClass, "drawFilledArea", "(I)V");
//	 printf("draw filled<\n");
//
//	 if (!methodId) return;
//
//	 mJniEnv->CallVoidMethod(mPainterObject, methodId, color);
//      }
//
//      // Draw  border
//      if (fill.GetBorderWidth() > 0 && fill.GetBorderColor().IsVisible()
//	    && fill.GetBorderWidth() >= mMinimumLineWidth) {
//	 double borderWidth = ConvertWidthToPixel(parameter, fill.GetBorderWidth());
//
//	 if (borderWidth >= parameter.GetLineMinWidthPixel()) {
//
//	    jfloatArray javaDash = NULL;
//	    float *dashArray = NULL;
//
//	    std::vector<double> dash = fill.GetBorderDash();
//
//	    if (!dash.empty()) {
//	       javaDash = mJniEnv->NewFloatArray(dash.size());
//
//	       dashArray = new float[dash.size()];
//
//	       for (unsigned int i = 0; i < dash.size(); i++) {
//		  dashArray[i] = dash[i] * borderWidth;
//	       }
//
//	       mJniEnv->SetFloatArrayRegion(javaDash, 0, dash.size(), dashArray);
//	    }
//	    printf("draw border\n");
//
//	    jmethodID methodId = mJniEnv->GetMethodID(mPainterClass, "drawAreaBorder", "(IF[F)V");
//
//	    printf("draw border<\n");
//
//	    jint javaColor = GetColorInt(fill.GetBorderColor());
//
//	    mJniEnv->CallVoidMethod(mPainterObject, methodId, javaColor, (jfloat) borderWidth,
//		  javaDash);
//
//	 }
//      }
   }

//void
//MapPainterCanvas::DrawArea(const FillStyle& style, const MapParameter& parameter, double x,
//	 double y, double width, double height) {
//
//      jint color = GetColorInt(style.GetFillColor());
//
//      jmethodID methodId = mJniEnv->GetMethodID(mPainterClass, "drawArea", "(IFFFF)V");
//
//      if (!methodId) return;
//
//      mJniEnv->CallVoidMethod(mPainterObject, methodId, color, (jfloat) x, (jfloat) y,
//	    (jfloat) width, (jfloat) height);
   //  }

//   void
//   MapPainterCanvas::DrawWay(const StyleConfig& styleConfig, const Projection& projection,
//	 const MapParameter& parameter, const WayData& data) {
//      Color color=data.lineStyle->GetLineColor();
//
//      if (data.lineStyle->HasDashes() &&
//          data.lineStyle->GetGapColor().GetA()>0.0) {
//        DrawPath(projection,
//                 parameter,
//                 data.lineStyle->GetGapColor(),
//                 data.lineWidth,
//                 emptyDash,
//                 data.startIsClosed ? data.lineStyle->GetEndCap() : data.lineStyle->GetJoinCap(),
//                 data.endIsClosed ? data.lineStyle->GetEndCap() : data.lineStyle->GetJoinCap(),
//                 data.transStart,data.transEnd);
//      }
//
//      DrawPath(projection,
//               parameter,
//               color,
//               data.lineWidth,
//               data.lineStyle->GetDash(),
//               data.startIsClosed ? data.lineStyle->GetEndCap() : data.lineStyle->GetJoinCap(),
//               data.endIsClosed ? data.lineStyle->GetEndCap() : data.lineStyle->GetJoinCap(),
//               data.transStart,data.transEnd);
//
//      waysDrawn++;
   //  }

//   void
//   MapPainterCanvas::DrawWays(const StyleConfig& styleConfig, const Projection& projection,
//	 const MapParameter& parameter, const MapData& data) {
//
//      for (std::list<WayData>::const_iterator it = wayData.begin(); it != wayData.end(); ++it) {
//	 //DrawWay(styleConfig, projection, parameter, *way);
//	 const WayData& way = *it;
//
//	 //data.transStart,
//	 //data.transEnd
//
//      }
//   }

   bool
   MapPainterCanvas::DrawMap(const StyleConfig& styleConfig, const Projection& projection,
	 const MapParameter& parameter, const MapData& data, JNIEnv *env, jobject object) {
      mJniEnv = env;
      mPainterClass = env->FindClass("osm/scout/MapPainterCanvas");
      mPainterObject = object;

      mMinimumLineWidth = parameter.GetLineMinWidthPixel() * 25.4 / parameter.GetDPI();

      if (mTagKeys.size() == 0) initKeys(styleConfig);

      return Draw(styleConfig, projection, parameter, data);
   }

   void
   MapPainterCanvas::initKeys(const StyleConfig& styleConfig) {
      jclass c = mJniEnv->FindClass("java/lang/String");

      jmethodID internalize = mJniEnv->GetMethodID(c, "intern", "()Ljava/lang/String;");
      jmethodID makeTag = mJniEnv->GetStaticMethodID(mPainterClass, "makeTag",
	    "(Ljava/lang/String;)Lorg/oscim/core/Tag;");

      for (std::vector<TypeInfo>::const_iterator tags =
	    styleConfig.GetTypeConfig()->GetTypes().begin();
	    tags != styleConfig.GetTypeConfig()->GetTypes().end(); tags++) {
	 const TypeInfo tag = *tags;
	 std::string s = tag.GetName();

	 jobject jtag = mJniEnv->CallStaticObjectMethod(mPainterClass, makeTag,
	       mJniEnv->NewStringUTF(s.c_str()));

	 mTags.push_back(mJniEnv->NewGlobalRef(jtag));

	// printf("tags >> %s \n", s.c_str());
      }

      for (std::vector<TagInfo>::const_iterator tags =
	    styleConfig.GetTypeConfig()->GetTags().begin();
	    tags != styleConfig.GetTypeConfig()->GetTags().end(); tags++) {
	 const TagInfo tag = *tags;
	 std::string s = tag.GetName();

	 jstring js = reinterpret_cast<jstring>(mJniEnv->NewGlobalRef(
	       mJniEnv->CallObjectMethod(mJniEnv->NewStringUTF(s.c_str()), internalize)));
	 mTagKeys.push_back(js);

	 //printf("keys >> %s \n", s.c_str());
      }
   }

   bool
   MapPainterCanvas::Draw(const StyleConfig& styleConfig, const Projection& projection,
	 const MapParameter& parameter, const MapData& data) {

      waysSegments = 0;
      waysDrawn = 0;
      waysLabelDrawn = 0;

      areasSegments = 0;
      areasDrawn = 0;
      areasLabelDrawn = 0;

      nodesDrawn = 0;
      labelsDrawn = 0;

      labels.clear();
      overlayLabels.clear();

      transBuffer.Reset();

      labelSpace = ConvertWidthToPixel(parameter, parameter.GetLabelSpace());
      shieldLabelSpace = ConvertWidthToPixel(parameter, parameter.GetPlateLabelSpace());
      sameLabelSpace = ConvertWidthToPixel(parameter, parameter.GetSameLabelSpace());

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

      //
      // Setup and Precalculation
      //

      StopClock prepareAreasTimer;

      PrepareAreas(styleConfig, projection, parameter, data);

      prepareAreasTimer.Stop();

      if (parameter.IsAborted()) {
	 return false;
      }

      StopClock prepareWaysTimer;

      PrepareWays(styleConfig, projection, parameter, data);

      prepareWaysTimer.Stop();

      if (parameter.IsAborted()) {
	 return false;
      }

      return true;
   }

   void
   MapPainterCanvas::PrepareAreas(const StyleConfig& styleConfig, const Projection& projection,
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
		     styleConfig.GetAreaFillStyle(area->GetType(), ring.GetAttributes(), projection,
			   parameter.GetDPI(), fillStyle);

		  } else if (ring.GetType() != typeIgnore) {
		     styleConfig.GetAreaFillStyle(ring.GetType(), ring.GetAttributes(), projection,
			   parameter.GetDPI(), fillStyle);
		  }

		  if (fillStyle.Invalid()) {
		     continue;
		  }

		  foundRing = true;

		  if (!IsVisible(projection, ring.nodes, fillStyle->GetBorderWidth() / 2)) {
		     continue;
		  }

		  //printf("%s\n", styleConfig.GetTypeConfig()->GetTypeInfo(area->GetType()).GetName().c_str());
		  DrawArea(data, area, i, ringId);

		  areasSegments++;
	       }
	    }

	    ringId++;
	 }
      }
   }

   void
   MapPainterCanvas::DrawArea(const std::vector<PolyData>& data, const AreaRef& area,
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
      jobject obj_points = mJniEnv->CallObjectMethod(mMapElement, mGetPoints, (jint) allPoints);
      jfloatArray arr_points = reinterpret_cast<jfloatArray>(obj_points);

      float *points = (float*) mJniEnv->GetPrimitiveArrayCritical(arr_points, &isCopy);
      if (points == NULL) {
	 return;
      }

      jobject obj_indices = mJniEnv->CallObjectMethod(mMapElement, mGetIndices, (jint) numRings);
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

      TypeId id = area->GetType();
      const std::vector<Tag>& tags = area->rings[outerId].attributes.GetTags();

      if (id >= 0 && id < mTags.size())
	 mJniEnv->CallVoidMethod(mMapElement, mAddTag2, mTags[id]);

      for (int i = 0, n = tags.size(); i < n; i++) {
	 const Tag& tag = tags[i];
	 //printf("add tag %d %d %s\n", n, tag.key, tag.value.c_str());
	 mJniEnv->CallVoidMethod(mMapElement, mAddTag, mTagKeys[tag.key],
	       mJniEnv->NewStringUTF(tag.value.c_str()));
      }

      mJniEnv->CallVoidMethod(mPainterObject, mProcessArea, mMapElement);

   }

   void
   MapPainterCanvas::DrawArea(const Projection& projection, const MapParameter& parameter,
	 const AreaData& area) {

   }

   void
   MapPainterCanvas::PrepareWays(const StyleConfig& styleConfig, const Projection& projection,
	 const MapParameter& parameter, const MapData& data) {
      //wayData.clear();
      //wayPathData.clear();
      size_t transStart = 0; // Make the compiler happy
      size_t transEnd = 0; // Make the compiler happy

      for (std::vector<WayRef>::const_iterator w = data.ways.begin(); w != data.ways.end(); ++w) {
	 const WayRef& way = *w;

//	 PrepareWaySegment(styleConfig, projection, parameter,
//	       ObjectFileRef(way->GetFileOffset(), refWay), way->GetAttributes(), way->nodes,
//	       way->ids);

	 transBuffer.TransformWay(projection, parameter.GetOptimizeWayNodes(), way->nodes,
	       transStart, transEnd, parameter.GetOptimizeErrorToleranceDots());

	 int numPoints = transEnd - transStart + 1;

	 unsigned char isCopy = 0;
	 jobject obj_points = mJniEnv->CallObjectMethod(mMapElement, mGetPoints, (jint) numPoints);
	 jfloatArray arr_points = reinterpret_cast<jfloatArray>(obj_points);

	 float *points = (float*) mJniEnv->GetPrimitiveArrayCritical(arr_points, &isCopy);
	 if (points == NULL) {
	    return;
	 }

	 jobject obj_indices = mJniEnv->CallObjectMethod(mMapElement, mGetIndices, (jint) 1);
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

	 const TypeId id = way->GetType();

	 if (id >= 0 && id < mTags.size())
	    mJniEnv->CallVoidMethod(mMapElement, mAddTag2, mTags[id]);

	 const std::string& name = way->GetName();
	 if (name.length() > 0) {
	    printf("add name %s\n", name.c_str());
	    mJniEnv->CallVoidMethod(mMapElement, mAddTag, mTagKeys[1],
		  mJniEnv->NewStringUTF(name.c_str()));
	 }

	 for (int i = 0, n = way->GetAttributes().GetTags().size(); i < n; i++) {
	    const Tag& tag = way->GetAttributes().GetTags()[i];

	    //printf("add tag %d %d %s\n", n, tag.key, tag.value.c_str());

	    mJniEnv->CallVoidMethod(mMapElement, mAddTag, mTagKeys[tag.key],
		  mJniEnv->NewStringUTF(tag.value.c_str()));
	 }

	 mJniEnv->CallVoidMethod(mPainterObject, mProcessPath, mMapElement);

      }

//      for (std::list<WayRef>::const_iterator p = data.poiWays.begin(); p != data.poiWays.end();
//	    ++p) {
//	 const WayRef& way = *p;
//
//	 PrepareWaySegment(styleConfig, projection, parameter,
//	       ObjectFileRef(way->GetFileOffset(), refWay), way->GetAttributes(), way->nodes,
//	       way->ids);
//      }

      //wayData.sort();
   }

   int
   MapPainterCanvas::GetColorInt(double r, double g, double b, double a) {
      int colorA = (int) floor(255 * a + 0.5);
      int colorR = (int) floor(255 * r + 0.5);
      int colorG = (int) floor(255 * g + 0.5);
      int colorB = (int) floor(255 * b + 0.5);

      int color = ((colorA << 24) | (colorR << 16) | (colorG << 8) | (colorB));

      return color;
   }

   int
   MapPainterCanvas::GetColorInt(Color color) {
      return GetColorInt(color.GetR(), color.GetG(), color.GetB(), color.GetA());
   }

   void
   MapPainterCanvas::DrawGround(const Projection& projection, const MapParameter& parameter,
	 const FillStyle& style) {

   }
}

#ifdef __cplusplus
extern "C" {
#endif

   jint
   Java_osm_scout_MapPainterCanvas_jniConstructor(JNIEnv *env, jobject object) {
      MapPainterCanvas *nativeMapPainter = new MapPainterCanvas(env);

      return gMapPainterArray->Add(nativeMapPainter);
   }

   void
   Java_osm_scout_MapPainterCanvas_jniDestructor(JNIEnv *env, jobject object, int mapPainterIndex) {
      MapPainterCanvas *nativeMapPainter = gMapPainterArray->GetAndRemove(mapPainterIndex);

      if (!nativeMapPainter)
	 printf("jniDestructor(): NULL object");
      else
	 delete nativeMapPainter;
   }

   jboolean
   Java_osm_scout_MapPainterCanvas_jniDrawMap(JNIEnv *env, jobject object, int mapPainterIndex,
	 int styleConfigIndex, int projectionIndex, int mapParameterIndex, int mapDataIndex) {
      MapPainterCanvas *nativeMapPainter = gMapPainterArray->Get(mapPainterIndex);

      if (!nativeMapPainter) {
	 printf("jniDrawMap(): NULL MapPainter object");
	 return JNI_FALSE;
      }

      StyleConfig *nativeStyleConfig = gStyleConfigArray->Get(styleConfigIndex);

      if (!nativeStyleConfig) {
	 printf("jniDrawMap(): NULL StyleConfig pointer");
	 return JNI_FALSE;
      }

      MercatorProjection *nativeProjection = gProjectionArray->Get(projectionIndex);

      if (!nativeProjection) {
	 printf("jniDrawMap(): NULL Projection pointer");
	 return JNI_FALSE;
      }

      MapParameter *nativeMapParameter = gMapParameterArray->Get(mapParameterIndex);

      if (!nativeMapParameter) {
	 printf("jniDrawMap(): NULL MapParameter pointer");
	 return JNI_FALSE;
      }

      printf("get data: %d\n", mapDataIndex);

      MapData *nativeMapData = gMapDataArray->Get(mapDataIndex);

      if (!nativeMapData) {
	 printf("jniDrawMap(): NULL MapData pointer");
	 return JNI_FALSE;
      }

      bool result = nativeMapPainter->DrawMap(*nativeStyleConfig, *nativeProjection,
	    *nativeMapParameter, *nativeMapData, env, object);

      if (result) {
	 printf("jniDrawMap(): DrawMap() Ok!");
      } else {
	 printf("jniDrawMap(): DrawMap() failed!");
      }

      return result;
   }

#ifdef __cplusplus
}
#endif

