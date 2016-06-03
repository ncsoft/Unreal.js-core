using UnrealBuildTool;

public class JavascriptWebSocket : ModuleRules
{
    public static string GetLibWebsocket(bool bIgnorePatchVersion = false)
    {
        string[] VersionHeader = Utils.ReadAllText("../Source/Runtime/Launch/Resources/Version.h").Replace("\r\n", "\n").Replace("\t", " ").Split('\n');
        string EngineVersionMajor = "4";
        string EngineVersionMinor = "0";
        string EngineVersionPatch = "0";
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
            else if (Line.StartsWith("#define ENGINE_PATCH_VERSION ") && !bIgnorePatchVersion)
            {
                EngineVersionPatch = Line.Split(' ')[2];
            }
        }
        return System.Int32.Parse(EngineVersionMajor) == 4 && System.Int32.Parse(EngineVersionMinor) >= 12 ? "libWebSockets" : "WebSockets";
    }

    public JavascriptWebSocket(TargetInfo Target)
	{
        PublicDependencyModuleNames.AddRange(new string[] { 
            "Core", 
            "CoreUObject", 
            "Engine",
            "V8",
            "Sockets",
            GetLibWebsocket(),
            "OnlineSubSystemUtils",
            "Networking"
        });
    }
}
