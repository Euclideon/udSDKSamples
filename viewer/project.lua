project "Viewer"
	kind "ConsoleApp"
	language "C++"
	flags { "FatalWarnings" }

	targetdir "../builds/viewer"
	debugdir "../builds/viewer"

	--Files to include
	files { "*.h", "*.cpp", "*.md", "*.lua" }
	includedirs { "../external/stb", "../features/shared" }
	
	includedirs { "../external/udcore/Include" }
	links { "udCore" }

	includedirs { "../external/imgui" }
	
	includedirs { "../external/sdl2/include"}
	filter { "system:windows" }
		libdirs { "../external/sdl2/lib/x64" }
		links { "SDL2.lib", "SDL2main.lib" }
	filter { "system:linux" }
		links { "SDL2" }

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
		frameworkdirs { "../../builds/features/getscenesfromudcloud" }
		links { "udSDK.framework" }
		prebuildcommands { 'cp -af "../../builds/udsdk/lib/udSDK.framework" "../../builds/features/getscenesfromudcloud/udSDK.framework"' }

	filter {}
