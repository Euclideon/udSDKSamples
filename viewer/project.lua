project "Viewer"
	kind "ConsoleApp"
	language "C++"
	flags { "FatalWarnings" }

	targetdir "../builds/viewer"
	debugdir "../builds/viewer"

	--Files to include
	includedirs { "../external/sdl2/include"}
	files { "**.h", "**.cpp", "*.md", "*.lua" }
	files { "../external/imgui/*.cpp", "../external/imgui/*.h" }
	files { "../external/imgui/backends/imgui_impl_sdl.cpp", "../external/imgui/backends/imgui_impl_sdl.h" }
	files { "../external/imgui/backends/imgui_impl_sdlrenderer.cpp", "../external/imgui/backends/imgui_impl_sdlrenderer.h" }
	includedirs { "../external/stb", "../features/shared" }
	includedirs { "../external/glew/include" }
	includedirs { "../external/udcore/Include" }
	includedirs { "../external/imgui" }

	links { "udCore" }
		
	--This project includes
	IncludeUDSDK()

	-- filters
	filter { "system:windows" }
		defines { "GLEW_STATIC" }

		staticruntime "on"
		libdirs { "../external/sdl2/lib/x64" }
		links { "GLEW" }
		links { "opengl32.lib", "glu32.lib", "SDL2.lib", "SDL2main.lib", "winmm.lib" }
		postbuildcommands { 'XCOPY /f /d /y "..\\external\\sdl2\\lib\\x64\\SDL2.dll" "$(TargetDir)\\"' }

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

	filter { "system:macosx" }
		frameworkdirs { "../../builds/features/getscenesfromudcloud" }
		links { "udSDK.framework" }
		prebuildcommands { 'cp -af "../../builds/udsdk/lib/udSDK.framework" "../../builds/features/getscenesfromudcloud/udSDK.framework"' }

	filter {}
