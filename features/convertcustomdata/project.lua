project "ConvertCustomData"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	flags { "FatalWarnings" }

	targetdir "../../builds/features/customconversion"
	debugdir "../../builds/features/customconversion"

	--Files to include
	files { "*.h", "*.cpp", "*.md", "*.lua" }
	includedirs { "../../external/stb", "../shared" }

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
		links { "udSDK", "z", "m" }

		--This need to be changed to work in other distros
		postbuildcommands { 'cp "' .. _OPTIONS["udsdk"] .. '/lib/ubuntu18.04_GCC_x64/libudSDK.so" "%{cfg.targetdir}/"' }

	filter { "system:macosx" }
		frameworkdirs { "../../builds/features/customconversion" }
		links { "udSDK.framework" }
		prebuildcommands { 'cp -af "../../builds/udsdk/lib/udSDK.framework" "../../builds/features/customconversion/udSDK.framework"' }

	filter {}
