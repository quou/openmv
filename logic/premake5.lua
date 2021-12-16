project "logic"
	kind "SharedLib"
	language "C"
	cdialect "gnu99"

	targetdir "../"
	objdir "obj"

	architecture "x64"

	staticruntime "on"

	files {
		"src/**.h",
		"src/**.c"
	}

	includedirs {
		"src",
		"../core/src",
		"../lua54/src"
	}

	links {
		"lua54",
		"core"
	}

	defines {
		"EXPORT_SYMBOLS"
	}

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "on"
		runtime "debug"

	filter "configurations:release"
		defines { "RELEASE" }
		optimize "on"
		runtime "release"
