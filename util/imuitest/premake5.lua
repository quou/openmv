project "imuitest"
	kind "ConsoleApp"
	language "C"
	cdialect "C99"

	targetdir "../../bin"
	objdir "obj"

	architecture "x64"
	staticruntime "on"

	files {
		"src/**.h",
		"src/**.c"
	}

	includedirs {
		"src",
		"../../core/src"
	}

	links {
		"core"
	}

	defines {
		"IMPORT_SYMBOLS",
		"_CRT_SECURE_NO_WARNINGS"
	}

	filter "system:linux"
		links {
			"m"
		}

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "on"
		runtime "debug"

	filter "configurations:release"
		defines { "RELEASE" }
		optimize "on"
		runtime "release"
