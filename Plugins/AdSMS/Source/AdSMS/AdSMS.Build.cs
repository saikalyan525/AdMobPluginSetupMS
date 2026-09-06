using UnrealBuildTool;

public class AdSMS : ModuleRules
{
    public AdSMS(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[]
        {
            "Core", "CoreUObject", "Engine", "UMG", "Slate", "SlateCore", "ImageWrapper"
        });

        if (Target.Platform == UnrealTargetPlatform.Android)
        {
            PrivateDependencyModuleNames.Add("Launch");
            AdditionalPropertiesForReceipt.Add(
                "AndroidPlugin",
                System.IO.Path.Combine(ModuleDirectory, "AdSMS_UPL.xml")
            );
        }
    }
}
