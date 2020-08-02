project "RenderingCustomVoxelShaders"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++11"
	flags { "FatalWarnings" }

	targetdir "../../builds/features/renderingcustomvoxelshaders"
	debugdir "../../builds/features/renderingcustomvoxelshaders"

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
		frameworkdirs { "../../builds/features/renderingcustomvoxelshaders" }
		links { "udSDK.framework" }
		prebuildcommands { 'cp -af "../../builds/udsdk/lib/udSDK.framework" "../../builds/features/renderingcustomvoxelshaders/udSDK.framework"' }

	filter {}
