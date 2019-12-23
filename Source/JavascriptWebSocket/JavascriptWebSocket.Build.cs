using UnrealBuildTool;
using System.IO;
using System;

public class JavascriptWebSocket : ModuleRules
{
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

        bool bPlatformSupportsLibWebsockets =
            Target.Platform == UnrealTargetPlatform.Win32 ||
            Target.Platform == UnrealTargetPlatform.Win64 ||
            Target.Platform == UnrealTargetPlatform.Android ||
            Target.Platform == UnrealTargetPlatform.Mac ||
            Target.IsInPlatformGroup(UnrealPlatformGroup.Unix);
            
        if (bPlatformSupportsLibWebsockets)
        {
            PublicDefinitions.Add("WITH_JSWEBSOCKET=1");
            HackWebSocketIncludeDir(Path.Combine(Directory.GetCurrentDirectory(), "ThirdParty", "libWebSockets", "libWebSockets"), Target);
        }
        else
        {
            PublicDefinitions.Add("WITH_JSWEBSOCKET=0");
        }
    }

    private void HackWebSocketIncludeDir(String WebsocketPath, ReadOnlyTargetRules Target)
    {
        string PlatformSubdir = Target.Platform.ToString();

        bool bHasZlib = false;

        if (Target.Platform == UnrealTargetPlatform.Win64 || Target.Platform == UnrealTargetPlatform.Win32)
        {
            PlatformSubdir = Path.Combine(PlatformSubdir, "VS" + Target.WindowsPlatform.GetVisualStudioCompilerVersionName());
            bHasZlib = true;

        }        
		else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PlatformSubdir = Path.Combine(PlatformSubdir, Target.Architecture);
        }

        PrivateDependencyModuleNames.Add("libWebSockets");

        if (bHasZlib)
        {
            PrivateDependencyModuleNames.Add("zlib");
        }
        PrivateIncludePaths.Add(Path.Combine(WebsocketPath, "include"));
        PrivateIncludePaths.Add(Path.Combine(WebsocketPath, "include", PlatformSubdir));
    }

}
