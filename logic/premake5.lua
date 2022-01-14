project "logic"
	kind "SharedLib"
	language "C"
	cdialect "C99"

	toolset "clang"

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
