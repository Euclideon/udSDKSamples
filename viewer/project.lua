project "Viewer"
	kind "ConsoleApp"
	language "C++"
	flags { "FatalWarnings" }

	targetdir "../builds/viewer"
	debugdir "../builds/viewer"

	--Files to include
	includedirs { "../external/sdl2/include"}
	files { "*.h", "*.cpp", "*.md", "*.lua" }
	files { "../external/imgui/*.cpp", "../external/imgui/*.h" }
	files { "../external/imgui/backends/imgui_impl_sdl.cpp", "../external/imgui/backends/imgui_impl_sdl.h" }
	files { "../external/imgui/backends/imgui_impl_sdlrenderer.cpp", "../external/imgui/backends/imgui_impl_sdlrenderer.h" }
	includedirs { "../external/stb", "../features/shared" }
	
	includedirs { "../external/udcore/Include" }
	links { "udCore" }

	includedirs { "../external/imgui" }
	
	--This project includes
	IncludeUDSDK()

	-- filters
	filter { "system:windows" }
		staticruntime "on"
		libdirs { "../external/sdl2/lib/x64" }
		links { "SDL2.lib", "SDL2main.lib", "winmm.lib" }
	filter { "system:linux" }
		links { "SDL2" }

	filter { "configurations:Debug" }
		symbols "On"
		optimize "Debug"
		removeflags { "FatalWarnings" }

	filter { "configurations:Release" }
		optimize "Full"

	filter { "system:linux" }
		links { "udSDK", "z", "m" }

		--This need to be changed to work in other distros
		postbuildcommands { 'cp "' .. _OPTIONS["udsdk"] .. '/lib/ubuntu18.04_GCC_x64/libudSDK.so" "%{cfg.targetdir}/"' }

	filter { "system:macosx" }
		frameworkdirs { "../../builds/features/getscenesfromudcloud" }
		links { "udSDK.framework" }
		prebuildcommands { 'cp -af "../../builds/udsdk/lib/udSDK.framework" "../../builds/features/getscenesfromudcloud/udSDK.framework"' }

	filter {}
