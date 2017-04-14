using UnrealBuildTool;

public class JavascriptUMG : ModuleRules
{
	public JavascriptUMG(TargetInfo Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", 
            "CoreUObject", 
            "Engine", 
            "InputCore", 
            "Slate",
			"SlateCore",
            "ShaderCore",
			"RenderCore",
			"RHI", 
            "UMG",
            "V8"
        });        
	}
}
