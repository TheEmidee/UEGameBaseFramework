---
layout: default
title: Settings
parent: Common UI
nav_order: 1
---

# Settings

You must update the file `DefaultGame.ini` with these entries:

```ini
[/Script/CommonUI.CommonUISettings]
bAutoLoadData=True
DefaultImageResourceObject=None
+PlatformTraits=(TagName="Platform.Trait.SupportsWindowedMode")
+PlatformTraits=(TagName="Platform.Trait.CanExitApplication")
+PlatformTraits=(TagName="Platform.Trait.SupportsChangingAudioOutputDevice")
+PlatformTraits=(TagName="Platform.Trait.SupportsBackgroundAudio")
+PlatformTraits=(TagName="Platform.Trait.Input.SupportsMouseAndKeyboard")
+PlatformTraits=(TagName="Platform.Trait.Input.HardwareCursor")
+PlatformTraits=(TagName="Platform.Trait.NeedsBrightnessAdjustment")
```