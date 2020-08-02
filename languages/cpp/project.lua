project "CPPSample"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++11"
	flags { "FatalWarnings" }

	targetdir "../../builds/samples/cpp"
	debugdir "../../builds/samples/cpp"

	--Files to include
	files { "**" }
	includedirs { "../../external/stb" }

	--This project includes
	IncludeUDSDK()

	-- filters
	filter { "configurations:Debug" }
		symbols "On"
		optimize "Debug"
		removeflags { "FatalWarnings" }

	filter { "configurations:Release" }
		optimize "Full"

	filter { "system:linux" }
		links { "z", "m" }

	filter { "system:macosx" }
		frameworkdirs { "../builds/udsdk/samples/CppSample" }
		links { "udSDK.framework" }
		prebuildcommands { 'cp -af "../../builds/udsdk/lib/udSDK.framework" "../../builds/udsdk/samples/CppSample/udSDK.framework"' }

	filter {}
