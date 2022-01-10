project "bootstrapper"
	kind "ConsoleApp"
	language "C"
	cdialect "C99"

	toolset "clang"

	targetdir "../bin"
	targetname "openmv"
	objdir "obj"

	architecture "x64"
	staticruntime "on"

	files {
		"src/bootstrapper.h",
		"src/bootstrapper.c",
		"src/main.c",
	}

	includedirs {
		"src",
		"../core/src"
	}

	links {
		"core"
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