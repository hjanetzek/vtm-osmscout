/*
 * Copyright 2013 Hannes Janetzek
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */
package org.oscim.osmscout;

import org.oscim.core.GeometryBuffer.GeometryType;
import org.oscim.core.MapElement;
import org.oscim.core.MercatorProjection;
import org.oscim.core.Tag;
import org.oscim.core.Tile;
import org.oscim.layers.tile.MapTile;
import org.oscim.tilesource.ITileDataSink;
import org.oscim.tilesource.ITileDataSource;
import org.oscim.utils.TileClipper;

import osm.scout.Database;
import osm.scout.MapData;
import osm.scout.ObjectTypeSets;
import osm.scout.StyleConfig;

public class TileDataSource implements ITileDataSource {

	private final int mJniRef;

	private Database mDatabase;
	private StyleConfig mStyleConfig;

	private osm.scout.MercatorProjection mProjection;
	// private MapPainterCanvas mMapPainterCanvas;
	private MapData mMapData;

	private final MapElement mMapElement;

	double mScale;
	double mOffsetX;
	double mOffsetY;

	TileClipper mTileClipper = new TileClipper(-16, -16, Tile.SIZE + 16, Tile.SIZE + 16);

	OsmScoutTileSource mTileSource;

	public TileDataSource(OsmScoutTileSource tileSource, Database database, StyleConfig styleConfig) {

		mTileSource = tileSource;
		mDatabase = database;
		mStyleConfig = styleConfig;

		mProjection = new osm.scout.MercatorProjection();

		mMapElement = new MapElement(1024, 32);

		mJniRef = jniConstructor();
	}

	protected void finalize() throws Throwable {

		try {
			jniDestructor(mJniRef);
		} finally {
			super.finalize();
		}
	}

	ITileDataSink mSink;

	@Override
	public QueryResult executeQuery(MapTile tile, ITileDataSink mapDataSink) {
		boolean success = false;
		synchronized (mTileSource) {

			double w = 1.0 / (1 << tile.zoomLevel);
			mScale = (1 << tile.zoomLevel) * Tile.SIZE;
			mOffsetX = tile.x * mScale;
			mOffsetY = tile.y * mScale;

			double minLon = MercatorProjection.toLongitude(tile.x);
			double maxLon = MercatorProjection.toLongitude(tile.x + w);
			double minLat = MercatorProjection.toLatitude(tile.y + w);
			double maxLat = MercatorProjection.toLatitude(tile.y);

			mProjection.set(minLon, minLat, maxLon, maxLat, (1 << tile.zoomLevel), Tile.SIZE);

			ObjectTypeSets typeSets = mStyleConfig.getObjectTypesWithMaxMag(
			        mProjection.getMagnification() / 2);

			mMapData = mDatabase.getObjects(typeSets, mProjection);

			// log.debug("ways " + mMapData.numWays());

			mSink = mapDataSink;

			success = jniDrawMap(mJniRef, mStyleConfig.getJniObjectIndex(),
			                     mProjection.getJniObjectIndex(),
			                     mTileSource.mMapParameter.getJniObjectIndex(),
			                     mMapData.getJniObjectIndex());
		}

		if (!success)
			return QueryResult.FAILED;

		return QueryResult.SUCCESS;
	}

	@Override
	public void destroy() {
		// TODO Auto-generated method stub

	}

	public void processArea(MapElement elem) {
		if (mSink == null)
			return;

		elem.type = GeometryType.POLY;

		if (mTileClipper.clip(elem))
			mSink.process(elem);

		elem.clear();
		elem.tags.clear();
	}

	public void processPath(MapElement elem) {
		if (mSink == null)
			return;

		elem.type = GeometryType.LINE;

		if (mTileClipper.clip(elem))
			mSink.process(elem);

		elem.clear();
		elem.tags.clear();
	}

	public static Tag makeTag(String s) {
		int sep = s.indexOf('_');
		if (sep > 0)
			return new Tag(s.substring(0, sep), s.substring(sep + 1));

		return new Tag(s, "yes");
	}

	private native int jniConstructor();

	private native void jniDestructor(int mapPainterIndex);

	private native boolean jniDrawMap(int mapPainterIndex,
	                                  int styleConfigIndex,
	                                  int projectionIndex,
	                                  int mapParameter,
	                                  int mapDataIndex);
}
