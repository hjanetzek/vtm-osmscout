package org.oscim.test;

import org.oscim.core.MapPosition;
import org.oscim.layers.tile.vector.MapTileLayer;
import org.oscim.osmscout.OsmScoutTileSource;
import org.oscim.theme.InternalRenderTheme;
import org.oscim.tilesource.TileSource;
import org.oscim.view.MapView;

import android.os.Bundle;
import android.os.Environment;
import android.view.Menu;

public class MapActivity extends org.oscim.android.MapActivity {

	static {
		System.loadLibrary("osmscout");
	}

	@Override
	public void onCreate(Bundle savedInstanceState) {
		super.onCreate(savedInstanceState);
		setContentView(R.layout.activity_map);

		mMapView = (MapView) findViewById(R.id.mapView);

		TileSource tileSource = new OsmScoutTileSource();

		String file = Environment.getExternalStorageDirectory() + "/berlin2";
		tileSource.setOption("file", file);

		MapPosition pos = new MapPosition();
		pos.setPosition(52.5, 13.3);
		pos.setScale(1 << 15);
		mMapView.setMapPosition(pos);

		// TileSource tileSource = new OSciMap4TileSource();
		// tileSource.setOption("url",
		// "http://city.informatik.uni-bremen.de/tiles/vtm");

		MapTileLayer l = mMapView.setBaseMap(tileSource);

		l.setRenderTheme(InternalRenderTheme.DEFAULT);
		// l.setRenderTheme(InternalRenderTheme.TRONRENDER);

		// mMap.setBackgroundMap(new BitmapTileLayer(mMap,
		// MapQuestAerial.INSTANCE));

		// mMapView.getLayerManager().add(new GenericOverlay(mMapView, new
		// GridRenderLayer(mMapView)));
		// mMapView.getLayerManager().add(new GenericOverlay(mMapView, new
		// HexagonRenderLayer(mMapView)));

		mMapView.setClickable(true);
		mMapView.setFocusable(true);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.activity_map, menu);
		return true;
	}
}
