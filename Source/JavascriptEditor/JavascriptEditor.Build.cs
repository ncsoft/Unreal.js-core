using System;
using UnrealBuildTool;

public class JavascriptEditor : ModuleRules
{
    public static Int32[] ParseEditorVersions()
    {
        string[] VersionHeader = Utils.ReadAllText("../Source/Runtime/Launch/Resources/Version.h").Replace("\r\n", "\n").Replace("\t", " ").Split('\n');
        string EngineVersionMajor = "4";
        string EngineVersionMinor = "0";
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
        }
        return new Int32[] { System.Int32.Parse(EngineVersionMajor), System.Int32.Parse(EngineVersionMinor) };
    }

    public JavascriptEditor(ReadOnlyTargetRules Target) : base(Target)
	{
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
                new string[]
				{
					"Core",
					"CoreUObject",
					"Engine",
					"Slate",
                    "Landscape",
                    "JavascriptUMG",
                    "RawMesh",
                    "NavigationSystem",
                    "WebBrowser",
                    "AppFramework",
                    "EditorWidgets",
                    "KismetWidgets",
                    "EditorStyle",
                    "UnrealEd",
                    "DetailCustomizations"
                }
            );  // @todo Mac: for some reason CoreUObject and Engine are needed to link in debug on Mac

        var vers = ParseEditorVersions();
        var EngineMajorVer = vers[0];
        var EngineMinorVer = vers[1];

        if (EngineMajorVer > 4)
        {
            PublicDependencyModuleNames.Add("BSPUtils");
        }

        if (Target.bBuildEditor == true)
        {
            PublicDependencyModuleNames.AddRange(
                new string[]
                {
                    "AssetRegistry",
                }
            );

            // Is VREditor Needed?
            if (EngineMajorVer > 4 || EngineMinorVer >= 14)
            {
                PrivateDependencyModuleNames.AddRange(new string []{ "LevelEditor", "ViewportInteraction", "VREditor" });
            }

            PrivateDependencyModuleNames.AddRange(
                new string[]
				    {
					    // ... add private dependencies that you statically link with here ...
					    "Core",
					    "CoreUObject",
					    "Engine",
					    "AssetTools",
					    "ClassViewer",
                        "InputCore",
                        "CurveTableEditor",
                        "PropertyEditor",
                        "AdvancedPreviewScene",
                        "Slate",
					    "SlateCore",
                        "EditorStyle",
					    "MainFrame",
					    "UnrealEd",
                        "WorkspaceMenuStructure",
                        "V8",
                        "UMG",
                        "Foliage",
                        "LandscapeEditor",
                        "EditorWidgets",
                        "KismetWidgets",
                        "Kismet",
                        "AnimationBlueprintEditor",
                        "AnimationEditor",
                        "ImageWrapper",
                        "RenderCore",
                        "RHI",
				        "DesktopPlatform",
                        "ToolMenus",
                        "SkeletalMeshEditor",
                        "StaticMeshEditor",
                        "Media",
                        "SlateNullRenderer",
                        "SlateRHIRenderer",
                        "SourceControl",
                        "Sockets"
                    }
            );

            if (EngineMajorVer > 4)
            {
                PrivateDependencyModuleNames.Add("EditorFramework");
            }
        }

        // Uncomment if you are using Slate UI
        // PrivateDependencyModuleNames.AddRange(new string[] { "Slate", "SlateCore" });

        // Uncomment if you are using online features
        // PrivateDependencyModuleNames.Add("OnlineSubsystem");
        // if (Target.IsInPlatformGroup(UnrealPlatformGroup.Windows))
        // {
        //		if (UEBuildConfiguration.bCompileSteamOSS == true)
        //		{
        //			DynamicallyLoadedModuleNames.Add("OnlineSubsystemSteam");
        //		}
        // }
    }
}
