project "udSDKCSample"
	kind "ConsoleApp"
	language "C"
	flags { "FatalWarnings" }

	targetdir "../../builds/samples/c"
	debugdir "../../builds/samples/c"

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

	filter { "system:windows" }
		links { "udSDK" }
		postbuildcommands { 'XCOPY /f /d /y "' .. _OPTIONS["udsdk"] .. '\\lib\\win_x64\\udSDK.dll" "$(TargetDir)\\"' }

	filter { "system:linux" }
		buildoptions { "-std=c99" }
		links { "udSDK", "z", "m" }

		--This need to be changed to work in other distros
		postbuildcommands { 'cp "' .. _OPTIONS["udsdk"] .. '/lib/ubuntu18.04_GCC_x64/libudSDK.so" "%{cfg.targetdir}/"' }

	filter { "system:macosx" }
		frameworkdirs { "../builds/udsdk/samples/CSample" }
		links { "udSDK.framework" }
		prebuildcommands { 'cp -af "../../builds/udsdk/lib/udSDK.framework" "../../builds/udsdk/samples/CSample/udSDK.framework"' }

	filter {}
