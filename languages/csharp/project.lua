project "CSSample"
	kind "ConsoleApp"
	language "C#"
	flags { "FatalWarnings" }
	location(".")

	targetdir "../../builds/samples/csharp"
	debugdir "../../builds/samples/csharp"

	postbuildcommands { 'XCOPY /f /d /y "' .. _OPTIONS["udsdk"] .. '\\lib\\win_x64\\udSDK.dll" "$(TargetDir)\\"' }

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
