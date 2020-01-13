project "vaultCSSample"
	kind "ConsoleApp"
	language "C#"
	flags { "FatalWarnings" }
	location(".")

	targetdir "../../builds/samples/csharp"
	debugdir "../../builds/samples/csharp"

	postbuildcommands { 'XCOPY /f /d /y "' .. _OPTIONS["vaultsdk"] .. '\\lib\\win_x64\\vaultSDK.dll" "$(TargetDir)\\"' }

	--Files to include
	files { "**" }

	links { "System.Drawing" }

	-- filters
	filter { "configurations:Debug" }
		symbols "On"
		optimize "Debug"
		removeflags { "FatalWarnings" }

	filter { "configurations:Release" }
		optimize "Full"

	filter {}
