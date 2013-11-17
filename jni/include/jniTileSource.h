#ifndef __MAP_PAINTER_CANVAS_H__
#define __MAP_PAINTER_CANVAS_H__

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

#include <osmscout/MapPainter.h>
#include <osmscout/util/Transformation.h>

namespace osmscout {

   class TileSource {

   private:

      JNIEnv *mJniEnv;
      jclass mPainterClass;
      jobject mPainterObject;

      jobject mMapElement;
      jclass mMapElementClass;

      jclass mTagClass;

      jmethodID mGetPoints;
      jmethodID mGetIndices;

      jmethodID mProcessArea;
      jmethodID mProcessPath;

      jmethodID mNewTag;

      jmethodID mAddTag;
      jmethodID mAddTag2;

      double mMinimumLineWidth; //! Minimum width a line must have to be visible

      std::vector<bool> mIconLoaded; // Vector for icon load status
      std::vector<bool> mPatternLoaded; // Vector for pattern load status

      std::vector<jstring> mTagKeys;
      std::vector<jobject> mTags;

      int
      GetColorInt(double r, double g, double b, double a);
      int
      GetColorInt(Color color);
      CoordBufferImpl<Vertex2D> *coordBuffer;

      /**
       Scratch variables for path optimization algorithm
       */
      TransBuffer transBuffer; //! Static (avoid reallocation) buffer of transformed coordinates

      struct OSMSCOUT_API PolyData
      {
	 size_t transStart; //! Start of coordinates in transformation buffer
	 size_t transEnd; //! End of coordinates in transformation buffer
      };
      size_t waysSegments;
      size_t waysDrawn;

      size_t areasSegments;
      size_t areasDrawn;

      size_t nodesDrawn;

   private:

      void
      initKeys(const StyleConfig& styleConfig);


   protected:


      void
      DrawPath(const Projection& projection, const MapParameter& parameter, const Color& color,
	    double width, const std::vector<double>& dash, LineStyle::CapStyle startCap,
	    LineStyle::CapStyle endCap, size_t transStart, size_t transEnd);

      void
      DrawArea(const std::vector<PolyData>& data, const AreaRef& area, size_t outerId,
	    size_t ringId);

      void
      DrawGround(const Projection& projection, const MapParameter& parameter,
	    const FillStyle& style);

      bool
      Draw(const StyleConfig& styleConfig, const Projection& projection,
	    const MapParameter& parameter, const MapData& data);

      void
      PrepareAreas(const StyleConfig& styleConfig, const Projection& projection,
	    const MapParameter& parameter, const MapData& data);

      void
      PrepareWays(const StyleConfig& styleConfig, const Projection& projection,
	    const MapParameter& parameter, const MapData& data);

      bool IsVisible(const Projection& projection,
                                    const std::vector<GeoCoord>& nodes,
                                    double pixelOffset) const;

   public:
      TileSource(JNIEnv *env);
      virtual
      ~TileSource();

      bool
      DrawMap(const StyleConfig& styleConfig, const Projection& projection,
	    const MapParameter& parameter, const MapData& data, JNIEnv *env, jobject object);
   };
}

#endif	// __MAP_PAINTER_CANVAS_H__
