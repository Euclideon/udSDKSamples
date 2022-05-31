require "vstudio"

table.insert(premake.option.get("os").allowed, { "android", "Google Android" })
table.insert(premake.option.get("os").allowed, { "ios", "Apple iOS" })

filter {}

newoption {
	trigger	= "has-python",
	description = "Adds Python sample to solution"
}

newoption {
	trigger     = "udsdk",
	value       = "Path",
	description = "Path to udSDK",
	default     = os.getenv("UDSDK_HOME")
}

if _OPTIONS["udsdk"] == nil then
	error "udSDK not installed correctly. (No UDSDK_HOME environment variable or --udsdk=[PATH] argument!)"
end

function getosinfo()
	local osname = os.target()
	local distroExtension = ""
	if os.target() == premake.LINUX then
		osname = os.outputof('lsb_release -ir | head -n2 | cut -d ":" -f 2 | tr -d "\n\t" | tr [:upper:] [:lower:]')
		if osname:startswith("centos") then
			osname = os.outputof('lsb_release -ir | head -n2 | cut -d ":" -f 2 | tr -d "\n\t" | tr [:upper:] [:lower:] | cut -d "." -f 1')
			distroExtension = "_rpm"
		else
			distroExtension = "_deb"
		end
	end

	return osname, distroExtension
end

function IncludeUDSDK()
	includedirs(_OPTIONS["udsdk"] .. "/include")
	libdirs { _OPTIONS["udsdk"] .. "/include" }

	local osname, distroExtension = getosinfo()

	if os.target() == premake.MACOSX then
		links { "udSDK.framework" }
	else
		links { "udSDK" }
	end

	if os.target() == premake.WINDOWS then
		postbuildcommands { 'XCOPY /f /d /y "' .. _OPTIONS["udsdk"] .. '\\lib\\win_x64\\udSDK.dll" "$(TargetDir)\\"' }
		os.copyfile(_OPTIONS["udsdk"] .. "/lib/win_x64/udSDK.dll", "builds/udSDK.dll")
		libdirs { _OPTIONS["udsdk"] .. "/lib/win_x64" }
	elseif os.target() == premake.MACOSX then
		os.execute("mkdir -p builds")

		-- copy dmg, mount, extract framework, unmount then remove.
		os.copyfile(_OPTIONS["udsdk"] .. "/lib/osx_x64/udSDK.dmg", "builds/udSDK.dmg")
		local device = os.outputof("/usr/bin/hdiutil attach -noverify -noautoopen builds/udSDK.dmg | egrep '^/dev/' | sed 1q | awk '{print $1}'")
		os.execute("cp -a -f /Volumes/udSDK/udSDK.framework builds/")
		os.execute("/usr/bin/hdiutil detach /Volumes/udSDK")
		os.execute("/usr/bin/hdiutil detach " .. device)
		os.execute("rm -r builds/udSDK.dmg")

		prelinkcommands {
			"rm -rf %{prj.targetdir}/%{prj.targetname}.app/Contents/Frameworks",
			"mkdir -p %{prj.targetdir}/%{prj.targetname}.app/Contents/Frameworks",
			"cp -af builds/udSDK.framework %{prj.targetdir}/%{prj.targetname}.app/Contents/Frameworks/",
			"cp -af /Library/Frameworks/SDL2.framework %{prj.targetdir}/%{prj.targetname}.app/Contents/Frameworks/",
		}
		linkoptions { "-rpath @executable_path/../Frameworks/" }
		frameworkdirs { "builds" }
	elseif os.target() == premake.ANDROID then
		filter { "architecture:x64" }
			libdirs { _OPTIONS["udsdk"] .. "/lib/android_x64" }
		filter { "architecture:arm64" }
			libdirs { _OPTIONS["udsdk"] .. "/lib/android_arm64" }
		filter {}
	else
		linkoptions { "-Wl,-rpath '-Wl,$$ORIGIN'" } -- Check beside the executable for the SDK
		libdirs { _OPTIONS["udsdk"] .. '/lib/' .. osname .. '_GCC_x64' }
		prebuildcommands { 'cp "' .. _OPTIONS["udsdk"] .. '/lib/' .. osname .. '_GCC_x64/libudSDK.so" "%{cfg.targetdir}/"' }
	end
end

solution "udSDKSamples"
	-- This hack just makes the VS project and also the makefile output their configurations in the idiomatic order
	if (_ACTION == "gmake" or _ACTION == "gmake2") and os.target() == "linux" then
		configurations { "Release", "Debug", "ReleaseClang", "DebugClang" }
		linkgroups "On"
		filter { "configurations:*Clang" }
			toolset "clang"
		filter { }
	elseif os.target() == "macosx" or os.target() == "ios" then
		configurations { "Release", "Debug" }
		toolset "clang"
	else
		configurations { "Debug", "Release" }
	end

	if os.target() == "android" or os.target() == "ios" then
		platforms { "x64", "arm64" }
	else
		platforms { "x64" }
	end

	cppdialect "C++14"
	pic "On"
	startproject "cSample"

	filter { "system:not windows" }
		visibility "Hidden"
	filter {}

	--Uncomment this when looking for memory leaks in code using udCore memory tracking
	--defines { "__MEMORY_DEBUG__" }

	group "external"
		--dofile "external/udCore/project.lua"

	group "languages"

	if os.target() ~= "android" and os.target() ~= "ios" and os.target() ~= "emscripten" then
		dofile "languages/c/project.lua"
		dofile "languages/cpp/project.lua"
	end

	if os.target() == premake.WINDOWS then
		dofile "languages/csharp/project.lua"
		dofile "integrations/winforms-csharp/project.lua"
	end

	group "features"
		if os.target() ~= "android" and os.target() ~= "ios" and os.target() ~= "emscripten" then
			dofile "features/convertcustomdata/project.lua"
			dofile "features/convertdem/project.lua"
			dofile "features/renderingcustomvoxelshaders/project.lua"
			dofile "features/getscenesfromudcloud/project.lua"
		end

	group "libraries"
		if os.target() ~= "android" and os.target() ~= "ios" and os.target() ~= "emscripten" then
			dofile "external/udcore/project.lua"
		end
