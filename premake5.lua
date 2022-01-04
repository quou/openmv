workspace "openmv"
	configurations { "debug", "release" }

include "lua54"
include "core"
include "logic"
include "bootstrapper"

include "util/packer"
include "util/mksdk"
include "util/script_tester"
