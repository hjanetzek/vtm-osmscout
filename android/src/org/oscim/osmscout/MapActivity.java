package org.oscim.osmscout;

import org.oscim.android.MapView;
import org.oscim.layers.tile.vector.VectorTileLayer;
import org.oscim.layers.tile.vector.labeling.LabelLayer;
import org.oscim.theme.ThemeFile;
import org.oscim.theme.VtmThemes;
import org.oscim.tiling.TileSource;

import android.os.Bundle;
import android.os.Environment;
import android.util.Log;
import android.view.Menu;
import android.view.MenuItem;

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

		String file = Environment.getExternalStorageDirectory().getPath() + "/bremen2/";
		//String file = "/sdcard/bremen2";
		tileSource.setOption("file", file);
		Log.d("yo!", file);
		VectorTileLayer l = mMap.setBaseMap(tileSource);

		//mMap.getLayers().add(new BuildingLayer(mMap, l.getTileLayer()));
		mMap.layers().add(new LabelLayer(mMap, l));

		//mMap.setTheme(InternalRenderTheme.DEFAULT);
		//mMap.setTheme(InternalRenderTheme.TRONRENDER);
		mMap.setTheme(VtmThemes.OSMARENDER);

		mMap.setMapPosition(53.1, 8.8, 1 << 15);
	}

	@Override
	public boolean onMenuItemSelected(int featureId, MenuItem item) {
		ThemeFile theme = null;

		switch (item.getItemId()) {
			case R.id.theme_default:
				theme = VtmThemes.DEFAULT;
				break;

			case R.id.theme_tubes:
				theme = VtmThemes.TRONRENDER;
				break;

			case R.id.theme_osmarender:
				theme = VtmThemes.OSMARENDER;
				break;

			case R.id.theme_newtron:
				theme = VtmThemes.NEWTRON;
				break;
		}

		if (theme == null)
			return false;

		mMap.setTheme(theme);
		item.setChecked(true);
		return true;
	}

	@Override
	public boolean onCreateOptionsMenu(Menu menu) {
		getMenuInflater().inflate(R.menu.theme_menu, menu);
		return true;
	}
}
