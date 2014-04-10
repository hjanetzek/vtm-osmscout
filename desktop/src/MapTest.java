import org.oscim.gdx.GdxMap;
import org.oscim.gdx.GdxMapApp;
import org.oscim.layers.tile.vector.VectorTileLayer;
import org.oscim.layers.tile.vector.labeling.LabelLayer;
import org.oscim.osmscout.OsmScoutTileSource;
import org.oscim.renderer.MapRenderer;
import org.oscim.theme.VtmThemes;
import org.oscim.tiling.TileSource;

public class MapTest extends GdxMap {
	static {
		// new SharedLibraryLoader().load("osmscout");
		System.loadLibrary("osmscout64");
	}

	@Override
	public void createLayers() {
		MapRenderer.setBackgroundColor(0xff888888);

		mMap.setMapPosition(53.072, 8.80, 1 << 15);

		TileSource tileSource = new OsmScoutTileSource();
		tileSource.setOption("file", "/home/jeff/dev/libosmscout/maps/bremen");

		VectorTileLayer l = mMap.setBaseMap(tileSource);

		// mMap.getLayers().add(new BuildingLayer(mMap, l.getTileLayer()));
		mMap.layers().add(new LabelLayer(mMap, l));

		// mMap.setTheme(InternalRenderTheme.DEFAULT);
		// mMap.setTheme(InternalRenderTheme.TRONRENDER);
		mMap.setTheme(VtmThemes.DEFAULT);

		// mMap.layers().add(new TileGridLayer(mMap));
		// TileSource ts = new
		// OSciMap4TileSource("http://opensciencemap.org/tiles/s3db");
		// S3DBLayer tl = new S3DBLayer(mMap, ts);
		// mMap.layers().add(tl);

	}

	public static void main(String[] args) {
		GdxMapApp.init();
		GdxMapApp.run(new MapTest(), null, 400);
	}
}
