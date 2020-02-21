require "vstudio"

table.insert(premake.option.get("os").allowed, { "android", "Google Android" })
table.insert(premake.option.get("os").allowed, { "ios", "Apple iOS" })

filter {}

newoption {
	trigger	= "has-python",
	description = "Adds Python sample to solution"
}

newoption {
	trigger     = "vaultsdk",
	value       = "Path",
	description = "Path to Vault SDK",
	default     = os.getenv("VAULTSDK_HOME")
}

if _OPTIONS["vaultsdk"] == nil then
	error "VaultSDK not installed correctly. (No VAULTSDK_HOME environment variable or --vaultsdk=[PATH] argument!)"
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

function IncludeVaultSDK()
	includedirs(_OPTIONS["vaultsdk"] .. "/include")
	libdirs { _OPTIONS["vaultsdk"] .. "/include" }

	local osname, distroExtension = getosinfo()

	if os.target() == premake.MACOSX then
		links { "vaultSDK.framework" }
	else
		links { "vaultSDK" }
	end

	if os.target() == premake.WINDOWS then
		postbuildcommands { 'XCOPY /f /d /y "' .. _OPTIONS["vaultsdk"] .. '\\lib\\win_x64\\vaultSDK.dll" "$(TargetDir)\\"' }
		os.copyfile(_OPTIONS["vaultsdk"] .. "/lib/win_x64/vaultSDK.dll", "builds/vaultSDK.dll")
		libdirs { _OPTIONS["vaultsdk"] .. "/lib/win_x64" }
	elseif os.target() == premake.MACOSX then
		os.execute("mkdir -p builds")

		-- copy dmg, mount, extract framework, unmount then remove.
		os.copyfile(_OPTIONS["vaultsdk"] .. "/lib/osx_x64/vaultSDK.dmg", "builds/vaultSDK.dmg")
		local device = os.outputof("/usr/bin/hdiutil attach -noverify -noautoopen builds/vaultSDK.dmg | egrep '^/dev/' | sed 1q | awk '{print $1}'")
		os.execute("cp -a -f /Volumes/vaultSDK/vaultSDK.framework builds/")
		os.execute("/usr/bin/hdiutil detach /Volumes/vaultSDK")
		os.execute("/usr/bin/hdiutil detach " .. device)
		os.execute("rm -r builds/vaultSDK.dmg")

		prelinkcommands {
			"rm -rf %{prj.targetdir}/%{prj.targetname}.app/Contents/Frameworks",
			"mkdir -p %{prj.targetdir}/%{prj.targetname}.app/Contents/Frameworks",
			"cp -af builds/vaultSDK.framework %{prj.targetdir}/%{prj.targetname}.app/Contents/Frameworks/",
			"cp -af /Library/Frameworks/SDL2.framework %{prj.targetdir}/%{prj.targetname}.app/Contents/Frameworks/",
		}
		linkoptions { "-rpath @executable_path/../Frameworks/" }
		frameworkdirs { "builds" }
	elseif os.target() == premake.ANDROID then
		filter { "architecture:x64" }
			libdirs { _OPTIONS["vaultsdk"] .. "/lib/android_x64" }
		filter { "architecture:arm64" }
			libdirs { _OPTIONS["vaultsdk"] .. "/lib/android_arm64" }
		filter {}
	else
		linkoptions { "-Wl,-rpath '-Wl,$$ORIGIN'" } -- Check beside the executable for the SDK
		libdirs { _OPTIONS["vaultsdk"] .. '/lib/' .. osname .. '_GCC_x64' }
		prebuildcommands { 'cp "' .. _OPTIONS["vaultsdk"] .. '/lib/' .. osname .. '_GCC_x64/libvaultSDK.so" "%{cfg.targetdir}/"' }
	end
end

solution "VaultSDKSamples"
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
		end
