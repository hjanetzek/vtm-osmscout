package org.vtm.osmscout;

import org.oscim.tiling.source.ITileDataSource;
import org.oscim.tiling.source.TileSource;

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

}
