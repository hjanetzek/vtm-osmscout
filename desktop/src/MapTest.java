
import org.oscim.gdx.GdxMap;
import org.oscim.gdx.GdxMapApp;
import org.oscim.layers.tile.vector.VectorTileLayer;
import org.oscim.layers.tile.vector.labeling.LabelLayer;
import org.oscim.osmscout.OsmScoutTileSource;
import org.oscim.renderer.MapRenderer;
import org.oscim.theme.InternalRenderTheme;
import org.oscim.tiling.source.TileSource;

import com.badlogic.gdx.utils.SharedLibraryLoader;

public class MapTest extends GdxMap {
	static {
		new SharedLibraryLoader().load("osmscout");
		// System.loadLibrary("osmscout64");
	}

	@Override
	public void createLayers() {
		MapRenderer.setBackgroundColor(0xff888888);

		mMap.setMapPosition(53.072, 8.80, 1 << 15);

		TileSource tileSource = new OsmScoutTileSource();
		tileSource.setOption("file", "/home/src/libosmscout/bremen");

		VectorTileLayer l = mMap.setBaseMap(tileSource);

		// mMap.getLayers().add(new BuildingLayer(mMap, l.getTileLayer()));
		mMap.getLayers().add(new LabelLayer(mMap, l.getTileLayer()));

		// mMap.setTheme(InternalRenderTheme.DEFAULT);
		// mMap.setTheme(InternalRenderTheme.TRONRENDER);
		mMap.setTheme(InternalRenderTheme.OSMARENDER);
	}

	public static void main(String[] args) {
		GdxMapApp.init();
		GdxMapApp.run(new MapTest(), null, 400);
	}
}
