using UnrealBuildTool;

public class JavascriptWebSocketV2 : ModuleRules
{
	public JavascriptWebSocketV2(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", 
            "CoreUObject", 
            "Engine",             
            "V8",
            "WebSockets"
        });

	}
}
