using UnrealBuildTool;

public class JavascriptHttp : ModuleRules
{
	public JavascriptHttp(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", 
            "CoreUObject", 
            "Engine",             
            "V8",
            "Http"
        });

        if (UEBuildConfiguration.bBuildEditor == true)
        {
            PrivateDependencyModuleNames.Add("UnrealEd");
        }
	}
}
