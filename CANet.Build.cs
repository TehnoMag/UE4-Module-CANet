// Copyright (c) 2007-2019, Delta-Proxima Team All rights reserved.

using UnrealBuildTool;

public class CANet : ModuleRules
{
    public CANet(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });
        //PrivateDependencyModuleNames.AddRange(new string[] { "ReplicationGraph" });
    }
}
