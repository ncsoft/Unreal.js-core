// Copyright 1998-2016 Epic Games, Inc. All Rights Reserved.

namespace UnrealBuildTool.Rules
{
	public class JavascriptGraphEditor : ModuleRules
	{
		public JavascriptGraphEditor(ReadOnlyTargetRules Target)
            : base(Target)
		{
            PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

            PublicDependencyModuleNames.AddRange(
				new string[]
				{
					"Core",
					"CoreUObject",
                    "Engine",
                    "UnrealEd",
                    "UMG",
                    "JavascriptEditor",
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
                    "KismetWidgets",
                    "JavascriptUMG",
                    "JavascriptEditor",
                    "InputCore",
                    "V8"
                }
				);

            if (Target.Type == TargetType.Editor)
            {
                PrivateDependencyModuleNames.AddRange(
                    new string[]
                    {
                    "EditorWidgets",
                    }
                    );
            }
		}
	}
}