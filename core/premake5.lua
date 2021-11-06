project "core"
	kind "SharedLib"
	language "C"
	cdialect "gnu99"

	targetdir "../bin"
	objdir "obj"

	architecture "x64"

	staticruntime "on"

	files {
		"src/common.h",
		"src/core.c",
		"src/core.h",
		"src/coresys.c",
		"src/coresys.h",
		"src/entity.c",
		"src/entity.h",
		"src/keytable.c",
		"src/keytable.h",
		"src/maths.h",
		"src/platform.c",
		"src/platform.h",
		"src/renderer.c",
		"src/res.c",
		"src/res.h",
		"src/table.c",
		"src/table.h",
		"src/ui.c",
		"src/ui.h",
		"src/util",
		"src/video.h",
		"src/video_gl.c",
		"src/util/gl.h",
		"src/util/stb_rect_pack.h",
		"src/util/stb_truetype.h",
		"src/util/util.c"
	}

	includedirs {
		"src"
	}

	defines {
		"EXPORT_SYMBOLS"
	}

	filter "system:linux"
		links {
			"X11",
			"GL",
			"m",
			"dl"
		}

	filter "system:windows"
		links {
			"opengl32",
			"gdi32",
			"user32",
			"kernel32",
			"winmm",
			"ssp"
		}

		defines {
			"_CRT_SECURE_NO_WARNINGS"
		}

	filter "configurations:debug"
		defines { "DEBUG" }
		symbols "on"
		runtime "debug"

	filter "configurations:release"
		defines { "RELEASE" }
		optimize "on"
		runtime "release"

	filter "system:linux"
		files {
			"src/platform_linux.c",
			"src/platform_x11.c"
		}
	
	filter "system:windows"
		files {
			"src/platform_windows.c"
		}
