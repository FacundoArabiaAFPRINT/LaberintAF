using UnrealBuildTool;
using System.Collections.Generic;

public class LaberintAFEditorTarget : TargetRules
{
	public LaberintAFEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("LaberintAF");
	}
}
