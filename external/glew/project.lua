project "GLEW"
	kind "StaticLib"
	language "C++"
	staticruntime "On"
	flags { "OmitDefaultLibrary" }
--	warnings "Off"

	files { "src/**.c", "include/**.h" }
	files { "project.lua" }

	includedirs { "include" }

	defines { "WIN32_LEAN_AND_MEAN", "GLEW_STATIC" }

	-- include common stuff
	dofile "../../../udCore/bin/premake-bin/common-proj.lua"
