using UnrealBuildTool;
using System.IO;
using System;

public class V8 : ModuleRules
{
    protected string ThirdPartyPath
    {
        get { return Path.GetFullPath(Path.Combine(ModuleDirectory, "..", "..", "ThirdParty")); }
    }

    public int[] GetV8Version()
    {
        string[] VersionHeader = Utils.ReadAllText(Path.Combine(ThirdPartyPath, "v8", "include", "v8-version.h")).Replace("\r\n", "\n").Replace("\t", " ").Split('\n');
        string VersionMajor = "0";
        string VersionMinor = "0";
        string VersionPatch = "0";
        foreach (string Line in VersionHeader)
        {
            if (Line.StartsWith("#define V8_MAJOR_VERSION"))
            {
                VersionMajor = Line.Split(' ')[2];
            }
            else if (Line.StartsWith("#define V8_MINOR_VERSION "))
            {
                VersionMinor = Line.Split(' ')[2];
            }
            else if (Line.StartsWith("#define V8_PATCH_VERSION "))
            {
                VersionPatch = Line.Split(' ')[2];
            }
        }
        return new int[] { Int32.Parse(VersionMajor), Int32.Parse(VersionMinor), Int32.Parse(VersionPatch) };
    }

    public V8(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;
        bLegacyPublicIncludePaths = false;
        ShadowVariableWarningLevel = WarningLevel.Error;
        PrivateIncludePaths.AddRange(new string[]
        {
            Path.Combine(ThirdPartyPath, "v8", "include")
        });

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "Sockets", "ApplicationCore", "NavigationSystem", "OpenSSL"
        });

        if (Target.bBuildEditor)
        {
            PublicDependencyModuleNames.AddRange(new string[]
            {
                "DirectoryWatcher"
            });
        }

        HackWebSocketIncludeDir(Path.Combine(Directory.GetCurrentDirectory(), "ThirdParty", "libWebSockets", "libwebsockets"), Target);

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[]
            {
                "UnrealEd"
            });
        }

        bEnableExceptions = true;

        LoadV8(Target);
    }

    private void HackWebSocketIncludeDir(String WebsocketPath, ReadOnlyTargetRules Target)
    {
        string PlatformSubdir = Target.Platform.ToString();

        bool bHasZlib = false;

        if (Target.IsInPlatformGroup(UnrealPlatformGroup.Windows))
        {
            PlatformSubdir = Path.Combine(PlatformSubdir, "VS" + Target.WindowsPlatform.GetVisualStudioCompilerVersionName());
            bHasZlib = true;

        }
		else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            PlatformSubdir = Path.Combine(PlatformSubdir, Target.Architecture.ToString());
        }

        PrivateDependencyModuleNames.Add("libWebSockets");

        if (bHasZlib)
        {
            PrivateDependencyModuleNames.Add("zlib");
        }
        PrivateIncludePaths.Add(Path.Combine(WebsocketPath, "include"));
        PrivateIncludePaths.Add(Path.Combine(WebsocketPath, "include", PlatformSubdir));
    }

    private bool LoadV8(ReadOnlyTargetRules Target)
    {
        int[] v8_version = GetV8Version();
        bool ShouldLink_libsampler = !(v8_version[0] == 5 && v8_version[1] < 3);
        bool ShouldLink_lib_v8_compiler = (v8_version[0] > 6 && v8_version[1] > 6);
        bool ShouldLink_lib_monolith = v8_version[0] > 8;

        if (Target.IsInPlatformGroup(UnrealPlatformGroup.Windows))
        {
            string LibrariesPath = Path.Combine(ThirdPartyPath, "v8", "lib");

            if (Target.Platform == UnrealTargetPlatform.Win64)
            {
                LibrariesPath = Path.Combine(LibrariesPath, "Win64");
            }
            else
            {
                LibrariesPath = Path.Combine(LibrariesPath, "Win32");
            }

            if (ShouldLink_lib_monolith)
            {
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_monolith.lib"));
            }
            else
            {
                if (Target.Configuration == UnrealTargetConfiguration.Debug)
                {
                    LibrariesPath = Path.Combine(LibrariesPath, "Debug");
                }
                else
                {
                    LibrariesPath = Path.Combine(LibrariesPath, "Release");
                }

                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_init.lib"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_initializers.lib"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_libbase.lib"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_libplatform.lib"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_nosnapshot.lib"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_libsampler.lib"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "torque_base.lib"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "torque_generated_initializers.lib"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "inspector.lib"));

                if (ShouldLink_lib_v8_compiler)
                {
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_compiler.lib"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_base_without_compiler_0.lib"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_base_without_compiler_1.lib"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "inspector_string_conversions.lib"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "torque_generated_definitions.lib"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "encoding.lib"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "bindings.lib"));
                }
                else
                {
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_base_0.lib"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "v8_base_1.lib"));
                }
            }

            PublicDefinitions.Add(string.Format("WITH_V8=1"));

            PublicDefinitions.Add(string.Format("USING_V8_PLATFORM_SHARED=0"));

            return true;
        }
        else if (Target.Platform == UnrealTargetPlatform.Android)
        {
            string LibrariesPath = Path.Combine(ThirdPartyPath, "v8", "lib", "Android");

            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "ARM64"));
            PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "ARMv7"));

            if (ShouldLink_lib_monolith)
            {
                PublicAdditionalLibraries.Add("v8_monolith");
            }
            else
            {
                PublicAdditionalLibraries.Add("v8_init");
                PublicAdditionalLibraries.Add("v8_initializers");
                PublicAdditionalLibraries.Add("v8_libbase");
                PublicAdditionalLibraries.Add("v8_libplatform");
                PublicAdditionalLibraries.Add("v8_nosnapshot");
                PublicAdditionalLibraries.Add("v8_libsampler");
                PublicAdditionalLibraries.Add("torque_generated_initializers");
                PublicAdditionalLibraries.Add("inspector");

                if (ShouldLink_lib_v8_compiler)
                {
                    PublicAdditionalLibraries.Add("v8_compiler");
                    PublicAdditionalLibraries.Add("v8_base_without_compiler");
                    PublicAdditionalLibraries.Add("inspector_string_conversions");
                    PublicAdditionalLibraries.Add("encoding");
                    PublicAdditionalLibraries.Add("bindings");
                    PublicAdditionalLibraries.Add("torque_generated_definitions");
                }
                else
                {
                    PublicAdditionalLibraries.Add("v8_base");
                }
            }

            PublicDefinitions.Add(string.Format("WITH_V8=1"));

            return true;
        }
        else if (Target.Platform == UnrealTargetPlatform.Linux)
        {
            string LibrariesPath = Path.Combine(ThirdPartyPath, "v8", "lib", "Linux");
            if (ShouldLink_lib_monolith)
            {
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_monolith.a"));
            }
            else
            {
                if (Target.Configuration == UnrealTargetConfiguration.Debug)
                {
                    LibrariesPath = Path.Combine(LibrariesPath, "Debug");
                }
                else
                {
                    LibrariesPath = Path.Combine(LibrariesPath, "Release");
                }

                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_init.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_initializers.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_libbase.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_libplatform.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_nosnapshot.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_libsampler.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libtorque_base.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libtorque_generated_initializers.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libinspector.a"));

                if (ShouldLink_lib_v8_compiler)
                {
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_compiler.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_base_without_compiler.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libinspector_string_conversions.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libencoding.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libbindings.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libtorque_generated_definitions.a"));
                }
                else
                {
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_base.a"));
                }
            }

            PublicDefinitions.Add(string.Format("WITH_V8=1"));

            return true;
        }
        else if (Target.Platform == UnrealTargetPlatform.Mac)
        {
            string LibrariesPath = Path.Combine(ThirdPartyPath, "v8", "lib", "Mac");
            if (ShouldLink_lib_monolith)
            {
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_monolith.a"));
            }
            else
            {
                if (Target.Configuration == UnrealTargetConfiguration.Debug)
                {
                    LibrariesPath = Path.Combine(LibrariesPath, "Debug");
                }
                else
                {
                    LibrariesPath = Path.Combine(LibrariesPath, "Release");
                }

                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_init.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_initializers.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_libbase.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_libplatform.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_nosnapshot.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_libsampler.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libtorque_base.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libtorque_generated_initializers.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libinspector.a"));


                if (ShouldLink_lib_v8_compiler)
                {
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_compiler.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_base_without_compiler.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libinspector_string_conversions.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libencoding.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libbindings.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libtorque_generated_definitions.a"));
                }
                else
                {
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_base.a"));
                }
            }

            PublicDefinitions.Add(string.Format("WITH_V8=1"));

            return true;
        }
        else if (Target.Platform == UnrealTargetPlatform.IOS)
        {
            string LibrariesPath = Path.Combine(ThirdPartyPath, "v8", "lib", "IOS");

            if (ShouldLink_lib_monolith)
            {
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_monolith.a"));
            }
            else
            {
                if (Target.Configuration == UnrealTargetConfiguration.Debug)
                {
                    LibrariesPath = Path.Combine(LibrariesPath, "Debug");
                }
                else
                {
                    LibrariesPath = Path.Combine(LibrariesPath, "Release");
                }

                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_init.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_initializers.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_libbase.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_libplatform.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_nosnapshot.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_libsampler.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libtorque_generated_initializers.a"));
                PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libinspector.a"));

                if (ShouldLink_lib_v8_compiler)
                {
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_compiler.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_base_without_compiler.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libinspector_string_conversions.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libencoding.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libbindings.a"));
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libtorque_generated_definitions.a"));
                }
                else
                {
                    PublicAdditionalLibraries.Add(Path.Combine(LibrariesPath, "libv8_base.a"));
                }
            }

            PublicDefinitions.Add(string.Format("WITH_V8=1"));

            return true;
        }

        PublicDefinitions.Add(string.Format("WITH_V8=0"));
        return false;
    }
}
