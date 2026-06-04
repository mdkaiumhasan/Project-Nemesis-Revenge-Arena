// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectNemesisTarget : TargetRules
{
    public ProjectNemesisTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.Add("ProjectNemesis");

        // উইন্ডোজ কম্পাইলারকে সরাসরি নির্দেশ দেওয়া হচ্ছে যেন সে কোনো ওয়ার্নিংকে এরর না বানায়
        bWarningsAsErrors = false;

        // MSVC কম্পাইলারের কড়া কমপ্লায়েন্স এবং ASCII ওয়ার্নিং ইগনোর করার জন্য
        ProjectDefinitions.Add("UE_BUILD_DEVELOPMENT_WITH_DEBUGGAME=0");
    }
}