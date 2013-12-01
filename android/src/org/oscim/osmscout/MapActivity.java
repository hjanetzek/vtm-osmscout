package org.oscim.osmscout;

import org.oscim.android.MapView;
import org.oscim.layers.tile.vector.VectorTileLayer;
import org.oscim.layers.tile.vector.labeling.LabelLayer;
import org.oscim.theme.InternalRenderTheme;
import org.oscim.tiling.source.TileSource;

import android.os.Bundle;
import android.os.Environment;
import android.view.Menu;

public class MapActivity extends org.oscim.android.MapActivity {

	protected MapView mMapView;

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

		VectorTileLayer l = mMap.setBaseMap(tileSource);

		//mMap.getLayers().add(new BuildingLayer(mMap, l.getTileLayer()));
		mMap.getLayers().add(new LabelLayer(mMap, l.getTileLayer()));

		//mMap.setTheme(InternalRenderTheme.DEFAULT);
		//mMap.setTheme(InternalRenderTheme.TRONRENDER);
		mMap.setTheme(InternalRenderTheme.OSMARENDER);

		mMap.setMapPosition(52.5, 13.3, 1 << 15);
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.activity_map, menu);
		return true;
	}
}
