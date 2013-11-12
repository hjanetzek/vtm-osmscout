import com.badlogic.gdx.jnigen.AntScriptGenerator;
import com.badlogic.gdx.jnigen.BuildConfig;
import com.badlogic.gdx.jnigen.BuildTarget;
import com.badlogic.gdx.jnigen.BuildTarget.TargetOs;

public class OsmscoutBuilder {
	static String[] sources = {
	        "libosmscout/libosmscout/src/osmscout/AreaAreaIndex.cpp",
	        "libosmscout/libosmscout/src/osmscout/AreaNodeIndex.cpp",
	        "libosmscout/libosmscout/src/osmscout/AreaWayIndex.cpp",
	        "libosmscout/libosmscout/src/osmscout/CityStreetIndex.cpp",
	        "libosmscout/libosmscout/src/osmscout/Database.cpp",
	        "libosmscout/libosmscout/src/osmscout/GroundTile.cpp",
	        "libosmscout/libosmscout/src/osmscout/Location.cpp",
	        "libosmscout/libosmscout/src/osmscout/Node.cpp",
	        "libosmscout/libosmscout/src/osmscout/OptimizeAreasLowZoom.cpp",
	        "libosmscout/libosmscout/src/osmscout/OptimizeWaysLowZoom.cpp",
	        "libosmscout/libosmscout/src/osmscout/TypeConfig.cpp",
	        "libosmscout/libosmscout/src/osmscout/TypeConfigLoader.cpp",
	        "libosmscout/libosmscout/src/osmscout/Types.cpp",
	        "libosmscout/libosmscout/src/osmscout/WaterIndex.cpp",
	        "libosmscout/libosmscout/src/osmscout/Way.cpp",
	        "libosmscout/libosmscout/src/osmscout/Area.cpp",
	        "libosmscout/libosmscout/src/osmscout/ost/Parser.cpp",
	        "libosmscout/libosmscout/src/osmscout/ost/Scanner.cpp",
	        "libosmscout/libosmscout/src/osmscout/util/Color.cpp",
	        "libosmscout/libosmscout/src/osmscout/util/File.cpp",
	        "libosmscout/libosmscout/src/osmscout/util/FileScanner.cpp",
	        "libosmscout/libosmscout/src/osmscout/util/FileWriter.cpp",
	        "libosmscout/libosmscout/src/osmscout/util/Geometry.cpp",
	        "libosmscout/libosmscout/src/osmscout/util/Magnification.cpp",
	        "libosmscout/libosmscout/src/osmscout/util/Projection.cpp",
	        "libosmscout/libosmscout/src/osmscout/util/StopClock.cpp",
	        "libosmscout/libosmscout/src/osmscout/util/String.cpp",
	        "libosmscout/libosmscout/src/osmscout/util/Transformation.cpp",
	        "libosmscout/libosmscout-map/src/osmscout/MapPainter.cpp",
	        "libosmscout/libosmscout-map/src/osmscout/StyleConfig.cpp",
	        "libosmscout/libosmscout-map/src/osmscout/StyleConfigLoader.cpp",
	        "libosmscout/libosmscout-map/src/osmscout/oss/Parser.cpp",
	        "libosmscout/libosmscout-map/src/osmscout/oss/Scanner.cpp",
	        "libosmscout-jni/jniDatabase.cpp",
	        "libosmscout-jni/jniMapData.cpp",
	        "libosmscout-jni/jniMapPainterCanvas.cpp",
	        "libosmscout-jni/jniMapParameter.cpp",
	        "libosmscout-jni/jniMercatorProjection.cpp",
	        "libosmscout-jni/jniObjectTypeSets.cpp",
	        "libosmscout-jni/jniOnLoad.cpp",
	        "libosmscout-jni/jniStyleConfig.cpp",
	        "libosmscout-jni/jniTypeConfig.cpp"
	};

	static String[] excludeCpp = {
	        "libosmscout/Demos/**",
	        "libosmscout/TravelJinni/**",
	        "libosmscout/libosmscout-import/**",
	};

	static String[] headers = { ".",
			"include",
	        "libosmscout/libosmscout/include",
	        "libosmscout/libosmscout-map/include"
	};

	// static String[] cppIncludes
	//
	public static void main(String[] args) {

		String cflags = " -ffast-math -std=gnu++0x";

		BuildTarget win32home = BuildTarget.newDefaultTarget(TargetOs.Windows,
		                                                     false);
		win32home.compilerPrefix = "";
		win32home.buildFileName = "build-windows32home.xml";
		win32home.excludeFromMasterBuildFile = true;
		win32home.headerDirs = headers;
		win32home.cIncludes = sources;
		win32home.cFlags += cflags;
		win32home.cppFlags += cflags;

		BuildTarget win32 = BuildTarget.newDefaultTarget(TargetOs.Windows,
		                                                 false);
		win32.headerDirs = headers;
		win32.cIncludes = sources;
		win32.cFlags += cflags;
		win32.cppFlags += cflags;

		BuildTarget win64 = BuildTarget
		        .newDefaultTarget(TargetOs.Windows, true);
		win64.headerDirs = headers;
		win64.cIncludes = sources;
		win64.cFlags += cflags;
		win64.cppFlags += cflags;

		BuildTarget lin32 = BuildTarget.newDefaultTarget(TargetOs.Linux, false);
		lin32.headerDirs = headers;
		lin32.cIncludes = sources;
		lin32.cFlags += cflags;
		lin32.cppFlags += cflags;

		BuildTarget lin64 = BuildTarget.newDefaultTarget(TargetOs.Linux, true);
		lin64.headerDirs = headers;
		lin64.cIncludes = sources;
		lin64.cppExcludes = excludeCpp;
		lin64.cppIncludes = sources;
		lin64.cFlags += cflags;
		lin64.cppFlags += cflags;
		
		// BuildTarget mac = BuildTarget.newDefaultTarget(TargetOs.MacOsX,
		// false);
		// mac.headerDirs = headers;
		// mac.cIncludes = sources;
		// mac.cFlags += cflags;
		// mac.cppFlags += cflags;
		// mac.linkerFlags += " -framework CoreServices -framework Carbon";

		BuildTarget android = BuildTarget.newDefaultTarget(TargetOs.Android,
		                                                   false);
		android.headerDirs = headers;
		android.cIncludes = sources;
		android.cppExcludes = excludeCpp;
		android.cppIncludes = sources;
		android.cFlags += cflags;
		android.cppFlags += cflags;
		android.linkerFlags += " -llog  -lstdc++";
		// BuildTarget ios = BuildTarget.newDefaultTarget(TargetOs.IOS, false);
		// ios.headerDirs = headers;
		// ios.cIncludes = sources;
		// ios.cFlags += cflags;
		// ios.cppFlags += cflags;

		// new NativeCodeGenerator().generate();
		new AntScriptGenerator().generate(new BuildConfig("osmscout"),
		                                  // win32home, win32, win64, lin32,
		                                  // android,
		                                  lin64);

		// BuildExecutor.executeAnt("jni/build-windows32home.xml", "-v clean");
		// BuildExecutor.executeAnt("jni/build-windows32home.xml", "-v");
		// BuildExecutor.executeAnt("jni/build.xml", "pack-natives -v");
	}
}
