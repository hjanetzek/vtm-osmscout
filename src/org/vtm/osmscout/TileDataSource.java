package org.vtm.osmscout;

import org.oscim.core.MercatorProjection;
import org.oscim.core.Tile;
import org.oscim.tiling.MapTile;
import org.oscim.tiling.source.ITileDataSink;
import org.oscim.tiling.source.ITileDataSource;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import osm.scout.Database;
import osm.scout.MapData;
import osm.scout.MapPainterCanvas;
import osm.scout.ObjectTypeSets;
import osm.scout.StyleConfig;

public class TileDataSource implements ITileDataSource {

	private static final Logger log = LoggerFactory.getLogger(TileDataSource.class);

	private Database mDatabase;
	private StyleConfig mStyleConfig;

	private osm.scout.MercatorProjection mProjection;
	private MapPainterCanvas mMapPainterCanvas;
	private MapData mMapData;

	double mScale;
	double mOffsetX;
	double mOffsetY;

	OsmScoutTileSource mTileSource;

	public TileDataSource(OsmScoutTileSource tileSource, Database database, StyleConfig styleConfig) {
		mTileSource = tileSource;
		mDatabase = database;
		mStyleConfig = styleConfig;

		mMapPainterCanvas = new MapPainterCanvas();

		mProjection = new osm.scout.MercatorProjection();
	}

	@Override
	public QueryResult executeQuery(MapTile tile, ITileDataSink mapDataSink) {
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

			// Get object type sets for current magnification
			ObjectTypeSets typeSets = mStyleConfig.getObjectTypesWithMaxMag(
			        mProjection.getMagnification() / 4);

			// Get database objects for given object type sets and current
			// projection
			mMapData = mDatabase.getObjects(typeSets, mProjection);

			// log.debug("ways " + mMapData.numWays());

			mMapPainterCanvas.setSink(mapDataSink);

			mMapPainterCanvas.drawMap(mStyleConfig, mProjection, mTileSource.mMapParameter,
			                          mMapData);

		}

		return QueryResult.SUCCESS;
	}

	@Override
	public void destroy() {
		// TODO Auto-generated method stub

	}

}
