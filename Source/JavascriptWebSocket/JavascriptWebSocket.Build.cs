using UnrealBuildTool;

public class JavascriptWebSocket : ModuleRules
{
    public static bool IsUpdated(bool bIgnorePatchVersion = false)
    {
        string[] VersionHeader = Utils.ReadAllText("../Source/Runtime/Launch/Resources/Version.h").Replace("\r\n", "\n").Replace("\t", " ").Split('\n');
        string EngineVersionMajor = "4";
        string EngineVersionMinor = "0";
        //string EngineVersionPatch = "0";
        foreach (string Line in VersionHeader)
        {
            if (Line.StartsWith("#define ENGINE_MAJOR_VERSION "))
            {
                EngineVersionMajor = Line.Split(' ')[2];
            }
            else if (Line.StartsWith("#define ENGINE_MINOR_VERSION "))
            {
                EngineVersionMinor = Line.Split(' ')[2];
            }
            /*else if (Line.StartsWith("#define ENGINE_PATCH_VERSION ") && !bIgnorePatchVersion)
            {
                EngineVersionPatch = Line.Split(' ')[2];
            }*/
        }
        return System.Int32.Parse(EngineVersionMajor) == 4 && System.Int32.Parse(EngineVersionMinor) >= 14;
    }

    public JavascriptWebSocket(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "V8",
            "Sockets",
            "OnlineSubSystemUtils",
            "Networking"
        });

        if (IsUpdated() && (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Win32 || Target.Platform == UnrealTargetPlatform.Mac))
        {
            Definitions.Add(string.Format("WITH_JSWEBSOCKET=1"));
            PublicDependencyModuleNames.Add("libWebSockets");
            AddEngineThirdPartyPrivateStaticDependencies(Target,"libWebSockets");
        }
        else
        {
            Definitions.Add(string.Format("WITH_JSWEBSOCKET=0"));
        }
    }
}
