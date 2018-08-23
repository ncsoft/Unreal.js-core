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
            AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL", "libWebSockets", "zlib");
            PrivateDependencyModuleNames.Add("SSL");
        }
        else
        {
            PublicDefinitions.Add("WITH_JSWEBSOCKET=0");
        }
    }


}
