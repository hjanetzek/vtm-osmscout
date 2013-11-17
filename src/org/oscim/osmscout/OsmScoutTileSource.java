package org.oscim.osmscout;

import org.oscim.core.BoundingBox;
import org.oscim.core.GeoPoint;
import org.oscim.tilesource.ITileDataSource;
import org.oscim.tilesource.MapInfo;
import org.oscim.tilesource.TileSource;

import osm.scout.Database;
import osm.scout.MapParameter;
import osm.scout.StyleConfig;

public class OsmScoutTileSource extends TileSource {
	Database mDatabase;
	StyleConfig mStyleConfig;
	MapParameter mMapParameter;

	String mMapPath = "/home/src/libosmscout/berlin2";

	public OsmScoutTileSource() {
		// Set icons and pattern paths in MapParameter
		mMapParameter = new MapParameter();

		// Activate sea/land rendering
		mMapParameter.setRenderSeaLand(true);
	}

	@Override
	public ITileDataSource getDataSource() {
		return new TileDataSource(this, mDatabase, mStyleConfig);
	}

	@Override
	public OpenResult open() {
		mDatabase = new Database();
		String file = options.get("file");

		if (file != null)
			mMapPath = file;

		if (!mDatabase.open(mMapPath)) {
			return new OpenResult("Error opening database in <" + mMapPath + ">");
		}

		mStyleConfig = new StyleConfig(mDatabase.getTypeConfig());

		if (!mStyleConfig.loadStyleConfig(mMapPath + "/standard.oss")) {
			return new OpenResult("Error loading style config");
		}

		return OpenResult.SUCCESS;
	}

	@Override
	public void close() {
	}

	private static final MapInfo mMapInfo =
	        new MapInfo(new BoundingBox(-180, -90, 180, 90),
	                    new Byte((byte) 4), new GeoPoint(53.11, 8.85),
	                    null, 0, 0, 0, "de", "comment", "author", null);

	@Override
	public MapInfo getMapInfo() {
		return mMapInfo;
	}

}
