project "winforms-csharp"
	kind "ConsoleApp"
	language "C#"
	flags { "FatalWarnings" }
	location(".")

	postbuildcommands { 'XCOPY /f /d /y "' .. _OPTIONS["vaultsdk"] .. '\\lib\\win_x64\\vaultSDK.dll" "$(TargetDir)\\"' }

	--Files to include
	files { "**" }
	files { "project.lua" }

	links { "System", "System.Drawing", "System.Windows", "System.Windows.Forms", "System.ComponentModel" }

	-- filters
	filter { "configurations:Debug" }
		symbols "On"
		optimize "Debug"
		removeflags { "FatalWarnings" }

	filter { "configurations:Release" }
		optimize "Full"

	filter {}

	targetdir "../../builds/integrations/winforms-csharp"
	debugdir "../../builds/integrations/winforms-csharp"
