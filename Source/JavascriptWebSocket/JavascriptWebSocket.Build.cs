using UnrealBuildTool;
using System.IO;
using System;

public class JavascriptWebSocket : ModuleRules
{
    protected string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty")); }
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

        bool bPlatformSupportsLibWebsockets =
            Target.Platform == UnrealTargetPlatform.Win32 ||
            Target.Platform == UnrealTargetPlatform.Win64 ||
            Target.Platform == UnrealTargetPlatform.Android ||
            Target.Platform == UnrealTargetPlatform.Mac ||
            Target.IsInPlatformGroup(UnrealPlatformGroup.Unix) ||
            Target.Platform == UnrealTargetPlatform.IOS ||
            Target.Platform == UnrealTargetPlatform.PS4 ||
            Target.Platform == UnrealTargetPlatform.Switch;

        bool bUsePlatformSSL = Target.Platform == UnrealTargetPlatform.Switch;

        bool bPlatformSupportsXboxWebsockets = Target.Platform == UnrealTargetPlatform.XboxOne;

        bool bShouldUseModule =
                bPlatformSupportsLibWebsockets ||
                bPlatformSupportsXboxWebsockets;

        if (bShouldUseModule)
        {
            PublicDefinitions.Add("WITH_JSWEBSOCKET=1");
            if (bPlatformSupportsLibWebsockets)
            {
                if (bUsePlatformSSL)
                {
                    PrivateDefinitions.Add("WITH_SSL=0");
                    AddEngineThirdPartyPrivateStaticDependencies(Target, "libWebSockets");
                }
                else
                {
                    AddEngineThirdPartyPrivateStaticDependencies(Target, "OpenSSL", "libWebSockets", "zlib");
                    PrivateDependencyModuleNames.Add("SSL");
                }
            }
        }
        else
        {
            PublicDefinitions.Add("WITH_JSWEBSOCKET=0");
        }
    }


}
