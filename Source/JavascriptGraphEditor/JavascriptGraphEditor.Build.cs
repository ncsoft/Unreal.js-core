// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class JavascriptGraphEditor : ModuleRules
	{
		public JavascriptGraphEditor(ReadOnlyTargetRules Target)
		{
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
            PrivateIncludePaths.AddRange(
				new string[] {
                    "JavascriptGraphEditor/Private",
				}
				);

			PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
                    "Engine",
                    "UnrealEd",
                    "UMG"
				}
				);

			PrivateDependencyModuleNames.AddRange(
				new string[]
				{
                    "AssetTools",
                    "Slate",
                    "SlateCore",
                    "GraphEditor",
                    "EditorStyle",
                    "JavascriptUMG",
                    "KismetWidgets",
                }
				);
		}
	}
}