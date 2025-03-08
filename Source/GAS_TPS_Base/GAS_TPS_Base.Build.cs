using UnrealBuildTool;

public class GAS_TPS_Base : ModuleRules
{
    public GAS_TPS_Base(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = ModuleRules.PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(
            new string[]
            {
                "Core",
                "CoreUObject",
                "Engine",
                "Slate",
                "SlateCore"
            }
        );

        PrivateDependencyModuleNames.AddRange(
            new string[]
            {
                "EnhancedInput",
                "MotionWarping",
                "GameplayTags",
                "AnimGraphRuntime",
                "AnimationWarpingRuntime",
                "AnimationLocomotionLibraryRuntime",
                "Chooser",
                "PoseSearch",
                "StructUtils",
                "BlendStack",
                "MotionTrajectory",
                "AIModule", 
                "UMG"
            }
        );
    }
}