using UnrealBuildTool;
using System.IO;
 
public class ReactiveEnvEditor : ModuleRules
{
    public ReactiveEnvEditor(ReadOnlyTargetRules Target) :base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PrivateIncludePaths.Add(Path.Combine(ModuleDirectory, "Private"));
        PublicIncludePaths.Add(Path.Combine(ModuleDirectory, "Public"));

        PublicDependencyModuleNames.AddRange(new string[] { "Engine", "Core" ,"CoreUObject","UnrealEd"});

        PrivateDependencyModuleNames.AddRange(new string[] { "ReactiveEnv",
                    "AssetTools",
                    "Slate",
                    "SlateCore",
                    "GraphEditor",
                    "PropertyEditor",
                    "EditorStyle",
                    "Kismet",
                    "KismetWidgets",
                    "AIGraph",
                    "EditorWidgets",
                    "MessageLog",
                    "ApplicationCore",
                    "TextureEditor",
                    "Projects"
        });
    }
}