project "lua54"
	kind "StaticLib"
	language "C"
	staticruntime "on"
	pic "on"

	toolset "clang"

	architecture "x64"

	targetdir "../bin/"
	objdir "obj"

	files {
		"src/**.h",
		"src/**.c"
	}

	filter "configurations:debug"
		runtime "debug"
		symbols "on"

		defines { "LUA_USE_APICHECK" }

	filter "configurations:release"
		runtime "release"
		optimize "on"
