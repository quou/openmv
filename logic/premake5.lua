project "logic"
	kind "SharedLib"
	language "C"
	cdialect "C99"

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
	}

	links {
		"core"
	}

	defines {
		"IMPORT_SYMBOLS",
		"LOGIC_EXPORT_SYMBOLS"
	}

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "on"
		runtime "debug"

	filter "configurations:release"
		defines { "RELEASE" }
		optimize "on"
		runtime "release"
