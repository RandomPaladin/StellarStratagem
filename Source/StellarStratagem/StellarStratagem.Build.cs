using UnrealBuildTool;

public class StellarStratagem : ModuleRules
{
	public StellarStratagem(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
	
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", 
			"CoreUObject", 
			"Engine", 
			"InputCore", 
			"OnlineSubsystem", 
			"OnlineSubsystemUtils", 
			"Networking",
		});

		PrivateDependencyModuleNames.AddRange(new string[] {  });
	}
}
