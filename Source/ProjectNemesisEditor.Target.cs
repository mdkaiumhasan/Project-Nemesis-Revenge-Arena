// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;
using System.Collections.Generic;

public class ProjectNemesisEditorTarget : TargetRules
{
    public ProjectNemesisEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Unreal5_7;
        ExtraModuleNames.Add("ProjectNemesis");

        // এডিটর বিল্ডের জন্যও গ্লোবাল ওয়ার্নিং ব্লক অফ করা হলো
        bWarningsAsErrors = false;
    }
}