/*
  This source is part of the libosmscout library
  Copyright (C) 2010  Tim Teulings

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307  USA
 */

package osm.scout;

public class MapData {

	private int mJniMapDataIndex;

	public MapData() {

		mJniMapDataIndex = jniConstructor();
	}

	public MapData(int jniMapDataIndex) {

		mJniMapDataIndex = jniMapDataIndex;
	}

	public int numWays() {
		return jniGetWays(mJniMapDataIndex);
	}

	protected void finalize() throws Throwable {

		try {
			jniDestructor(mJniMapDataIndex);
		} finally {
			super.finalize();
		}
	}

	public int getJniObjectIndex() {
		return mJniMapDataIndex;
	}

	// Native methods
	private native int jniConstructor();

	private native void jniDestructor(int jniObjectTypeSetsIndex);

	private native int jniGetWays(int jniObjectTypeSetsIndex);
}
