using UnrealBuildTool;

public class JavascriptUMG : ModuleRules
{
	public JavascriptUMG(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "InputCore",
            "Slate",
			"SlateCore",
			"RenderCore",
			"RHI",
            "UMG",
            "V8"
        });
    }
}
