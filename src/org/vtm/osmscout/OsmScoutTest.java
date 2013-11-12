package org.vtm.osmscout;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import osm.scout.Database;
import osm.scout.MapData;
import osm.scout.MapPainterCanvas;
import osm.scout.MapParameter;
import osm.scout.MercatorProjection;
import osm.scout.ObjectTypeSets;
import osm.scout.StyleConfig;

import com.badlogic.gdx.utils.SharedLibraryLoader;

public class OsmScoutTest {
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

		mProjection = new MercatorProjection();
		mProjection.setSize(1000, 1000);
		mProjection.setPos(13.3, 52.5);
		mProjection.setMagnification(1 << 13);

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

		new SharedLibraryLoader().load("osmscout");

		new OsmScoutTest().init();
	}
}
