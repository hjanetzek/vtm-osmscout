/*
 * Copyright 2013 Hannes Janetzek
 *
 * This program is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
 * PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program. If not, see <http://www.gnu.org/licenses/>.
 */

package org.oscim.osmscout;

import org.oscim.tiling.ITileDataSource;
import org.oscim.tiling.TileSource;

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
}
