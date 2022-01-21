project "core"
	kind "SharedLib"
	language "C"
	cdialect "gnu99" -- GNU because of POSIX features in the platform.

	toolset "clang"

	targetdir "../bin"
	objdir "obj"

	architecture "x64"

	staticruntime "on"

	files {
		"src/audio.c",
		"src/audio.h",
		"src/common.h",
		"src/core.c",
		"src/core.h",
		"src/coresys.c",
		"src/coresys.h",
		"src/coroutine.c",
		"src/coroutine.h",
		"src/dynlib.h",
		"src/entity.c",
		"src/entity.h",
		"src/keytable.c",
		"src/keytable.h",
		"src/lsp.c",
		"src/lsp.h",
		"src/maths.h",
		"src/physics.c",
		"src/physics.h",
		"src/platform.c",
		"src/platform.h",
		"src/renderer.c",
		"src/res.c",
		"src/res.h",
		"src/table.c",
		"src/table.h",
		"src/tiled.c",
		"src/tiled.h",
		"src/ui.c",
		"src/ui.h",
		"src/util",
		"src/util/KHR/khrplatform.h",
		"src/util/glad.c",
		"src/util/glad.h",
		"src/util/stb_rect_pack.h",
		"src/util/stb_truetype.h",
		"src/util/util.c",
		"src/video.h",
		"src/video_gl.c"
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
			"dl",
			"pthread"
		}

	filter "system:windows"
		links {
			"opengl32",
			"gdi32",
			"user32",
			"kernel32",
			"winmm"
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
			"src/platform_x11.c",
			"src/dynlib_linux.c"
		}

		postbuildcommands {
			"ctags -R"
		}
	
	filter "system:windows"
		files {
			"src/platform_windows.c",
			"src/dynlib_windows.c"
		}
