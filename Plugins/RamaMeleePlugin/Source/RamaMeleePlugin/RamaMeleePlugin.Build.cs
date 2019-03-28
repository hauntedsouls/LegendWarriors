// Copyright 2015 by Nathan "Rama" Iyer. All Rights Reserved.
using UnrealBuildTool;

public class RamaMeleePlugin : ModuleRules
{
	public RamaMeleePlugin(ReadOnlyTargetRules Target) : base(Target)
	{
        PrivatePCHHeaderFile = "Private/RamaMeleePluginPrivatePCH.h";

        PCHUsage = PCHUsageMode.UseSharedPCHs;
		
        PublicDependencyModuleNames.AddRange(
			new string[] { 
				"Core", 
				"CoreUObject", 
				"Engine", 
				"InputCore",
                "PhysX"
			}
		);
		 
		//APEX EXCLUSIONS
		if (Target.Platform != UnrealTargetPlatform.Android && Target.Platform != UnrealTargetPlatform.HTML5 && Target.Platform != UnrealTargetPlatform.IOS)
		{
			PrivateDependencyModuleNames.AddRange(
			new string[]
			{
				"APEX"
			}
			);
		}
	}
	 
	
}
