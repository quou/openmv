workspace "openmv"
	configurations { "debug", "release" }

include "core"
include "logic"
include "bootstrapper"

include "util/packer"
include "util/mksdk"
include "util/test"
include "util/imuitest"
include "util/pixmaker"
