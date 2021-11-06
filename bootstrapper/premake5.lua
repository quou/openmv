project "bootstrapper"
	kind "ConsoleApp"
	language "C"
	cdialect "C99"

	targetdir "../bin"
	targetname "openmv"
	objdir "obj"

	architecture "x64"
	staticruntime "on"

	files {
		"src/**.h",
		"src/**.c"
	}

	includedirs {
		"src",
		"../core/src"
	}

	links {
		"core",
		"dl"
	}

	defines {
		"IMPORT_SYMBOLS"
	}

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "on"
		runtime "debug"

	filter "configurations:release"
		defines { "RELEASE" }
		optimize "on"
		runtime "release"
