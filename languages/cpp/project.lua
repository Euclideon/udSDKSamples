project "vaultCppSample"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++11"
	flags { "FatalWarnings" }
	tags { "vault-project" }

	targetdir "../../builds/samples/cpp"
	debugdir "../../builds/samples/cpp"

	--Files to include
	files { "**" }
	includedirs { "../../external/stb" }

	--This project includes
	IncludeVaultSDK()

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
		frameworkdirs { "../builds/vaultsdk/samples/CppSample" }
		links { "vaultSDK.framework" }
		prebuildcommands { 'cp -af "../../builds/vaultsdk/lib/vaultSDK.framework" "../../builds/vaultsdk/samples/CppSample/vaultSDK.framework"' }

	filter {}