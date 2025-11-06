using UnrealBuildTool;

public class StellarStratagemServerTarget : TargetRules
{
	public StellarStratagemServerTarget( TargetInfo Target) : base(Target)
	{
		Type = TargetType.Server;
		DefaultBuildSettings = BuildSettingsVersion.V2;
		ExtraModuleNames.AddRange( new string[] { "StellarStratagem" } );
	}
}
