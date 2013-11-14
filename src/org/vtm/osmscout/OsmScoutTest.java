package org.vtm.osmscout;

import org.oscim.tiling.MapTile;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import osm.scout.Database;
import osm.scout.MapData;
import osm.scout.MapPainterCanvas;
import osm.scout.MapParameter;
import osm.scout.MercatorProjection;
import osm.scout.ObjectTypeSets;
import osm.scout.StyleConfig;

public class OsmScoutTest {
	static {
		System.loadLibrary("osmscout64");
		System.setProperty(org.slf4j.impl.SimpleLogger.DEFAULT_LOG_LEVEL_KEY, "TRACE");
	}

	private static final Logger log = LoggerFactory.getLogger(OsmScoutTest.class);

	// OsmScout objects
	private Database mDatabase = null;
	private MercatorProjection mProjection = null;
	private StyleConfig mStyleConfig = null;
	private MapParameter mMapParameter = null;
	private MapPainterCanvas mMapPainterCanvas;
	private MapData mMapData;

	String mMapPath = "/home/src/libosmscout/berlin2";

	void init() {
		mDatabase = new Database();

		if (!mDatabase.open(mMapPath)) {

			finishActivity("Error opening database in <" + mMapPath + ">");
			return;
		}

		mStyleConfig = new StyleConfig(mDatabase.getTypeConfig());

		if (!mStyleConfig.loadStyleConfig(mMapPath + "/standard.oss")) {

			finishActivity("Error loading style config");
			return;
		}

		mMapPainterCanvas = new MapPainterCanvas();

		// Set icons and pattern paths in MapParameter
		mMapParameter = new MapParameter();

		// Activate sea/land rendering
		mMapParameter.setRenderSeaLand(true);
		// mMapParameter.setLowZoomOptimization(true);

		mProjection = new MercatorProjection();
		// mProjection.setSize(4096, 4096);
		// mProjection.setPos(13.3, 52.5);
		// mProjection.setMagnification(1 << 13);

		MapTile tile = new MapTile(8800, 5373, (byte) 14);
		double w = 1.0 / (1 << tile.zoomLevel);

		double minLon =
		        org.oscim.core.MercatorProjection.toLongitude(tile.x);
		double maxLon = org.oscim.core.MercatorProjection.toLongitude(tile.x
		                                                              + w);
		double minLat = org.oscim.core.MercatorProjection.toLatitude(tile.y +
		                                                             w);
		double maxLat = org.oscim.core.MercatorProjection.toLatitude(tile.y);
		mProjection.set(minLon, minLat, maxLon, maxLat, (1 << 14), 4096);

		// mProjection.set(tile);

		// Get object type sets for current magnification
		ObjectTypeSets typeSets = mStyleConfig.getObjectTypesWithMaxMag(
		        mProjection.getMagnification());

		// Get database objects for given object type sets and current
		// projection
		mMapData = mDatabase.getObjects(typeSets, mProjection);

		log.debug("ways " + mMapData.numWays());

		// new Thread(new Runnable() {
		// @Override
		// public void run() {
		mMapPainterCanvas.drawMap(mStyleConfig, mProjection, mMapParameter, mMapData);
		// }
		// }, "mapthread").start();
		//

		// try {
		// wait();
		// } catch (InterruptedException e1) {
		// // TODO Auto-generated catch block
		// e1.printStackTrace();
		// }
		//
		// try {
		// Thread.sleep(2000);
		// } catch (InterruptedException e) {
		// e.printStackTrace();
		// }
		//
		log.debug("ok!");

	}

	private void finishActivity(String string) {
		log.debug(string);
	}

	public static void main(String[] args) {

		// new SharedLibraryLoader().load("osmscout");

		new OsmScoutTest().init();
	}
}
