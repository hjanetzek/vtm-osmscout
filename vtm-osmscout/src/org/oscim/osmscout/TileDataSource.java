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
import org.oscim.tiling.ITileDataSink;
import org.oscim.tiling.ITileDataSink.QueryResult;
import org.oscim.tiling.ITileDataSource;
import org.oscim.utils.geom.TileClipper;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import osm.scout.Database;
import osm.scout.MapData;
import osm.scout.ObjectTypeSets;
import osm.scout.StyleConfig;

public class TileDataSource implements ITileDataSource {
	private final static Logger log = LoggerFactory.getLogger(TileDataSource.class);

	private final int mJniRef;

	private Database mDatabase;
	private StyleConfig mStyleConfig;

	private osm.scout.MercatorProjection mProjection;
	// private MapPainterCanvas mMapPainterCanvas;

	private final MapElement mMapElement;

	// double mScale;
	// double mOffsetX;
	// double mOffsetY;

	TileClipper mTileClipper = new TileClipper(-16, -16, Tile.SIZE + 16, Tile.SIZE + 16);

	final OsmScoutTileSource mTileSource;

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
	public void query(MapTile tile, ITileDataSink mapDataSink) {
		if (mTileSource == null) {
			mapDataSink.completed(QueryResult.FAILED);
			return;
		}
		boolean success = false;
		synchronized (mTileSource) {
			double scale = (1 << tile.zoomLevel);
			double w = 1.0 / scale;
			// mScale = (1 << tile.zoomLevel) * (Tile.SIZE) + 1;
			// mOffsetX = tile.x * mScale;
			// mOffsetY = tile.y * mScale;

			// double minLon = MercatorProjection.toLongitude(tile.x);
			// double maxLon = MercatorProjection.toLongitude(tile.x + w);
			// double minLat = MercatorProjection.toLatitude(tile.y + w);
			// double maxLat = MercatorProjection.toLatitude(tile.y);

			double lon = MercatorProjection.toLongitude(tile.x + w / 2);
			double lat = MercatorProjection.toLatitude(tile.y + w / 2);

			// mProjection.set(minLon, minLat, maxLon, maxLat, scale, Tile.SIZE
			// + 1);

			mProjection.set(lon, lat, scale, Tile.SIZE, Tile.SIZE);

			ObjectTypeSets typeSets = mStyleConfig.getObjectTypesWithMaxMag(
			    mProjection.getMagnification() / 2);

			MapData mapData = mDatabase.getObjects(typeSets, mProjection);
			if (mapData == null) {
				mapDataSink.completed(QueryResult.FAILED);
				log.debug("no data");
				return;
			}

			mSink = mapDataSink;

			success = jniDrawMap(mJniRef, mStyleConfig.getJniObjectIndex(),
			                     mProjection.getJniObjectIndex(),
			                     mTileSource.mMapParameter.getJniObjectIndex(),
			                     mapData.getJniObjectIndex());
		}

		mSink.completed(success ? QueryResult.SUCCESS : QueryResult.FAILED);
		// mTileSource.close();
		// mTileSource = null;
	}

	@Override
	public void destroy() {
		// TODO Auto-generated method stub

	}

	public void processArea() {
		if (mSink == null)
			return;

		MapElement e = mMapElement;

		e.type = GeometryType.POLY;

		if (mTileClipper.clip(e))
			mSink.process(e);

		e.clear();
		e.tags.clear();
	}

	public void processPath() {
		if (mSink == null)
			return;
		MapElement e = mMapElement;

		e.type = GeometryType.LINE;

		if (mTileClipper.clip(e))
			mSink.process(e);

		e.clear();
		e.tags.clear();
	}

	public void addTag(String key, String value) {
		mMapElement.tags.add(new Tag(key, value));
	}

	public void addTag(Tag tag) {
		mMapElement.tags.add(tag);
	}

	public short[] ensureIndexSize(int size) {
		return mMapElement.ensureIndexSize(size, false);
	}

	public float[] ensurePointSize(int size) {
		return mMapElement.ensurePointSize(size, false);
	}

	public static Tag makeTag(String s) {
		int sep = s.indexOf('_');
		if (sep > 0)
			return new Tag(s.substring(0, sep), s.substring(sep + 1));

		return new Tag(s, "yes");
	}

	private native int jniConstructor();

	private native void jniDestructor(int tileSourceIndex);

	private native boolean jniDrawMap(int tileSourceIndex,
	        int styleConfigIndex,
	        int projectionIndex,
	        int mapParameter,
	        int mapDataIndex);

}
